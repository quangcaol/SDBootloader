# STM32 Mavlink Bootloader
The purpose of this project is to leverage mavlink **file transfer protocol** in order to flash new program into STM32. In this example, i use **USART** as the physical layer to transmit mavlink package. In the future, i will try on ethernet,Canard to alter usart.
- Arm-gcc
- STM32F411RE Nucleo Board
	- 512kb FLASH - 8 sectors
- USART2
- Systick
- Bootloader is ~9Kb with ```-Os``` optimazation
## Flash sector
| Address | Sector | Program |
| :---:  | :---: | :---: |
| 0x0800 0000 | Sector 0 | Bootloader |
| 0x0800 4000 | Sector 1 | Bootloader |
| 0x0800 8000 | Sector 2 | Bootloader |
| 0x0800 C000 | Sector 3 | Side app |
| 0x0801 0000 | Sector 4 | Side app |
| 0x0802 0000 | Sector 5 | Side app |
| 0x0804 0000 | Sector 6 | Main app |
| 0x0806 0000 | Sector 7 | Main app |

Bootloader program will be placed in 48Kb section, Side app is in 208 Kb section and Main app is in 256Kb section 

### **Side App is the secondary application for backup purpose if Main app is fail**
## How do i use it ?
The project is written by STM32CubeIDE, so its recommended to use STM32CubeIDE.

---
- Complete ```port.c``` to match your hardware
- Modify ```ll_serial.c``` to use your desired usart device
- Modify ```ll_flash.c``` to match your flash requirement
- Build and upload to your hardware

If you wish to change the address of bootloader please modify MEMORY section in ```./STM32F411RETX_FLASH.ld``` and change ```VECT_TAB_OFFSET``` in ```./Core/Src/system_stm32f4xx.c```

## Application for bootloader
In ```Example``` folder contains example for main app and side. If you wish to modify application address, follow above steps.
## Bootloader Behavior
- 5 second at boot, waiting for mavlink ftp create file message, LED will blink at 1 Hz.
- 3 second at final stage, LED will blink at 0.2 Hz to indicate readiness to launch program
- If upload is progess, Timeout is 5 seconds, LED will blink at 0.5Hz

## Uploader programs
The example of uploader is in ```script``` folder.

Run ```python3 ./script/uploader.py -h``` for more information
## Contribute
Feel free to help my project.

