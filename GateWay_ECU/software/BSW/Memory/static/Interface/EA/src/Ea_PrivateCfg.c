/*
 * Ea_PrivateCfg.c
 *
 *  Created on: 2020-2-17
 *      Author: Sahar
 */

#include "Ea_Cfg.h"
#include "Ea_PrivateTypes.h"

/*Map pre-compile configurations for each block in this container */

Ea_BlockConfigType Ea_BlockSConfig[EA_BLOCKS_NUM] =
{
     {
          .BlockNumber              = EA_BLOCK_0_NUMBER     ,
          .PhysicalStartAddress     = 0                     ,   /*Initialized in init function*/
          .BlockSize                = EA_BLOCK_0_SIZE       ,
     },
     {
          .BlockNumber              = EA_BLOCK_1_NUMBER     ,
          .PhysicalStartAddress     = 0                     ,   /*Initialized in init function*/
          .BlockSize                = EA_BLOCK_1_SIZE       ,
     }
};
