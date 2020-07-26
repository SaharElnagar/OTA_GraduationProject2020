/*
 * CAN.c
 *
 *  Created on: ??þ/??þ/????
 *      Author: hager mohamed
 */
#include "CAN.h"



void CAN1_init(void){
// gpio_init
// PD0 : CAN1_RX
// PD1 : CAN1_TX
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER &=~(GPIO_MODER_MODER0 + GPIO_MODER_MODER1 );
	GPIOD->MODER |= GPIO_MODER_MODER0_1 + GPIO_MODER_MODER1_1;
	GPIOD->AFR[0] =(GPIOD->AFR[0] &~ 0x000000FF) | 0x00000099;  //AF9
	GPIOD->OTYPER &=~(GPIO_OTYPER_OT_0 |GPIO_OTYPER_OT_1);     //push-pull
	GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPDR0 | GPIO_PUPDR_PUPDR1);
	GPIOD->PUPDR |= (GPIO_PUPDR_PUPDR0_0 | GPIO_PUPDR_PUPDR1_0);   //pull up
	GPIOD->OSPEEDR |=GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1;

	RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;   //clock enable;

//Automatic bus-off management
//Transmit FIFO priority driven by the request order
//Initialization request
	CAN1->MCR |=CAN_MCR_ABOM | CAN_MCR_TXFP |CAN_MCR_INRQ;
	CAN1->MCR &=~ CAN_MCR_SLEEP;  // exit Sleep mode
//wait for Initialization mode
	while((CAN1->MSR & CAN_MSR_SLAK) || !(CAN1->MSR & CAN_MSR_INAK) );

//timing parameters
//CAN Bit Time Calculation website
//tPCLK = 16 MHZ
//BaudRate =500 Kbit/s
//Sample-Point at: 87.5%
//Number of time quanta = 16
//Seg_1=13 tq   +  Seg_2=2 tq    + SYNC_SEG=1 tq
	CAN1->BTR =0x001c0001;
//    CAN1->BTR |=CAN_BTR_LBKM;    // Loop Back Mode enabled
//	CAN1->BTR |=CAN_BTR_SILM;      //silent mode

//For receiption (filter configuration)
CAN1->FMR |=CAN_FMR_FINIT;   // Initialization mode for the filters

////filter 0 :Dual 16-bit scale configuration
////          Identifier Mask mode
////filter numbers : #0   form 0x00 to 0x0f
////                 #1   not used
//CAN1->FS1R &=~CAN_FS1R_FSC0;
//CAN1->FM1R &=~CAN_FM1R_FBM0;
//CAN1->sFilterRegister[0].FR1 =(0x00<<5)+(0x7f0<<21);   //filter number  #0
//CAN1->sFilterRegister[0].FR2 =(0x00<<5)+(0x7f0<<21);   //filter number  #1

//filter 1 :Single 32-bit scale configuration
//         :Identifier List mode
//filter numbers : #2 0x88
//                 #3 not used
CAN1->FS1R |=CAN_FS1R_FSC1;
CAN1->FM1R |=CAN_FM1R_FBM1;
CAN1->sFilterRegister[1].FR1 = (0x88 << 21); //filter number  #2
CAN1->sFilterRegister[1].FR2 = (0x88 << 21); //filter number  #3


/**********************************CAN2***************************************/
////filter 14 :Dual 16-bit scale configuration
////          Identifier Mask mode
////filter numbers : #0   form 0x0f to 0xff
////                 #1   not used
//CAN1->FS1R &=~(1<<14);
//CAN1->FM1R &=~(1<<14);
//CAN1->sFilterRegister[14].FR1 =(0x0F<<5)+(0x70F<<21);   //filter number  #27
//CAN1->sFilterRegister[14].FR2 =(0x0F<<5)+(0x70F<<21);   //filter number  #28
////filter 15 :Single 32-bit scale configuration
////         :Identifier List mode
////filter numbers : #2 0x88
////                 #3 not used
//CAN1->FS1R |=(1<<15);
//CAN1->FM1R |=(1<<15);
//CAN1->sFilterRegister[15].FR1 = (0x88 << 21); //filter number  #29
//CAN1->sFilterRegister[15].FR2 = (0x88 << 21); //filter number  #30
//
//CAN1->FFA1R |=(1<<14) | (1<<15);  //Filter 14,15 assigned to FIFO 1
//CAN1->FA1R |=(1<<14)| (1<<15);      //activate filter 14,15

