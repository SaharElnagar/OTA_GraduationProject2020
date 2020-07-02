/*******************************************************************************
**                                                                            **
**  FILENAME     : Fee.h                                                      **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2020-3-12                                                  **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Yomna Mokhtar                                              **
**                                                                            **
**                                                                            **
*******************************************************************************/


#ifndef FEE_H_
#define FEE_H_

#include "Std_Types.h"
#include "Fee_Cfg.h"

/*******************************************************************************/
//          Macro Definitions
/*******************************************************************************/


/*******************************************************************************/
//          Development Errors' IDs
/*******************************************************************************/
#define FEE_E_UNINIT                          0x01
#define FEE_E_INVALID_BLOCK_NO                0x02
#define FEE_E_INVALID_BLOCK_OFS               0x03
#define FEE_E_PARAM_POINTER                   0x04
#define FEE_E_INVALID_BLOCK_LEN               0x05
#define FEE_E_INIT_FAILED                     0x09

/*******************************************************************************/
//          Runtime Errors' IDs
/*******************************************************************************/
#define FEE_E_BUSY                            0x06
#define FEE_E_INVALID_CANCEL                  0x08

/*******************************************************************************/
//          Module ID
/*******************************************************************************/
#define FEE_MODULE_ID                         (21U)

/*******************************************************************************/
//          Instance ID
/*******************************************************************************/
#define FEE_0_INSTANCE_ID                     (0U)

/*******************************************************************************/
//          FEE APIs IDs
/*******************************************************************************/
#define FEE_INIT_API_ID                           0x00
#define FEE_SETMODE_API_ID                        0x01
#define FEE_READ_API_ID                           0x02
#define FEE_WRITE_API_ID                          0x03
#define FEE_CANCEL_API_ID                         0x04
#define FEE_GETSTATUS_API_ID                      0x05
#define FEE_GETJOBRESULT_API_ID                   0x06
#define FEE_INVALIDATEBLOCK_API_ID                0x07
#define FEE_GETVERSIONINFO_API_ID                 0x08
#define FEE_ERASEIMMEDIATEBLOCK_API_ID            0x09
#define FEE_JOBENDNOTIFICATION_API_ID             0x10
#define FEE_JOBERRORNOTIFICATION_API_ID           0x11
#define FEE_MAIN_API_ID                           0x12



typedef struct{


}Fee_ConfigType;


void Fee_Init( const Fee_ConfigType* ConfigPtr );
void Fee_SetMode( MemIf_ModeType Mode );
Std_ReturnType Fee_Read( uint16 BlockNumber, uint16 BlockOffset, uint8* DataBufferPtr, uint16 Length );
Std_ReturnType Fee_Write( uint16 BlockNumber, const uint8* DataBufferPtr );
void Fee_Cancel( void );
MemIf_StatusType Fee_GetStatus( void );
MemIf_JobResultType Fee_GetJobResult( void );
Std_ReturnType Fee_InvalidateBlock( uint16 BlockNumber );
void Fee_GetVersionInfo( Std_VersionInfoType* VersionInfoPtr );
Std_ReturnType Fee_EraseImmediateBlock( uint16 BlockNumber );
void Fee_JobEndNotification( void );
void Fee_JobErrorNotification( void );
void Fee_MainFunction( void );

#endif

