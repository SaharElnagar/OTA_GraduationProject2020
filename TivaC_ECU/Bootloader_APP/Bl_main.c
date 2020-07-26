/*******************************************************************************
**                                                                            **
**  FILENAME     : Bl_main.c                                                  **
**                                                                            **
**  DATE         : 2020-3-12                                                  **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Sahar Elnagar                                              **
**  																																					**
**  Description  : bootloader application supports 2 App sections with 				**
**								 cancel update feature .																		**													
*******************************************************************************/


/*****************************************************************************************/
/*                                   Include  headres               				             */
/*****************************************************************************************/
#include "uart.h"
#include "sysctl.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_gpio.h"
#include "inc/hw_flash.h"
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ssi.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "bl_config.h"
#include "bl_uart.h"
#include "Packet.h"
#include "bl_flash.h"
#include "eeprom.h"
#include "PLL.h"
#include "GPIO.h"
#include "MapComDevice.h"


/*****************************************************************************************/
/*                                   Linker sections 				                             */
/*****************************************************************************************/
uint8_t Packet_Buffer[FLASH_PAGE_SIZE] __attribute__((section ("FLASH_MIRROR")));

/*****************************************************************************************/
/*                                   Local Macro Definitions                             */
/*****************************************************************************************/
#define UPDATE_REQ_VAL 					0xAAAA
#define PAGE_SIZE 0x400 
#define WORD_SIZE	4
#define DEBUG__

/*****************************************************************************************/
/*             Local Type Definitions                                                    */
/*****************************************************************************************/
typedef struct
{
	uint16_t PacketsNum ;
  uint16_t ExtraBytes ;
	uint32_t CheckSum ;
}str_FrameInfo ;


/*****************************************************************************************/
/*                                Global Variables                                       */
/*****************************************************************************************/
uint16_t frSize ;
uint32_t pui32Data[1];
uint32_t pui32Read[1];
static uint16_t Cmd_Status;
static uint8_t  Frame_Size[8];     // little endian
static str_FrameInfo FrameInfo ;
uint32_t Flash_error;
uint16_t PageId=0 ;
/*****************************************************************************************/
/*                                   Local Function Declaration                          */
/*****************************************************************************************/
uint32_t Get_CurrentAppStartAddress(void);
uint32_t Get_AvailableAppSection(void) ;
void Set_CurrentAppsection(uint32_t CurrentAPPAdrs) ;
void __main(void){}

/*****************************************************************************************/
/*                                   extern functions from assembly                      */
/*****************************************************************************************/	
extern void StartApplication(uint32_t);
void EnableInterrupts(void);
	
/*****************************************************************************************/
/*                                   Global Function Definition                          */
/*****************************************************************************************/

/****************************************************************************************/
//    Function Name           : INIT_EEPROM
//    Function Description    : Initialize the EEPROM module
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : Std_ReturnType
//    Requirement             : none
//    Notes                   :
/****************************************************************************************/	
void INIT_EEPROM(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
	EEPROMInit();	
}

/****************************************************************************************/
//    Function Name           : ConfigureDevice
//    Function Description    : Initializes all the modules needed before update starts
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
//    Requirement             : none
//    Notes                   :
/****************************************************************************************/
void ConfigureDevice(void)
{
	
	/*Set the system clock 80 MHz*/ 
	PLL_Init();
	/*Delay to make sure clock starts correctly*/
	SysCtlDelay(20000000);
	/*Initialize uart if update device uses uart*/
	#ifdef UART_UPDATE
		UART_Init();
	#else
		  CanIf_Init();
	#endif
	/*Initialize CAN if update device uses CAN*/


		/*Initialize PortF*/
		PortF_Init();
}

