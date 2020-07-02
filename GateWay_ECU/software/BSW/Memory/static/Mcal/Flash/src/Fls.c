/*******************************************************************************
**                                                                            **
**  FILENAME     : Fls.c                                                      **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2019-12-1                                                  **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Yomna Mokhtar                                              **
**                                                                            **
**                                                                            **
*******************************************************************************/






/*****************************************************************************************/
/*                                   Include Common headres                              */
/*****************************************************************************************/
#include "hw_types.h"
#include "hw_flash.h"
#include "hw_nvic.h"

/*****************************************************************************************/
/*                                   Include Other  headres                              */
/*****************************************************************************************/
#include "Det.h"

/*****************************************************************************************/
/*                                   Include Component headres                           */
/*****************************************************************************************/
#include "Fls.h"

/*****************************************************************************************/
/*                                   Local Macro Definition                              */
/*****************************************************************************************/
#define NVIC_EN0_INT29      0x20000000  // Interrupt 29 enable "Flash control interrupt"
#define SECTOR_WORDS		255
#define ERASED_BYTE			0xFF
#define ERASED_WORD			0xFFFFFFFF


#define MAINFUNCTION_SYNCHRONOUS_MODE            STD_OFF

/*Internal states to handle asynchronous main functions*/
#define FLS_WRITE_WORDS         0x00
#define FLS_END_JOB             0x01
#define FLS_FAILED_JOB          0x02
#define FLS_READ_WORD           0x03
#define FLS_ERASE_SECTOR        0x04

/*****************************************************************************************/
/*                            Local Variables' Definition                                  */
/*****************************************************************************************/
static Fls_ConfigType* Fls_ConfigPtr = NULL;

static Fls_AddressType FlsJob_FlsAddress;
static Fls_LengthType FlsJob_Length = 1;
static uint8* FlsJob_DataBufferAddPtr;

//static uint16 i = 0;
//static uint8 j = 0;
//static boolean End_Job_Flag = 0;


/*****************************************************************************************/
/*                          Module Status variables                                      */
/*****************************************************************************************/
static MemIf_StatusType Module_Status = MEMIF_UNINIT;
static MemIf_JobResultType Job_Result               ;
static JOB_PENDING_TYPE Fls_PENDING_JOB             ;
static uint8 InternalState = 0;

/*****************************************************************************************/
/*                                   Local Function Declaration                          */
/*****************************************************************************************/

static void Fls_Write_AC(void);
static void Fls_Erase_AC(void);
static void Fls_Read_AC(void);
static void Fls_Compare_AC(void);
#if (FlsBlankCheckApi == STD_ON)
		static void Fls_BlankCheck_AC(void);
#endif

#if (FlsUseInterrupts == STD_ON)
	static void FlashIntEnable(uint32 ui32IntFlags);
	void FLASH_Handler(void);
#endif



