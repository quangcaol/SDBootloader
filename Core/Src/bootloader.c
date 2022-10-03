/*
 * bootloader.c
 *
 *  Created on: Sep 12, 2022
 *      Author: quangcaol
 */
#include "bootloader.h"


#include "string.h"


/** Handle SD Card logging  **/

#if defined(SD_LOG)
FATFS FatFs; 	//Fatfs handle
FRESULT fres; //Result after operations

#define WRITE_LOG(log)	do { \
					write_log(log); \
					} \
					while(0)
#else
#define WRITE_LOG(log) while(0);
#endif

/** Handle UART logging  **/

#if defined(SERIAL_LOG)
#define UART_LOG(...) do { \
							 user_printf(__VA_ARGS__); \
							 } \
							 while(0)
#else
#define UART_LOG(...) while(0);
#endif

/**********      Static Variable definition    *********/

/**
 * @brief binary filename for application
 *
 */
static const char * main_app_path = "main_app.bin";
static const char * side_app_path = "side_app.bin";

/**
 * @brief Store led blink previous ticks
 */
uint32_t led_ticks = 0;

/**
 * @brief  Store timeout previous ticks
 */
uint32_t timeout_ticks = 0;

/**
 * @brief  Store button hold time
 */
uint16_t btn_time = 0;


static enum upload_error error;

/**
 * @brief global boolaoder state variable
 */
static enum bootloader_state state;

/**
 * @brief global button state variable
 */
static enum button_state btn;

/**
 * @brief global application varibale
 */
static enum application app;


/********** 				Function Declaration				**********/

/**
 * @brief Bootloader statemachine engine
 * 
 * @param tick tick elapsed
 * @return int 0 1
 */
static void handle_bootlader_statemachine(uint32_t tick);

/**
 * @brief Upload payload to flash at app address plus offset
 * 
 * @param app Application address
 * @param offset offset from the begin address
 * @param payload buffer
 * @param size buffer size
 * @return int 0 
 */
static void upload_app(uint8_t app,const uint32_t offset,const uint8_t * payload,const uint16_t size);

/**
 * @brief Handle upload process
 * 
 * @param app target application 
 * @return int -1 0
 */
static int upload_sequence(uint8_t app);

/**
 * @brief Blink a led to indicate succession 
 * 
 */
static void successful_blink();

/**
 * @brief blink a led to indicate failure 
 * 
 */
static void failed_blink();

/**
 * @brief write string to log file on sdcard
 * 
 * @param log string to write 
 */
static void write_log(const char * log);

/**
 * 
 * @brief Jump to application
 *
 * @param path main app is 1
 */
static void jump2app(uint8_t app);

/**
 * @brief  Erase memory app
 *
 * @param app main app is 1
 */
static void erase_app(uint8_t app);

/**
 * @brief Hardfault from stm32f411_it.h 
 * 
 */
extern void HardFault_Handler(void);


/*******                   Function Definition                       ********/

