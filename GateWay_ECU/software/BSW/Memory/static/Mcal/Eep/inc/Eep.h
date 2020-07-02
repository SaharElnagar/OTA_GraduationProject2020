
/*******************************************************************************
**                                                                            **
**  FILENAME     : Eep.h                                                      **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2019-11-22                                                 **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Sahar Elnagar                                              **
**                                                                            **
**                                                                            **
*******************************************************************************/

#ifndef EEP_H_
#define EEP_H_

#include "Std_Types.h"
#include "Eep_Cfg.h"
#include "Eep_Types.h"




#ifndef NULL_PTR
#define NULL_PTR  ((void*)0)
#endif

//*****************************************************************************
//
//  Define word size in EEPROM
//
//*****************************************************************************
#define PHYSICAL_WORD_SIZE          4

/*******************************************************************************/
//  EEPROM Module ID
/*******************************************************************************/
#define EEPROM_DRIVER_ID            (90U)

/*******************************************************************************/
//  Instance Id
/*******************************************************************************/
#define EEP_INSTANCE_ID                 0x00

/*******************************************************************************/
//  EEPROM APIs IDs
/*******************************************************************************/
#define EEP_INIT_API_ID             (0U)
#define EEP_READ_API_ID             (2U)
#define EEP_WRITE_API_ID            (3U)
#define EEP_ERASE_API_ID            (4U)
#define EEP_COMPARE_API_ID          (5U)

/*******************************************************************************/
//  EEPROM Size in bytes is 2K bytes
/*******************************************************************************/
#define EEP_SIZE                     2048

/*******************************************************************************/
//  EEPROM Address Range (word addressable)
/*******************************************************************************/
#define EEP_START_ADDRESS                 (0U)
#define EEP_END_ADDRESS                   EEP_SIZE-4

/*******************************************************************************/
//  EEPROM Minimum number of bytes to read
/*******************************************************************************/
#define MIN_LENGTH                    (1U)


/*******************************************************************************/
//  Module Development Errors
/*******************************************************************************/
#define EEP_E_INIT_FAILED           0x10
#define EEP_E_PARAM_ADDRESS         0x11
#define EEP_E_PARAM_DATA            0x12
#define EEP_E_PARAM_LENGTH          0x13
#define EEP_E_PARAM_POINTER         0x23
#define EEP_E_UNINIT                0x20

/*******************************************************************************/
//  Module RunTime Errors
/*******************************************************************************/
#define EEP_E_BUSY                  0x21
#define EEP_E_TIMEOUT               0x22

/*******************************************************************************/
// Published information
/*******************************************************************************/
#define EepAllowedWriteCycles      (500000) 
#define EepEraseUnitSize           EEP_SIZE            /*Mass Erase only*/
#define EepEraseValue              0xFFFFFFFF          /*Value of an erased EEPROM cell.*/
#define EepMinimumAddressType      (16U)               /*Minimum expected size of Eep_AddressType, 16 bits */
#define EepMinimumLengthType       (16U)               /*Minimum expected size of Eep_LengthType.s*/
#define EepReadUnitSize            (4U)                /*Size of smallest readable EEPROM data unit in bytes,4bytess*/
#define EepTotalSize               EEP_SIZE            /*Total size of EEPROM in bytes.*/
#define EepWriteUnitSize           (4U)                /*Size of smallest writeable EEPROM data unit in bytes*/


/*******************************************************************************/
//  Functions Prototypes
/*******************************************************************************/
void Eep_Init( const Eep_ConfigType* ConfigPtr );
Std_ReturnType Eep_Read(Eep_AddressType EepromAddress,uint8* DataBufferPtr,Eep_LengthType Length );
Std_ReturnType Eep_Write(Eep_AddressType EepromAddress, const uint8* DataBufferPtr,Eep_LengthType Length );
Std_ReturnType Eep_Erase(Eep_AddressType EepromAddress,Eep_LengthType Length ) ;
void Eep_Cancel(void);
MemIf_StatusType Eep_GetStatus(void);
MemIf_JobResultType Eep_GetJobResult(void) ;
void Eep_MainFunction(void);

#endif /* EEP_H_ */


