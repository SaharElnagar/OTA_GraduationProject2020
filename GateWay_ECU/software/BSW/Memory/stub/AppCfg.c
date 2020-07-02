/*
 * Application_LCfg.c
 *
 *
 *      Author: Sahar
 */
#include "NvM_Cfg.h"
#include "NvM.h"

 uint8 Block2_PRAMBLOCK[NVM_NVRAM_BLOCK_2_LENGTH];
 uint8 Block2_ROMBLOCK[NVM_NVRAM_BLOCK_2_LENGTH]={2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};

 uint8 Block3_ROMBLOCK[NVM_NVRAM_BLOCK_3_LENGTH]={3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3};

 uint8 Block4_PRAMBLOCK[NVM_NVRAM_BLOCK_4_LENGTH];
 uint8 Block4_ROMBLOCK[NVM_NVRAM_BLOCK_4_LENGTH]={4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};



 void NvM_MultiBlockCallbackFunction(uint8 ServiceId,NvM_RequestResultType JobResult)
 {}