/******************************************************************************/
//For interrupt
//CAN1->IER |=CAN_IER_FMPIE0;
//NVIC->ISER[0] |=(1<<20);             //interrupt number #20

CAN1->FFA1R &=~(CAN_FFA1R_FFA0 | CAN_FFA1R_FFA1);  //Filter 0,1 assigned to FIFO 0
CAN1->FA1R |=CAN_FA1R_FACT0 | CAN_FA1R_FACT1;      //activate filter 0,1
CAN1->FMR &=~CAN_FMR_FINIT;                        //Active filters mode
CAN1->MCR &=~(CAN_MCR_INRQ | CAN_MCR_SLEEP);       //request normal mode


}


enum_error_type CAN1_transmit(uint8_t MailBox,CAN_message_TypeDef msg){
	enum_error_type ret_val=E_OK;
//check if transmission mailbox empty
	if(CAN1->TSR &(1 <<(26+MailBox))){
		//reset identifier register
		CAN1->sTxMailBox[MailBox].TIR=0;
		//standard id or Extended
		//Data frame or remote frame
		CAN1->sTxMailBox[MailBox].TIR |=(msg.IDE<<1)|(msg.RTR<<2);
		//set CAN identifier
		if(msg.IDE) CAN1->sTxMailBox[MailBox].TIR |=(msg.id<<3);
		else CAN1->sTxMailBox[MailBox].TIR |=(msg.id<<21);
		//set data length
		CAN1->sTxMailBox[MailBox].TDTR =(CAN1->sTxMailBox[MailBox].TDTR &(~0x0f))|msg.data_legth;
		//reset transmit data registers
		CAN1->sTxMailBox[MailBox].TDLR =0;
		CAN1->sTxMailBox[MailBox].TDHR =0;
		//set TX data
        for (int i=0;i<msg.data_legth;i++){
        	if(i<4) CAN1->sTxMailBox[MailBox].TDLR |=(msg.pdata[i]<<(8*i));
        	else CAN1->sTxMailBox[MailBox].TDHR |=(msg.pdata[i]<<(8*(i-4)));
        }
        CAN1->sTxMailBox[MailBox].TIR |=CAN_TI0R_TXRQ;  //transmission request
	}
	else{
		ret_val=E_NOT_OK;
	}
	return ret_val;
}

enum_error_type CAN1_receive(CAN_message_TypeDef* msg,uint8_t* match_index){
	enum_error_type ret_val=E_OK;
	//check fifo messge pending
	if(CAN1->RF0R & CAN_RF0R_FMP0){
	*match_index=(CAN1->sFIFOMailBox[0].RDTR & CAN_RDT0R_FMI)>>8;
    msg->data_legth= (CAN1->sFIFOMailBox[0].RDTR &CAN_RDT0R_DLC);  //get data length
    msg->IDE = (CAN1->sFIFOMailBox[0].RIR & CAN_RI0R_IDE)>>2;
    msg->RTR = (CAN1->sFIFOMailBox[0].RIR & CAN_RI0R_RTR)>>1;
    if(msg->IDE) msg->id =(CAN1->sFIFOMailBox[0].RIR & (CAN_RI0R_EXID|CAN_RI0R_STID))>>3;
    else msg->id =(CAN1->sFIFOMailBox[0].RIR & CAN_RI0R_STID)>>21;
    for(int i=0 ;i<msg->data_legth;i++){
    	if(i<4) msg->pdata[i] =(CAN1->sFIFOMailBox[0].RDLR >> (i*8)) & 0xFF;
    	else    msg->pdata[i] =(CAN1->sFIFOMailBox[0].RDHR >> ((i-4)*8)) & 0xFF;
    }
    CAN1->RF0R |=CAN_RF0R_RFOM0;       //release FIFO
	}else if(CAN1->RF1R & CAN_RF1R_FMP1){
		*match_index=(CAN1->sFIFOMailBox[1].RDTR & CAN_RDT1R_FMI)>>8;
		msg->data_legth= (CAN1->sFIFOMailBox[1].RDTR &CAN_RDT1R_DLC);  //get data length
	    msg->IDE = (CAN1->sFIFOMailBox[1].RIR & CAN_RI1R_IDE)>>2;
	    msg->RTR = (CAN1->sFIFOMailBox[1].RIR & CAN_RI1R_RTR)>>1;
	    if(msg->IDE) msg->id =(CAN1->sFIFOMailBox[1].RIR & (CAN_RI1R_EXID|CAN_RI1R_STID))>>3;
	    else msg->id =(CAN1->sFIFOMailBox[1].RIR & CAN_RI1R_STID)>>21;
	    for(int i=0 ;i<msg->data_legth;i++){
	    	if(i<4) msg->pdata[i] =(CAN1->sFIFOMailBox[1].RDLR >> (i*8)) & 0xFF;
	    	else    msg->pdata[i] =(CAN1->sFIFOMailBox[1].RDHR >> ((i-4)*8)) & 0xFF;
	    }
	    CAN1->RF1R |=CAN_RF1R_RFOM1;       //release FIFO
	}else{
		ret_val=E_NOT_OK;
	}
	return ret_val;
}


