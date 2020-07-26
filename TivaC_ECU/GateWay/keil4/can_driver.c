#include "can_driver.h"
#include "CAN_HW_TYPES.h"
#include "tm4c123gh6pm.h"

void GPIO_Init (Port_Name Port_Base){
		volatile uint32_t delay ;
		delay = SYSCTL_RCGC2_R ;
	
		if(Port_Base == Port_A) ACCESS_REG(SYSCTL_RCGC2)|=GPIOA; // Clock For Port A 
		if(Port_Base == Port_B) ACCESS_REG(SYSCTL_RCGC2)|=GPIOB; // clock For Port B 
   	if(Port_Base == Port_E) ACCESS_REG(SYSCTL_RCGC2)|=GPIOE; // clock For Port E 
  	if(Port_Base == Port_F) ACCESS_REG(SYSCTL_RCGC2)|=GPIOF; // clock For Port F
		
	  //Unlock PORTS
		ACCESS_REG(Port_Base+LOCK_R) = GPIO_LOCK_KEY ;
		ACCESS_REG(Port_Base+CR_R) 	 = 0xFF ; 
	
		// PCTL ,DIR , AFSEL , AMSEL ,DEN :
		if ( Port_Base == Port_A ){
			//Port Control
		ACCESS_REG(Port_Base+PCTL_R) &=~ 0x0FF;						 		//CLEAR_PCL 	
		ACCESS_REG(Port_Base+PCTL_R) |= GPIO_PCTL_PA0_CAN1RX; // PMC0 control CAN1RX
		ACCESS_REG(Port_Base+PCTL_R) |= GPIO_PCTL_PA1_CAN1TX; // PMC1 control CAN1TX
			//Direction Bit :
		CLEAR_BIT((Port_Base+DIR_R) ,	(1<<BIT0)); // RX >> Input 
		SET_BIT ((Port_Base+DIR_R)  ,	(1<<BIT1)); // TX >> Output
			//Alternate Function :
		SET_BIT ((Port_Base+AFSEL_R),	(1<<BIT0)); // Enable Alternate Fun Pin0
		SET_BIT ((Port_Base+AFSEL_R),	(1<<BIT1)); // Enable Alternate Fun Pin1	
			//Digital Enable:
		SET_BIT ((Port_Base+DEN_R),	(1<<BIT0)); // Enable Digital Pin0
		SET_BIT ((Port_Base+DEN_R),	(1<<BIT1)); // Enable Digital Pin1
			
	}else if ((Port_Base == Port_B) || (Port_Base == Port_E)){
		//Disable Analog Mode :
		CLEAR_BIT ((Port_Base+AMSEL_R),(1<<BIT4));
		CLEAR_BIT ((Port_Base+AMSEL_R),(1<<BIT5));
		//Port Control :
		ACCESS_REG(Port_Base+PCTL_R) &=~ (0x0FF0000);						 		//CLEAR_PCL 	
		ACCESS_REG(Port_Base+PCTL_R) |= GPIO_PCTL_PB4_CAN0RX; // PMC4 control CAN0RX
		ACCESS_REG(Port_Base+PCTL_R) |= GPIO_PCTL_PB5_CAN0TX; // PMC5 control CAN0TX
		//Direction Bit :
		CLEAR_BIT((Port_Base+DIR_R) ,	(1<<BIT4)); // RX >> Input 
		SET_BIT ((Port_Base+DIR_R)  ,	(1<<BIT5)); // TX >> Output

		//Alternate Function :
		SET_BIT ((Port_Base+AFSEL_R),	(1<<BIT4)); // Enable Alternate Fun Pin4
		SET_BIT ((Port_Base+AFSEL_R),	(1<<BIT5)); // Enable Alternate Fun Pin5
					//Digital Enable:
		SET_BIT ((Port_Base+DEN_R),	(1<<BIT4)); // Enable Digital Pin4
		SET_BIT ((Port_Base+DEN_R),	(1<<BIT5)); // Enable Digital Pin5
		
	}else { // Port_F
		//Port Control :
		ACCESS_REG(Port_Base+PCTL_R) &=~ 0x0F00F;						//CLEAR_PCL 
		ACCESS_REG (Port_Base+PCTL_R) |= GPIO_PCTL_PF0_CAN0RX;  // PMC0 control CAN0RX
		ACCESS_REG (Port_Base+PCTL_R) |= GPIO_PCTL_PF3_CAN0TX;  // PMC3 control CAN0TX
		//Direction Bit :
		CLEAR_BIT((Port_Base+DIR_R),	(1<<BIT0)); // RX >> Input 
		SET_BIT ((Port_Base+DIR_R) ,	(1<<BIT3)); // TX >> Output
				//Alternate Function 
		SET_BIT ((Port_Base+AFSEL_R),	(1<<BIT0)); // Enable Alternate Fun Pin0
		SET_BIT ((Port_Base+AFSEL_R),	(1<<BIT3)); // Enable Alternate Fun Pin3
				//Digital Enable:
		SET_BIT ((Port_Base+DEN_R),	(1<<BIT0)); // Enable Digital Pin0
		SET_BIT ((Port_Base+DEN_R),	(1<<BIT3)); // Enable Digital Pin3
	}

}

