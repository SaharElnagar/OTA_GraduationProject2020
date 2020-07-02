/*
 * NvM_Lcfg.c
 *
 *
 *      Author: Sahar
 */


#include "NvM_Types.h"
#include "NvM_Cfg.h"
#include "Std_Types.h"
#include "Fee_Cfg.h"
#include "AppCfg.h"



NvMBlockDescriptorType NvMBlockDescriptor[NUMBER_OF_NVM_BLOCKS] =
 {
         /*Block 0 Saved*/
         {},
         /*Block 1 Saved*/
         {},
         /*Block 2 Configurations*/
         {
             .NvMBlockCrcType               =   NVM_CRC32           ,
             .NvMBlockManagement            =   NVM_BLOCK_NATIVE    ,
             .NvMBlockUseCrc                =   STD_ON              ,
             .NvMBlockUseSetRamBlockStatus  =   STD_ON              ,
             .NvMCalcRamBlockCrc            =   STD_ON              ,
             .NvMMaxNumOfWriteRetries       =   1                   ,
             .NvMInitBlockCallback          =   NULL                ,
             .NvMNvBlockBaseNumber          =   NVM_NVRAM_BLOCK_2_BASENUMBER    ,
             .NvMNvBlockLength              =   NVM_NVRAM_BLOCK_2_LENGTH        ,
             .NvMNvBlockNum                 =   1                               ,
             .NvMNvramDeviceId              =   0                               ,
             .NvMRamBlockDataAddress        =   Block2_PRAMBLOCK                ,
             .NvMRomBlockDataAddress        =   Block2_ROMBLOCK                 ,
             .NvMRomBlockNum                =   1                               ,
             .NvMSelectBlockForReadAll      =   STD_ON                          ,
             .NvMSelectBlockForWriteAll     =   STD_ON                          ,
             .NvMSingleBlockCallback        =   Block_2_NvMSingleBlockCallback  ,
         },
         /*Block 3 Configurations*/
         {
             .NvMBlockCrcType               =   NVM_CRC32           ,
             .NvMBlockManagement            =   NVM_BLOCK_DATASET   ,
             .NvMBlockUseCrc                =   STD_ON              ,
             .NvMBlockUseSetRamBlockStatus  =   STD_ON              ,
             .NvMCalcRamBlockCrc            =   STD_ON              ,
             .NvMMaxNumOfWriteRetries       =   1                   ,
             .NvMInitBlockCallback          =   NULL                ,
             .NvMNvBlockBaseNumber          =   NVM_NVRAM_BLOCK_3_BASENUMBER    ,
             .NvMNvBlockLength              =   NVM_NVRAM_BLOCK_3_LENGTH        ,
             .NvMNvBlockNum                 =   2                               ,
             .NvMNvramDeviceId              =   0                               ,
             .NvMRamBlockDataAddress        =   NULL                        ,
             .NvMRomBlockDataAddress        =   Block3_ROMBLOCK                 ,
             .NvMRomBlockNum                =   1                               ,
             .NvMSelectBlockForReadAll      =   STD_OFF                         ,
             .NvMSelectBlockForWriteAll     =   STD_ON                          ,
             .NvMSingleBlockCallback        =   Block_2_NvMSingleBlockCallback  ,
         }
         ,
         {
             .NvMBlockCrcType               =   NVM_CRC32           ,
             .NvMBlockManagement            =   NVM_BLOCK_NATIVE    ,
             .NvMBlockUseCrc                =   STD_ON              ,
             .NvMBlockUseSetRamBlockStatus  =   STD_ON              ,
             .NvMCalcRamBlockCrc            =   STD_ON              ,
             .NvMMaxNumOfWriteRetries       =   1                   ,
             .NvMInitBlockCallback          =   NULL                ,
             .NvMNvBlockBaseNumber          =   NVM_NVRAM_BLOCK_4_BASENUMBER    ,
             .NvMNvBlockLength              =   NVM_NVRAM_BLOCK_4_LENGTH        ,
             .NvMNvBlockNum                 =   1                               ,
             .NvMNvramDeviceId              =   0                               ,
             .NvMRamBlockDataAddress        =   Block4_PRAMBLOCK                ,
             .NvMRomBlockDataAddress        =   Block4_ROMBLOCK                 ,
             .NvMRomBlockNum                =   1                               ,
             .NvMSelectBlockForReadAll      =   STD_ON                          ,
             .NvMSelectBlockForWriteAll     =   STD_ON                          ,
             .NvMSingleBlockCallback        =   Block_2_NvMSingleBlockCallback  ,
         }
 };