//CAN1_init must be call after calling this function
void CAN2_init(void){
// gpio_init
// PB12 : CAN2_RX
// PB13 : CAN2_TX
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOBEN;
	GPIOB->MODER &=~(GPIO_MODER_MODER12 + GPIO_MODER_MODER13 );
	GPIOB->MODER |= GPIO_MODER_MODER12_1 + GPIO_MODER_MODER13_1;
	GPIOB->AFR[1] =(GPIOB->AFR[1] &~ 0x00FF0000) | 0x00990000;  //AF9
	GPIOB->OTYPER &=~(GPIO_OTYPER_OT_12 |GPIO_OTYPER_OT_13);     //push-pull
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR12 | GPIO_PUPDR_PUPDR13);
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPDR12_0 | GPIO_PUPDR_PUPDR13_0);   //pull up
	GPIOB->OSPEEDR |=GPIO_OSPEEDER_OSPEEDR12 | GPIO_OSPEEDER_OSPEEDR13;

	RCC->APB1ENR |= RCC_APB1ENR_CAN2EN;   //clock enable;

//Automatic bus-off management
//Transmit FIFO priority driven by the request order
//Initialization request
	CAN2->MCR |=CAN_MCR_ABOM | CAN_MCR_TXFP |CAN_MCR_INRQ;
	CAN2->MCR &=~ CAN_MCR_SLEEP;  // exit Sleep mode
//wait for Initialization mode
	while((CAN2->MSR & CAN_MSR_SLAK) || !(CAN2->MSR & CAN_MSR_INAK) );

//timing parameters
//CAN Bit Time Calculation website
//tPCLK = 16 MHZ
//BaudRate =500 Kbit/s
//Sample-Point at: 87.5%
//Number of time quanta = 16
//Seg_1=13 tq   +  Seg_2=2 tq    + SYNC_SEG=1 tq
	CAN2->BTR =0x001c0001;
 //   CAN2->BTR |=CAN_BTR_LBKM;    // Loop Back Mode enabled
  //  CAN2->BTR |=CAN_BTR_SILM;      //silent mode

//For interrupt
CAN2->IER |=CAN_IER_FMPIE1;        //fifo 1
NVIC->ISER[2] |=(1<<1);            //interrupt number #65


CAN2->FMR &=~CAN_FMR_FINIT;                        //Active filters mode
CAN2->MCR &=~(CAN_MCR_INRQ | CAN_MCR_SLEEP);       //request normal moe


}

enum_error_type CAN2_transmit(uint8_t MailBox,CAN_message_TypeDef msg){
	char ret_val=E_OK;
//check if transmission mailbox empty
	if(CAN2->TSR &(1 <<(26+MailBox))){
		//reset identifier register
		CAN2->sTxMailBox[MailBox].TIR=0;
		//standard id or Extended
		//Data frame or remote frame
		CAN2->sTxMailBox[MailBox].TIR |=(msg.IDE<<1)|(msg.RTR<<2);
		//set CAN identifier
		if(msg.IDE) CAN2->sTxMailBox[MailBox].TIR |=(msg.id<<3);
		else CAN2->sTxMailBox[MailBox].TIR |=(msg.id<<21);
		//set data length
		CAN2->sTxMailBox[MailBox].TDTR =(CAN2->sTxMailBox[MailBox].TDTR &(~0x0f))|msg.data_legth;
		//reset transmit data registers
		CAN2->sTxMailBox[MailBox].TDLR =0;
		CAN2->sTxMailBox[MailBox].TDHR =0;
		//set TX data
        for (int i=0;i<msg.data_legth;i++){
        	if(i<4) CAN2->sTxMailBox[MailBox].TDLR |=(msg.pdata[i]<<(8*i));
        	else CAN2->sTxMailBox[MailBox].TDHR |=(msg.pdata[i]<<(8*(i-4)));
        }
        CAN2->sTxMailBox[MailBox].TIR |=CAN_TI0R_TXRQ;  //transmission request
	}
	else{
		ret_val=E_NOT_OK;
	}
	return ret_val;
}