/*********************************************************************************************************/


void CANInit(uint32_t Base,uint8_t Mode){
	
	if(Base == CAN0_BASE){		
		//Enable Clock :
		SET_BIT(SYSCTL_RCGC0 , SYSCTL_RCGC0_CAN0 ) ;				 // Clock For CAN0	
	}else{
		SET_BIT(SYSCTL_RCGC0 , SYSCTL_RCGC0_CAN1 ) ;  			 // Clock For CAN1
	 
	}
	
	//CAN CONFIGURATION :
	SET_BIT((Base+CAN_CTL_R) , CAN_CTL_INIT);

	while(CAN0_IF1CRQ_R & CAN_IF1CRQ_BUSY) ; //wait while CAN0 Module is BUSY,set Flag .
	
	// Check Mode :

	if(Mode == Test_Mode){
		SET_BIT((Base+CAN_CTL_R) , CAN_CTL_TEST);    // Enable Test Mode	
		SET_BIT((Base+CAN_TST_R) , CAN_TST_LBACK );  // Enable LoopBack	
	}else{
		CLEAR_BIT((Base+CAN_CTL_R) , CAN_CTL_TEST);  // Disable Test Mode	
		CLEAR_BIT((Base+CAN_CTL_R) , CAN_TST_LBACK); // Disable LoopBack	
	}

}

/*********************************************************************************************************/


void CANBitTimingSet (uint32_t Base, tCANBitClkParms *psClkParms ){


			//Enable Access CANBIT
	SET_BIT((Base+CAN_CTL_R) , CAN_CTL_CCE) ; 
	
	ACCESS_REG((Base+CAN_BIT_R))&= ~(0xFFFF);

	ACCESS_REG((Base+CAN_BIT_R))|= ((psClkParms -> prescaler -1)); //BRP_SEG SET 
	ACCESS_REG((Base+CAN_BIT_R))|= ((( psClkParms -> ui32SJW-1)&0x3)<<6); //Phase2_SEG SET 
	ACCESS_REG((Base+CAN_BIT_R))|= (((psClkParms -> ui32SyncPropPhase1Seg-1)&0xF)<<8); //Prop_SEG + Phase1_SEG SET 
	ACCESS_REG((Base+CAN_BIT_R))|= ((( psClkParms->ui32Phase2Seg-1 )&0x7)<<12); //SJW_SEG SET 
	
		//Disable Access CANBIT
	CLEAR_BIT((Base+CAN_CTL_R) , CAN_CTL_CCE) ; 

}

/*********************************************************************************************************/


void CANEnable (uint32_t Base){
		
	//Out From Initialization Mode To Start Transmit :
	CLEAR_BIT((Base+CAN_CTL_R) , CAN_CTL_INIT);

}


