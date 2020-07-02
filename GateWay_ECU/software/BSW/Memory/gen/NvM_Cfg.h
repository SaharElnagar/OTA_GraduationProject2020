/*
 * NvM_Cfg.h
 *
 *
 *      Author: Sahar
 */

#ifndef NVM_CFG_H_
#define NVM_CFG_H_

#include "Std_types.h"
#include "Fee_Cfg.h"

/*****************************************************************************************/
/*                                  NvMCommon Container                                  */
/*****************************************************************************************/

/*ECUC_NvM_00491
 *Preprocessor switch to enable some API calls which are related to NVM API configuration classes.
 */
#define NVM_API_CONFIG_CLASS

/*ECUC_NvM_00550
 *This parameter specifies whether BswM is informed about the current status
 *of the multiblock job. True: call BswM_NvM_CurrentJobMode if ReadAll and WriteAll are started,
 *finished, canceled  False: do not inform BswM at all
 */
#define NVM_BSWM_MULTIBLOCK_JOBSTATUS_INFORMATION

/*ECUC_NvM_00492
 *Configuration ID regarding the NV memory layout.
 *This configuration ID shall be published as e.g.
 *a SW-C shall have the possibility to write it to NV memory*/
#define NVM_COMPILED_CONFIG_ID

/*ECUC_NvM_0049
 *If CRC is configured for at least one NVRAM block,
 *this parameter defines the maximum number of bytes which shall
 * be processed within one cycle of job processing*/
#define NVM_CRC_NUM_OF_BYTES

/*ECUC_NvM_00494
 *Defines the number of least significant bits which shall be used to address a certain data set of a
 *NVRAM block within the interface to the memory hardware abstraction. 0..8:
 *Number of bits which are used for dataset or redundant block addressing.
 *0: No dataset or redundant NVRAM blocks are configured at all, no selection bits required.
 *1: In case of redundant NVRAM blocks are configured, but no dataset NVRAM blocks.
 */
#define NVM_DATASET_SELECTION_BITS              (0x02U)

/*ECUC_NvM_00495
 *Switches the development error detection and notification on or off.
 * true: detection and notification is enabled.
 * false: detection and notification is disabled.
 */
#define NVM_DEV_ERROR_DETECT                    STD_ON

/*ECUC_NvM_00496
 *Preprocessor switch to enable switching memory drivers to fast mode during performing
 *NvM_ReadAll and NvM_WriteAll
 *true: Fast mode enabled.
 *false: Fast mode disabled.
 */
#define NVM_DRV_MODE_SWITCH

/*ECUC_NvM_00497
 *Preprocessor switch to enable the dynamic configuration management handling by the NvM_ReadAll request.
 *true: Dynamic configuration management handling enabled.
 *false: Dynamic configuration management handling disabled.
 *This parameter affects all NvM processing related to Block with ID 1 and all
 *processing related to Resistant to Changed Software. If the Dynamic Configuration is disabled,
 *Block 1 cannot be used by NvM*/
#define NVM_DYNAMIC_CONFIGURATIONS

/*ECUC_NvM_00498
 *Preprocessor switch to enable job prioritization handling
 *true: Job prioritization handling enabled.
 *false: Job prioritization handling disabled.
 */
#define NVM_JOB_PRIORITIZATION       STD_ON

/* ECUC_NvM_00500
 * Entry address of the common callback routine which shall be invoked on termination
 * of each asynchronous multi block request
 */
#define NVM_MULTI_BLOCK_CALLBACK

/* ECUC_NvM_00501
 * Preprocessor switch to enable/disable the polling mode in the NVRAM Manager and at the same time disable/enable the callback functions useable by lower layers
 * true: Polling mode enabled, callback function usage disabled.
 * false: Polling mode disabled, callback function usage enabled.*/
#define NVM_POLLING_MODE                STD_OFF

/* ECUC_NvM_00518
 * Defines the number of retries to let the application copy data to or from the
 */
#define NVM_REPEAT_MIRROR_OPERATIONS

/* ECUC_NvM_00502
 * Preprocessor switch to enable the API NvM_SetRamBlockStatus.
 * true: API NvM_SetRamBlockStatus enabled.
 * false: API NvM_SetRamBlockStatus disabled.*/
#define NVM_SET_RAM_BLOCK_STATUS_API

/* ECUC_NvM_00503
 * Defines the number of queue entries for the immediate priority job queue.
 *  If NVM_JOB_PRIORITIZATION is switched OFF this parameter shall be out of scope.
 */
#define NVM_SIZE_IMMEDIATE_JOB_QUEUE        (100U)

/* ECUC_NvM_00504
 * Defines the number of queue entries for the standard job queue
 */
#define NVM_SIZE_STANDARD_JOB_QUEUE         (10U)

/* ECUC_NvM_00500
 * Entry address of the common callback routine which shall be invoked on termination
 * of each asynchronous multi block request
 */
#define NvMMultiBlockCallback               STD_OFF

#define MAX_NVM_BLOCK_SIZE                  (100U)
/*****************************************************************************************/
/*                                 NvMBlockDescriptor                                    */
/*****************************************************************************************/

#define NUMBER_OF_NVM_BLOCKS                (5U)

#define CRC_SIZE                            (4U)
/*ECUC_NvM_00481
 * Identification of a NVRAM block via a unique block identifier. Implementation Type: NvM_BlockIdType.
 *min = 2 max = 2^(16- NVM_DATASET_SELECTION_BITS)-1
 *Reserved NVRAM block IDs: 0 -> to derive multi block request results via
 *NvM_GetErrorStatus 1 -> redundant NVRAM block which holds the configuration ID
 *(generation tool should check that this block is correctly configured from type,CRC and size point of view)
 */

/***************************BLOCK_2 CFG********************************/
#define NVM_NVRAM_BLOCK_2_ID                (2U)
#define NVM_NVRAM_BLOCK_2_BASENUMBER        (BLOCK_4_NUMBER>>NVM_DATASET_SELECTION_BITS)
#define NVM_NVRAM_BLOCK_2_LENGTH            (BLOCK_1_SIZE-4)

/***************************BLOCK_3 CFG********************************/
#define NVM_NVRAM_BLOCK_3_ID                (3U)
#define NVM_NVRAM_BLOCK_3_BASENUMBER        (BLOCK_2_NUMBER>>NVM_DATASET_SELECTION_BITS)
#define NVM_NVRAM_BLOCK_3_LENGTH            (BLOCK_2_SIZE-4)

/***************************BLOCK_4 CFG********************************/
#define NVM_NVRAM_BLOCK_4_ID                (4U)
#define NVM_NVRAM_BLOCK_4_BASENUMBER        (BLOCK_9_NUMBER>>NVM_DATASET_SELECTION_BITS)
#define NVM_NVRAM_BLOCK_4_LENGTH            (BLOCK_9_SIZE-4)


#define MAX_NVM_BLOCK_SIZE                  (100U)

#endif /* BSW_GEN_NVM_CFG_H_ */
