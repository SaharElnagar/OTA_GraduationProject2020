#ifndef __PACKET_H__
#define __PACKET_H__

//*****************************************************************************
// commands status
//*****************************************************************************

#define REQUEST_TO_UPDATE						           0x1111
#define RECEIVE_FREAME_INFO_REQUEST            0x2222
#define SEND_FRAME_INFO                        0x3333
#define SEND_PACKET      						           0x4444
#define READY_TO_RECEIVE_UPDATE                0x5555
#define SEND_NEW_PACKET							           0x6666
#define END_OF_FRAME 								           0x7777
#define CANCEL_UPDATE								           0x9999
#define UPDATE_SUCCESS							           0xAAAA

#define PACKETS_NUM 			0 
#define EXTRA_BYTES 			868
#define CHECKSUM					0XE4321

#define PACKET_SIZE				1024

#endif