/*********************************************************************************************************/

void CANDisable (uint32_t Base){
	
	if(Base == CAN0_BASE){		
		//Disable Clock :
		CLEAR_BIT(SYSCTL_RCGC0_R , SYSCTL_RCGC0_CAN0 ) ;				 // Clock For CAN0	
	}else{
		CLEAR_BIT(SYSCTL_RCGC0_R , SYSCTL_RCGC0_CAN1 ) ;  			 // Clock For CAN1
	}
	/*Enter Initialization Mode :
	SET_BIT((Base+CAN_CTL_R) , CAN_CTL_INIT);
	*/
}

/*********************************************************************************************************/
// flags -> CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS 

void CANIntEnable (uint32_t Base, uint32_t ui32IntFlags){

		if(Base == CAN0_BASE){
				NVIC_EN1_R |= (1<<7);
		} else if(Base == CAN1_BASE){
				NVIC_EN1_R |= (1<<8);
		}
		
    SET_BIT((Base+ CAN_CTL_R) , ui32IntFlags);
		
}

/*********************************************************************************************************/

void CANIntDisable (uint32_t Base, uint32_t ui32IntFlags){

    CLEAR_BIT( (Base + CAN_CTL_R) , ui32IntFlags);
    
}

/*********************************************************************************************************/

void CANIntClear (uint32_t Base, uint32_t ui32IntClr){

    CLEAR_BIT ( (Base+CAN_INT_R) , ui32IntClr); 
}

/*********************************************************************************************************/

uint32_t CANIntStatus (uint32_t Base, tCANIntStsReg eIntStsReg){
    uint32_t return_val;
	
    if (eIntStsReg == CAN_INT_STS_CAUSE){
        return_val = READ_REG( (Base + CAN_INT_R) );
    }else {
        return_val  =  READ_REG (Base+CAN_MSG1INT_R) & 0x00ff;  // not sure if it's 0x00ff or 0xff00 masking INTPND bits
        return_val |= (READ_REG(Base+CAN_MSG2INT_R) <<16 );
    }
    return return_val;
}
/*********************************************************************************************************/

void CANTransmitMessageSet (uint32_t Base, uint8_t ObjID, tCANConfigTXMsgObj *MsgObject){
	
			while (BIT_IS_SET ((Base + CAN_IF1CRQ_R) , CAN_IF1CRQ_BUSY));

		CLEAR_BIT((Base + CAN_IF1ARB2_R), CAN_IF1ARB2_MSGVAL);

		 tCANFRAME_TYPE Frame_Type = STANDARD_FRAME;

		// Check the message ID
		if(((MsgObject -> Msg_ID) > CAN_MAX_STANDARD_ID) && ((MsgObject -> Msg_ID) < CAN_MAX_EXTENDED_ID) && (MsgObject->Flags & MSG_OBJ_EXTENDED_ID)){
			
			 Frame_Type = EXTENDED_FRAME;
			
		} else if((MsgObject -> Msg_ID) > CAN_MAX_EXTENDED_ID){
			 return;    //UNValid Msg ID
		}

		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_WRNRD);		 // Set WRNRD bit = 1
		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_ARB);			 // Set ARB bit to enable access arbitration bits
		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_CONTROL);	 // Set Control bit to enable access control bits
		/*******************STANDARD FRAME*******************/
		if(Frame_Type == STANDARD_FRAME){
			
			CLEAR_BIT((Base + CAN_IF1ARB2_R), CAN_IF1ARB2_XTD);
			CLEAR_BIT((Base + CAN_IF1MSK2_R), CAN_IF1MSK2_MXTD);
			ACCESS_REG(Base + CAN_IF1ARB2_R) &= ~(0x1FFF);								 // First clear ARB2 Field
			ACCESS_REG(Base + CAN_IF1ARB2_R) |= ((MsgObject->Msg_ID) << 2);	// Write the message ID
			
			
		}
		/*******************EXTENDED FRAME*******************/
		 else if (Frame_Type == EXTENDED_FRAME){
			
			SET_BIT((Base + CAN_IF1ARB2_R), CAN_IF1ARB2_XTD);
			 
			ACCESS_REG(Base + CAN_IF1ARB1_R) &= ~(0xFFFF);													      // First clear ARB1 Field
			ACCESS_REG(Base + CAN_IF1ARB1_R) |= (MsgObject->Msg_ID & 0x0000FFFF);		      // Write first part of ID
			
			ACCESS_REG(Base + CAN_IF1ARB2_R) &= ~(0x1FFF);													      // First clear ARB2 Field
			ACCESS_REG(Base + CAN_IF1ARB2_R) |= ((MsgObject->Msg_ID & 0x1FFF0000) >> 16);	// Write Second part of ID
			
		}
		 
			// Set Direction bit to indicate transmit message object
			SET_BIT((Base + CAN_IF1ARB2_R), CAN_IF1ARB2_DIR);
			
			if(MsgObject->Flags & MSG_OBJ_TX_INT_ENABLE){
					SET_BIT((Base + CAN_IF1MCTL_R), CAN_IF1MCTL_TXIE);				// enable interrupt on successful transmission
			}
			

		// Set message Valid bit
		SET_BIT((Base + CAN_IF1ARB2_R), CAN_IF1ARB2_MSGVAL);
		// Mark this bit as end of buffer because no use of FIFOs in transmit mode
		SET_BIT((Base + CAN_IF1MCTL_R), CAN_IF1MCTL_EOB);
	
		ACCESS_REG((Base + CAN_IF1CRQ_R)) = ObjID;				// Write Message number into MNUM field to start transfer
	
}

