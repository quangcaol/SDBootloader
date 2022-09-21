/*
 * mavlink_handle.h
 *
 *  Created on: Sep 9, 2022
 *      Author: quangcaol
 */

#ifndef INC_MAVLINK_HANDLE_H_
#define INC_MAVLINK_HANDLE_H_

#include "main.h"
#include "c_library_v2/common/mavlink.h"

#define SYSTEM_ID 	1
#define COMPONENT_ID 	1

/**
 * @brief FTP sepecial field
 * 
 */
#define DATA		12
#define SIZE		4
#define OPCODE		3
#define SEQ		0
#define SESSION 	2
#define OFFSET		8


enum ftp_state{
	wait,
	down,
	quit,
	timeout
};

enum opcode {
	terminate = 1,
	read = 5,
	create = 6,
	write = 7,
	ack = 128,
	nak = 129
};

struct mavlink_handle_s {

	mavlink_message_t main;
	mavlink_status_t status;
	mavlink_file_transfer_protocol_t ftp;
	mavlink_file_transfer_protocol_t ftp_tx;

	enum ftp_state state;

	uint8_t mavlink_buffer[280];

	uint8_t payload_len;
	uint32_t offset;
	uint16_t sequence;
	uint8_t session;
	uint8_t uploader_ready;
	uint8_t data_ready;
	uint8_t error;
	uint8_t path;
};





/**
 * @brief init mavlink handle
 * 
 * @param handle mavlink struct used to init
 * @return int8_t 
 */
int8_t mavlink_init(struct mavlink_handle_s * handle);

/**
 * @brief Mavlink parse and handle receive message
 * 
 * @param handle mavlink struct
 * @param ch 
 * @return int8_t 
 */
int8_t mavlink_receive(struct mavlink_handle_s * handle,char ch);

/**
 * @brief Mavlink ftp state handler
 * 
 * @param handle mavlink struct
 * @return int8_t 
 */
int8_t mavlink_handle_ftp(struct mavlink_handle_s * handle);

/**
 * @brief send mavlink ftp
 * 
 * @param handle mavlink struct
 * @param isAck send Ack or Nak 
 * @return int8_t 
 */
int8_t mavlink_ftp_send(struct mavlink_handle_s * handle,uint8_t isAck);



#endif /* INC_MAVLINK_HANDLE_H_ */
