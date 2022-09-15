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

#define SYSTEM_ID 		1
#define COMPONENT_ID 	1

#define DATA		12
#define SIZE		4
#define OPCODE		3
#define SEQ			0
#define SESSION 	2
#define OFFSET		8


struct mavlink_handle_s {
	uint8_t payload_len;
	uint32_t offset; 

	uint8_t mavlink_buffer[256];


	mavlink_message_t main;
	mavlink_status_t status;
	mavlink_file_transfer_protocol_t ftp;
	mavlink_file_transfer_protocol_t ftp_tx;

	uint16_t sequence;
	uint8_t session;
	uint8_t uploader_ready;
	uint8_t data_ready;
	uint8_t error;
	uint8_t path;
};

enum opcode {
	terminate = 1,
	read = 5,
	create = 6,
	write = 7,
	ack = 128,
	nak = 129
};




int8_t mavlink_init(struct mavlink_handle_s * handle);

int8_t mavlink_receive(struct mavlink_handle_s * handle,char ch);

int8_t mavlink_handle_ftp(struct mavlink_handle_s * handle);

int8_t mavlink_ftp_send(struct mavlink_handle_s * handle,uint8_t isAck);



#endif /* INC_MAVLINK_HANDLE_H_ */