/*********************************************************************************************************/

void CANReceiveMessageSet (uint32_t Base, uint8_t ObjID, tCANConfigRXMsgObj *MsgObject){
	
		while (BIT_IS_SET ((Base + CAN_IF2CRQ_R) , CAN_IF2CRQ_BUSY));

		CLEAR_BIT((Base + CAN_IF2ARB2_R), CAN_IF2ARB2_MSGVAL);
	
		tCANFRAME_TYPE Frame_Type = STANDARD_FRAME;
		
		// Check the message ID
		if(((MsgObject -> Msg_ID) > CAN_MAX_STANDARD_ID) && ((MsgObject -> Msg_ID) < CAN_MAX_EXTENDED_ID) && (MsgObject->Flags & MSG_OBJ_EXTENDED_ID)){
			
			 Frame_Type = EXTENDED_FRAME;
			
		} else if((MsgObject -> Msg_ID) > CAN_MAX_EXTENDED_ID){
			 return;    //UNValid Msg ID
		}
		
		// Check using mask or not
		if(MsgObject->Flags & MSG_OBJ_USE_ID_FILTER){
			
			SET_BIT((Base+CAN_IF2MCTL_R), CAN_IF2MCTL_UMASK);
			SET_BIT((Base + CAN_IF2CMSK_R), CAN_IF2CMSK_MASK);			 // Set Mask bit
		}
		
		SET_BIT((Base + CAN_IF2CMSK_R), CAN_IF2CMSK_WRNRD);		 // Set WRNRD bit = 1
		SET_BIT((Base + CAN_IF2CMSK_R), CAN_IF2CMSK_ARB);			 // Set ARB bit to enable access arbitration bits
		SET_BIT((Base + CAN_IF2CMSK_R), CAN_IF2CMSK_CONTROL);	 // Set Control bit to enable access control bits
		
		//SET_BIT((Base + CAN_IF2CMSK_R), CAN_IF2CMSK_CLRINTPND); // Clear Interrupt Pending Bit
		//SET_BIT((Base + CAN_IF2CMSK_R), CAN_IF2CMSK_TXRQST);		 // Access Transmision request
		SET_BIT((Base + CAN_IF2CMSK_R), CAN_IF2CMSK_DATAA);		 // Access Data Bytes 0 to 3
		SET_BIT((Base + CAN_IF2CMSK_R), CAN_IF2CMSK_DATAB);		 // Access Data Bytes 4 to 7
		
		/*******************STANDARD FRAME*******************/
		if(Frame_Type == STANDARD_FRAME){
			
			CLEAR_BIT((Base + CAN_IF2ARB2_R), CAN_IF2ARB2_XTD);

			if(MsgObject->Flags & MSG_OBJ_USE_ID_FILTER){
			
				ACCESS_REG(Base + CAN_IF2MSK2_R) &= ~(0x1FFF);						   // First clear Mask2 Field
				ACCESS_REG(Base + CAN_IF2MSK2_R) |= ((MsgObject->Msg_ID_MSK) << 2); // Write the ID mask
				CLEAR_BIT(Base + CAN_IF2MSK2_R, CAN_IF2MSK2_MXTD);					 // Clear MXTD Bit
				SET_BIT(Base + CAN_IF2MSK2_R, CAN_IF2MSK2_MDIR);					   // The message direction DIR bit is used for acceptance filtering
			}
			
			ACCESS_REG(Base + CAN_IF2ARB2_R) &= ~(0x1FFF);								  // First clear ARB2 Field
			ACCESS_REG(Base + CAN_IF2ARB2_R) |= ((MsgObject->Msg_ID) << 2);	// Write the message ID
			
		}
		/*******************EXTENDED FRAME*******************/
		 else if (Frame_Type == EXTENDED_FRAME){
			
			SET_BIT((Base + CAN_IF2ARB2_R), CAN_IF2ARB2_XTD);
			
			if(MsgObject->Flags & MSG_OBJ_USE_ID_FILTER){
				
					ACCESS_REG(Base + CAN_IF2MSK1_R) &= ~(0xFFFF);						                // First clear Mask1 Field
					ACCESS_REG(Base + CAN_IF2MSK1_R) |= (MsgObject->Msg_ID_MSK & 0x0000FFFF);	// Write first part of ID_MSK
					
					ACCESS_REG(Base + CAN_IF2MSK2_R) &= ~(0x1FFF);						                // First clear Mask2 Field
					ACCESS_REG(Base + CAN_IF2MSK2_R) |= ((MsgObject->Msg_ID_MSK & 0x1FFF0000) >> 16);	// Write Second part of ID_MSK
					SET_BIT(Base + CAN_IF2MSK2_R, CAN_IF2MSK2_MXTD);						              // Set MXTD Bit
					CLEAR_BIT(Base + CAN_IF2MSK2_R, CAN_IF2MSK2_MDIR);					                // The message direction DIR bit is used for acceptance filtering
			}
			ACCESS_REG(Base + CAN_IF2ARB1_R) &= ~(0xFFFF);													      // First clear ARB1 Field
			ACCESS_REG(Base + CAN_IF2ARB1_R) |= (MsgObject->Msg_ID & 0x0000FFFF);		      // Write first part of ID
			
			ACCESS_REG(Base + CAN_IF2ARB2_R) &= ~(0x1FFF);													      // First clear ARB2 Field
			ACCESS_REG(Base + CAN_IF2ARB2_R) |= ((MsgObject->Msg_ID & 0x1FFF0000) >> 16);	// Write Second part of ID
			
		}
		 
			// Clear Direction bit to indicate Receive message object
			CLEAR_BIT((Base + CAN_IF2ARB2_R), CAN_IF2ARB2_DIR);
			
			if(MsgObject->Flags & MSG_OBJ_RX_INT_ENABLE){
					SET_BIT((Base + CAN_IF2MCTL_R), CAN_IF2MCTL_RXIE);		// enable interrupt on successful reception
			}
			
			
		// Set message Valid bit
		SET_BIT((Base + CAN_IF2ARB2_R), CAN_IF2ARB2_MSGVAL);
		
		// Check using FIFO or not
		if(MsgObject->Flags & MSG_OBJ_FIFO){
			CLEAR_BIT((Base + CAN_IF2MCTL_R), CAN_IF2MCTL_EOB);
		} else{
			SET_BIT((Base + CAN_IF2MCTL_R), CAN_IF2MCTL_EOB);
		}
		
		ACCESS_REG((Base + CAN_IF2CRQ_R)) = ObjID;										// Write Message number into MNUM field to start transfer
	
}

