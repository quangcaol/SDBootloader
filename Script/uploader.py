import serial
import time
from pymavlink import mavutil
import sys
import os

from enum import Enum



os.environ["MAVLINK20"] = "1"



class ftp_state(Enum):
	init = 0
	connect = 1
	upload  = 2
	terminate = 3
	timeout = 4

class ftp_payload(Enum):
	DATA = 12
	SIZE = 4
	OPCODE = 3
	SEQ = 0
	SESSION = 2
	OFFSET = 8
	MAX = 251

class ftp_opcode(Enum):
	terminate = 1
	read = 5
	create = 6
	write = 7
	ack = 128
	nak = 129

class ftp_app(Enum):
	main_app = 1
	side_app = 0
	main_app_size = 262144
	side_app_size = 212992

class mavlink_ftp():
	def __init__(self,com,baud,path,isMain,chunk_size) -> None:
		self.current_state = ftp_state.init
		self.com_port = mavutil.mavlink_connection(com,baud)
		self.image_path = path
		self.upload_path = int(isMain)
		self.session = 0
		self.seq = 0
		self.chunk = chunk_size

	def insert_seq(payload,seq):

		payload[ftp_payload.SEQ.value] = (seq & 0xFF00) >> 8
		payload[ftp_payload.SEQ.value+1] = (seq & 0xFF)
		pass

	def get_seq(payload):
		return payload[ftp_payload.SEQ.value] << 8 | payload[ftp_payload.SEQ.value+1]

	def insert_offset(payload,offset):
		payload[ftp_payload.OFFSET.value] = (offset & 0xFF000000) >> 24
		payload[ftp_payload.OFFSET.value+1] = (offset & 0xFF0000) >> 16
		payload[ftp_payload.OFFSET.value+2] = (offset & 0xFF00) >> 8
		payload[ftp_payload.OFFSET.value+3] = (offset & 0xFF)
		pass

	def load_image(self):
		with open(self.image_path,"rb") as fd:
			self.image = fd.read()
		self.image_len = len(self.image)

	def print_image(self,head):
		addr = 0x8000000
		index = 0
		print("Address   :  0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7    |        |")
		for _ in range(head):
			print(hex(addr), ':  ',end='')
			
			raw_data = self.image[index:index+8].hex(':').split(':')

			if (len(raw_data) < 8 and raw_data[0] != ''):
				missing_byte = 8 - len(raw_data)
				raw_data += ['00']*missing_byte
			elif (raw_data[0] == ''):
				raw_data = ['00']*8

			print("{0}   {1}   {2}   {3}   {4}   {5}   {6}   {7}     |{8:8s}|".format(*raw_data,self.image[index:index+8].decode('ascii')))
			addr += 0x8
			index += 8

	def ftp_parser(self,msg):
		recv_seq = mavlink_ftp.get_seq(msg.payload)
		if (recv_seq != self.seq):
			print("Wrong sequence")

		self.session = msg.payload[ftp_payload.SESSION.value]

		match ftp_opcode(msg.payload[ftp_payload.OPCODE.value]):
			case ftp_opcode.ack:
				return True
			case ftp_opcode.nak:
				print("Receive error {:}".format(msg.payload[ftp_payload.DATA.value]))
				return False
		
		return False

	def update(self):
		match self.current_state:
			case ftp_state.init:
				print("Initialize Mavlink FTP Connection")
				self.current_state = ftp_state.connect
			case ftp_state.connect:
				payload = [0]*ftp_payload.MAX.value
				mavlink_ftp.insert_seq(payload,self.seq)
				payload[ftp_payload.OPCODE.value] = ftp_opcode.create.value
				payload[ftp_payload.SIZE.value] = 1
				payload[ftp_payload.DATA.value] = self.upload_path
				print("Send create file FTP message")
				self.com_port.mav.file_transfer_protocol_send(0,1,1,payload)
				msg = self.com_port.recv_match(timeout=5,blocking=True,type="FILE_TRANSFER_PROTOCOL")

				if (msg is None):
					print("Client is not respond")
					self.current_state = ftp_state.timeout
					return

				if (self.ftp_parser(msg)):
					self.current_state = ftp_state.upload
					return
				

			case ftp_state.upload:
				offset = 0

				self.load_image()

				if (len(self.image) > ftp_app.main_app_size.value and self.upload_path == ftp_app.main_app):
					print("Image size is larger than app flash size")
					exit(1)
				elif (len(self.image) > ftp_app.main_app_size.value and self.upload_path == ftp_app.main_app):
					print("Image size is larger than app flash size")
					exit(1)
				
				for i in range(0,int(self.image_len/self.chunk)+1):


					image_chunk = self.image[offset:(offset+self.chunk)]


					self.seq += 1
					payload = [0]*(ftp_payload.DATA.value) + list(image_chunk) + [255]*(ftp_payload.MAX.value-len(image_chunk)-ftp_payload.DATA.value)
					mavlink_ftp.insert_seq(payload,self.seq)
					payload[ftp_payload.SESSION.value] = self.session
					payload[ftp_payload.OPCODE.value] = ftp_opcode.write.value
					payload[ftp_payload.SIZE.value] = len(image_chunk)
					mavlink_ftp.insert_offset(payload,offset)


					print("Send upload file FTP message")
					self.com_port.mav.file_transfer_protocol_send(0,1,1,payload)


					msg = self.com_port.recv_match(timeout=15,blocking=True,type="FILE_TRANSFER_PROTOCOL")


					if (msg is None):
						print("Client is not respond")
						self.current_state = ftp_state.timeout
						return

					if (self.ftp_parser(msg)):
						print("Upload progress :{0:0.2f}%".format(offset*100/self.image_len))
						print(offset)
						offset += len(image_chunk)
					else:
						self.current_state = ftp_state.timeout
						return

				self.current_state = ftp_state.terminate
				print("Upload progress :{0:0.2f}%".format(100))

			case ftp_state.terminate:
				payload = [0]*ftp_payload.MAX.value
				payload[ftp_payload.SESSION.value] = self.session
				payload[ftp_payload.OPCODE.value] = ftp_opcode.terminate.value

				print("Send terminate session FTP message")
				self.com_port.mav.file_transfer_protocol_send(0,1,1,payload)

				msg = self.com_port.recv_match(timeout=15,blocking=True,type="FILE_TRANSFER_PROTOCOL")

				if (msg is None):
					print("Client is not respond")
					self.current_state = ftp_state.timeout
					return

				print("Completed")
				exit(1)

			case ftp_state.timeout:
				print("Timeout and Exit")
				exit(1)
			case _:
				print("No")
				exit(0)
	

	
	



		

if __name__ == "__main__":
	ftp = mavlink_ftp("/dev/ttyACM0",115200,"./main_app.bin",1,16)


	while(True):
		ftp.update()
		time.sleep(0.1)