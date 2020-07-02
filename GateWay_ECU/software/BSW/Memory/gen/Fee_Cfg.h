/*******************************************************************************
**                                                                            **
**  FILENAME     : Fee_Cfg.h                                                  **
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


#ifndef FEE_CFG_H_
#define FEE_CFG_H_

#include "MemIf_Types.h"
#include "Fls.h"
#include "NvM_Cbk.h"


#define FeeDevErrorDetect								STD_ON

#define FeeMainFunctionPeriod						    1

#define FeeNvmJobEndNotification				        NvM_JobEndNotification

#define FeeNvmJobErrorNotification			            NvM_JobErrorNotification

#define FeePollingMode									STD_OFF

#define FeeSetModeSupported							    STD_OFF

#define FeeVersionInfoApi								STD_ON

//The size in bytes to which logical blocks shall be aligned = 1KB = 1024 BYTE
#define FeeVirtualPageSize							0x00000400

/*Reference to the device this block is stored in*/
#define  FEE_DEVICE_INDEX                      (0U)
/*****************************************************************************************/
/*FeeBlockConfiguration                  																								 */
/*Configuration of block specific parameters for the Flash EEPROM Emulation module       */
/*****************************************************************************************/
/*Number of blocks configured*/
#define BLOCKS_NUM                          (10U)

/*Block identifier (handle). 0x0000 and 0xFFFF shall not be used for block numbers (see SWS_Ea_00006).
  Range:   min = 2^NVM_DATASET_SELECTION_BITS
  max = 0xFFFF -2^NVM_DATASET_SELECTION_BITS
*/
#define BLOCK_1_NUMBER                   (1U)
#define BLOCK_2_NUMBER                   (2U)
#define BLOCK_3_NUMBER                   (3U)
#define BLOCK_4_NUMBER                   (4U)
#define BLOCK_5_NUMBER                   (5U)
#define BLOCK_6_NUMBER                   (6U)
#define BLOCK_7_NUMBER                   (7U)
#define BLOCK_8_NUMBER                   (8U)
#define BLOCK_9_NUMBER                   (9U)
#define BLOCK_10_NUMBER                  (10U)

/*Size of a logical block in bytes. */

#define BLOCK_1_SIZE                    (20U)
#define BLOCK_2_SIZE                    (20U)
#define BLOCK_3_SIZE                    (20U)
#define BLOCK_4_SIZE                    (20U)
#define BLOCK_5_SIZE                    (200U)
#define BLOCK_6_SIZE                    (50U)
#define BLOCK_7_SIZE                    (4U)
#define BLOCK_8_SIZE                    (17U)
#define BLOCK_9_SIZE                    (25U)
#define BLOCK_10_SIZE                   (30U)

#define MAX_CONFIGURED_BLOCK_SIZE       BLOCK_5_SIZE

/*sum Size of all logical blocks in bytes. */
#define BLOCKS_SIZE                     (BLOCK_1_SIZE +BLOCK_2_SIZE+ BLOCK_3_SIZE+ BLOCK_4_SIZE+ BLOCK_5_SIZE+\
                                         BLOCK_6_SIZE +BLOCK_7_SIZE+ BLOCK_8_SIZE+ BLOCK_9_SIZE+ BLOCK_10_SIZE)

/*Size of a logical block in bytes. */
#define  BLOCK_1_WRITE_CYCLES            (32U)




#endif