/*********************************************************************************************************/

void CANMessageGet (uint32_t Base, MsgObjID ObjID, tCANReadRXData *psMsgObject, bool_t bClrPendingInt){
		while (BIT_IS_SET ((Base + CAN_IF2CRQ_R) , CAN_IF2CRQ_BUSY));

		CLEAR_BIT((Base + CAN_IF2CMSK_R), CAN_IF2CMSK_WRNRD);		// clear WRNRD bit = 0
	  ACCESS_REG((Base + CAN_IF2CRQ_R)) = ObjID;							// Set the MNUM field to the Message object ID	
		while (BIT_IS_SET ((Base + CAN_IF2CRQ_R) , CAN_IF2CRQ_BUSY));
		// Reading data and store it into the message objsct data field
		psMsgObject->Msg_Data[0] = (ACCESS_REG((Base + CAN_IF2DA1_R)) & 0x00FF);
		psMsgObject->Msg_Data[1] = ((ACCESS_REG((Base + CAN_IF2DA1_R)) & 0xFF00) >> 8);
	
		psMsgObject->Msg_Data[2] = (ACCESS_REG((Base + CAN_IF2DA2_R)) & 0x00FF);
		psMsgObject->Msg_Data[3] = ((ACCESS_REG((Base + CAN_IF2DA2_R)) & 0xFF00) >> 8);
	
		psMsgObject->Msg_Data[4] = (ACCESS_REG((Base + CAN_IF2DB1_R)) & 0x00FF);
		psMsgObject->Msg_Data[5] = ((ACCESS_REG((Base + CAN_IF2DB1_R)) & 0xFF00) >> 8);
	
		psMsgObject->Msg_Data[6] = (ACCESS_REG((Base + CAN_IF2DB2_R)) & 0x00FF);
		psMsgObject->Msg_Data[7] = ((ACCESS_REG((Base + CAN_IF2DB2_R)) & 0xFF00) >> 8);
		// Message Length
		psMsgObject->Msg_Length = (uint8_t)(ACCESS_REG((Base + CAN_IF2MCTL_R)) & 0xF);
	
		if(bClrPendingInt == TRUE){
				CLEAR_BIT((Base + CAN_IF2MCTL_R), CAN_IF2MCTL_INTPND);
		}
		SET_BIT((Base + CAN_IF2CMSK_R), CAN_IF2CMSK_WRNRD);		 // Set WRNRD bit = 1

		ACCESS_REG((Base + CAN_IF2CRQ_R)) = ObjID;										// Write Message number into MNUM field to start transfer

}
	