/*****************************************************************************************
* Function Description : Performs processing of write job Access code.									 *
*												 Called by Fls_MainFunction when a write job is pending          *
******************************************************************************************/
static void Fls_Write_AC(void){
#if(MAINFUNCTION_SYNCHRONOUS_MODE == STD_ON)
		#if (FlsWriteVerificationEnabled == STD_ON)
				// save job parameters before processing the write job
				Fls_AddressType Compare_Address = FlsJob_FlsAddress;
				Fls_LengthType Compare_length = FlsJob_Length;
				uint8* Compare_data = FlsJob_DataBufferAddPtr;
		#endif
				
        /*Get the boundries of the required place to write data in*/
		Fls_AddressType Start_Sector_Add = FlsJob_FlsAddress;
		Fls_AddressType End_Sector_Add = FlsJob_FlsAddress + FlsJob_Length;
	
		while(Start_Sector_Add % FlsSectorSize != 0){           /*Get the statr address of that sector*/
				Start_Sector_Add = Start_Sector_Add - FlsPageSize;
		}
		while(End_Sector_Add % FlsSectorSize != 0){             /*We could have simply added the sector size to the start address*/
				End_Sector_Add = End_Sector_Add + FlsPageSize;
		}
		
		// Some local variables
		Fls_AddressType Current_Address = Start_Sector_Add;
		uint16 Word_Num = ((FlsJob_FlsAddress) % FlsSectorSize)/4;
		uint16 Sectors_Num = (End_Sector_Add - Start_Sector_Add + 1) / FlsSectorSize;
		uint32 Sector_Buffer[SECTOR_WORDS + 1];
		uint32* Ptr_Buffer = Sector_Buffer;
		
		while(Sectors_Num--){
				// update Word_Num for the new sector
				if(Current_Address != Start_Sector_Add){
						Word_Num = 0;
				}
				
				// loop over the sector words and read them into buffer
				for(i = 0; i <= SECTOR_WORDS; i++){
						Sector_Buffer[i] = (*(uint32 *)Current_Address);
					  Current_Address += FlsPageSize;
				}
				
				// return the current sector address to its value again
				Current_Address -= FlsSectorSize;
				
				// Erase the sector
				HWREG(FLASH_FMA) = Current_Address;
				HWREG(FLASH_FMC) = FLASH_FMC_WRKEY | FLASH_FMC_ERASE;
				
				// Wait until the block has been erased
				while(HWREG(FLASH_FMC) & FLASH_FMC_ERASE)
				{
				}
				
				
				#if (FlsEraseVerificationEnabled == STD_ON)

						// Copy job parameters to local variables
						Fls_AddressType Address = Current_Address;
						Fls_LengthType length = FlsSectorSize;
						
						uint8 Byte_Buffer;
						
						while(length){
							
							Byte_Buffer = (*(uint8 *) Address++);
							length--;
							
							if(Byte_Buffer != ERASED_BYTE){
									Job_Result = MEMIF_JOB_FAILED;
									 Det_ReportRuntimeError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_MAIN_API_ID, FLS_E_VERIFY_ERASE_FAILED);
							}
						}
		
				#endif
				
				
				// Update the internal buffer
				while( FlsJob_Length != 0){
						Sector_Buffer[Word_Num] = 0;
						for(j = 0; j < 4; j++){
								Sector_Buffer[Word_Num] += ((*FlsJob_DataBufferAddPtr++) << (j * 8));
						}
						
						Word_Num++;
						FlsJob_Length -= FlsPageSize;
						if(Word_Num > SECTOR_WORDS){
								break;
						}
				}
				
				Ptr_Buffer = Sector_Buffer;
				
				// Writing the updated sector into Flash
				for(i = 0; i < 8; i++){
					if(i == 7 && Sectors_Num == 0){
							End_Job_Flag = 1;
					}
					// Set the address of this block of words.
					//~(0x7F) --> first 7 bits must be 0 so that the address is divisible by 128 (32 word * 4 bytes)
					HWREG(FLASH_FMA) = Current_Address & ~(0x7F);
					
					// Loop over the words in this 32-word block.
					while(((Current_Address & 0x7c) || (HWREG(FLASH_FWBVAL) == 0)))
					{
						// Write this word into the write buffer.
						HWREG(FLASH_FWBN + (Current_Address & 0x7c)) = *Ptr_Buffer++;
						Current_Address += FlsPageSize;
						
					}
					// Program the contents of the write buffer into flash.
					HWREG(FLASH_FMC2) = FLASH_FMC2_WRKEY | FLASH_FMC2_WRBUF;
					
					/* 
					 *Wait until the write buffer has been programmed
					 *so that data don't be overwritten on each other
					 */
					while(HWREG(FLASH_FMC2) & FLASH_FMC2_WRBUF)
					{
					}
							
					
					#if (FlsUseInterrupts == STD_OFF)
						
						// Check if an access violation error occurred.
						if(HWREG(FLASH_FCRIS) & (FLASH_FCRIS_ARIS | FLASH_FCRIS_VOLTRIS |
																		 FLASH_FCRIS_INVDRIS | FLASH_FCRIS_PROGRIS))
						{
							 HWREG(FLASH_FCMISC) |= FLASH_FCMISC_AMISC | FLASH_FCMISC_VOLTMISC  | FLASH_FCMISC_INVDMISC | FLASH_FCMISC_PROGMISC;
							 Job_Result = MEMIF_JOB_FAILED;
							  Det_ReportTransientFault(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_MAIN_API_ID, FLS_E_WRITE_FAILED);
						}
					#endif
				}
		}
		
		
		#if (FlsWriteVerificationEnabled == STD_ON)
		
				uint8 Byte_buffer;
				
				while(Compare_length){
					
					Byte_buffer = (*(uint8 *) Compare_Address++);
					Compare_length--;
					
					if(Byte_buffer != *Compare_data++){
							Job_Result = MEMIF_JOB_FAILED;
							 Det_ReportRuntimeError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_MAIN_API_ID, FLS_E_VERIFY_WRITE_FAILED);
					}
				}
					
		#endif
		
		#if (FlsUseInterrupts == STD_OFF)
			#if (FlsJobEndNotificationEnable == STD_ON)
					
					if(Job_Result != MEMIF_JOB_FAILED && Fls_ConfigPtr->FlsJobEndNotification != NULL){
							Fls_ConfigPtr->FlsJobEndNotification();
					}
					
			#endif
		#endif
