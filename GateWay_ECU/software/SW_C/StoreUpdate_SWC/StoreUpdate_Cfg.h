/*
 * StoreUpdate_Cfg.h
 *
 *  Created on: June 23, 2020
 *      Author: Sahar Elnager
 */

#ifndef SW_C_STOREUPDATE_SWC_STOREUPDATE_CFG_H_
#define SW_C_STOREUPDATE_SWC_STOREUPDATE_CFG_H_

#include "Ea_Cfg.h"
#include "NvM_Cfg.h"

#define PACKET_SIZE                 1024
#define FRAME_INFO_SIZE             20
#define UPDATED_ECUS_NUM_INDEX      0
#define FIRST_ECU_SIZE_INDEX        8
#define SECOND_ECU_SIZE_INDEX       16
#define FIRST_ECU_ID_INDEX          4
#define SECOND_ECU_ID_INDEX         12

/***************************************************************************/
/*                             EA blocks CFG                               */
/***************************************************************************/
#define NvM_FRAME_INFO_BLOCK_ID                     NVM_NVRAM_BLOCK_2_ID
#define NvM_FRAME_INFO_BLOCK_BASE                   NVM_NVRAM_BLOCK_2_BASENUMBER
#define NvM_FRAME_INFO_BLOCK_SIZE                   NVM_NVRAM_BLOCK_2_LENGTH
#define NvM_FRAME_INFO_BLOCK_DEVICE_ID              EA_DEVICE_INDEX


/***************************************************************************/
/*                             FEE blocks CFG                              */
/***************************************************************************/
#define NvM_PACKET_1_BLOCK_ID                   NVM_NVRAM_BLOCK_3_ID
#define NvM_PACKET_1_BLOCK_BASE                 NVM_NVRAM_BLOCK_3_BASENUMBER
#define NvM_PACKET_1_BLOCK_DEVICE_ID            FEE_DEVICE_INDEX
#define NvM_PACKET_1_BLOCK_SIZE                 NVM_NVRAM_BLOCK_3_LENGTH

#define NvM_PACKET_2_BLOCK_ID                   NVM_NVRAM_BLOCK_4_ID
#define NvM_PACKET_2_BLOCK_BASE                 NVM_NVRAM_BLOCK_4_BASENUMBER
#define NvM_PACKET_2_BLOCK_DEVICE_ID            FEE_DEVICE_INDEX
#define NvM_PACKET_2_BLOCK_SIZE                 NVM_NVRAM_BLOCK_4_LENGTH

#define NvM_PACKET_3_BLOCK_ID                   NVM_NVRAM_BLOCK_5_ID
#define NvM_PACKET_3_BLOCK_BASE                 NVM_NVRAM_BLOCK_5_BASENUMBER
#define NvM_PACKET_3_BLOCK_DEVICE_ID            FEE_DEVICE_INDEX
#define NvM_PACKET_3_BLOCK_SIZE                 NVM_NVRAM_BLOCK_5_LENGTH

#define NvM_PACKET_4_BLOCK_ID                   NVM_NVRAM_BLOCK_6_ID
#define NvM_PACKET_4_BLOCK_BASE                 NVM_NVRAM_BLOCK_6_BASENUMBER
#define NvM_PACKET_4_BLOCK_DEVICE_ID            FEE_DEVICE_INDEX
#define NvM_PACKET_4_BLOCK_SIZE                 NVM_NVRAM_BLOCK_6_LENGTH


#define NvM_PACKET_5_BLOCK_ID                   NVM_NVRAM_BLOCK_7_ID
#define NvM_PACKET_5_BLOCK_BASE                 NVM_NVRAM_BLOCK_7_BASENUMBER
#define NvM_PACKET_5_BLOCK_DEVICE_ID            FEE_DEVICE_INDEX
#define NvM_PACKET_5_BLOCK_SIZE                 NVM_NVRAM_BLOCK_7_LENGTH

#define NvM_PACKET_6_BLOCK_ID                   NVM_NVRAM_BLOCK_8_ID
#define NvM_PACKET_6_BLOCK_BASE                 NVM_NVRAM_BLOCK_8_BASENUMBER
#define NvM_PACKET_6_BLOCK_DEVICE_ID            FEE_DEVICE_INDEX
#define NvM_PACKET_6_BLOCK_SIZE                 NVM_NVRAM_BLOCK_8_LENGTH

#define NvM_PACKET_7_BLOCK_ID                   NVM_NVRAM_BLOCK_9_ID
#define NvM_PACKET_7_BLOCK_BASE                 NVM_NVRAM_BLOCK_9_BASENUMBER
#define NvM_PACKET_7_BLOCK_DEVICE_ID            FEE_DEVICE_INDEX
#define NvM_PACKET_7_BLOCK_SIZE                 NVM_NVRAM_BLOCK_9_LENGTH

#define NvM_PACKET_8_BLOCK_ID                   NVM_NVRAM_BLOCK_10_ID
#define NvM_PACKET_8_BLOCK_BASE                 NVM_NVRAM_BLOCK_10_BASENUMBER
#define NvM_PACKET_8_BLOCK_DEVICE_ID            FEE_DEVICE_INDEX
#define NvM_PACKET_8_BLOCK_SIZE                 NVM_NVRAM_BLOCK_10_LENGTH

#endif /* SW_C_STOREUPDATE_SWC_STOREUPDATE_CFG_H_ */
