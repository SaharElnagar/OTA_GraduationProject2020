/*
 * main.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: hager mohamed
 */

#ifndef MAIN_H_
#define MAIN_H_
#include "stm32f4xx.h"
#include "common.h"
#include "LCD.h"
#include "USART.h"
#include "CAN.h"
#include "Flash_if.h"

typedef struct {
	uint16_t PacketsNum;
	uint16_t ExtraBytes;
	uint32_t CheckSum;
} str_FrameInfo;

typedef enum {
	BL_OK = 0,
	BL_NO_SPACE = 0x1,
	BL_ERASE_FAIL = 0x2,
	BL_WRITE_FAIL = 0x4,
	BL_INVALID_APP_ADD = 0x8,
	BL_NO_BACKUP = 0x10
} enum_bootloader_error_t;

#define PACKET_SIZE                1024
#define FrameInfo_SIZE              8

#define IDEL                        0x0000
#define REQUEST_TO_UPDATE		    0x1111
#define SEND_FREAME_INFO            0x2222
#define RECEIVE_FRAME_INFO          0x3333
#define READY_TO_RECEIVE_UPDATE     0x4444
#define RECEIVE_PACKET   			0x5555
#define SEND_NEW_PACKET				0x6666
#define END_OF_FRAME 				0x7777
#define JUMP_TO_NEW_APP				0x8888
#define CANCEL_UPDATE				0x9999
#define UPDATE_SUCCESS				0xAAAA
#define RESTORE_BACKUP              0xBBBB
#define RESTORE_BACKUP_SUCCESS      0xCCCC
#define ERROR                       0xDDDD

//#define DISPLAY                1

//#define UART_ENABLE_UPDATE     1
#define CAN_ENABLE_UPDATE    1

#ifdef UART_ENABLE_UPDATE
#define Communication_init   		USART2_init
#define Communication_reset         USART2_reset
#define transmit_blocking 		    USART2_transmit_blocking
#define receive_blocking 			USART2_receive_blocking
#elif  CAN_ENABLE_UPDATE
#define Communication_init   		CAN_init
#define Communication_reset         CAN_reset
#define transmit_blocking    		CAN_transmit_blocking
#define receive_blocking 			CAN_receive_blocking
#endif


#endif /* MAIN_H_ */
