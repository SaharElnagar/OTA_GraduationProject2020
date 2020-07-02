/*******************************************************************************
 **                                                                            **
 **  FILENAME     : Ea.c                                                       **
 **                                                                            **
 **  VERSION      : 4.3.1                                                      **
 **                                                                            **
 **  DATE         : 2020-2-17                                                  **
 **                                                                            **
 **  PLATFORM     : TIVA C                                                     **
 **                                                                            **
 **  AUTHOR       : Sahar Elnagar                                              **
 **                                                                            **
 *******************************************************************************/

#ifndef EA_H_
#define EA_H_

#include "Eep.h"
#include "Ea_Cfg.h"

/*******************************************************************************/
//  EEPROM Abstraction Interface  Module ID
/*******************************************************************************/
#define EA_MODULE_ID                    (40U)

/*******************************************************************************/
//  Instance Id
/*******************************************************************************/
#define EA_0_INSTANCE_ID                 (0U)

/*******************************************************************************/
//  EEPROM Abstraction Interface  APIs IDs
/*******************************************************************************/
#define EA_INIT_API_ID             (0U)
#define EA_READ_API_ID             (2U)
#define EA_WRITE_API_ID            (3U)
#define EA_CANCEL_API_ID           (4U)
#define EA_GET_STATUS_API_ID       (5U)
#define EA_INVALIDATE_BLOCK_API    (7U)
#define EA_MAINFUNCTION            (12U)

/*******************************************************************************/
//  Module Development Errors
/*******************************************************************************/
/*API service called while module is not (yet) initialized*/
#define EA_E_UNINIT                (1U)

/*API service called with invalid block number */
#define EA_E_INVALID_BLOCK_NO      (2U)

/*API service called with invalid block offset */
#define EA_E_INVALID_BLOCK_OFS     (3U)

/*API service called with invalid pointer argument*/
#define EA_E_PARAM_POINTER         (4U)

/*API service called with invalid block length information*/
#define EA_E_INVALID_BLOCK_LEN     (5U)

/*Ea_Init failed*/
#define  EA_E_INIT_FAILED          (9U)


/*******************************************************************************/
//  Module RunTime Errors
/*******************************************************************************/
/*API service called while module is busy*/
#define EA_E_BUSY                  (6U)

/*Ea_Cancel called while no job was pending */
#define EA_E_INVALID_CANCEL        (8U)

typedef struct
{

} Ea_ConfigType;

void Ea_Init(const Ea_ConfigType* ConfigPtr );
Std_ReturnType Ea_Read(uint16 BlockNumber,uint16 BlockOffset,uint8* DataBufferPtr, uint16 Length ) ;
Std_ReturnType Ea_Write( uint16 BlockNumber,const uint8* DataBufferPtr ) ;
MemIf_StatusType Ea_GetStatus(void) ;
MemIf_JobResultType Ea_GetJobResult(void);
Std_ReturnType Ea_InvalidateBlock(uint16 BlockNumber);
void Ea_MainFunction(void) ;
#endif
