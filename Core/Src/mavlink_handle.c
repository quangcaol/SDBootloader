/*
 * mavlink_handle.c
 *
 *  Created on: Sep 9, 2022
 *      Author: quangcaol
 */


#include "mavlink_handle.h"

#include <stdlib.h>



int8_t mavlink_init(struct mavlink_handle_s * handle)
{
	memset(&handle->main,0,sizeof(mavlink_message_t));
	memset(&handle->status,0,sizeof(mavlink_status_t));
	memset(&handle->ftp,0,sizeof(mavlink_file_transfer_protocol_t));
	memset(&handle->ftp_tx,0,sizeof(mavlink_file_transfer_protocol_t));
	memset(&handle->mavlink_buffer,0,256);

	handle->ftp.target_network = 0;
	handle->ftp.target_system = 0;
	handle->ftp.target_component =0;

	handle->uploader_ready = 0;
	handle->data_ready = 0;
	handle->error = 0;
	handle->session = 0;
	handle->sequence = 0;

	return 1;
}

int8_t mavlink_receive(struct mavlink_handle_s * handle,char ch)
{
	int8_t res = 0;
	if (!mavlink_parse_char(MAVLINK_COMM_0,ch,&handle->main,&handle->status)) return -1;

	switch (handle->main.msgid)
	{
	case MAVLINK_MSG_ID_FILE_TRANSFER_PROTOCOL:
		mavlink_handle_ftp(handle);
		break;

	default:
		break;
	}
	return 1;

}

int8_t mavlink_handle_ftp(struct mavlink_handle_s * handle)
{
	mavlink_msg_file_transfer_protocol_decode(&handle->main,&handle->ftp);

	handle->sequence = (uint16_t)(handle->ftp.payload[0] << 8) |  handle->ftp.payload[1];

	switch(handle->ftp.payload[3])
	{
		case terminate:
			handle->uploader_ready = 0;
			handle->session = 0;
			break;
		case read:
			break;
		case create:
			handle->session++;
			handle->path = handle->ftp.payload[DATA];
			handle->uploader_ready = 1;
			break;
		case write:
			handle->data_ready = 1;
			handle->payload_len = handle->ftp.payload[SIZE];
			handle->offset = handle->ftp.payload[OFFSET] << 24
					| handle->ftp.payload[OFFSET+1] << 16
					| handle->ftp.payload[OFFSET+2] << 8
					| handle->ftp.payload[OFFSET+3];
			break;
		case ack:
			break;
		case nak:
			break;
		default:
			break;
	}

	return 1;
}

int8_t mavlink_ftp_send(struct mavlink_handle_s * handle,uint8_t isAck)
{
	handle->ftp_tx.payload[SEQ] = (handle->sequence & 0xFF00) >> 8;
	handle->ftp_tx.payload[SEQ+1] = handle->sequence & 0xFF;
	handle->ftp_tx.payload[SESSION] = handle->session;
	handle->ftp_tx.payload[SIZE] = 0;
	handle->ftp_tx.payload[OPCODE] = (isAck == 1 )? ack : nak;
	if (isAck)
	{
		handle->ftp_tx.payload[DATA] = handle->error;
	}
	mavlink_msg_file_transfer_protocol_encode(SYSTEM_ID,COMPONENT_ID,&handle->main,&handle->ftp_tx);
	uint16_t buffer_len = mavlink_msg_to_send_buffer(handle->mavlink_buffer,&handle->main);

	for (uint8_t i = 0 ; i< buffer_len; ++i)
	{
		LL_USART_TransmitData8(USART2, handle->mavlink_buffer[i]);
		while(!LL_USART_IsActiveFlag_TXE(USART2));
	}
	while(!LL_USART_IsActiveFlag_TC(USART2));

	return 1;
}