enum_error_type CAN2_receive(CAN_message_TypeDef* msg,uint8_t* match_index){
	enum_error_type ret_val=E_OK;
	//check fifo messge pending
	if(CAN2->RF0R & CAN_RF0R_FMP0){
	*match_index=(CAN2->sFIFOMailBox[0].RDTR & CAN_RDT0R_FMI)>>8;
    msg->data_legth= (CAN2->sFIFOMailBox[0].RDTR &CAN_RDT0R_DLC);  //get data length
    msg->IDE = (CAN2->sFIFOMailBox[0].RIR & CAN_RI0R_IDE)>>2;
    msg->RTR = (CAN2->sFIFOMailBox[0].RIR & CAN_RI0R_RTR)>>1;
    if(msg->IDE) msg->id =(CAN2->sFIFOMailBox[0].RIR & (CAN_RI0R_EXID|CAN_RI0R_STID))>>3;
    else msg->id =(CAN2->sFIFOMailBox[0].RIR & CAN_RI0R_STID)>>21;
    for(int i=0 ;i<msg->data_legth;i++){
    	if(i<4) msg->pdata[i] =(CAN2->sFIFOMailBox[0].RDLR >> (i*8)) & 0xFF;
    	else    msg->pdata[i] =(CAN2->sFIFOMailBox[0].RDHR >> ((i-4)*8)) & 0xFF;
    }
    CAN2->RF0R |=CAN_RF0R_RFOM0;       //release FIFO
	}else if(CAN2->RF1R & CAN_RF1R_FMP1){
		*match_index=(CAN2->sFIFOMailBox[1].RDTR & CAN_RDT1R_FMI)>>8;
		msg->data_legth= (CAN2->sFIFOMailBox[1].RDTR &CAN_RDT1R_DLC);  //get data length
	    msg->IDE = (CAN2->sFIFOMailBox[1].RIR & CAN_RI1R_IDE)>>2;
	    msg->RTR = (CAN2->sFIFOMailBox[1].RIR & CAN_RI1R_RTR)>>1;
	    if(msg->IDE) msg->id =(CAN2->sFIFOMailBox[1].RIR & (CAN_RI1R_EXID|CAN_RI1R_STID))>>3;
	    else msg->id =(CAN2->sFIFOMailBox[1].RIR & CAN_RI1R_STID)>>21;
	    for(int i=0 ;i<msg->data_legth;i++){
	    	if(i<4) msg->pdata[i] =(CAN2->sFIFOMailBox[1].RDLR >> (i*8)) & 0xFF;
	    	else    msg->pdata[i] =(CAN2->sFIFOMailBox[1].RDHR >> ((i-4)*8)) & 0xFF;
	    }
	    CAN2->RF1R |=CAN_RF1R_RFOM1;       //release FIFO
	}else{
		ret_val=E_NOT_OK;
	}
	return ret_val;
}


void CAN_init(void){
	CAN1_init();
}


enum_error_type CAN_transmit_blocking(uint8_t * pdata, uint32_t size) {
	enum_error_type ret_val = E_OK;
	CAN_message_TypeDef TX_msg = { .IDE = 0, .RTR = 0, .id = CAN_TRANSMIT_ID,
			.data_legth = size, .pdata = pdata };
	while (size > 0) {
		if (size <= 8) {
			TX_msg.data_legth = size;
			ret_val |= CAN1_transmit(0, TX_msg);
			size = 0;
		} else {
			TX_msg.data_legth = 8;
			ret_val |= CAN1_transmit(0, TX_msg);
			size -= 8;
			TX_msg.pdata += 8;
		}
	}
	return ret_val;
}

enum_error_type CAN_receive_blocking(uint8_t * pdata, uint32_t size) {
	enum_error_type ret_val = E_OK;
	uint8_t match_index;
	CAN_message_TypeDef RX_msg;
	RX_msg.pdata = pdata;
	while (size > 0) {
		ret_val |= CAN1_receive(&RX_msg, &match_index);
		size -= RX_msg.data_legth;
		RX_msg.pdata += RX_msg.data_legth;
	}
	return ret_val;
}