#else
static uint32 WordCounter = 0;
/*************************Asynchronous Mode********************/
    switch(InternalState)
    {
        case FLS_WRITE_WORDS :
            if(WordCounter < FlsJob_Length)
            {
                HWREG(FLASH_FMA) = FlsJob_FlsAddress + WordCounter;
                HWREG(FLASH_FMD) = *(uint32 *)(FlsJob_DataBufferAddPtr + WordCounter);
                HWREG(FLASH_FMC) = FLASH_FMC_WRKEY | FLASH_FMC_WRITE;

               //
               // Wait until the flash has been programmed.
               //
               while(HWREG(FLASH_FMC) & FLASH_FMC_WRITE)
               {
               }
               WordCounter+=4;
            }
            else
            {
                /*go to End job state*/
                InternalState = FLS_END_JOB ;
            }
        break;
        case FLS_END_JOB :
            /*Set end jobs flags*/
            WordCounter = 0;
            Job_Result = MEMIF_JOB_OK ;
            Module_Status = MEMIF_IDLE ;
            Fls_ConfigPtr->FlsJobEndNotification();
            Fls_PENDING_JOB = NO_JOB ;
        break;
    }
#endif
}



/*****************************************************************************************
* Function Description : Performs processing of erase job Access code.									 *
*												 Called by Fls_MainFunction when an erase job is pending         *
******************************************************************************************/
static void Fls_Erase_AC(void){
		
		#if (FlsEraseVerificationEnabled == STD_ON)

				// save job parameters before processing the Erase job
				Fls_AddressType EAddress = FlsJob_FlsAddress;
				Fls_LengthType Elength = FlsJob_Length;
				
		#endif
		
	switch (InternalState)
	{
        case FLS_ERASE_SECTOR:

           if( FlsJob_Length )
            {
                // Erase the block
                HWREG(FLASH_FMA) = FlsJob_FlsAddress;
                HWREG(FLASH_FMC) = FLASH_FMC_WRKEY | FLASH_FMC_ERASE;

                // Wait until the block has been erased
                while(HWREG(FLASH_FMC) & FLASH_FMC_ERASE)
                {
                }
                #if (FlsUseInterrupts == STD_OFF)
                  // Check if an access violation or erase error occurred
                   if(HWREG(FLASH_FCRIS) & (FLASH_FCRIS_ARIS | FLASH_FCRIS_VOLTRIS | FLASH_FCRIS_ERRIS))
                   {
                       HWREG(FLASH_FCMISC) |= FLASH_FCMISC_AMISC | FLASH_FCMISC_VOLTMISC  | FLASH_FCMISC_ERMISC;
                       Job_Result = MEMIF_JOB_FAILED;
                       Module_Status = MEMIF_IDLE ;
                       Fls_PENDING_JOB = NO_JOB ;
                       if( Fls_ConfigPtr->FlsJobErrorNotification != NULL)
                       {
                           Fls_ConfigPtr->FlsJobErrorNotification();
                       }
                   }
                #endif
                FlsJob_FlsAddress += FlsSectorSize;
                FlsJob_Length -= FlsSectorSize;
            }
           else
           {
               Job_Result = MEMIF_JOB_OK  ;
               InternalState = FLS_END_JOB ;
           }
        break;
        case FLS_END_JOB:
            Module_Status = MEMIF_IDLE ;
            Fls_PENDING_JOB = NO_JOB ;
            if(Job_Result != MEMIF_JOB_FAILED && Fls_ConfigPtr->FlsJobEndNotification != NULL)
            {
                Fls_ConfigPtr->FlsJobEndNotification();
            }
        break;
    }
			
		#if (FlsEraseVerificationEnabled == STD_ON)
		
				uint32 Word_Buffer;
					
				while(Elength){
					
						Word_Buffer = (*(uint32 *)EAddress);
						EAddress += FlsPageSize;
						Elength -= FlsPageSize;

						if(Word_Buffer != ERASED_WORD){
								Job_Result = MEMIF_JOB_FAILED;
								 Det_ReportRuntimeError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_MAIN_API_ID, FLS_E_VERIFY_ERASE_FAILED);
						}
				}
				
			#endif
				
}



