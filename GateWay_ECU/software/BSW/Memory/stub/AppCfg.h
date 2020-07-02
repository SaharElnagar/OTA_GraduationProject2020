/*
 * Application_Cfg.h
 *
 *
 *      Author: Sahar
 */

#ifndef SERVICE_NVM_INC_APPLICATION_CFG_H_
#define SERVICE_NVM_INC_APPLICATION_CFG_H_

#include "NvM_Types.h"
#include "NvM_Cfg.h"
Std_ReturnType Block_2_NvMSingleBlockCallback(uint8 ServiceId,NvM_RequestResultType JobResult) ;
void NvM_MultiBlockCallbackFunction(uint8 ServiceId,NvM_RequestResultType JobResult);

extern uint8 Block2_PRAMBLOCK[NVM_NVRAM_BLOCK_2_LENGTH];
extern uint8 Block2_ROMBLOCK[NVM_NVRAM_BLOCK_2_LENGTH];

extern  uint8 Block3_ROMBLOCK[NVM_NVRAM_BLOCK_3_LENGTH];
extern  uint8 Block4_ROMBLOCK[NVM_NVRAM_BLOCK_4_LENGTH] ;
extern  uint8 Block4_PRAMBLOCK[NVM_NVRAM_BLOCK_4_LENGTH];


#endif /* SERVICE_NVM_INC_APPLICATION_CFG_H_ */
