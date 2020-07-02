
/*
 * Eep_Cfg.h
 *
 *  Created on: Feb 17, 2020
 *      Author: Sahar Elnagar
 */

#ifndef EA_CFG_H_
#define EA_CFG_H_

#include "MemIf_Types.h"
#include "Eep.h"
#include "NvM_Cbk.h"



/*******************************************************************************/
//  EaGeneral Container's configuration
/*******************************************************************************/

/*Switches the development error detection and notification on or off.
           true: detection and notification is enabled.
          false: detection and notification is disabled.
*/
#define EA_DEV_ERROR_DETECT                 (1U)

/*The period between successive calls to the main function in seconds.*/
#define EA_MAIN_FUNCTION_PERIOD

/*Mapped to the job end notification routine provided by the upper layer module
 * (NvM_JobEndNotification).
 */
#define EA_NNM_JOB_END_NOTIFICATION         (1U)

/*Mapped to the job error notification routine provided by the
 * upper layer module (NvM_JobErrorNotification).
 */
#define EA_NVM_JOB_ERROR_NOTIFICATION       (1U)
/*Pre-processor switch to enable and disable the polling mode for this module.
    true: Polling mode enabled, callback functions (provided to EEP module) disabled.
    false: Polling mode disabled, callback functions (provided to EEP module) enabled
 */
#define EA_POLLING_MODE                     (1U)

/* Compile switch to enable / disable the function Ea_SetMode*/
#define EA_SET_MODE_SUPPORTED               (0U)

/* Pre-processor switch to enable / disable the API
 * to read out the modules version information.
 * true: Version info API enabled. false: Version info API disabled.
 */
#define EA_VERSION_INFO_API                 (0U)

/*The size in bytes to which logical blocks shall be aligned.
 * [SWS_Ea_00075] The configuration of the Ea module
 * shall be such that the virtual page size (defined in EA_VIRTUAL_PAGE_SIZE)
 *  is an integer multiple of the physical page size
 * */
/*My Note :Setting the virtual page size depends on
 *  how much data we want to combine together as a unit
 *  */
#define EA_VIRTUAL_PAGE_SIZE                PHYSICAL_WORD_SIZE

/*******************************************************************************/
// EaBlockConfiguration  Container's configuration
//Configuration of block specific parameters for the EEPROM abstraction module.
/*******************************************************************************/

/*Number of blocks configured*/
#define EA_BLOCKS_NUM                           (2U)

/*Block identifier (handle). 0x0000 and 0xFFFF shall not be used for block numbers (see SWS_Ea_00006).
  Range:   min = 2^NVM_DATASET_SELECTION_BITS
  max = 0xFFFF -2^NVM_DATASET_SELECTION_BITS
  Note: Depending on the number of bits set aside for dataset selection several other block numbers
  shall also be left out to ease implementation.
*/
#define EA_BLOCK_0_NUMBER                       (1U)
#define EA_BLOCK_1_NUMBER                       (5U)


/*Size of a logical block in bytes. */
#define  EA_BLOCK_0_SIZE                        (32U)
#define  EA_BLOCK_1_SIZE                        (16U)


/*  Marker for high priority data. true: Block contains immediate data.
 *  false: Block does not contain immediate data.
 */
#define EA_BLOCK_0_IMMEDIATE_DATA             (0U)             /*False*/
#define EA_BLOCK_1_IMMEDIATE_DATA             (0U)             /*False*/

/*Reference to the device this block is stored in*/
#define  EA_DEVICE_INDEX                      (1U)

/* ECUC_Ea_00122 :   EaNvmJobErrorNotification
 * Mapped to the job error notification routine provided by the upper layer
 * module (NvM_JobErrorNotification).*/
#define EaNvmJobErrorNotification           NvM_JobErrorNotification

/* ECUC_Ea_00121 :  EaNvmJobEndNotification
 * Mapped to the job end notification routine provided by the upper layer
 * module (NvM_JobEndNotification) */
#define EaNvmJobEndNotification             NvM_JobEndNotification
#endif /* EEP_CFG_H_ */
