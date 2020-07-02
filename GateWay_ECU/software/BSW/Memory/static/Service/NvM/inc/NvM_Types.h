/*
 * NvM_Types.h
 *
 *
 *      Author: Sahar
 */

#ifndef BSW_STATIC_SERVICE_NVM_TYPES_H_
#define BSW_STATIC_SERVICE_NVM_TYPES_H_

#include "Std_Types.h"
                /*****************************************************************************************/
                /*                   Implementation Data Types                                           */
                /*****************************************************************************************/
typedef uint8 NvMBlockCrcType ;
#define NVM_CRC16                   ((NvMBlockCrcType)0U)
#define NVM_CRC32                   ((NvMBlockCrcType)1U)
#define NVM_CRC8                    ((NvMBlockCrcType)2U)

typedef uint8 NvMBlockManagementType ;
#define NVM_BLOCK_DATASET           ((NvMBlockManagementType)0U)
#define NVM_BLOCK_NATIVE            ((NvMBlockManagementType)1U)
#define NVM_BLOCK_REDUNDANT         ((NvMBlockManagementType)2U)

/*[SWS_NvM_00470]
  This is an asynchronous request result returned by the API service NvM_GetErrorStatus.
  The availability of an asynchronous request result can be additionally signaled via a callback function.
 */
typedef uint8 NvM_RequestResultType ;
#define NVM_REQ_OK                  ((NvM_RequestResultType)0U)
#define NVM_REQ_NOT_OK              ((NvM_RequestResultType)1U)
#define NVM_REQ_PENDING             ((NvM_RequestResultType)2U)
#define NVM_REQ_INTEGRITY_FAILED    ((NvM_RequestResultType)3U)
#define NVM_REQ_BLOCK_SKIPPED       ((NvM_RequestResultType)4U)
#define NVM_REQ_NV_INVALIDATED      ((NvM_RequestResultType)5U)
#define NVM_REQ_CANCELED            ((NvM_RequestResultType)6U)
#define NVM_REQ_RESTORED_FROM_ROM   ((NvM_RequestResultType)8U)

/*[SWS_NvM_00471]
  Identification of a NVRAM block via a unique block identifier.
  Reserved NVRAM block IDs: 0 -> to derive multi block request results via NvM_GetErrorStatus
                            1 -> redundant NVRAM block which holds the configuration ID
 */
typedef uint16 NvM_BlockIdType ;

/* Since this type is used for compliance purposes only
 * (meaning that NvM_Init will now have a pointer to this type as parameter, based on SWS_BSW_00047)
 * it will be left to the developer to chose how to implement it, considering it has no use for the
 * NvM module in any way
 */
typedef void NvM_ConfigType ;

/*[SWS_NvM_00848]*/
typedef const void* ConstVoidPtr;

typedef struct
{
    uint8 bla;
   // NvMEaRefType*NvMEaRef ;
   // NvMFeeRefTYpe*NvMFeeRef ;
}NvMTargetBlockReferenceType ;

