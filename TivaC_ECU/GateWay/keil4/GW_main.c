/*
 * GW_main.c
 *
 *  Created on: March 16, 2020
 *      Author: Sahar Elnagar
 */


/*****************************************************************************************/
/*                                   Include Component headres                           */
/*****************************************************************************************/
#include "Packet.h"
#include "stdint.h"
#include "bl_CAN.h"
#include "tm4c123gh6pm.h"
#include "sysctl.h"

uint8_t Packet_Buffer[(PACKETS_NUM*PACKET_SIZE)+EXTRA_BYTES]=
	{
	0x10,0x04,0x00,0x20,0x6D,0x12,0x00,0x00,0x71,0x12,0x00,0x00,0x73,0x12,0x00,0x00,
	0x75,0x12,0x00,0x00,0x77,0x12,0x00,0x00,0x79,0x12,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7B,0x12,0x00,0x00,
	0x7D,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x12,0x00,0x00,0x81,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x00,0x00,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,
	0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x83,0x12,0x00,0x00,0x00,0xF0,0x3A,0xB8,
	0xFE,0xE7,0xFE,0xE7,0xFE,0xE7,0xFE,0xE7,0xFE,0xE7,0xFE,0xE7,0xFE,0xE7,0xFE,0xE7,
	0xFE,0xE7,0xFE,0xE7,0x08,0xB5,0x12,0x48,0x00,0x68,0x40,0xF0,0x20,0x00,0x11,0x49,
	0xC1,0xF8,0x08,0x01,0x0E,0x48,0x00,0x68,0x00,0x90,0x0F,0x48,0x0F,0x49,0x08,0x60,
	0x1F,0x20,0x09,0x1D,0x08,0x60,0x00,0x20,0x09,0x1D,0x08,0x60,0x09,0x1D,0x08,0x60,
	0x0E,0x20,0x0B,0x49,0x08,0x60,0x00,0x20,0x09,0x49,0x20,0x31,0x08,0x60,0x11,0x20,
	0x06,0x49,0x10,0x39,0x08,0x60,0x1F,0x20,0x04,0x49,0x09,0x1F,0x08,0x60,0x08,0xBD,
	0x08,0xE1,0x0F,0x40,0x00,0xE0,0x0F,0x40,0x4B,0x43,0x4F,0x4C,0x20,0x55,0x02,0x40,
	0x00,0x54,0x02,0x40,0xFF,0xF7,0xCE,0xFF,0x2D,0xE0,0x17,0x48,0x00,0x68,0x00,0xF0,
	0x10,0x00,0x16,0x49,0x08,0x60,0x16,0x48,0xD0,0xF8,0xFC,0x03,0x00,0xF0,0x01,0x00,
	0x14,0x49,0x08,0x60,0x11,0x48,0x00,0x68,0x30,0xB1,0x08,0x46,0x00,0x68,0x18,0xB1,
	0x00,0x20,0x0D,0x49,0x08,0x60,0x16,0xE0,0x0C,0x48,0x00,0x68,0x30,0xB1,0x0D,0x48,
	0x00,0x68,0x18,0xB9,0x08,0x20,0x08,0x49,0x08,0x60,0x0C,0xE0,0x07,0x48,0x00,0x68,
	0x30,0xB9,0x08,0x48,0x00,0x68,0x18,0xB1,0x02,0x20,0x03,0x49,0x08,0x60,0x02,0xE0,
	0x04,0x20,0x01,0x49,0x08,0x60,0xD0,0xE7,0xFC,0x53,0x02,0x40,0x00,0x00,0x00,0x20,
	0x00,0x50,0x02,0x40,0x04,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
	};
/*****************************************************************************************/
/*                                   Local Function Definition                           */
/*****************************************************************************************/
void PortF_Init(void);

/*****************************************************************************************/
/*                                   Local types Definition                              */
/*****************************************************************************************/  	
typedef struct
{
	uint16_t PacketsNum ;
  uint16_t ExtraBytes ;
}str_FrameInfo ;

