/*
 * NvM.h
 *
 *  Created on: Jun 4, 2020
 *      Author: Sahar
 */

#ifndef BSW_STATIC_SERVICE_NVM_INC_NVM_H_
#define BSW_STATIC_SERVICE_NVM_INC_NVM_H_


#include "NvM_Types.h"
#include "NvM_Cfg.h"

/*****************************************************************************************/
/*                                  NvM Module Id and instance                           */
/*****************************************************************************************/

#define NVRAM_MANAGER_ID                            (0x20U)
#define NVRAM_MANAGER_INSTANCE                      (0x00U)

/*****************************************************************************************/
/*                                  NvM APIs IDs                                         */
/*****************************************************************************************/

#define NVM_INIT_API_ID                             (0x00U)
#define NVM_SET_DATAINDEX_API_ID                    (0x01U)
#define NVM_GET_DATAINDEX_API_ID                    (0x02U)
#define NVM_GET_ERROR_STATUS_API_ID                 (0x04U)
#define NVM_SET_RAMBLOCK_STATUS_API_ID              (0x05U)
#define NVM_READBLOCK_API_ID                        (0x06U)
#define NVM_WRITEBLOCK_API_ID                       (0x07U)
#define NVM_RESTORE_BLOCKDEFAULTS_API_ID            (0x08U)
#define NVM_INVALIDATEBLOCK_API_ID                  (0x0BU)
#define NVM_READ_ALL_API_ID                         (0x0CU)
#define NVM_WRITE_ALL_API_ID                        (0x0DU)


/*****************************************************************************************/
/*                                  NvM Development errors                               */
/*****************************************************************************************/
#define NVM_E_PARAM_BLOCK_ID                        (0x0AU)
#define NVM_E_PARAM_BLOCK_TYPE                      (0x0BU)
#define NVM_E_PARAM_BLOCK_DATA_IDX                  (0x0CU)
#define NVM_E_PARAM_ADDRESS                         (0x0DU)
#define NVM_E_PARAM_DATA                            (0x0EU)
#define NVM_E_PARAM_POINTER                         (0x0FU)
#define NVM_E_BLOCK_WITHOUT_DEFAULTS                (0x11U)
#define NVM_E_NOT_INITIALIZED                       (0x14U)
#define NVM_E_BLOCK_PENDING                         (0x15U)
#define NVM_E_BLOCK_CONFIG                          (0x18U)
#define NVM_E_WRITE_ONCE_STATUS_UNKNOWN             (0x1AU)

/*****************************************************************************************/
/*                                  NvM Runtime errors                                   */
/*****************************************************************************************/
/*[SWS_NvM_00948]  The run-time error NVM_E_QUEUE_FULL shall be reported to DET,
 * by the NvM module, each time a request cannot be queued because the related
 * queue is full. (SRS_Mem_00038) */
#define NVM_E_QUEUE_FULL                            (0xA0U)


/*****************************************************************************************/
/*                              Global Functions Prototypes                              */
/*****************************************************************************************/
void NvM_Init(const NvM_ConfigType* ConfigPtr );
Std_ReturnType NvM_WriteBlock( NvM_BlockIdType BlockId, const void* NvM_SrcPtr ) ;
Std_ReturnType NvM_ReadBlock( NvM_BlockIdType BlockId, void* NvM_DstPtr ) ;
Std_ReturnType NvM_InvalidateNvBlock( NvM_BlockIdType BlockId ) ;
void NvM_WriteAll( void ) ;
void NvM_MainFunction( void ) ;

#endif /* BSW_STATIC_SERVICE_NVM_INC_NVM_H_ */