void handle_bootlader_statemachine(uint32_t ticks)
{
	led_ticks += ticks;
	timeout_ticks += ticks;

	switch (state)
	{
	case INITAL:
	{

		if (timeout_ticks > INITALIZATION_INTERVAL) {
			timeout_ticks = 0;
			WRITE_LOG("GOES TO BOOT STATE \r\n");
			state = BOOT;
		}

		if (led_ticks > WAIT_LED_BLINK_INTERVAL)
		{
			GPIO_Toggle(LD2_GPIO_Port, LD2_Pin);
			led_ticks = 0;
			UART_LOG("Wait for user input \r\n");
		}

		if (!GPIO_Read(B1_GPIO_Port, B1_Pin))
		{
			btn_time += ticks;
			break;
		}

		if (btn_time >= BUTTON_HOLD) {
			timeout_ticks = 0;
			state = UPLOAD;
			app = SIDE_APP;
			WRITE_LOG("GOES TO SIDE APP UPLOAD STATE\r\n");
			UART_LOG("Start download side app \r\n");
		}
		else if (btn_time >= BUTTON_PRESS) {
			timeout_ticks = 0;
			state = UPLOAD;
			app = MAIN_APP;
			WRITE_LOG("GOES TO MAIN APP UPLOAD STATE\r\n");
			UART_LOG("Start download main app \r\n");
		}

		btn_time = 0;
		break;
	}
	case UPLOAD:
	{

		if (led_ticks > UPLOAD_LED_BLINK_INTERVAL)
		{
			GPIO_Toggle(LD2_GPIO_Port, LD2_Pin);
			led_ticks = 0;
		}

		if (timeout_ticks < UPLOAD_INTERVAL) break;

		if (upload_sequence(app) < 0)
		{
			error = (app == MAIN_APP) ? MAIN_APP_ERROR : SIDE_APP_ERROR;
			failed_blink();
		}

		timeout_ticks = 0;
		UART_LOG("Goes to BOOT state \r\n Wait for user input \r\n");
		state = BOOT;
		break;
	}
	case BOOT:
	{
		if (led_ticks > TERMINATE_LED_BLINK_INTERVAL)
		{
			GPIO_Toggle(LD2_GPIO_Port, LD2_Pin);
			led_ticks = 0;
		}

		if (timeout_ticks < BOOT_INTERVAL) break;

		app = GPIO_Read(B1_GPIO_Port, B1_Pin);

		switch (error)
		{
		case NO_ERROR:
			UART_LOG("BOOTING TO APP %d \r\n",app);
			jump2app(app);
			break;
		case MAIN_APP_ERROR:
			UART_LOG("BOOTING TO SIDE APP \r\n");
			erase_app(MAIN_APP_ERROR);
			jump2app(SIDE_APP);
			break;
		case SIDE_APP_ERROR:
			UART_LOG("BOOTING TO MAIN APP \r\n");
			erase_app(SIDE_APP);
			jump2app(MAIN_APP);
			break;
		default:
			break;
		}
		UART_LOG("Fail to boot \r\n");
		WRITE_LOG("FAILED TO BOOT\r\n");
		HardFault_Handler();

		break;
	}
	default:
		break;
	}

	return;
}

void jump2app(uint8_t app){
	uint32_t jumpAddress = 0;
	pFunction Jump_To_Application;

	__IO uint32_t * APP_ADDR = (app) ? (uint32_t* )MAIN_APP_ADDR : (uint32_t *)SIDE_APP_ADDR;

	if (((* APP_ADDR) & 0x2FFE0000) == 0x20020000)
	{
		peripheral_deinit();
		jumpAddress = *(APP_ADDR + 1);
		Jump_To_Application = (pFunction) jumpAddress;
		__set_MSP(*(volatile uint32_t * )jumpAddress);
		Jump_To_Application();
	}
}

void upload_app(uint8_t app,const uint32_t offset,const uint8_t * payload,const uint16_t size){
	ll_flash_unlock();
	if (app)
	{
		while (ll_flash_write(MAIN_APP_ADDR+offset, payload, size) < 0);
	}
	else
	{
		while (ll_flash_write(SIDE_APP_ADDR+offset, payload, size) < 0);
	}
	ll_flash_lock();

	return;
}

