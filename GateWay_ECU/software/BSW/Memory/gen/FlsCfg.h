/*******************************************************************************
**                                                                            **
**  FILENAME     : FlsCfg.h                                                   **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2019-12-1                                                  **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Yomna Mokhtar                                              **
**                                                                            **
**                                                                            **
*******************************************************************************/
#ifndef FLS_CFG_H_
#define FLS_CFG_H_
	
#include "MemIf_Types.h"


//Flash access code loaded on job start / unloaded on job end or error.
#define FlsAcLoadOnJobStart 							STD_ON

//the lower boundary for read / write / erase and compare jobs.
#define FlsBaseAddress										0x0

//enable/disable the Fls_BlankCheck function.
#define FlsBlankCheckApi									STD_ON

//enable and disable the Fls_Cancel function.
#define FlsCancelApi											STD_ON

//enable and disable the Fls_Compare function.
#define FlsCompareApi											STD_ON

//Switches the development error detection and notification on or off
#define FlsDevErrorDetect									STD_ON

//Index of the driver, used by FEE
#define FlsDriverIndex	

//Compile switch to enable erase verification
#define FlsEraseVerificationEnabled				        STD_OFF

//Compile switch to enable and disable the Fls_GetJobResult function.
#define FlsGetJobResultApi								STD_ON

//Compile switch to enable and disable the Fls_GetStatus function
#define FlsGetStatusApi									STD_ON

//Compile switch to enable and disable the Fls_SetMode function
#define FlsSetModeApi									STD_ON

//Compile switch to enable timeout supervision
#define FlsTimeoutSupervisionEnabled 			STD_ON

//The total amount of flash memory in bytes
#define FlsTotalSize										  0x00040000

/*true: Job processing triggered by interrupt (hardware controlled)
false: Job processing not triggered by interrupt (software controlled)
or the underlying hardware does not support interrupt mode for flash operations
*/
#define FlsUseInterrupts 									STD_OFF

//switch to enable / disable the API to read out the modules version information
#define FlsVersionInfoApi									STD_ON

//switch to enable write verification
#define FlsWriteVerificationEnabled				STD_ON

/*******************************************************************************************/

//switch to enable/disable access the erase flash access code
#define	FlsAcEraseEnable									STD_OFF

//switch to enable/disable access the write flash access code
#define	FlsAcWriteEnable									STD_OFF

//switch to enable/disable configuring cycle time of calls of the flash driver's main function
#define	FlsCallCycleEnable								STD_OFF

//switch to enable/disable configuring default mode of flash driver
#define	FlsDefaultModeEnable							STD_ON

//switch to enable/disable Job End Notification
#define	FlsJobEndNotificationEnable				STD_ON

//switch to enable/disable Job Error Notification
#define	FlsJobErrorNotificationEnable			STD_ON

//switch to enable/disable configuring max number of bits to read or compare in one cycle in fast mode
#define	FlsMaxReadFastModeEnable					STD_ON

//switch to enable/disable configuring max number of bits to read or compare in one cycle in normal mode
#define	FlsMaxReadNormalModeEnable				STD_ON

//switch to enable/disable configuring max number of bits to write in one cycle in fast mode
#define	FlsMaxWriteFastModeEnable					STD_ON

//switch to enable/disable configuring max number of bits to write in one cycle in normal mode
#define	FlsMaxWriteNormalModeEnable				STD_ON

//switch to enable/disable configuring Erase/Write protection settings
#define	FlsProtectionEnable								STD_ON

/*******************************************************************************************/

//Number of continuous sectors with identical values for FlsSectorSize and FlsPageSize
#define FlsNumberOfSectors

//Size of one page of this sector = 4 byte
#define FlsPageSize											 0x00000004

//Size of one sector = 1 KB = 1024 byte
#define FlsSectorSize  									    0x00000400

//Start address of the first sector
#define FlsSectorStartaddress							0



#endif