/*********************************************************************************************************/

void CANMessageClear(uint32_t Base, uint32_t ObjID){

		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_WRNRD);		  // Set WRNRD bit = 1
		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_ARB);			  // Set ARB bit to enable access arbitration bits
		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_CONTROL);	  // Set Control bit to enable access control bits
		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_CLRINTPND); // Clear Interrupt Pending Bit
		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_DATAA);		  // Access Data Bytes 0 to 3
		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_DATAB);		  // Access Data Bytes 4 to 7
	
		
		ACCESS_REG(Base + CAN_IF1MSK1_R) &= ~(0xFFFF);					// Clear Mask1 Field	
		ACCESS_REG(Base + CAN_IF1MSK2_R) &= ~(0x1FFF);					// Clear Mask2 Field
		ACCESS_REG(Base + CAN_IF1ARB1_R) &= ~(0xFFFF);					// Clear ARB1 Field
		ACCESS_REG(Base + CAN_IF1ARB2_R) &= ~(0x1FFF);					// Clear ARB2 Field
	
		ACCESS_REG((Base + CAN_IF1MCTL_R)) &= ~(0xF);			  		// Clear DLC Field
		
		ACCESS_REG((Base + CAN_IF1DA1_R)) = 0;									// Clear Data Registers
		ACCESS_REG((Base + CAN_IF1DA2_R)) = 0;
		ACCESS_REG((Base + CAN_IF1DB1_R)) = 0;
		ACCESS_REG((Base + CAN_IF1DB2_R)) = 0;
	
		CLEAR_BIT((Base + CAN_IF1ARB2_R), CAN_IF1ARB2_MSGVAL);	// Clear message Valid bit
		ACCESS_REG((Base + CAN_IF1CRQ_R)) = ObjID;
		
}

