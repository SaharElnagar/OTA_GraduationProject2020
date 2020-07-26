
#include "bl_CAN.h"
#include "hw_can.h"
#include "can_driver.h"
#include  "Packet.h"
#include "bl_config.h"

#ifdef CAN_UPDATE


/*****************************************************************************************/
/*                           Local functions Definitions                                 */
/*****************************************************************************************/
static void
_CANDataRegRead( uint8_t *pui8Data, uint32 *pui32Register, uint8_t ui32Size) ;

/*****************************************************************************************/
/*                           Functios Declaration                                        */
/*****************************************************************************************/


/****************************************************************************************/
/*    Function Name           : Can_Init                                                */
/*    Function Description    : Init Can module																				  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Notes                   :                                                         */
/****************************************************************************************/
void CanIf_Init(void) 
{ 

	/* Init GPIO PORT*/
	GPIO_Init(Port_B);
	/* Init CAN controller with non loopBack mode*/
	CANInit(CANx_BASE, Real_Mode);

	/* Set BitRate*/
	tCANBitClkParms BitTime ={10,2,13,2}  ;
	tCANBitClkParms *PBitTime = &BitTime ;		
	CANBitTimingSet (CANx_BASE, PBitTime) ;	// 80 MHx clock , 500Kbps bit rate

	/*	Init Transmit message object*/
	tCANConfigTXMsgObj msgT = {0x5,0};
	CANTransmitMessageSet(CANx_BASE,HWOBJ_TRANSMIT_NUM, &msgT);

	/*	Init Transmit message object*/
	tCANConfigRXMsgObj msgRR0 = {0x2,0x7FF, MSG_OBJ_USE_ID_FILTER};		
	CANReceiveMessageSet(CANx_BASE, HWOBJ_RECEIVE_NUM, &msgRR0);

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
void CanReceiveBlocking(uint8_t*DataPtr, uint16_t size)
{
	uint8_t ReceivedLength   = 0 ;	
	uint16_t StartByteNum = 0;
	/*Check parameters if reporting is availbale*/
	
	/********************************************/
	 while(size-=ReceivedLength)
	 {
		/*
		 Check New data received
		 */
		 if(ReceiveOk(HWOBJ_RECEIVE_NUM,CANx_BASE))
		 {	 
				/*
				 Set HW registers to read a messages data , length and ID
				*/
				HWREG(CANx_BASE + CAN_O_IF2CMSK) = (CAN_IF2CMSK_DATAA | CAN_IF1CMSK_DATAB |\
																						CAN_IF2CMSK_CONTROL | CAN_IF1CMSK_MASK|\
																						CAN_IF2CMSK_ARB);			 
				HWREG(CANx_BASE + CAN_O_IF2CRQ)   =  HWOBJ_RECEIVE_NUM ;
			 
			 /*Read Received data Length*/
				 ReceivedLength = HWREG(CANx_BASE + CAN_O_IF2MCTL) & CAN_IF2MCTL_DLC_M ;
			 
			 /*Read Received Data*/
				_CANDataRegRead(DataPtr+StartByteNum,(uint32*)( CANx_BASE+CAN_O_IF2DA1), ReceivedLength) ;
			 
			 /*Clear Receive Flag*/
			  HWREG(CANx_BASE + CAN_O_IF2CMSK) = CAN_IF1CMSK_NEWDAT;
				HWREG(CANx_BASE + CAN_O_STS) &=~CAN_STS_RXOK;               \
				HWREG(CANx_BASE + CAN_O_IF2CRQ)   = HWOBJ_RECEIVE_NUM ;
			  StartByteNum += ReceivedLength;
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
void CanTransmitBlocking(const uint8_t*DataPtr ,uint16_t size)
{
	  /*flag used to indicate if we transmitted for the first time*/
	  uint8_t FirstTransmitFlag = 0 ;
	  /*Function CanTransmitBlocking_Function can have any number of bytes to transmit
	    but function CAN_Write can only send 8 bytes , so we have to change
      the start byte pointed by the DataPtr every time we call CAN_Write	
	*/
	  uint8_t StartByteNum = 0;
		while(size)
		{
			/*Check if transmit ok to transmit again or if it's the first transmit*/
			if(TransmitOk(HWOBJ_TRANSMIT_NUM,CANx_BASE) ||FirstTransmitFlag == 0)
			{				
				if(size>= MAX_DATA_LENGTH)
				{
					//	Delay(10000);
					/*Send 8 bytes of data*/
					str_TransmitMessageInfo MessageInfo = {(uint8_t*)(DataPtr+StartByteNum),MAX_DATA_LENGTH} ;
					CAN_Write(CANx_BASE,HWOBJ_TRANSMIT_NUM,&MessageInfo);
					/*Set the first time tranmit flag*/
				   FirstTransmitFlag = 1;
					/*Decrement data size by 8*/
					size-= MAX_DATA_LENGTH ;
					
					/*Increment StartByteNum by 8*/
					StartByteNum+= MAX_DATA_LENGTH ;
				}
				else
				{
					//Delay(10000);
				 /*Send remaining bytes of data*/
					str_TransmitMessageInfo MessageInfo = {(uint8_t*)(DataPtr+StartByteNum),size} ;
					CAN_Write(CANx_BASE,HWOBJ_TRANSMIT_NUM,&MessageInfo);					
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

#endif