/*****************************************************************************************
* Function Description : Performs processing of read job Access code.									   *
*												 Called by Fls_MainFunction when a read job is pending.          *
******************************************************************************************/
static void Fls_Read_AC(void)
{
    if(FlsJob_Length)
    {
        *FlsJob_DataBufferAddPtr++ = (*(uint8 *)FlsJob_FlsAddress++);
        FlsJob_Length--;

        // Check if an error occurred while read
        if(HWREG(FLASH_FCRIS) & (FLASH_FCRIS_ARIS | FLASH_FCRIS_VOLTRIS))
        {
                HWREG(FLASH_FCMISC) |= FLASH_FCMISC_AMISC | FLASH_FCMISC_VOLTMISC;
                Job_Result = MEMIF_JOB_FAILED;
                Module_Status = MEMIF_IDLE ;
                Fls_PENDING_JOB = NO_JOB ;
                Fls_ConfigPtr->FlsJobErrorNotification();
        }
    }
    if(FlsJob_Length == 0)
    {
        #if (FlsJobEndNotificationEnable == STD_ON)

            if(Job_Result != MEMIF_JOB_FAILED && Fls_ConfigPtr->FlsJobEndNotification != NULL)
            {
                    Job_Result = MEMIF_JOB_OK;
                    Module_Status = MEMIF_IDLE ;
                    Fls_ConfigPtr->FlsJobEndNotification();
                    Fls_PENDING_JOB = NO_JOB ;
            }

        #endif
    }
}



/*****************************************************************************************
* Function Description : Performs processing of compare job Access code.							   *
*												 Called by Fls_MainFunction when a compare job is pending.       *
******************************************************************************************/
static void Fls_Compare_AC(void){
		
		uint8 BYTE_Buffer;
		
		while(FlsJob_Length){
				
				BYTE_Buffer = (*(uint8 *)FlsJob_FlsAddress++);
				FlsJob_Length--;
			
				if(BYTE_Buffer != *FlsJob_DataBufferAddPtr++){
						Job_Result = MEMIF_BLOCK_INCONSISTENT;
				}
				
				// Check if an error occurred while read
				if(HWREG(FLASH_FCRIS) & (FLASH_FCRIS_ARIS | FLASH_FCRIS_VOLTRIS)){
						HWREG(FLASH_FCMISC) |= FLASH_FCMISC_AMISC | FLASH_FCMISC_VOLTMISC;
						Job_Result = MEMIF_JOB_FAILED;
						 Det_ReportTransientFault(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_MAIN_API_ID, FLS_E_COMPARE_FAILED);
				}
		}
		#if (FlsJobEndNotificationEnable == STD_ON)
					
				if(Job_Result != MEMIF_JOB_FAILED && Fls_ConfigPtr->FlsJobEndNotification != NULL){
						Fls_ConfigPtr->FlsJobEndNotification();
				}
					
		#endif
}



/*****************************************************************************************
* Function Description : Performs processing of verification job Access code.					   *
*												 Called by Fls_MainFunction when a verification job is pending   *
******************************************************************************************/
#if (FlsBlankCheckApi == STD_ON)
static void Fls_BlankCheck_AC(void){
		
		uint8 BYTE_Buffer;
		
		while(FlsJob_Length){
				
				BYTE_Buffer = (*(uint8 *)FlsJob_FlsAddress++);
				FlsJob_Length--;
			
				if(BYTE_Buffer != ERASED_BYTE){
						Job_Result = MEMIF_BLOCK_INCONSISTENT;
				}
				
				// Check if an error occurred while read
				if(HWREG(FLASH_FCRIS) & (FLASH_FCRIS_ARIS | FLASH_FCRIS_VOLTRIS)){
						HWREG(FLASH_FCMISC) |= FLASH_FCMISC_AMISC | FLASH_FCMISC_VOLTMISC;
						Job_Result = MEMIF_JOB_FAILED;
						 Det_ReportTransientFault(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_MAIN_API_ID, FLS_E_COMPARE_FAILED);
				}
		}
		
		#if (FlsJobEndNotificationEnable == STD_ON)
					
				if(Job_Result != MEMIF_JOB_FAILED && Fls_ConfigPtr->FlsJobEndNotification != NULL){
						Fls_ConfigPtr->FlsJobEndNotification();
				}
				
		#endif
}
#endif 



/*****************************************************************************************
* Function Description : Enable Interrupts for Flash module.                					   *
******************************************************************************************/
#if (FlsUseInterrupts == STD_ON)
static void FlashIntEnable(uint32 ui32IntFlags){
	
		// Enable flash control interrupts in the NVIC
		HWREG(NVIC_EN0) |= NVIC_EN0_INT29;
		// Enable the specified interrupts
		HWREG(FLASH_FCIM) |= ui32IntFlags;
}
#endif