/*********************************************************************************************************/

void CANRetrySet (uint32_t Base, bool_t bAutoRetry){
		
		if(bAutoRetry == TRUE){
				CLEAR_BIT((Base + CAN_CTL_R), CAN_CTL_DAR);		// Enable Retransmission mode
		} else if(bAutoRetry == FALSE){
				SET_BIT((Base + CAN_CTL_R), CAN_CTL_DAR);			// Disable Retransmission mode
		}
			
}

/*********************************************************************************************************/

bool_t CANRetryGet (uint32_t Base){
	
		bool_t Return_Val = BIT_IS_CLEAR((Base + CAN_CTL_R), CAN_CTL_DAR);
		return Return_Val;
	
}

/*********************************************************************************************************/

uint32_t CANStatusGet (uint32_t Base, tCANStsReg eStatusReg){
		
		uint32_t Return_Val;
		if(eStatusReg == CAN_STS_CONTROL){
			
				Return_Val = (ACCESS_REG(Base + CAN_STS_R) & 0xFF);
			
		} else if(eStatusReg == CAN_STS_TXREQUEST){
			
				Return_Val = (ACCESS_REG(Base + CAN_TXRQ1_R) & 0xFFFF) | ((ACCESS_REG(Base + CAN_TXRQ2_R) & 0xFFFF) << 16);
			
		} else if(eStatusReg == CAN_STS_NEWDAT){
			
				Return_Val = (ACCESS_REG(Base + CAN_NWDA1_R) & 0xFFFF) | ((ACCESS_REG(Base + CAN_NWDA2_R) & 0xFFFF) << 16);
			
		} else if(eStatusReg == CAN_STS_MSGVAL){
			
				Return_Val = (ACCESS_REG(Base + CAN_MSG1VAL_R) & 0xFFFF) | ((ACCESS_REG(Base + CAN_MSG2VAL_R) & 0xFFFF) << 16);
			
		}
	
		return Return_Val;

}


void CAN_Write(uint32_t Base, uint8_t ObjID, str_TransmitMessageInfo *MsgObject){
		
	while (BIT_IS_SET ((Base + CAN_IF1CRQ_R) , CAN_IF1CRQ_BUSY))
	{}

	CLEAR_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_WRNRD);	
	ACCESS_REG((Base + CAN_IF1CRQ_R)) = ObjID;
	while (BIT_IS_SET ((Base + CAN_IF1CRQ_R) , CAN_IF1CRQ_BUSY))
   {}
	
		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_WRNRD);		 // Set WRNRD bit = 1
		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_DATAA);		 // Access Data Bytes 0 to 3
		SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_DATAB);		 // Access Data Bytes 4 to 7
	  SET_BIT((Base + CAN_IF1CMSK_R), CAN_IF1CMSK_CONTROL);
	
		ACCESS_REG((Base + CAN_IF1MCTL_R)) &= ~(0xF);			            //First clear DLC Field
		ACCESS_REG((Base + CAN_IF1MCTL_R)) |= MsgObject->Msg_Length;	// Write the message length code

		// Data
		ACCESS_REG((Base + CAN_IF1DA1_R)) = (MsgObject -> Msg_Data[0]);
		ACCESS_REG((Base + CAN_IF1DA1_R)) |= ((MsgObject -> Msg_Data[1]) << 8);

		ACCESS_REG((Base + CAN_IF1DA2_R)) = (MsgObject -> Msg_Data[2]);
		ACCESS_REG((Base + CAN_IF1DA2_R)) |= ((MsgObject -> Msg_Data[3]) << 8);

		ACCESS_REG((Base + CAN_IF1DB1_R)) = (MsgObject -> Msg_Data[4]);
		ACCESS_REG((Base + CAN_IF1DB1_R)) |= ((MsgObject -> Msg_Data[5]) << 8);

		ACCESS_REG((Base + CAN_IF1DB2_R)) = (MsgObject -> Msg_Data[6]);
		ACCESS_REG((Base + CAN_IF1DB2_R)) |= ((MsgObject -> Msg_Data[7]) << 8);
		
		// Request transmission of this message object
		ACCESS_REG((CAN0_BASE + CAN_IF1MCTL_R)) |= (CAN_IF1MCTL_TXRQST | CAN_IF1MCTL_NEWDAT);

		// Write Message number into MNUM field to start transfer
		ACCESS_REG((Base + CAN_IF1CRQ_R)) = ObjID;				

}