/****************************************************************************************/
//    Function Name           : StorePacketInAppSec
//    Function Description    : Store 1024 bytes in the required address
//    Parameter in            : PageId , AppStartAddress 
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
//    Requirement             : none
//    Notes                   :
/****************************************************************************************/
void StorePacketInAppSec(uint16_t PageId,uint32_t AppStartAddress)
{
	/**erase this page**/
	BLInternalFlashErase(AppStartAddress+PageId);
	
	/**Write Page**/
	BLInternalFlashProgram(AppStartAddress+PageId,(uint8_t*) Packet_Buffer,PAGE_SIZE)	;
}

/****************************************************************************************/
//    Function Name           : Flush_PacketBuffer
//    Function Description    : Clears the temporary packet buffer
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
//    Requirement             : none
//    Notes                   :
/****************************************************************************************/
void Flush_PacketBuffer(void)
{
  uint16_t count;
	for(count=0;count<FLASH_PAGE_SIZE;count++)
	{
			Packet_Buffer[count]=0;
	}
}

/****************************************************************************************/
//    Function Name           : Read_Update_notification_Request
//    Function Description    : Check if new update is requested
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
//    Requirement             : none
//    Notes                   :
/****************************************************************************************/
#ifdef UART_ENABLE_UPDATE
void Read_Update_notification_Request(void)
{	
    write_update_notification_Request(&Cmd_Status);
}

#endif
/****************************************************************************************/
//    Function Name           : Bl_main
//    Function Description    : Performs updating main states
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
//    Requirement             : none
//    Notes                   :
/****************************************************************************************/
int Bl_main(void)
{
	  /*variable to save the required app section to save update in*/
	  static uint32_t NewAppAddress = 0 ;
	
	  /*Initialize required peripherals*/
	  ConfigureDevice();
	 // EnableInterrupts();
    while(1)
		{
/**************************UART_UPDATE_MAIN_STATES*******************/		
			switch(Cmd_Status) 
			{
/****************case: NEW_UPDATE_REQUEST***************/ 
					
				case REQUEST_TO_UPDATE :
						/*This case is only entered when Uart interrupt ocuured 
							with update request
						 */
						Cmd_Status = SEND_FREAME_INFO;
						/*disable ISR*/
						//HWREG(UARTx_BASE+UART_O_IM)&=~UART_INT_RX;
				break;
/****************case: SEND_FRAME_INFO_REQUEST***************/  
				
				case SEND_FREAME_INFO :         
						/*Send to request the expected frame information*/
						CanTransmitBlocking((uint8_t*)&Cmd_Status,CMD_SIZE); 
						Cmd_Status =	RECEIVE_FRAME_INFO;	 
				break;
/****************case: RECEIVE_FRAME_INFO***************/  		 		
				
				case RECEIVE_FRAME_INFO :		
						CanReceiveBlocking(Frame_Size,8);
						FrameInfo.PacketsNum = Frame_Size[0] + (Frame_Size[1]<<8) ;
						FrameInfo.ExtraBytes = Frame_Size[2] + (Frame_Size[3]<<8) ;
						*((uint64_t*)Frame_Size) = 0 ; 
						Cmd_Status=READY_TO_RECEIVE_UPDATE;
				break;  
/****************case: READY_TO_RECEIVE_UPDATE***************/  		  
				case READY_TO_RECEIVE_UPDATE :          
						CanTransmitBlocking((uint8_t*)&Cmd_Status,CMD_SIZE); 
						Cmd_Status =	RECEIVE_PACKET;	 	
						NewAppAddress = Get_AvailableAppSection();	
				break;
/****************case: RECEIVE_PACKET***************/  		 
				case RECEIVE_PACKET :
						/*Clear Temp buffer */
						Flush_PacketBuffer();
						/*Check if it's a complete packet or less*/
						if(FrameInfo.PacketsNum>0)
						{
								CanReceiveBlocking(Packet_Buffer ,PAGE_SIZE);
								FrameInfo.PacketsNum-=1 ;
								StorePacketInAppSec(PageId,NewAppAddress);
								Cmd_Status = SEND_NEW_PACKET ;
						}
						else
						{
							CanReceiveBlocking(Packet_Buffer , FrameInfo.ExtraBytes);	
							FrameInfo.ExtraBytes = 0; 
							StorePacketInAppSec(PageId,NewAppAddress);
							Cmd_Status = END_OF_UPDATE ;
						}					 			
						PageId+=0x400;
					break;
/****************case: SEND_NEW_FRAME***************/ 								 
					case SEND_NEW_PACKET :
							CanTransmitBlocking((uint8_t*)&Cmd_Status,CMD_SIZE); 
							Cmd_Status= RECEIVE_PACKET ;
					break ;
/****************case: CHECK_CANCEL_UPDATE***************/ 				
					case CHECK_CANCEL_UPDATE :
						/*Check if cancelling update is requested*/
						 CanReceiveBlocking((uint8_t*)&Cmd_Status,CMD_SIZE);
						 if(Cmd_Status == CANCEL_UPDATE_REQUEST)
						 {
								/*Cancel request saved in eeprom*/
								pui32Data[0]= 0 ;
								EEPROMProgram(pui32Data, EEPROM_UPDATE_REQUEST_ADDRESS, sizeof(pui32Data));
								StartApplication(Get_CurrentAppStartAddress());
						 }
						 else
						 {
								Cmd_Status = SEND_NEW_PACKET ;
						 }
					break;

/****************case: END_OF_UPDATE***************/ 							
					case END_OF_UPDATE :
						    Cmd_Status = UPDATE_SUCCESS ;
							/*Transmit update success to Gateway*/
					     	CanTransmitBlocking((uint8_t*)&Cmd_Status,CMD_SIZE); 
					   /*clear Update request */
							pui32Data[0]= 0 ;
							EEPROMProgram(pui32Data, EEPROM_UPDATE_REQUEST_ADDRESS, sizeof(pui32Data));
							Set_CurrentAppsection(NewAppAddress);
							StartApplication(NewAppAddress);		 
					break;	
					default :
							 Cmd_Status = REQUEST_TO_UPDATE ;
					break;
		}
	}	
}