/*****************************************************************************************/
/*                              Global Functions Definition                              */
/*****************************************************************************************/
	
	
/*****************************************************************************************
* Function Description : initialize the FLS module (software)														 *
*												 and all flash memory relevant registers (hardware) 						 *
*												 with parameters provided in the given configuration set.        *
*																																												 *
* Param: ConfigPtr Is a pointer to a configuration set																	 *
******************************************************************************************/
void Fls_Init( const Fls_ConfigType* ConfigPtr ){

	Fls_ConfigPtr = (Fls_ConfigType *)ConfigPtr;
	Module_Status = MEMIF_IDLE;
	Job_Result = MEMIF_JOB_OK;
	Fls_PENDING_JOB = NO_JOB;
	
	#if (FlsUseInterrupts == STD_ON)
			FlashIntEnable(FLASH_FCIM_PROGMASK | FLASH_FCIM_ERMASK | FLASH_FCIM_INVDMASK | FLASH_FCIM_VOLTMASK | FLASH_FCIM_EMASK | FLASH_FCIM_PMASK | FLASH_FCIM_AMASK);
	#endif

}


/*****************************************************************************************
* Function Description : erase one or more complete flash sectors.
*												 copy the given parameters to FLS module internal variables
*												 and initiate an erase job.
*
*	Param: TargetAddress Is flash memory address offset.
*        it will be added to flash memory base address.
*				 It must be alligned to a flash sector boundary.
*
* Param: Length Is Number of bytes to erase.
*				 It must be alligned to a flash sector boundary.
******************************************************************************************/
Std_ReturnType Fls_Erase( Fls_AddressType TargetAddress, Fls_LengthType Length ){

		while((Length % FlsSectorSize != 0)){
				Length++;
		}

		#if (FlsDevErrorDetect == STD_ON)
		
				if(Module_Status == MEMIF_UNINIT){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_ERASE_API_ID, FLS_E_UNINIT);
						return E_NOT_OK;
				}
				
				if(Module_Status == MEMIF_BUSY){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_ERASE_API_ID, FLS_E_BUSY);
						return E_NOT_OK;
				}
				
				if(((TargetAddress + FlsBaseAddress) % FlsSectorSize != 0) || (TargetAddress > FlsTotalSize - 1)){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_ERASE_API_ID, FLS_E_PARAM_ADDRESS);
						return E_NOT_OK;
				}
					
				if(Length <= 0 || (TargetAddress + Length > FlsTotalSize) || ((TargetAddress + Length) % FlsSectorSize != 0)){
			
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_ERASE_API_ID, FLS_E_PARAM_LENGTH);
						return E_NOT_OK;
				}
				
		#endif
		
		
		FlsJob_FlsAddress = TargetAddress;
		FlsJob_Length = Length;
		InternalState = FLS_ERASE_SECTOR ;
		Fls_PENDING_JOB = ERASE_JOB;
		Module_Status = MEMIF_BUSY;
		Job_Result = MEMIF_JOB_PENDING;
				
		return E_OK;

}

/*****************************************************************************************
* Function Description : write one or more complete flash pages to the flash device.
*												 copy the given parameters to Fls module internal variables
*												 and initiate a write job.
*	Param: TargetAddress Is falsh memory address offset.
*        it will be added to flash memory base address.
*				 It must be alligned to a flash page boundary.
*																																												 *
* Param: SourceAddressPtr Is Pointer to source data buffer.
*																																												 *
* Param: Length Is Number of bytes to write.
*				 It must be alligned to a flash page boundary.
******************************************************************************************/
Std_ReturnType Fls_Write( Fls_AddressType TargetAddress, const uint8* SourceAddressPtr, Fls_LengthType Length ){
		
		
		#if (FlsDevErrorDetect == STD_ON)
	
				if(Module_Status == MEMIF_UNINIT){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_WRITE_API_ID, FLS_E_UNINIT);
						return E_NOT_OK;
				}
				
				if(Module_Status == MEMIF_BUSY){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_WRITE_API_ID, FLS_E_BUSY);
						return E_NOT_OK;
				}
				
				if(((TargetAddress + FlsBaseAddress) % FlsPageSize != 0) || (TargetAddress > FlsTotalSize - 1)){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_WRITE_API_ID, FLS_E_PARAM_ADDRESS);
						return E_NOT_OK;
				}
				
				if(Length <= 0 || (TargetAddress + Length > FlsTotalSize) || ((TargetAddress + Length) % FlsPageSize != 0)){
			
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_WRITE_API_ID, FLS_E_PARAM_LENGTH);
						return E_NOT_OK;
				}
				
				if(SourceAddressPtr == NULL){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_WRITE_API_ID, FLS_E_PARAM_DATA);
						return E_NOT_OK;
				}
				
		#endif
		
		FlsJob_FlsAddress = TargetAddress;
		FlsJob_DataBufferAddPtr = (uint8 *)SourceAddressPtr;
		FlsJob_Length = Length;
		InternalState = FLS_WRITE_WORDS ;
		Fls_PENDING_JOB = WRITE_JOB;
		Module_Status = MEMIF_BUSY;
		Job_Result = MEMIF_JOB_PENDING;
				
		return E_OK;

}