uint8_t TransmitOk(uint8_t MsgObjID,uint32_t BaseAddress)
{
  uint32_t Read_TXRQ_register = 0 ;
	uint8_t  Read_STS_register  = 0 ;
	
	/*Read Status register to check TXOK transmitted message successfully*/
	 Read_STS_register =(uint8_t) ACCESS_REG(BaseAddress + CAN_STS_R) & CAN_STS_TXOK;

	/*The CANTXRQ1 holds the first 16 messages*/
	if(MsgObjID<= 16 )
	{
			Read_TXRQ_register = ACCESS_REG(BaseAddress + CAN_TXRQ1_R) ;
	}
	/*The CANTXRQ2 holds the last 16 messages*/
	else if(MsgObjID<=32)
	{
			Read_TXRQ_register = ACCESS_REG(BaseAddress + CAN_TXRQ1_R)<<16 ;
	}
	else
	{
			/**MISRA Rule**/
	}
	/*If TXOK and message request has been cleared and Call Tx_Confirmation*/
   if(!(Read_TXRQ_register &(1<<(MsgObjID-1)))&& Read_STS_register )
	 {
			/*Clear TxOk flag*/
			ACCESS_REG(BaseAddress + CAN_STS_R) &=~ CAN_STS_TXOK;
			/*Return 1 as message successfully transmitted*/
			 return 1 ;
	 }
	 else
	 {
		 /*Message is not transmitted ok*/ 
			return 0;
	 }
}

uint8_t ReceiveOk(uint8_t MsgObjID,uint32_t BaseAddress)
{
	uint32_t Read_NWDAT_register = 0 ;
	uint8_t  Read_STS_register   = 0 ;
	
	/*Read Status register to check RXOK received message successfully*/
	 Read_STS_register =(uint8_t) ACCESS_REG(BaseAddress + CAN_STS_R) & CAN_STS_RXOK;

	/*The CANTXRQ1 holds the first 16 messages*/
	if(MsgObjID<= 16 )
	{
			Read_NWDAT_register = ACCESS_REG(BaseAddress + CAN_NWDA1_R) ;
	}
	/*The CANTXRQ2 holds the last 16 messages*/
	else if(MsgObjID<=32)
	{
			Read_NWDAT_register = ACCESS_REG(BaseAddress + CAN_NWDA2_R)<<16 ;
	}
	else
	{
			/**MISRA Rule**/
	}
   if( (Read_NWDAT_register &(1<<(MsgObjID-1)))&& Read_STS_register )
	 {
		 /*Clear TxOk flag*/
			ACCESS_REG(BaseAddress + CAN_STS_R) &=~ CAN_STS_RXOK;
			/*Return 1 as message successfully Received*/
			 return 1 ;
	 }
	 else
	 {
		 /*Message is not Received ok*/ 
			return 0;
	 }
}
