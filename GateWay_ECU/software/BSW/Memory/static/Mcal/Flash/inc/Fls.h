/*******************************************************************************
**                                                                            **
**  FILENAME     : Fls.h                                                      **
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
#ifndef FLS_H_
#define FLS_H_

#include "Std_Types.h"
#include "FlsCfg.h"




#define FLS_SPECIFIED_ERASE_CYCLES          (100000)
/*******************************************************************************/
//			Development Errors' IDs
/*******************************************************************************/
#define FLS_E_PARAM_CONFIG 		 		    0x01
#define FLS_E_PARAM_ADDRESS 	 		    0x02
#define FLS_E_PARAM_LENGTH  	 		    0x03
#define FLS_E_PARAM_DATA			 	    0x04
#define FLS_E_UNINIT 					    0x05
#define FLS_E_BUSY 						    0x06
#define FLS_E_PARAM_POINTER 	 		    0x0a

/*******************************************************************************/
//			Transient Faults' IDs
/*******************************************************************************/
#define FLS_E_ERASE_FAILED				0x01
#define FLS_E_WRITE_FAILED				0x02
#define FLS_E_READ_FAILED				0x03
#define FLS_E_COMPARE_FAILED			0x04
#define FLS_E_UNEXPECTED_FLASH_ID	    0x05

/*******************************************************************************/
//			Runtime Errors' IDs
/*******************************************************************************/
#define FLS_E_VERIFY_ERASE_FAILED	0x07
#define FLS_E_VERIFY_WRITE_FAILED	0x08
#define FLS_E_TIMEOUT							0x09

/*******************************************************************************/
//			Module ID
/*******************************************************************************/
#define FLASH_DRIVER_ID				  	(92U)

/*******************************************************************************/
//			Instance ID
/*******************************************************************************/
#define FLS_INSTANCE_ID          	  	0

/*******************************************************************************/
//			Flash APIs IDs
/*******************************************************************************/
#define FLS_ERASE_API_ID     	  	0x01
#define FLS_WRITE_API_ID		 	0x02
#define FLS_CANCEL_API_ID		 	0x03
#define FLS_GETSTATUS_API_ID	  	0x04
#define FLS_GETJOBRESULT_API_ID 	0x05
#define FLS_MAIN_API_ID				0x06
#define FLS_READ_API_ID				0x07
#define FLS_COMPARE_API_ID			0x08
#define FLS_SETMODE_API_ID			0x09
#define FLS_GETVERSIONINFO_API_ID	0x10
#define FLS_BLANKCHECK_API_ID		0x0a


/*******************************************************************************/
//			Type Definitions
/*******************************************************************************/
typedef uint32 Fls_AddressType;
typedef uint32 Fls_LengthType;


/*******************************************************************************/
//			Enum Type Definitions
/*******************************************************************************/
typedef uint8 JOB_PENDING_TYPE;
#define NO_JOB										(JOB_PENDING_TYPE)0
#define READ_JOB									(JOB_PENDING_TYPE)1
#define WRITE_JOB   							    (JOB_PENDING_TYPE)2
#define ERASE_JOB									(JOB_PENDING_TYPE)3
#define COMPARE_JOB								    (JOB_PENDING_TYPE)4
#define VERIFICATION_JOB					        (JOB_PENDING_TYPE)5

/*******************************************************************************/
//  Container for runtime configuration parameters of the FLASH driver.
//   Implementation Type: Fls_ConfigType.
/*******************************************************************************/
typedef struct
{
	void (*FlsJobEndNotification)(void);
	void (*FlsJobErrorNotification)(void);
}Fls_ConfigType;
	

/*******************************************************************************/
//			Function Definitions
/*******************************************************************************/
void Fls_Init( const Fls_ConfigType* ConfigPtr );
Std_ReturnType Fls_Erase( Fls_AddressType TargetAddress, Fls_LengthType Length );
Std_ReturnType Fls_Write( Fls_AddressType TargetAddress, const uint8* SourceAddressPtr, Fls_LengthType Length );
#if (FlsCancelApi == STD_ON)
	void Fls_Cancel( void );
#endif
#if (FlsGetStatusApi == STD_ON)
	MemIf_StatusType Fls_GetStatus( void );
#endif
#if (FlsGetJobResultApi == STD_ON)
	MemIf_JobResultType Fls_GetJobResult( void );
#endif
Std_ReturnType Fls_Read( Fls_AddressType SourceAddress, uint8* TargetAddressPtr, Fls_LengthType Length );
#if (FlsCompareApi == STD_ON)
	Std_ReturnType Fls_Compare( Fls_AddressType SourceAddress, const uint8* TargetAddressPtr, Fls_LengthType Length );
#endif
#if (FlsSetModeApi == STD_ON)
	void Fls_SetMode( MemIf_ModeType Mode );
#endif
void Fls_GetVersionInfo( Std_VersionInfoType* VersioninfoPtr );

#if (FlsBlankCheckApi == STD_ON)
	Std_ReturnType Fls_BlankCheck( Fls_AddressType TargetAddress, Fls_LengthType Length );
#endif

void Fls_MainFunction( void );

#if (FlsUseInterrupts == STD_ON)
	void FLASH_Handler(void);
#endif

#endif