/*****************************************************************************************/
/* ECUC_NvM_00061 :  NvMBlockDescriptor
 * Container for a management structure to configure the composition of a given
 * NVRAM Block Management Type. Its multiplicity describes the number of configured
 * NVRAM blocks, one block is required to be configured. The NVRAM block descriptors
 *  are condensed in the NVRAM block descriptor table.
*****************************************************************************************/
typedef struct
{
    /*ECUC_NvM_00476
     *Defines CRC data width for the NVRAM block.
     *Default: NVM_CRC16, i.e. CRC16 will be used if NVM_BLOCK_USE_CRC==true */
    NvMBlockCrcType         NvMBlockCrcType  ;

    /*ECUC_NvM_00477
     *Defines the job priority for a NVRAM block (0 = Immediate priority).*/
    uint8                   NvMBlockJobPriority  ;

    /*ECUC_NvM_00062
     *Defines the block management type for the NVRAM block*/
    NvMBlockManagementType  NvMBlockManagement   ;

    /*ECUC_NvM_00557
     *Defines whether the RAM Block shall be auto validated during shutdown phase.
     *true: if auto validation mechanism is used,
     *false: otherwise
     */
    boolean                 NvMBlockUseAutoValidation ;

    /*ECUC_NvM_00036
     *Defines CRC usage for the NVRAM block, i.e. memory space for CRC is reserved in RAM and NV memory.
      true: CRC will be used for this NVRAM block.
      false: CRC will not be used for this NVRAM block.
     */
    boolean                 NvMBlockUseCrc  ;

    /*ECUC_NvM_00556
     *Defines whether the CRC of the RAM Block shall be compared during a write job
     *with the CRC which was calculated during the last successful read or write job.
     *true: if compare mechanism is used,
     *false: otherwise */
    boolean                 NvMBlockUseCRCCompMechanism ;

    /*ECUC_NvM_00552
     Defines if NvMSetRamBlockStatusApi shall be used for this block or not.
     Note: If NvMSetRamBlockStatusApi is disabled this configuration parameter shall be ignored.
     true: calling of NvMSetRamBlockStatus for this RAM block shall set the status of the RAM block.
     false: calling of NvMSetRamBlockStatus for this RAM block shall be ignored
     */
    boolean                 NvMBlockUseSetRamBlockStatus  ;

    /*ECUC_NvM_00519
     Defines whether an explicit synchronization mechanism with a RAM mirror
     and callback routines for transferring data to and from NvM module's
     RAM mirror is used for NV block. true if synchronization mechanism is used,
     false otherwise.*/
    boolean                 NvMBlockUseSyncMechanism ;

    /*ECUC_NvM_00033
      Defines an initial write protection of the NV block
      true: Initial block write protection is enabled.
      false: Initial block write protection is disabled. */
    boolean                 NvMBlockWriteProt  ;

    /*ECUC_NvM_00551
      This parameter specifies whether BswM is informed about the current status of the specified block.
      True: Call BswM_NvM_CurrentBlockMode on changes
      False: Don't inform BswM at all*/
    boolean                 NvMBswMBlockStatusInformation;

    /*ECUC_NvM_00119
      Defines CRC (re)calculation for the permanent RAM block or NVRAM blocks which are
      configured to use explicit synchronization mechanism. true: CRC will be (re)calculated
      for this permanent RAM block.
      false: CRC will not be (re)calculated for this permanent RAM block.
     */
    boolean                 NvMCalcRamBlockCrc ;

    /*ECUC_NvM_00116
      Entry address of a block specific callback routine which shall be called if no
      ROM data is available for initialization of the NVRAM block. If not configured,
      no specific callback routine shall be called for initialization of the NVRAM block with default data.
     */
    Std_ReturnType (*NvMInitBlockCallback)(void)  ;

    /*ECUC_NvM_00533
      Defines the maximum number of read retries.
     */
    uint8                   NvMMaxNumOfReadRetries ;

    /*ECUC_NvM_00499
      Defines the maximum number of write retries for a NVRAM block with [ECUC_NvM_00061].
      Regardless of configuration a consistency check (and maybe write retries) are always
      forced for each block which is processed by the request NvM_WriteAll and NvM_WriteBlock
      */
    uint8                   NvMMaxNumOfWriteRetries ;

    /*ECUC_NvM_00478
      Configuration parameter to perform the link between the NVM_NVRAM_BLOCK_IDENTIFIER used by
      the SW-Cs and the FEE_BLOCK_NUMBER expected by the memory abstraction modules.
      The parameter value equals the FEE_BLOCK_NUMBER or EA_BLOCK_NUMBER shifted to the
      right by NvMDatasetSelectionBits bits. (ref. to chapter 7.1.2.1). Calculation
      Formula: value = TargetBlockReference.[Ea/Fee]BlockConfiguration.[Ea/Fee]BlockNumber >> NvMDatasetSelectionBits
     */
    uint16                  NvMNvBlockBaseNumber    ;

    /*ECUC_NvM_00479
      Defines the NV block data length in bytes.*/
    uint16                  NvMNvBlockLength       ;

    /*ECUC_NvM_00480
       Defines the number of multiple NV blocks in a contiguous area according to the given block management type.
       1-255 For NVRAM blocks to be configured of block management type NVM_BLOCK_DATASET.
       The actual range is limited according to SWS_NvM_00444.
       1 For NVRAM blocks to be configured of block management type NVM_BLOCK_NATIVE
       2 For NVRAM blocks to be configured of block management type NVM_BLOCK_REDUNDANT
     */
    uint8                   NvMNvBlockNum         ;

    /*ECUC_NvM_00035
      Defines the NVRAM device ID where the NVRAM block is located.
      Calculation Formula: value = TargetBlockReference.
      [Ea/Fee]BlockConfiguration.[Ea/Fee]DeviceIndex
     */
    uint8                   NvMNvramDeviceId      ;

    /*ECUC_NvM_00482
     Defines the start address of the RAM block data.
     If this is not configured, no permanent RAM data block is
     available for the selected block management type.
     */
    uint8*                 NvMRamBlockDataAddress  ;

    /* ECUC_NvM_00521
     * Entry address of a block specific callback routine which shall be called in order
     * to let the application copy data from the NvM module's mirror to RAM block.
     */
    Std_ReturnType (* NvMReadRamBlockFromNvCallback)(const void* NvMBuffer) ;

    /*
      ECUC_NvM_00483
      Defines whether a NVRAM block shall be treated resistant to configuration changes or not.
      If there is no default data available at configuration time then the application shall
      be responsible for providing the default initialization data. In this case the application
      has to use NvM_GetErrorStatus()to be able to distinguish between first initialization and corrupted data.
      true: NVRAM block is resistant to changed software. false: NVRAM block is not resistant to changed software.
     */
    boolean                 NvMResistantToChangedSw  ;

    /*ECUC_NvM_00484
      Defines the start address of the ROM block data.
      If not configured, no ROM block is available for the selected block management type.
     */
    uint8*                  NvMRomBlockDataAddress   ;

    /*ECUC_NvM_00485
     Defines the number of multiple ROM blocks in a contiguous area according to the given block management type.
     0-254 For NVRAM blocks to be configured of block management type NVM_BLOCK_DATASET.
     The actual range is limited according to SWS_NvM_00444.
     0-1 For NVRAM blocks to be configured of block management type NVM_BLOCK_NATIVE
     0-1 For NVRAM blocks to be configured of block management type NVM_BLOCK_REDUNDANT
     */
    uint8                   NvMRomBlockNum          ;

    /*ECUC_NvM_00558
     Defines whether a block will be processed or not by NvM_FirstInitAll. A block can be configured to be processed even if it doesn't have permanent RAM and/or explicit synchronization.
     TRUE: block will be processed by NvM_FirstInitAll
     FALSE: block will not be processed by NvM_FirstInitAll
     */
    boolean                 NvMSelectBlockForFirstInitAll  ;

    /*ECUC_NvM_00117
      Defines whether a NVRAM block shall be processed during NvM_ReadAll or not.
      This configuration parameter has only influence on those NVRAM blocks which are configured to
      have a permanent RAM block or which are configured to use explicit synchronization mechanism.
      true: NVRAM block shall be processed by NvM_ReadAll false: NVRAM block shall not be processed by NvM_ReadAll
     */
    boolean                 NvMSelectBlockForReadAll  ;

    /*ECUC_NvM_00549
      Defines whether a NVRAM block shall be processed during NvM_WriteAll or not.
      This configuration parameter has only influence on those NVRAM blocks which are configured to
      have a permanent RAM block or which are configured to use explicit synchronization mechanism.
      true: NVRAM block shall be processed by NvM_WriteAll
      false: NVRAM block shall not be processed by NvM_WriteAll
     */
    boolean                 NvMSelectBlockForWriteAll  ;

    /*ECUC_NvM_00506
      Entry address of the block specific callback routine which shall be invoked on
      termination of each asynchronous single block request
     */
    Std_ReturnType (*NvMSingleBlockCallback)(uint8 ServiceId,NvM_RequestResultType JobResult );

    /*ECUC_NvM_00532
     *Defines if the Static Block ID check is enabled.
     false: Static Block ID check is disabled.
     true: Static Block ID check is enabled.
     */
    boolean                 NvMStaticBlockIDCheck  ;

    /*ECUC_NvM_00072*/
    boolean                 NvMWriteBlockOnce      ;

    /*
     ECUC_NvM_00520
     Entry address of a block specific callback routine which shall be called in order
     to let the application copy data from RAM block to NvM module's mirror.
     Implementation type: Std_ReturnType */
    Std_ReturnType (*NvM_WriteRamBlockToNvm)(void* NvMBuffer);

    /*ECUC_NvM_00534
       Defines if Write Verification is enabled.
       false: Write verification is disabled.
       true: Write Verification is enabled. */
    boolean                  NvMWriteVerification  ;

    /*ECUC_NvM_00538
     *Defines the number of bytes to compare in each step when comparing
      the content of a RAM Block and a block read back.*/
    uint16                   NvMWriteVerificationDataSize ;

    /*This parameter is just a container for the parameters for EA and FEE*/
    NvMTargetBlockReferenceType*     NvMTargetBlockReference;
}NvMBlockDescriptorType ;


#endif /* BSW_STATIC_SERVICE_NVM_TYPES_H_ */