/*****************************************************************************************
* Function Description : cancel an ongoing flash read, write, erase or compare job			 *
*												 shall abort a running job synchronously so that directly        *
*												 after returning from this function a new job can be started     *
******************************************************************************************/
#if (FlsCancelApi == STD_ON)

void Fls_Cancel( void ){
	
		#if (FlsDevErrorDetect == STD_ON)
				if(Module_Status == MEMIF_UNINIT){
					 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_CANCEL_API_ID, FLS_E_UNINIT);
					return;
				}
		#endif
		
		FlsJob_FlsAddress = 0;
		FlsJob_Length = 0;
		FlsJob_DataBufferAddPtr = NULL;
	
		Module_Status = MEMIF_IDLE;
		
		if(Job_Result == MEMIF_JOB_PENDING){
				Job_Result = MEMIF_JOB_CANCELED;
				Fls_PENDING_JOB = NO_JOB;
		}
		#if (FlsJobErrorNotificationEnable == STD_ON)
				Fls_ConfigPtr->FlsJobErrorNotification();
		#endif
		
}
		
#endif
		


/*****************************************************************************************
* Function Description : return the FLS module state synchronously                			 *
******************************************************************************************/
#if (FlsGetStatusApi == STD_ON)
		
MemIf_StatusType Fls_GetStatus( void ){
	  return Module_Status;
}
		
#endif
		

/*****************************************************************************************
* Function Description : return the result of the last job synchronously.          			 *
*												 The erase, write, read and compare functions 									 *
*												 shall share the same job result,i.e. only the 									 * 
*												 result of the last job can be queried.  									       *
*												 The FLS module shall overwrite the job result with  						 *
*												 MEMIF_JOB_PENDING if the FLS module has accepted a new job.     *
******************************************************************************************/
#if (FlsGetJobResultApi == STD_ON)
		
MemIf_JobResultType Fls_GetJobResult( void ){
	
		#if (FlsDevErrorDetect == STD_ON)
				if(Module_Status == MEMIF_UNINIT){
					 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_GETJOBRESULT_API_ID, FLS_E_UNINIT);
					return MEMIF_JOB_FAILED;
				}
		#endif
				
		return Job_Result;
}

#endif
		

/*****************************************************************************************
* Function Description : read from flash memory                			                     *
*												 copy the given parameters to FLS module 												 *
*												 internal variables and initiate a read job											 *
*																																												 *
* Param: SourceAddress Is flash memory address offset.																	 *
*				 it will be added to flash memory base address.												           *
*																																												 *
*	Param: TargetAddressPtr Is Pointer to target data buffer                               *
*																																												 *
* Param: Length Is Number of bytes to read.	 																					   *
******************************************************************************************/
Std_ReturnType Fls_Read( Fls_AddressType SourceAddress, uint8* TargetAddressPtr, Fls_LengthType Length ){

		#if (FlsDevErrorDetect == STD_ON)
	
				if(Module_Status == MEMIF_UNINIT){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_READ_API_ID, FLS_E_UNINIT);
						return E_NOT_OK;
				}
				
				if(Module_Status == MEMIF_BUSY){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_READ_API_ID, FLS_E_BUSY);
						return E_NOT_OK;
				}
				
				if (SourceAddress > (FlsTotalSize - 1)){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_READ_API_ID, FLS_E_PARAM_ADDRESS);
						return E_NOT_OK;
				}
				
				if(Length <= 0 || (SourceAddress + Length > FlsTotalSize)){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_READ_API_ID, FLS_E_PARAM_LENGTH);
						return E_NOT_OK;
				}
				
				if(TargetAddressPtr == NULL){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_READ_API_ID, FLS_E_PARAM_DATA);
						return E_NOT_OK;
				}
		#endif
	
		FlsJob_FlsAddress = SourceAddress;
		FlsJob_DataBufferAddPtr = TargetAddressPtr;
		FlsJob_Length = Length;
		InternalState = FLS_READ_WORD ;
		Fls_PENDING_JOB = READ_JOB;
		Module_Status = MEMIF_BUSY;
		Job_Result = MEMIF_JOB_PENDING;
				
		return E_OK;

}