int upload_sequence(uint8_t app)
{

	 FIL fil; 		//File handle

#if !defined(SD_LOG)
	 FRESULT fres; //Result after operations
	 FATFS FatFs; 	//Fatfs handle

	//some variables for FatFs
	 fres = f_mount(&FatFs, "/", 1); //1=mount now
	 if (fres != FR_OK) {
		  UART_LOG("f_mount error (%i)\r\n", fres);
		  return -1;
	 }
	 UART_LOG("f_mount is successed \r\n");
	 successful_blink();
#endif


	fres = f_open(&fil, (app > 0) ? main_app_path : side_app_path, FA_READ);
	if (fres != FR_OK) {
		UART_LOG("f_open error (%i)\r\n",fres);
		WRITE_LOG("FAILED TO OPEN\r\n");
		return -1;
	}
	WRITE_LOG("OPENED APP BINARY\r\n");

	UART_LOG("f_open succeed \r\n");
	successful_blink();

	erase_app(app);

	UART_LOG("Erase sector is succeed \r\n");
	successful_blink();
	WRITE_LOG("ERASED SECTOR\r\n");

	uint8_t done_flag = 0;

	__IO uint32_t temp_read_size = 0,read_size = 0;
	uint32_t offset = 0;

	uint8_t read_buff[CHUNK_OF_DATA] = {0};

	while (!done_flag)
	{
		if ((fres = f_read(&fil, read_buff, CHUNK_OF_DATA, (UINT *)&read_size)) != FR_OK)
		{
			UART_LOG("f_read error (%i)\r\n",fres);
			WRITE_LOG("FAILED TO READ\r\n");
			return -1;
		}

		GPIO_Toggle(LD2_GPIO_Port, LD2_Pin);

		UART_LOG("Download application at offset %x \r\n",offset);
		WRITE_LOG("UPLOADING ....\r\n");

		temp_read_size = read_size;

		if (temp_read_size < CHUNK_OF_DATA) done_flag = 1;

		upload_app(app, offset, read_buff, temp_read_size);

		offset += temp_read_size;
	}

	UART_LOG("Download completed \r\n");
	successful_blink();
	//Be a tidy kiwi - don't forget to close your file!
	f_close(&fil);
	UART_LOG("Close file \r\n");
	//We're done, so de-mount the drive
	WRITE_LOG("UPLOAD SUCCESSFUL\r\n");
	f_mount(NULL, "", 0);
	UART_LOG("Umount file \r\n");
	return 0;
}

void erase_app(uint8_t app)
{
	ll_flash_unlock();
	if (app)
	{
		ll_flash_erase(SECTOR_6);
		ll_flash_erase(SECTOR_7);
	}
	else {
		ll_flash_erase(SECTOR_4);
		ll_flash_erase(SECTOR_3);
		ll_flash_erase(SECTOR_5);
	}
	ll_flash_lock();
	return;
}

int bootloader_mainloop()
{
	uint32_t state_machine_ticks = get_systick();

	state = INITAL;
	btn = UNPRESS;
	app = MAIN_APP;
	error = NO_ERROR;



#if defined(SD_LOG)
	 fres = f_mount(&FatFs, "/", 1); //1=mount now
	 if (fres != FR_OK) {
		  UART_LOG("f_mount error (%i)\r\n", fres);
		  state = BOOT;
	 }
	 UART_LOG("f_mount is successed \r\n");
	 failed_blink();
#endif

	WRITE_LOG("INITALIZE BOOT \r\n");

	for(;;)
	{
		handle_bootlader_statemachine(get_systick()-state_machine_ticks);
		state_machine_ticks = get_systick();
		delay_ms(20);
	}

}

void successful_blink()
{
	 for (int i =0; i < 5; i++)
	 {
		 GPIO_Toggle(LD2_GPIO_Port, LD2_Pin);
		 delay_ms(50);
	 }
	return;
}


void failed_blink()
{
	 for (int i =0; i < 5; i++)
	 {
		 GPIO_Toggle(LD2_GPIO_Port, LD2_Pin);
		 delay_ms(200);
	 }
	return;
}

void write_log(const char * log)
{
	FIL fil;
	FRESULT result;
	result = f_open(&fil, "log", FA_OPEN_APPEND | FA_WRITE );
	UART_LOG("result = %d \r\n",result);
	if (result != FR_OK) return;

	UINT len = strlen(log);

	result = f_write(&fil, log, len, NULL);


	f_close(&fil);

	return;

}
