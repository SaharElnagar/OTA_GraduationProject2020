/*
 * CAN.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: hager mohamed
 */

#ifndef CAN_H_
#define CAN_H_

#include "stm32f4xx.h"
#include "common.h"

#define CAN_TRANSMIT_ID         0x66


typedef struct {
	uint8_t IDE;  //1 -> extended identifier,   0 -> standard identifier
	uint8_t RTR;  //1 -> Remote frame,          0 -> Data frame
	uint32_t id;  // message identifier
	uint8_t data_legth;  //number of data bytes 0 - 8
	uint8_t *pdata;      // pointer to the data
}CAN_message_TypeDef;

void CAN_init(void);

enum_error_type CAN_transmit_blocking(uint8_t * pdata, uint32_t size);
enum_error_type CAN_receive_blocking(uint8_t * pdata, uint32_t size);

//MailBox :mail box number 0,1,2
enum_error_type CAN1_transmit(uint8_t MailBox,CAN_message_TypeDef msg);
enum_error_type CAN2_transmit(uint8_t MailBox,CAN_message_TypeDef msg);

enum_error_type CAN1_receive(CAN_message_TypeDef* msg,uint8_t* match_index);
enum_error_type CAN2_receive(CAN_message_TypeDef* msg,uint8_t* match_index);
#endif /* CAN_H_ */