/*****************************************************************************************
* Function Description : compare the contents of an area of flash memory with that       *
*												 of an application data buffer.                                  *
*												 copy the given parameters to FLS module 												 *
*												 internal variables and initiate a compare job									 *
*																																												 *
* Param: SourceAddress Is flash memory address offset.																	 *
*				 it will be added to flash memory base address.												           *
*																																												 *
*	Param: TargetAddressPtr Is Pointer to target data buffer                               *
*																																												 *
* Param: Length Is Number of bytes to compare.	 																				 *
******************************************************************************************/
#if (FlsCompareApi == STD_ON)
Std_ReturnType Fls_Compare( Fls_AddressType SourceAddress, const uint8* TargetAddressPtr, Fls_LengthType Length ){

		#if (FlsDevErrorDetect == STD_ON)
	
				if(Module_Status == MEMIF_UNINIT){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_COMPARE_API_ID, FLS_E_UNINIT);
						return E_NOT_OK;
				}
				
				if(Module_Status == MEMIF_BUSY){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_COMPARE_API_ID, FLS_E_BUSY);
						return E_NOT_OK;
				}
				
				if(SourceAddress > FlsTotalSize - 1){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_COMPARE_API_ID, FLS_E_PARAM_ADDRESS);
						return E_NOT_OK;
				}
				
				if(Length <= 0 || (SourceAddress + Length > FlsTotalSize)){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_COMPARE_API_ID, FLS_E_PARAM_LENGTH);
						return E_NOT_OK;
				}
				
				if(TargetAddressPtr == NULL){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_COMPARE_API_ID, FLS_E_PARAM_DATA);
						return E_NOT_OK;
				}
		#endif
		
		FlsJob_FlsAddress = SourceAddress;
		FlsJob_DataBufferAddPtr = (uint8 *)TargetAddressPtr;
		FlsJob_Length = Length;
	
		Fls_PENDING_JOB = COMPARE_JOB;
		Module_Status = MEMIF_BUSY;
		Job_Result = MEMIF_JOB_PENDING;
				
		return E_OK;

}
#endif



/*****************************************************************************************
* Function Description : set the FLS module’s operation mode to the                      *
*												 given “Mode” parameter                                          *
*																																												 *
* Param: Mode:																																					 *
*						  MEMIF_MODE_SLOW: Slow read access / normal SPI access.										 *
*							MEMIF_MODE_FAST: Fast read access / SPI burst access.											 *
******************************************************************************************/
#if (FlsSetModeApi == STD_ON)
void Fls_SetMode( MemIf_ModeType Mode ){
	
		#if (FlsDevErrorDetect == STD_ON)
				if(Module_Status == MEMIF_BUSY){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_SETMODE_API_ID, FLS_E_BUSY);
						return;
				}
		#endif


}
#endif



/*****************************************************************************************
* Function Description : Returns the version information of this module.                 *
*																																												 *
* Param: VersioninfoPtr	Pointer to where to store the version information of this module *																																				 *
******************************************************************************************/
void Fls_GetVersionInfo( Std_VersionInfoType* VersioninfoPtr ){

		#if (FlsDevErrorDetect == STD_ON)
				if(VersioninfoPtr == NULL){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_GETVERSIONINFO_API_ID, FLS_E_PARAM_POINTER);
						return;
				}
		#endif
		VersioninfoPtr->moduleID = FLASH_DRIVER_ID;
		VersioninfoPtr->instanceID = FLS_INSTANCE_ID;
		/*******************************************************************************/
		//  Rest of version info
		/*******************************************************************************/
}



/*****************************************************************************************
* Function Description : verify, whether a given memory area has been erased but 				 *
*												 not (yet) re-programmed.                                        *
*												 copy the given parameters to FLS module 												 *
*												 internal variables and initiate the verification job						 *
*																																												 *
* Param: TargetAddress Address in flash memory from which blank check should be started. *
*																																												 *
* Param: Length Is Number of bytes to be checked for erase pattern.											 *
******************************************************************************************/
#if (FlsBlankCheckApi == STD_ON)
Std_ReturnType Fls_BlankCheck( Fls_AddressType TargetAddress, Fls_LengthType Length ){

		#if (FlsDevErrorDetect == STD_ON)
		
				if(Module_Status == MEMIF_UNINIT){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_BLANKCHECK_API_ID, FLS_E_UNINIT);
						return E_NOT_OK;
				}
				
				if(Module_Status == MEMIF_BUSY){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_BLANKCHECK_API_ID, FLS_E_BUSY);
						return E_NOT_OK;
				}
				
				if(TargetAddress > FlsTotalSize - 1){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_BLANKCHECK_API_ID, FLS_E_PARAM_ADDRESS);
						return E_NOT_OK;
				}
					
				if(Length <= 0 || (TargetAddress + Length > FlsTotalSize)){
			
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_BLANKCHECK_API_ID, FLS_E_PARAM_LENGTH);
						return E_NOT_OK;
				}
				
		#endif
		
		FlsJob_FlsAddress = TargetAddress;
		FlsJob_Length = Length;
	
		Fls_PENDING_JOB = VERIFICATION_JOB;
		Module_Status = MEMIF_BUSY;
		Job_Result = MEMIF_JOB_PENDING;
				
		return E_OK;

}
#endif