/*****************************************************************************************/
/*                                   main function                                       */
/*****************************************************************************************/ 
int main(void)
{
	volatile uint8_t SW1=0,SW2=0;
	uint8_t Frame[8] ;
	uint16_t Cmd_Status ;
	str_FrameInfo FrameInfo =
  {
	 PACKETS_NUM,
	 EXTRA_BYTES
	} ;
	
	PLL_Init();
	Can_Init();
	PortF_Init();
	while(1)
	{
		/*Gateway state machine to update one ECU*/
		 switch(Cmd_Status) 
		 {
		 case REQUEST_TO_UPDATE:
					 /* 
						* Send to the ECU request to update
						*/
		        CanTransmitBlocking_Function(2,(uint8_t*)&Cmd_Status);
						Cmd_Status = RECEIVE_FREAME_INFO_REQUEST ;
		     
		 break;
		 case RECEIVE_FREAME_INFO_REQUEST:
					 /* 
						* Wait till the ECU request the frame info
						* Then go to SEND_FRAME_INFO
						*/
						CanReceiveBlocking_Function(2,(uint8_t*)&Cmd_Status);
		       
		 break;
		 case SEND_FRAME_INFO:
				 /*  
					* Send the frame info
					* Then go to READY_TO_RECEIVE_UPDATE
					*/
		      (*(uint16_t*)Frame)      = PACKETS_NUM ;
		      (*(uint16_t*)(Frame+2))  = EXTRA_BYTES ;
		      (*(uint32_t*)(Frame+4))  = CHECKSUM    ;
					CanTransmitBlocking_Function(8,Frame)  ;
          Cmd_Status = READY_TO_RECEIVE_UPDATE;		 
		 break;
		 case READY_TO_RECEIVE_UPDATE :
				 /*
					* Wait for the ECU to be ready to receive the update 
					*/
		      Cmd_Status = 0;
					CanReceiveBlocking_Function(2,(uint8_t*)&Cmd_Status);	         			 
		 break;
		 case SEND_PACKET:
				 /*
					* Send a packet to the ECU
					*/					
					if(FrameInfo.PacketsNum>0)
					{
						CanTransmitBlocking_Function(PACKET_SIZE,Packet_Buffer);
						FrameInfo.PacketsNum-=1 ;
						Cmd_Status = SEND_NEW_PACKET ;
					}
					else
					{
						CanTransmitBlocking_Function(EXTRA_BYTES,Packet_Buffer);
						FrameInfo.ExtraBytes = 0 ;
						Cmd_Status = END_OF_FRAME ;
					}						
		 break;
		 case SEND_NEW_PACKET:
				 /*
					* Request to send new packet 
					*/
					CanTransmitBlocking_Function(2,(uint8_t*)&Cmd_Status);
					Cmd_Status = SEND_PACKET ;			 
		 break;
		 case END_OF_FRAME:
				 /*Wait For ECU to confirm update succeeded*/
	        CanReceiveBlocking_Function(2,(uint8_t*)&Cmd_Status);					
		 break;
		 case UPDATE_SUCCESS:
			    GPIO_PORTF_DATA_R &= ~0xF;
          GPIO_PORTF_DATA_R |=  0x8 ;
		 break;
		 default:
				 /*Wait for start indication*/						 
					SW1 = GPIO_PORTF_DATA_R&0x10;     /* read PF4 into SW1*/
					if(!SW1 )
				 {
					  GPIO_PORTF_DATA_R &= ~0xF;
            GPIO_PORTF_DATA_R |=  0x4 ;
						Cmd_Status = REQUEST_TO_UPDATE ;
				 }
				 else
				 {
				 /*MISRA Rule*/
				 }
		 break;
	 }
	}
}

void PortF_Init(void){
  SYSCTL_RCGC2_R |= SYSCTL_RCGCGPIO_R5;                    // 1) F clock
  while(!(SYSCTL_PRGPIO_R &SYSCTL_PRGPIO_R5)){};           // delay
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0
}

