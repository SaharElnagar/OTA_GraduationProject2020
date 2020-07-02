
#include "CANIf.h"
#include "hw_can.h"
#include "can_driver.h"
#include "Can_Cfg.h"


extern tCANConfigTXMsgObj CANConfigTXMsgObj[CAN_TX_MESSAGES_NUM] ;
extern tCANConfigRXMsgObj CANConfigRXMsgObj[CAN_RX_MESSAGES_NUM] ;

/*****************************************************************************************/
/*                           Local functions Definitions                                 */
/*****************************************************************************************/
static void
_CANDataRegRead( uint8_t *pui8Data, uint32 *pui32Register, uint8_t ui32Size) ;

/*****************************************************************************************/
/*                           Functios Declaration                                        */
/*****************************************************************************************/
void Delay(uint32_t val);

/****************************************************************************************/
/*    Function Name           : Can_Init                                                */
/*    Function Description    : Init Can module																				  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Notes                   :                                                         */
/****************************************************************************************/
void Can_Init(void) 
{ 
    uint32_t count = 0 ;

    /* Init GPIO PORT*/
	GPIO_Init(Port_B);
	/* Init CAN controller with non loopBack mode*/
	CANInit(CANx_BASE, Real_Mode);

	/* Set BitRate*/
	tCANBitClkParms BitTime ={10,2,13,2}  ; /*500Kbps 80MHz*/
	tCANBitClkParms *PBitTime = &BitTime ;		
	CANBitTimingSet (CANx_BASE, PBitTime) ;

	/*Initialize TX message objects*/
	for(count =0 ;count <CAN_TX_MESSAGES_NUM; count++)
	{
	    CANTransmitMessageSet(CANx_BASE ,count+1 ,&CANConfigTXMsgObj[count]);
	}

    /*Initialize TX message objects*/
    for(count =0 ;count <CAN_RX_MESSAGES_NUM; count++)
    {
        CANReceiveMessageSet(CANx_BASE ,count+1 ,&CANConfigRXMsgObj[count]);
    }

	/*	Start Bus */
	CANEnable(CANx_BASE);
}

/****************************************************************************************/
/*    Function Name           : CanReceiveBlocking_Function                             */
/*    Function Description    : Receives number of bytes (size) in a blocking manner    */
/*                              and save them in buffer referenced by DataPtr pointer   */
/*    Parameter in            : uint16_t size                                           */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : uint8_t* DataPtr                                        */
/*    Return value            : none                                                    */
/*    Notes                   :                                                         */
/****************************************************************************************/
void CanReceiveBlocking_Function(uint16_t size, uint8_t*DataPtr,uint8_t MessageNum)
{
	uint8_t ReceivedLength   = 0 ;	
	/*Check parameters if reporting is availbale*/
	
	/********************************************/
	 while(size-=ReceivedLength)
	 {
		/*
		 Check New data received
		 */
		 if(ReceiveOk(MessageNum,CANx_BASE))
		 {	 
				/*
				 Set HW registers to read a messages data , length and ID
				*/
				HWREG(CANx_BASE + CAN_O_IF2CMSK) = (CAN_IF2CMSK_DATAA | CAN_IF1CMSK_DATAB |\
																						CAN_IF2CMSK_CONTROL | CAN_IF1CMSK_MASK|\
																						CAN_IF2CMSK_ARB);			 
				HWREG(CANx_BASE + CAN_O_IF2CRQ)   =  MessageNum ;
			 
			 /*Read Received data Length*/
				 ReceivedLength = HWREG(CANx_BASE + CAN_O_IF2MCTL) & CAN_IF2MCTL_DLC_M ;
			 
			 /*Read Received Data*/
				_CANDataRegRead(DataPtr,(uint32*)( CANx_BASE+CAN_O_IF2DA1), ReceivedLength) ;
			 
			 /*Clear Receive Flag*/
			  HWREG(CANx_BASE + CAN_O_IF2CMSK) = CAN_IF1CMSK_NEWDAT;
				HWREG(CANx_BASE + CAN_O_STS) &=~CAN_STS_RXOK;               \
				HWREG(CANx_BASE + CAN_O_IF2CRQ)   = MessageNum ;
		}
	  else
	  {
			/*Reset Received data length*/
			  ReceivedLength = 0;
	  }
	}		 
}


/****************************************************************************************/
/*    Function Name           : CanTransmitBlocking_Function                            */
/*    Function Description    : Receives number of bytes (size) in a blocking manner    */
/*                              and save them in buffer referenced by DataPtr pointer   */
/*    Parameter in            : uint16_t size ,DataPtr , DataLength                     */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Notes                   :                                                         */
/****************************************************************************************/
void CanTransmitBlocking_Function(uint16_t size,const uint8_t*DataPtr ,uint8_t MessageNum)
{
	  /*flag used to indicate if we transmitted for the first time*/
	  uint8_t FirstTransmitFlag = 0 ;
	  /*Function CanTransmitBlocking_Function can have any number of bytes to transmit
	    but function CAN_Write can only send 8 bytes , so we have to change
      the start byte pointed by the DataPtr every time we call CAN_Write	
	*/
	  uint16_t StartByteNum = 0;
		while(size)
		{
			/*Check if transmit ok to transmit again or if it's the first transmit*/
			if(TransmitOk(MessageNum,CANx_BASE) ||FirstTransmitFlag == 0)
			{
			  Delay(100000);
				if(size>= MAX_DATA_LENGTH)
				{
					/*Send 8 bytes of data*/
					str_TransmitMessageInfo MessageInfo = {(uint8_t*)(DataPtr+StartByteNum),MAX_DATA_LENGTH} ;
					CAN_Write(CANx_BASE,MessageNum,&MessageInfo);
					/*Set the first time tranmit flag*/
				   FirstTransmitFlag = 1;
					/*Decrement data size by 8*/
					size-= MAX_DATA_LENGTH ;
					
					/*Increment StartByteNum by 8*/
					StartByteNum+= MAX_DATA_LENGTH ;
				}
				else
				{
					/*Send remaining bytes of data*/
					str_TransmitMessageInfo MessageInfo = {(uint8_t*)(DataPtr+StartByteNum),size} ;
					CAN_Write(CANx_BASE,MessageNum,&MessageInfo);
					
					/*make data size = 0*/
					size = 0 ;
				}
			}				
		}
}

static void
_CANDataRegRead( uint8_t *pui8Data, uint32 *pui32Register, uint8_t ui32Size)
{
    uint32_t ui32Idx, ui32Value;

    //
    // Loop always copies 1 or 2 bytes per iteration.
    //
    for(ui32Idx = 0; ui32Idx < ui32Size; )
    {
        //
        // Read out the data 16 bits at a time since this is how the registers
        // are aligned in memory.
        //
        ui32Value = HWREG(pui32Register++);

        //
        // Store the first byte.
        //
        pui8Data[ui32Idx++] = (uint8_t)ui32Value;

        //
        // Only read the second byte if needed.
        //
        if(ui32Idx < ui32Size)
        {
            pui8Data[ui32Idx++] = (uint8_t)(ui32Value >> 8);
        }
    }
}