/*****************************************************************************************
* Function Description : perform the processing of the flash read,											 *
*												 write, erase and compare jobs. 					                       *
*												 the FLS module’s environment shall call the function            *
*												 Fls_MainFunction cyclically.                                    *
******************************************************************************************/
void Fls_MainFunction( void ){


		#if (FlsDevErrorDetect == STD_ON)
				
				if(Module_Status == MEMIF_UNINIT){
						 Det_ReportError(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_BLANKCHECK_API_ID, FLS_E_UNINIT);
				}
		#endif
	
        switch (Fls_PENDING_JOB)
        {
            case WRITE_JOB:
                Fls_Write_AC();
            break;

            case ERASE_JOB :
                Fls_Erase_AC();
            break;

            case READ_JOB :
                Fls_Read_AC();
            break;

            case COMPARE_JOB :
                Fls_Compare_AC();
            break;

            #if (FlsBlankCheckApi == STD_ON)
            case VERIFICATION_JOB :
                Fls_BlankCheck_AC();
            break;
            #endif
        }
}



/*****************************************************************************************
* Function Description : Flash Interrupt handler.											 									 *
******************************************************************************************/
#if (FlsUseInterrupts == STD_ON)
	void FLASH_Handler(void){
		
		if((Fls_PENDING_JOB == WRITE_JOB) && (HWREG(FLASH_FCRIS) & FLASH_FCRIS_PRIS)){
			
				HWREG(FLASH_FCMISC) |= FLASH_FCMISC_PMISC;
			
				// Check if an access violation error occurred.
				if(HWREG(FLASH_FCRIS) & (FLASH_FCRIS_ARIS | FLASH_FCRIS_VOLTRIS |
																 FLASH_FCRIS_INVDRIS | FLASH_FCRIS_PROGRIS))
				{
					 HWREG(FLASH_FCMISC) |= FLASH_FCMISC_AMISC | FLASH_FCMISC_VOLTMISC  | FLASH_FCMISC_INVDMISC | FLASH_FCMISC_PROGMISC; 
					 Job_Result = MEMIF_JOB_FAILED;
					  Det_ReportTransientFault(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_MAIN_API_ID, FLS_E_WRITE_FAILED);
				}
				
				if(End_Job_Flag == 1){
						End_Job_Flag = 0;
						#if (FlsJobEndNotificationEnable == STD_ON)
						
							if(Job_Result != MEMIF_JOB_FAILED && Fls_ConfigPtr->FlsJobEndNotification != NULL){
									Fls_ConfigPtr->FlsJobEndNotification();
							}
						
						#endif
				}
		}
		
		
		if((Fls_PENDING_JOB == ERASE_JOB) && (HWREG(FLASH_FCRIS) & FLASH_FCRIS_PRIS)){
			
				HWREG(FLASH_FCMISC) |= FLASH_FCMISC_PMISC;
			
				// Check if an access violation or erase error occurred
				if(HWREG(FLASH_FCRIS) & (FLASH_FCRIS_ARIS | FLASH_FCRIS_VOLTRIS |
																 FLASH_FCRIS_ERRIS))
				{
						HWREG(FLASH_FCMISC) |= FLASH_FCMISC_AMISC | FLASH_FCMISC_VOLTMISC  | FLASH_FCMISC_ERMISC; 
						Job_Result = MEMIF_JOB_FAILED;
						 Det_ReportTransientFault(FLASH_DRIVER_ID, FLS_INSTANCE_ID, FLS_MAIN_API_ID, FLS_E_ERASE_FAILED);
				}
				if(End_Job_Flag == 1){
						End_Job_Flag = 0;
						#if (FlsJobEndNotificationEnable == STD_ON)
						
							if(Job_Result != MEMIF_JOB_FAILED && Fls_ConfigPtr->FlsJobEndNotification != NULL){
									Fls_ConfigPtr->FlsJobEndNotification();
							}
						
						#endif
				}
		}
		
	}
#endif