uint32_t Get_AvailableAppSection(void)
{
	uint32_t AvailableAdrs ; 
	/*Get available App section*/
	EEPROMRead(&AvailableAdrs, EEPROM_AVAILABLE_APP_SECTION_ADDRESS, WORD_SIZE);	
	return AvailableAdrs;
}

void Set_CurrentAppsection(uint32_t CurrentAPPAdrs)
{
	uint32_t AvailableAppSection ;
		/*Set current APP section*/
		EEPROMProgram(&CurrentAPPAdrs, EEPROM_CURRENT_APP_START_ADDRESS, WORD_SIZE);
	if(CurrentAPPAdrs == FIRST_APP_SECTION_ADDRESS)
	{
		AvailableAppSection = SECOND_APP_SECTION_ADDRESS ;
	}
	else
	{
		AvailableAppSection = FIRST_APP_SECTION_ADDRESS ;
	}
	/*Set Available app section*/
	EEPROMProgram(&AvailableAppSection, EEPROM_AVAILABLE_APP_SECTION_ADDRESS, WORD_SIZE);
}
	
uint32_t Get_CurrentAppStartAddress(void)
{
	uint32_t AvailableAdrs ; 
	/*Get current App section*/
	EEPROMRead(&AvailableAdrs, EEPROM_CURRENT_APP_START_ADDRESS, WORD_SIZE);	
	return AvailableAdrs;
}

void CheckForceUpdate(void)
{
		/*Read EepRom if it doesn't have Requested value to update
	    Do not updatee*/
	/* EEPROMProgram-Function is used to write data into EEPROM*/
	 EEPROMRead(pui32Read, EEPROM_UPDATE_REQUEST_ADDRESS, sizeof(pui32Read));
	 if(pui32Read[0] ==0 )
	 {
				StartApplication(Get_CurrentAppStartAddress());
	 }
}

