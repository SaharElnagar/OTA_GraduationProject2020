#ifndef __PACKET_H__
#define __PACKET_H__

#define CAN_UPDATE
//#define UART_UPDATE

//*****************************************************************************
// Bootloader commands status
//*****************************************************************************
#define CMD_SIZE										0x2

#define REQUEST_TO_UPDATE						0x1111
#define SEND_FREAME_INFO            0x2222
#define RECEIVE_FRAME_INFO          0x3333
#define READY_TO_RECEIVE_UPDATE     0x4444
#define RECEIVE_PACKET   						0x5555
#define SEND_NEW_PACKET							0x6666
#define END_OF_UPDATE 							0x7777
#define CHECK_CANCEL_UPDATE					0x8888
#define CANCEL_UPDATE_REQUEST				0x9999
#define UPDATE_SUCCESS							0xAAAA


 /*
  *Send ready to receive command
  */
     void Ready_To_Receive(void);
 
  /*
   *Send ready to receive command
   */
 

#endif

