/*******************************************************************************
**                                                                            **
**  FILENAME             : NvM.c                                              **
**                                                                            **
**  AUTOSAR VERSION      : 4.3.1                                              **
**                                                                            **
**                                                                            **
**  PLATFORM             : TIVA C                                             **
**                                                                            **
**  AUTHOR               : Yomna Mokhtar , Sahar Elnagar                      **
**                                                                            **
**                                                                            **
*******************************************************************************/

/*****************************************************************************************/
/*                                   Include headers                                     */
/*****************************************************************************************/

#include "NvM.h"
#include "Det.h"
#include "NvM_PrivateTypes.h"
#include "Queue.h"
#include "MemIf.h"
#include "MemIf_Types.h"


/*****************************************************************************************/
/*                               Local Macros Definition                                 */
/*****************************************************************************************/

#define CRC_POLYNOMIAL              (0x04C11DB7)

/*Permenant Ram block status*/
#define INVALID_UNCHANGED           ((PRamStatusType)0U)
#define VALID_UNCHANGED             ((PRamStatusType)1U)
#define VALID_CHANGED               ((PRamStatusType)2U)


/*Permenant Ram block status*/
#define INVALID_UNCHANGED           ((PRamStatusType)0U)
#define VALID_UNCHANGED             ((PRamStatusType)1U)
#define VALID_CHANGED               ((PRamStatusType)2U)

#define IDLE_JOB                    (0xFF)
#define CRC_PADDING_KEY             (0xFFFFFFFFU)

/*Empty Queue size*/
#define EMPTY_QUEUE                 (0U)

#define  CRC_SIZE                   (4U)
/*Module States*/
#define MODULE_UNINITIALIZED        ((ModuleStateType)0U)
#define INIT_DONE                   ((ModuleStateType)1U)

#define IDLE_REQUEST                (0xFFU)

/*Block Type of the non volatile memory*/
#define NV_BLOCK                    (0U)
#define ROM_BLOCK                   (1U)

/*Read Block states */
#define GET_MEMIF_BLOCK_ID          (10U)
#define READ_NV_BLOCK               (1U)
#define READ_ROM_BLOCK              (2U)
#define CHECK_CRC                   (3U)
#define END_JOB                     (4U)

/*Write Block states*/
#define CALC_CRC                    (0U)
#define WRITE_NV_BLOCK              (1U)
#define WRITE_OK                    (2U)
#define WRITE_FAILED                (3U)
#define WRITE_END                   (4U)

/*Invalidate Block states*/
#define INVALIDATE_NV_BLOCK         (0U)
#define INVALIDATE_OK               (1U)
#define INVALIDATE_FAILED           (2U)
#define INVALIDATE_END              (3U)

/*Restore from default states*/
#define RESTORE_FROM_ROM            (0U)

/*Main function states*/
#define GET_JOB                     (0U)
#define EXECUTE_SINGLE_JOB          (1U)
#define EXECUTE_CRC_JOB             (2U)
#define EXECUTE_MULTI_JOB           (3U)

/*CRC calculations states*/
#define CALC_ALIGNED_CRC            (1U)
#define ADD_CRC_PAD                 (2U)
#define FILL_TEMP_BUFFER            (3U)

/*****************************************************************************************/
/*                                   Local variables Definition                          */
/*****************************************************************************************/

/*Administrative block for each NVRAM block
 *The “Administrative Block” is a “Basic Storage Object”.
 *It resides in RAM. The  “Administrative Block” is a mandatory part of a “NVRAM Block”.
 */
static AdministrativeBlockType AdministrativeBlock[NUMBER_OF_NVM_BLOCKS];

static MultiBlockRequestType MultiBlcokRequest ;

static uint16 BlockNumber ;
static MemIf_JobResultType MemIf_JobResult ;

#if(NVM_POLLING_MODE == STD_OFF)
/*save end job status from underlying layer Successful or failed*/
static EndJobStatusType EndJobStatus ;
#endif

/*Variable to save module state*/
static ModuleStateType ModuleState = MODULE_UNINITIALIZED ;

/*Variable to hold the current job being processed by the main function*/
static Job_Parameters Current_Job ;

/*Variable to store the next internal state for the crc job*/

static uint32 CRCTempbuffer;
static uint8 TempBuffer[MAX_NVM_BLOCK_SIZE+ CRC_SIZE] ;


/*****************************************************************************************/
/*                                   extern variables                                    */
/*****************************************************************************************/

/*Blocks configurations */
extern NvMBlockDescriptorType NvMBlockDescriptor[NUMBER_OF_NVM_BLOCKS] ;

extern Job_Parameters Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE] ;
extern Queue_Indices_Struct Stand_Queue_Indeces ;
#if (NVM_JOB_PRIORITIZATION == STD_ON)
  extern Job_Parameters Immediate_Job_Queue[NVM_SIZE_IMMEDIATE_JOB_QUEUE] ;
  extern Queue_Indices_Struct Immed_Queue_Indeces  ;
#endif

extern NvM_BlockIdType CRC_Job_Queue[NVM_SIZE_CRC_JOB_QUEUE] ;
extern Queue_Indices_Struct CRC_Queue_Indeces  ;

extern boolean Standard_Queue_Empty ;
extern boolean Standard_Queue_FULL ;

#if (NVM_JOB_PRIORITIZATION == STD_ON)
extern boolean Immediate_Queue_Empty  ;
extern boolean Immediate_Queue_FULL  ;
#endif

extern boolean CRC_Queue_Empty ;
extern boolean CRC_Queue_Full ;

/*****************************************************************************************/
/*                                   Local Functions Prototypes                          */
/*****************************************************************************************/

static void NvM_MainFunction_ReadBlock(void) ;
static void NvM_MainFunction_WriteBlock(void) ;
static void NvM_MainFunction_InvalidateBlock(void) ;
static void NvM_MainFunction_WriteAll( void ) ;
static void NvM_MainFunction_ReadAll(void) ;
static Std_ReturnType CalculateCRC(uint16 blockId ,uint8* RamAddress) ;
static void EmptyTempBuffer(uint16 Length,uint8* RAMBlock,uint32* crc) ;
static void NvM_MainFunction_CalcCRC(void) ;
static void ReadRomBlock(uint8* SourceAddress , uint8* TargetAddress,uint16 Length) ;
static uint32 Compute_CRC32_Simple(uint8* bytes,uint16 len);
static void NvM_MainFunction_RestoreBlockDefaults(void) ;
/*****************************************************************************************/
/*                                   Global Function Definition                          */
/*****************************************************************************************/



/****************************************************************************************/
/*    Function Name           : NvM_Init                                                */
/*    Function Description    : Service for resetting all internal variables.           */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00447                                           */
/*    Notes                   :[SWS_NvM_00881]The Configuration pointer ConfigPtr shall */
/*                             always have a NULL value.                            */
/****************************************************************************************/

void NvM_Init(const NvM_ConfigType* ConfigPtr )
{
    uint32 counter = 0;
    /*[SWS_NvM_00399] The function NvM_Init shall reset all internal variables,
     * e.g. the queues, request flags, state machines, to their initial values.
     * It shall signal “INIT DONE” internally, e.g. to enable job processing
     * and queue management. (SRS_BSW_00101, SRS_BSW_00406)
     */

    /*Initialize Queues*/
    Init_Queues() ;

    /*[SWS_NvM_00192]The function NvM_Init shall set the data set index
     * of all NVRAM blocks of type NVM_BLOCK_DATASET to zero.
     */
    for( counter = 0; counter < NUMBER_OF_NVM_BLOCKS ; counter++)
    {
        /*check if block type is data set*/
        if(NvMBlockDescriptor[counter].NvMBlockManagement == NVM_BLOCK_DATASET )
        {
            AdministrativeBlock[counter].DataSetIndex = 0 ;
        }

        /*Initialize block status in Administrative block*/
        AdministrativeBlock[counter].BlockResultStatus = NVM_REQ_OK ;

        /*After the Initialization the RAM Block is in state INVALID/UNCHANGED until it is
          updated via NvM_ReadAll,
         */
        AdministrativeBlock[counter].PRAMStatus        = INVALID_UNCHANGED ;
    }

    /*Initialize End job status*/
    #if(NVM_POLLING_MODE == STD_OF)
     EndJobStatus.EndJobFailed  = 0 ;
     EndJobStatus.EndJobSuccess = 0;
    #endif

    /*signal “INIT DONE” internally*/
    ModuleState = INIT_DONE ;
}


/****************************************************************************************/
/*    Function Name           : NvM_SetDataIndex                                        */
/*    Function Description    : Service for resetting all internal variables.           */
/*    Parameter in            : BlockId , DataIndex                                     */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00448                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_SetDataIndex(NvM_BlockIdType BlockId, uint8 DataIndex )
{
    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*Check if Module not internally initialized
     *[SWS_NvM_00707] The NvM module’s environment shall have initialized
     *the NvM module before it calls the function NvM_SetDataIndex.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_DATAINDEX_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*check if Block ID is not in range*/
    else if( (BlockId <=1 ) || (BlockId >= NUMBER_OF_NVM_BLOCKS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_DATAINDEX_API_ID , NVM_E_PARAM_BLOCK_ID ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00599] If development error detection is enabled for NvM module,
     * the function NvM_SetDataIndex shall report the DET error NVM_E_PARAM_BLOCK_DATA_IDX
     * when DataIndex parameter exceeds the total number of configured data sets
     */
    else if(DataIndex >= (1<<NVM_DATASET_SELECTION_BITS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_DATAINDEX_API_ID , NVM_E_PARAM_BLOCK_DATA_IDX ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*Check if block management type not data set
     *[SWS_NvM_00264] For blocks with block management different from NVM_BLOCK_DATASET,
     *NvM_SetDataIndex shall return without any effect in production mode.
     *Further, E_NOT_OK shall be returned
     */
    else if(NvMBlockDescriptor[BlockId].NvMBlockManagement != NVM_BLOCK_DATASET )
    {
        rtn_val = E_NOT_OK ;
    }
    /*
     [SWS_NvM_00598] If development error detection is enabled for NvM module,
     the function NvM_SetDataIndex shall report the DET error NVM_E_BLOCK_PENDING
     when NVRAM block identifier is already queued or currently in progress
     */
    else if(Search_Queue(BlockId) == E_OK)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_DATAINDEX_API_ID ,NVM_E_BLOCK_PENDING) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    else
    {
        /*Save data index in the administrative block*/
        AdministrativeBlock[BlockId].DataSetIndex = DataIndex ;
    }

    return rtn_val ;
}

/****************************************************************************************/
/*    Function Name           : NvM_GetDataIndex                                        */
/*    Function Description    : Service for getting the currently set DataIndex of      */
/*                              a data set NVRAM block                                  */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : DataIndexPtr                                            */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00449                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_GetDataIndex(NvM_BlockIdType BlockId, uint8* DataIndexPtr )
{
    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*Check if Module not internally initialized */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_DATAINDEX_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00605] If development error detection is enabled for NvM module,
     * the function NvM_GetDataIndex shall report the DET error NVM_E_PARAM_DATA
     * when a NULL pointer is passed via the parameter DataIndexPtr.
     */
    else if(DataIndexPtr == NULL)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_DATAINDEX_API_ID ,NVM_E_PARAM_DATA) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*check if Block ID is not in range*/
    else if( (BlockId <=1 ) || (BlockId >= NUMBER_OF_NVM_BLOCKS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_DATAINDEX_API_ID , NVM_E_PARAM_BLOCK_ID ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* [SWS_NvM_00265] For blocks with block management different from NVM_BLOCK_DATASET,
     * NvM_GetDataIndex shall set the index pointed by DataIndexPtr to zero. Further,
     * E_NOT_OK shall be returned.
     */
    else if(NvMBlockDescriptor[BlockId].NvMBlockManagement != NVM_BLOCK_DATASET )
    {
        rtn_val = E_NOT_OK ;
    }
    else
    {
        /*Save data index in the administrative block*/
         *DataIndexPtr = AdministrativeBlock[BlockId].DataSetIndex ;
    }
    return rtn_val ;
}


/****************************************************************************************/
/*    Function Name           : NvM_GetErrorStatus                                      */
/*    Function Description    : read the block dependent error/status information.      */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : RequestResultPtr                                        */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00451                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/

Std_ReturnType NvM_GetErrorStatus( NvM_BlockIdType BlockId, NvM_RequestResultType* RequestResultPtr)
{
    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*
     * [SWS_NvM_00710] The NvM module’s environment shall have initialized the NvM module before
     * it calls the function NvM_GetErrorStatus.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_ERROR_STATUS_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*check if the input parameter pointer is  NULL*/
    else if(RequestResultPtr == NULL)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_ERROR_STATUS_API_ID ,NVM_E_PARAM_POINTER) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    else
    {
        /*[SWS_NvM_00015] The function NvM_GetErrorStatus shall read the block dependent error/status information
          in the administrative part of a NVRAM block.
          The status/error information of a NVRAM block shall be set by a former or current asynchronous
          request.  (SRS_Mem_00020)
          */
        *RequestResultPtr = AdministrativeBlock[BlockId].BlockResultStatus ;
    }
    return rtn_val ;
}

/****************************************************************************************/
/*    Function Name           : NvM_RestoreBlockDefaults                                */
/*    Function Description    : Service to restore the default data to its corresponding*/
/*                              RAM block.                                              */
/*    Parameter in            : BlockId , NvM_DestPtr                                   */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement             : SWS_NvM_00456                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_RestoreBlockDefaults( NvM_BlockIdType BlockId, void* NvM_DestPtr )
{
    /*Variable to save the job request information*/
    Job_Parameters JobInfo ;

    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*[SWS_NvM_00625] If development error detection is enabled for NvM module,
     * the function NvM_RestoreBlockDefaults shall report the DET error NVM_E_NOT_INITIALIZED
     * when NVM is not yet initialized.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_RESTORE_BLOCKDEFAULTS_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00626] If development error detection is enabled for NvM module,
     * the function NvM_RestoreBlockDefaults shall report the DET error NVM_E_BLOCK_PENDING
     * when NVRAM block identifier is already queued or currently in progress.
     */
    else if(AdministrativeBlock[BlockId].BlockResultStatus == NVM_REQ_PENDING)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_RESTORE_BLOCKDEFAULTS_API_ID ,NVM_E_BLOCK_PENDING) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* [SWS_NvM_00629] If development error detection is enabled for NvM module,
     * the function NvM_RestoreBlockDefaults shall report the DET error NVM_E_PARAM_ADDRESS
     * when no permanent RAM block and no explicit synchronization are configured and a
     * NULL pointer is passed via the parameter NvM_DstPtr.
     */
    else if(NvM_DestPtr == NULL && NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress == NULL )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_RESTORE_BLOCKDEFAULTS_API_ID ,NVM_E_PARAM_ADDRESS) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* [SWS_NvM_00630] If development error detection is enabled for NvM module,
     * the function NvM_RestoreBlockDefaults shall report the DET error NVM_E_PARAM_BLOCK_ID
     * when the passed BlockID is out of range.
     */
    else if( (BlockId <=1 ) || (BlockId >= NUMBER_OF_NVM_BLOCKS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_RESTORE_BLOCKDEFAULTS_API_ID , NVM_E_PARAM_BLOCK_ID ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00885] If the block has no default data, it has no InitBlockCallbackFunction
     * configured and the development error detection is enabled then the NvM_RestoreBlockDefaults
     * API shall report the error NVM_E_BLOCK_WITHOUT_DEFAULTS error to the Det module.k
     */
    else if(NvMBlockDescriptor[BlockId].NvMInitBlockCallback == NULL &&\
            NvMBlockDescriptor[BlockId].NvMRomBlockDataAddress == NULL)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_RESTORE_BLOCKDEFAULTS_API_ID , NVM_E_BLOCK_WITHOUT_DEFAULTS ) ;
        #endif
        rtn_val = E_NOT_OK;
    }
    /*Accessing data index bigger than the configured number for ROM blocks
     */
    else if(NvMBlockDescriptor[BlockId].NvMBlockManagement == NVM_BLOCK_DATASET &&
            AdministrativeBlock[BlockId].DataSetIndex >= NvMBlockDescriptor[BlockId].NvMRomBlockNum)
    {
        rtn_val = E_NOT_OK ;
    }
    /*Return E_NOT_OK if Queue is FULL*/
  /*TODO  else if(Queue.size == NVM_SIZE_STANDARD_JOB_QUEUE)
    {
        rtn_val = E_NOT_OK ;
    }*/
    else
    {
        /*[SWS_NvM_00224] The function NvM_RestoreBlockDefaults shall take over the given parameters,
         * queue the request in the job queue and return.
         */
        JobInfo.Block_Id   = BlockId ;
        JobInfo.ServiceId  = NVM_RESTORE_BLOCKDEFAULTS_API_ID ;

        /*Check if the required RAM block is temporary or permanent*/
        if(NvM_DestPtr != NULL )
        {
            JobInfo.RAM_Ptr = (uint8*) NvM_DestPtr ;
        }
        else
        {
            JobInfo.RAM_Ptr  = NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress ;
        }

        /*[SWS_NvM_00227] The job of the function NvM_RestoreBlockDefaults shall
         * invalidate a RAM block before copying default data to the RAM if a
         * permanent RAM block is requested or before explicit synchronization callback
         */
        AdministrativeBlock[BlockId].PRAMStatus =INVALID_UNCHANGED ;

        JobInfo.Job_InternalState = RESTORE_FROM_ROM;

        /*Save job request as pending in the Administrative block*/
        AdministrativeBlock[BlockId].BlockResultStatus = NVM_REQ_PENDING ;

        /*Initialize End job status*/
        #if(NVM_POLLING_MODE == STD_OF)
         EndJobStatus.EndJobFailed  = 0 ;
         EndJobStatus.EndJobSuccess = 0;
        #endif

        /*Save job request in the queue , if queue Full it will return E_NOT_OK */
         Job_Enqueue(JobInfo) ;

    }
    return rtn_val ;
}

/****************************************************************************************/
/*    Function Name           : NvM_SetRamBlockResultStatus                             */
/*    Function Description    : Service for setting the RAM block status of a permanent */
/*                              RAM block                                               */
/*    Parameter in            : BlockId , BlockChanged                                  */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement             : SWS_NvM_00453                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_SetRamBlockResultStatus( NvM_BlockIdType BlockId, boolean BlockChanged )
{
    Std_ReturnType rtn_val =E_OK ;

    /*[SWS_NvM_00643] If development error detection is enabled for NvM module,
     * the function NvM_SetRamBlockResultStatus shall report the DET error NVM_E_NOT_INITIALIZED
     * when NVM not yet initialized.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_RAMBLOCK_STATUS_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00644] If development error detection is enabled for NvM module,
     * the function NvM_SetRamBlockResultStatus shall report the DET error NVM_E_BLOCK_PENDING
     * when NVRAM block identifier is already queued or currently in progress.
     */
    else if(AdministrativeBlock[BlockId].BlockResultStatus == NVM_REQ_PENDING)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_RAMBLOCK_STATUS_API_ID ,NVM_E_BLOCK_PENDING) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00645] If development error detection is enabled for NvM module,
     * the function NvM_SetRamBlockResultStatus shall report the DET error NVM_E_PARAM_BLOCK_ID
     * when the passed BlockID is out of range.
     */
    else if( (BlockId <=1 ) || (BlockId >= NUMBER_OF_NVM_BLOCKS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_RAMBLOCK_STATUS_API_ID , NVM_E_PARAM_BLOCK_ID ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*Check if it's not a permanent block */
    else if(NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress == NULL)
    {
        rtn_val = E_NOT_OK ;
    }
    else
    {
        /*check requested state */
        if(BlockChanged == TRUE)
        {
            /*[SWS_NvM_00406]  When the “BlockChanged” parameter passed to the function
             * NvM_SetRamBlockResultStatus is TRUE, the corresponding permanent RAM block or
             * the content of the RAM mirror in the NvM module ( in case of explicit synchronization)
             * is valid and changed.
             */
            AdministrativeBlock[BlockId].PRAMStatus = VALID_CHANGED ;

            /*[SWS_NvM_00121] For blocks with a permanently configured RAM,
             * the function NvM_SetRamBlockResultStatus shall request the recalculation of
             * CRC in the background, i.e. the CRC recalculation shall be processed by
             * the NvM_MainFunction, if the given “BlockChanged” parameter is TRUE and
             * CRC calculation in RAM is configured
             */
            if(NvMBlockDescriptor[BlockId].NvMCalcRamBlockCrc)
            {
                CRCJob_Enqueue(BlockId);
            }
        }
        else
        {
            /*[SWS_NvM_00405] When the “BlockChanged” parameter passed to the function
             * NvM_SetRamBlockResultStatus is FALSE the corresponding RAM block is either
             * invalid or unchanged (or both).
             */
            AdministrativeBlock[BlockId].PRAMStatus = INVALID_UNCHANGED ;
        }
    }

    return rtn_val ;
}


/****************************************************************************************/
/*    Function Name           : NvM_ReadBlock                                           */
/*    Function Description    : Service to copy the data of the NV block to its         */
/*                              corresponding RAM block                                 */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : NvM_DstPtr                                              */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00454                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_ReadBlock( NvM_BlockIdType BlockId, void* NvM_DstPtr )
{
    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*Variables used in calculations*/
    uint16 dataIndex , NvNumberOfBlocks ;

    /*Variable to save the job request information*/
    Job_Parameters JobInfo ;

    dataIndex = AdministrativeBlock[BlockId].DataSetIndex ;
    NvNumberOfBlocks = NvMBlockDescriptor[BlockId].NvMNvBlockNum ;

    /* Check if Module not internally initialized
     * [SWS_NvM_00614] If development error detection is enabled for NvM module,
     * the function NvM_ReadBlock shall report the DET error NVM_E_NOT_INITIALIZED
     * when NVM is not yet initialized.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_READBLOCK_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }

    /* [SWS_NvM_00615] If development error detection is enabled for NvM module,
     * the function NvM_ReadBlock shall report the DET error NVM_E_BLOCK_PENDING
     * when NVRAM block identifier is already queued or currently in progress.
     */
    else if(AdministrativeBlock[BlockId].BlockResultStatus == NVM_REQ_PENDING)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_READBLOCK_API_ID ,NVM_E_BLOCK_PENDING) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* [SWS_NvM_00616] If development error detection is enabled for NvM module,
     * the function NvM_ReadBlock shall report the DET error NVM_E_PARAM_ADDRESS
     * when no permanent RAM block and no explicit synchronization are configured
     * and a NULL pointer is passed via the parameter NvM_DstPtr.
     */
    else if((uint8*)NvM_DstPtr == NULL && NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress == NULL )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_READBLOCK_API_ID ,NVM_E_PARAM_ADDRESS) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* [SWS_NvM_00618] If development error detection is enabled for NvM module,
     * the function NvM_ReadBlock shall report the DET error NVM_E_PARAM_BLOCK_ID
     * when the passed BlockID is out of range.
     */
    else if( (BlockId <=1 ) || (BlockId >= NUMBER_OF_NVM_BLOCKS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_READBLOCK_API_ID , NVM_E_PARAM_BLOCK_ID ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* wrong data set value , return E_NOT_OK
     */
    else if(NvMBlockDescriptor[BlockId].NvMBlockManagement == NVM_BLOCK_DATASET &&\
            AdministrativeBlock[BlockId].DataSetIndex >= (1<<NVM_DATASET_SELECTION_BITS))
    {
        rtn_val = E_NOT_OK;
    }
    /*Return E_NOT_OK if the required index is in ROM and ROM address not configured*/
    else if(dataIndex>= NvNumberOfBlocks && NvMBlockDescriptor[BlockId].NvMRomBlockDataAddress == NULL )
    {
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00355] The job of the function NvM_ReadBlock shall not copy the NV
      block to the corresponding RAM block if the NVRAM block management type is
      NVM_BLOCK_DATASET and the NV block selected by the dataset index is
      invalidate.
     */
    else if(AdministrativeBlock[BlockId].BlockResultStatus == NVM_REQ_NV_INVALIDATED)
    {
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00651] The job of the function NvM_ReadBlock shall not copy the NV
      block to the corresponding RAM block if the NVRAM block management type is
      NVM_BLOCK_DATASET and the NV block selected by the dataset index is
      inconsistent.
     */
    else if(AdministrativeBlock[BlockId].BlockResultStatus == NVM_REQ_INTEGRITY_FAILED)
    {
        rtn_val = E_NOT_OK ;
    }
    /*Return E_NOT_OK if Queue is FULL*/
    else if(Standard_Queue_FULL)
    {
        rtn_val = E_NOT_OK ;
    }
    /*No detected errors*/
    else
    {
        /*[SWS_NvM_00195] The function NvM_ReadBlock shall take over the given
          parameters, queue the read request in the job queue and return.
         (SRS_Mem_00016)
         */
        JobInfo.Block_Id    = BlockId ;
        JobInfo.ServiceId = NVM_READBLOCK_API_ID ;

        /*Check if the required RAM block is temporary or permanent*/
        if(NvM_DstPtr != NULL )
        {
            JobInfo.RAM_Ptr = (uint8*) NvM_DstPtr ;
        }
        else
        {
            JobInfo.RAM_Ptr = NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress ;
        }

        JobInfo.Job_InternalState = GET_MEMIF_BLOCK_ID;

        /*Save job request as pending in the Administrative block*/
        AdministrativeBlock[BlockId].BlockResultStatus = NVM_REQ_PENDING ;

        /*Initialize End job status*/
        #if(NVM_POLLING_MODE == STD_OF)
         EndJobStatus.EndJobFailed  = 0 ;
         EndJobStatus.EndJobSuccess = 0;
        #endif

        /*Save job request in the queue , if queue Full it will return E_NOT_OK */
        rtn_val = Job_Enqueue(JobInfo) ;

        /*[SWS_NvM_00198] The function NvM_ReadBlock shall invalidate a permanent
         * RAM block immediately when the block is successfully enqueued or the
         * job processing starts, i.e. copying data from NV memory or ROM to RAM.
         * If the block has a synchronization callback (NvM_ReadRamBlockFromNvm)
         * configured the invalidation will be done just before NvMReadRamBlockFromNvM is called.
         */
        if(NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress != NULL)
        {
            AdministrativeBlock[BlockId].PRAMStatus = INVALID_UNCHANGED ;
        }
    }
    return rtn_val ;
}


/****************************************************************************************/
/*    Function Name           : NvM_WriteBlock                                          */
/*    Function Description    : Service to copy the data of the RAM block to its        */
/*                              corresponding NV block.                                 */
/*    Parameter in            : BlockId, NvM_SrcPtr                                     */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement             : SWS_NvM_00455                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType NvM_WriteBlock( NvM_BlockIdType BlockId, const void* NvM_SrcPtr )
{
    /*Variable to save return value*/
    Std_ReturnType Return_Val = E_OK ;
    Job_Parameters WriteJob ;

    /* [SWS_NvM_00619] If development error detection is enabled for NvM module,
     * the function NvM_WriteBlock shall report the DET error NVM_E_NOT_INITIALIZED
     * when NVM not yet initialized.
     */
    if(ModuleState == MODULE_UNINITIALIZED)
    {

       #if(NVM_DEV_ERROR_DETECT == STD_ON)
           Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_WRITEBLOCK_API_ID, NVM_E_NOT_INITIALIZED) ;
       #endif
       Return_Val = E_NOT_OK ;

    }
    /* [SWS_NvM_00620] If development error detection is enabled for NvM module,
     * the function NvM_WriteBlock shall report the DET error NVM_E_BLOCK_PENDING
     * when NVRAM block identifier is already queued or currently in progress.
     */
    else if(AdministrativeBlock[BlockId].BlockResultStatus == NVM_REQ_PENDING)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_WRITEBLOCK_API_ID ,NVM_E_BLOCK_PENDING) ;
        #endif
        Return_Val = E_NOT_OK ;
    }
    /* [SWS_NvM_00622] If development error detection is enabled for NvM module,
     * the function NvM_WriteBlock shall report the DET error NVM_E_PARAM_ADDRESS
     * when no permanent RAM block and no explicit synchronization are configured,
     * and a NULL pointer is passed via the parameter NvM_SrcPtr
     */
    else if((NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress == NULL) &&\
            (NvMBlockDescriptor[BlockId].NvMBlockUseSyncMechanism == FALSE) && (NvM_SrcPtr == NULL))
    {

        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_WRITEBLOCK_API_ID, NVM_E_PARAM_ADDRESS) ;
        #endif

    }
    /* [SWS_NvM_00624]
     * If development error detection is enabled for NvM module,
     * the function NvM_WriteBlock shall report the DET error NVM_E_PARAM_BLOCK_ID
     * when the passed BlockID is out of range
     */
    else if((BlockId <= 1) || (BlockId >= NUMBER_OF_NVM_BLOCKS))
    {

        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_WRITEBLOCK_API_ID, NVM_E_PARAM_BLOCK_ID) ;
        #endif
    }
    else
    {
        WriteJob.ServiceId = NVM_WRITEBLOCK_API_ID ;
        WriteJob.Block_Id = BlockId ;

        /*Save job request as pending in the Administrative block*/
        AdministrativeBlock[BlockId].BlockResultStatus = NVM_REQ_PENDING ;

        /* [SWS_NvM_00210]
         * If the function NvM_WriteBlock is provided with a valid RAM block address then it is used
         */
        if(NvM_SrcPtr != NULL){
            WriteJob.RAM_Ptr = (uint8*)NvM_SrcPtr ;
        }

        /*[SWS_NvM_00900]
         * If the function NvM_WriteBlock is provided with NULL as a RAM block address
         * and it has a permanent RAM block configured then the permanent RAM block is used.
         */
        else if(NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress != NULL){
            WriteJob.RAM_Ptr = (uint8*)NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress ;
        }

        /*[SWS_NvM_00901]
         * If the function NvM_WriteBlock is provided with NULL as a RAM block address
         * and it has the explicit synchronization configured then the explicit synchronization is used
         */
        else if (NvMBlockDescriptor[BlockId].NvMBlockUseSyncMechanism == TRUE){
            /*Use explicit sync*/
            /*Use RAM mirror pointer*/
        }

        Return_Val = Job_Enqueue(WriteJob) ;
    }

    return Return_Val;
}


/****************************************************************************************/
/*    Function Name           : NvM_InvalidateNvBlock                                   */
/*    Function Description    : Service to invalidate a NV block.                       */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement             : SWS_NvM_00459                                          */
/*    Notes                   :                                                         */
/****************************************************************************************/

Std_ReturnType NvM_InvalidateNvBlock( NvM_BlockIdType BlockId )
{
    /*Variable to save return value*/
    Std_ReturnType Return_Val = E_OK ;

    /* [SWS_NvM_00638]
     * If development error detection is enabled for NvM module,
     * the function NvM_InvalidateNvBlock shall report the DET error
     * NVM_E_NOT_INITIALIZED when NVM is not yet initialized.
     */
    if(ModuleState != INIT_DONE){

       #if(NVM_DEV_ERROR_DETECT == STD_ON)
           Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_INVALIDATEBLOCK_API_ID, NVM_E_NOT_INITIALIZED) ;
       #endif
       Return_Val = E_NOT_OK ;

    }

    /* [SWS_NvM_00639]
     * If development error detection is enabled for NvM module,
     * the function NvM_InvalidateNvBlock shall report the DET error
     * NVM_E_BLOCK_PENDING when NVRAM block identifier is already
     * queued or currently in progress.
     */
    else if(AdministrativeBlock[BlockId].BlockResultStatus == NVM_REQ_PENDING)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_INVALIDATEBLOCK_API_ID ,NVM_E_BLOCK_PENDING) ;
        #endif
        Return_Val = E_NOT_OK ;
    }

    /* [SWS_NvM_00642]
     * If development error detection is enabled for NvM module,
     * the function NvM_InvalidateNvBlock shall report the DET error
     * NVM_E_PARAM_BLOCK_ID when the passed BlockID is out of range.
     */
    else if((BlockId <= 1) || (BlockId >= NUMBER_OF_NVM_BLOCKS)){

       #if(NVM_DEV_ERROR_DETECT == STD_ON)
           Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_INVALIDATEBLOCK_API_ID, NVM_E_PARAM_BLOCK_ID) ;
       #endif
       Return_Val = E_NOT_OK ;
    }
    /* [SWS_NvM_00664]
     * The function NvM_InvalidateNvBlock shall return with E_NOT_OK if a ROM block
     * of a dataset NVRAM block is referenced by the BlockId parameter.
     */
    else if((NvMBlockDescriptor[BlockId].NvMBlockManagement == NVM_BLOCK_DATASET) &&
            (AdministrativeBlock[BlockId].DataSetIndex >= NvMBlockDescriptor[BlockId].NvMNvBlockNum))
    {
        Return_Val = E_NOT_OK ;
    }

    /* [SWS_NvM_00239]
     * The function NvM_InvalidateNvBlock shall take over
     * the given parameters, queue the request and return.
     */
    else{

       Job_Parameters InvalidateJob = {NVM_INVALIDATEBLOCK_API_ID, BlockId} ;
       Return_Val = Job_Enqueue(InvalidateJob) ;

       /*Save job request as pending in the Administrative block*/
       AdministrativeBlock[BlockId].BlockResultStatus = NVM_REQ_PENDING ;
    }

    return Return_Val ;
}

/****************************************************************************************/
/*    Function Name           : NvM_ReadAll                                             */
/*    Function Description    : Initiates a multi block read request.                   */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00460                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
void NvM_ReadAll(void)
{
    /*[SWS_NvM_00646] If development error detection is enabled for NvM module,
     * the function NvM_ReadAll shall report the DET error NVM_E_NOT_INITIALIZED
     * when NVM is not yet initialized.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_READ_ALL_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
    }

    /*[SWS_NvM_00243] The function NvM_ReadAll shall signal the request to the NvM
      module and return. The NVRAM Manager shall defer the processing of the requested
      ReadAll until all single block job queues are empty.
     */
    if(MultiBlcokRequest.ResultStatus != NVM_REQ_PENDING)
    {
        MultiBlcokRequest.request = NVM_READ_ALL_API_ID ;
        MultiBlcokRequest.ResultStatus = NVM_REQ_PENDING ;
        MultiBlcokRequest.Internal_state = READ_NV_BLOCK;
    }
}


/****************************************************************************************/
/*    Function Name           : NvM_WriteAll                                            */
/*    Function Description    : Initiates a multi block write request.                  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00461                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/

void NvM_WriteAll( void )
{

    /*[SWS_NvM_00647]
     * If development error detection is enabled for NvM module,
     * the function NvM_WriteAll shall report the DET error
     * NVM_E_NOT_INITIALIZED when NVM is not yet initialized.
     */
    if(ModuleState != INIT_DONE){

       #if(NVM_DEV_ERROR_DETECT == STD_ON)
           Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_WRITE_ALL_API_ID, NVM_E_NOT_INITIALIZED) ;
       #endif

    }

    /* [SWS_NvM_00254]
     * The function NvM_WriteAll shall signal the request to the NvM module and return.
     * The NVRAM Manager shall defer the processing of the requested WriteAll until
     * all single block job queues are empty.
     */
    if(MultiBlcokRequest.ResultStatus != NVM_REQ_PENDING)
    {

        MultiBlcokRequest.request = NVM_WRITE_ALL_API_ID ;
        MultiBlcokRequest.ResultStatus = NVM_REQ_PENDING ;
        MultiBlcokRequest.Internal_state = CALC_CRC ;

    }
}

void NvM_JobEndNotification(void)
{

    EndJobStatus.EndJobSuccess = 1;
}

void NvM_JobErrorNotification (void)
{
    EndJobStatus.EndJobFailed  = 1 ;
}
/****************************************************************************************/
/*    Function Name           : NvM_MainFunction                                        */
/*    Function Description    : Service for performing the processing of the NvM jobs.  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00464                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/
void NvM_MainFunction( void )
{
    static uint8 Current_State = GET_JOB ;

    /* [SWS_NvM_00257]
     * The NvM module shall only do/start job processing, queue management and CRC
     * recalculation if the NvM_Init function has internally set an “INIT DONE” signal
     */
    if(ModuleState == INIT_DONE)
    {

        switch(Current_State)
        {
          /* case 0 : Get a job */
          case GET_JOB :

              /* First get a single block job request if existed */
              Get_SingleJob( &Current_Job ) ;

              if(Current_Job.ServiceId != NVM_INIT_API_ID)
              {
                  Current_State = EXECUTE_SINGLE_JOB ;
              }
              /* Second get a CRC request if existed */
              else if(CRC_Queue_Empty == FALSE)
              {
                  Current_State = EXECUTE_CRC_JOB ;
              }
              /* Third get a multi block request if existed */
              else if(MultiBlcokRequest.ResultStatus == NVM_REQ_PENDING)
              {
                  Current_State = EXECUTE_MULTI_JOB ;
              }
              break ;

          /* case 1 : Execute a single block job request */
          case EXECUTE_SINGLE_JOB :

              switch(Current_Job.ServiceId)
              {
                case NVM_READBLOCK_API_ID :

                    NvM_MainFunction_ReadBlock() ;
                    break ;

                case NVM_WRITEBLOCK_API_ID :

                    NvM_MainFunction_WriteBlock() ;
                    break ;

                case NVM_INVALIDATEBLOCK_API_ID :

                    NvM_MainFunction_InvalidateBlock() ;
                    break ;
                case NVM_RESTORE_BLOCKDEFAULTS_API_ID :
                    NvM_MainFunction_RestoreBlockDefaults();
                break ;
                case NVM_INIT_API_ID :

                    Current_State = GET_JOB ;
                    break ;
              }
              break ;

          /* case 2 : Execute a CRC request job */
          case EXECUTE_CRC_JOB :

              NvM_MainFunction_CalcCRC();
              break ;

          /* case 3 : Execute a multi block job request */
          case EXECUTE_MULTI_JOB :

              switch(MultiBlcokRequest.request)
              {
                 case NVM_READ_ALL_API_ID :
                     NvM_MainFunction_ReadAll();
                 break ;

                 case NVM_WRITE_ALL_API_ID :
                     NvM_MainFunction_WriteAll() ;
                 break ;
              }
              if((MultiBlcokRequest.ResultStatus == NVM_REQ_OK) || (MultiBlcokRequest.ResultStatus == NVM_REQ_NOT_OK))
              {
                  Current_State = GET_JOB ;
              }
              break ;
        }
    }
}

/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_ReadBlock                              */
/*    Function Description    : Asynchronous processing of ReadBlock job                */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
static void NvM_MainFunction_ReadBlock(void)
{
    MemIf_StatusType    MemIf_Status    ;
    uint16 dataIndex =0 , NvNumberOfBlocks  ;
    uint8  DeviceId ;
    uint8* ROMAddress ,*ROMStartAddress ;
    uint16 Length =0 ;
    Std_ReturnType rtn_val = E_OK  ;
    static uint8 Current_State = GET_MEMIF_BLOCK_ID;

    switch(Current_State)
    {
/********************case : GET_MEMIF_BLOCK_ID*******************/
    case GET_MEMIF_BLOCK_ID :
        dataIndex = AdministrativeBlock[Current_Job.Block_Id].DataSetIndex  ;
        NvNumberOfBlocks  = NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockNum ;

        /*Check if the required index is a ROM block
         * [SWS_NvM_00146] If the basic storage object ROM block is selected as optional part,
         * the index range which normally selects a dataset is extended to the ROM to make it
         * possible to select a ROM block instead of a NV block. The index covers all NV/ROM
         * blocks which may build up the NVRAM Dataset block.
         */
        if(dataIndex >= NvNumberOfBlocks )
        {
            /* Go to next state to read data from ROM block*/
            /* [SWS_NvM_00354] The job of the function NvM_ReadBlock shall copy the
             * ROM block to RAM and set the request result to NVM_REQ_OK if the NVRAM
             * block management type is NVM_BLOCK_DATASET and the dataset index points
             * at a ROM block.
             */
            Current_State = READ_ROM_BLOCK ;
        }
        else
        {
            BlockNumber = (NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber <<\
                    NVM_DATASET_SELECTION_BITS ) ;
            BlockNumber = BlockNumber + dataIndex ;
            /*Go to next state to read data from block*/
                Current_State = READ_NV_BLOCK ;
        }
    break ;
/********************case : READ_NV_BLOCK*******************/
    case READ_NV_BLOCK :

        DeviceId = NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId ;
        Length   = NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength ;

        /*Read underlying device status*/
        MemIf_Status    = MemIf_GetStatus(DeviceId);

        /*Initiate a request if underlying device not busy*/
         if(MemIf_Status != MEMIF_BUSY && EndJobStatus.EndJobSuccess == 0 && EndJobStatus.EndJobFailed ==0)
         {
             /*Read block data and CRC in Tempbuffer*/
             rtn_val = MemIf_Read(DeviceId , BlockNumber , TempBuffer,Length+CRC_SIZE) ;
         }
         if(rtn_val == E_NOT_OK)
         {
             Current_State  = END_JOB ;
             AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_NOT_OK ;
         }

         /*Read Job result*/
         MemIf_JobResult = MemIf_GetJobResult(DeviceId)  ;

         if(MemIf_JobResult == MEMIF_JOB_OK)
         {
             /*Move data and CRC to their places from temp buffer*/
             EmptyTempBuffer(Length,Current_Job.RAM_Ptr,\
                     &AdministrativeBlock[Current_Job.Block_Id].PrevCRCVal);

             /*Check if CRC is required for this block*/
            if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockUseCrc == STD_ON)
            {
                /*Go to check CRC state*/
                Current_State = CALC_CRC ;
            }
            else
            {
                Current_State  = END_JOB ;
            }

            /*Set request state to OK*/
            AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_OK ;

            /*[SWS_NvM_00200] The job of the function NvM_ReadBlock shall set the RAM block
             * to valid and assume it to be unchanged after a successful copy process of the
             * NV block to RAM.
             */
            AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = VALID_UNCHANGED ;
         }
             /*[SWS_NvM_00657] The job of the function NvM_ReadBlock shall load the default
              * values according to processing of NvM_RestoreBlockDefaults (also set the
              * request result to NVM_REQ_RESTORED_FROM_ROM) if the read request passed to
              * the underlying layer fails (MemIf reports MEMIF_JOB_FAILED or MEMIF_BLOCK_INCONSISTENT)
              * and if the default values are available.*/
         else if((MemIf_JobResult == MEMIF_JOB_FAILED))
         {
             /*Job failed , the get default data*/
             Current_State = READ_ROM_BLOCK ;
             AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_NOT_OK ;
         }
         else if(MemIf_JobResult == MEMIF_BLOCK_INCONSISTENT)
         {
             /*Job failed , the get default data*/
             Current_State = READ_ROM_BLOCK ;
             AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_INTEGRITY_FAILED ;
         }
         else if(MemIf_JobResult == MEMIF_BLOCK_INVALID)
         {
             AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_NV_INVALIDATED ;
             Current_State = END_JOB ;

         }

    break ;
/********************case : READ_ROM_BLOCK*******************/
    case READ_ROM_BLOCK :
        /*Check if ROM block address is  configured*/
        if(NvMBlockDescriptor[Current_Job.Block_Id].NvMRomBlockDataAddress != NULL )
        {
            Length    = NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength ;
            dataIndex = AdministrativeBlock[Current_Job.Block_Id].DataSetIndex ;
            ROMStartAddress = NvMBlockDescriptor[Current_Job.Block_Id].NvMRomBlockDataAddress ;

            /*Calculate the required ROM Address based on Index and ROM start address*/
            ROMAddress = (ROMStartAddress)+(Length*dataIndex) ;

            ReadRomBlock( ROMAddress,Current_Job.RAM_Ptr ,  Length);

            if(dataIndex>0)
            {
                /*Reading a ROM block*/
                AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_OK;
            }
            else
            {
                /*Reading default*/
                AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_RESTORED_FROM_ROM;
            }
             /*[SWS_NvM_00366] The job of the function NvM_ReadBlock shall set the RAM
              * block to valid and assume it to be changed if the default values are copied
              * to the RAM successfully.
              */
             AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = VALID_CHANGED ;
                Current_State = END_JOB ;
        }

        /*Check if NvMInitBlockCallback is  configured */
        else if(NvMBlockDescriptor[Current_Job.Block_Id].NvMInitBlockCallback != NULL)
        {
            rtn_val=NvMBlockDescriptor[Current_Job.Block_Id].NvMInitBlockCallback();

            /*Check return value*/
            if(rtn_val == E_OK )
            {
                 Current_State = END_JOB ;

                 /*[SWS_NvM_00202] The job of the function NvM_ReadBlock shall load the default
                    values according to processing of NvM_RestoreBlockDefaults (also set the request
                    result to NVM_REQ_RESTORED_FROM_ROM) if the recalculated CRC is not equal
                    to the CRC stored in NV memory.
                  */
                 AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_RESTORED_FROM_ROM ;

                 /*[SWS_NvM_00366] The job of the function NvM_ReadBlock shall set the RAM
                  * block to valid and assume it to be changed if the default values are copied
                  * to the RAM successfully.
                  */
                 AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = VALID_CHANGED ;
            }
            else
            {
                Current_State = END_JOB;
                /*[SWS_NvM_00658] NvM_ReadBlock: If there are no default values available,
                 * the RAM blocks shall remain invalid.
                 */
                AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = INVALID_UNCHANGED;
            }
        }
        else
        {
            Current_State = END_JOB ;
            /*[SWS_NvM_00658] NvM_ReadBlock: If there are no default values available,
             * the RAM blocks shall remain invalid.
             */
            AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = INVALID_UNCHANGED;
        }
        break;
/********************case : CalculateCRC*******************/
    case CALC_CRC :
        /*[SWS_NvM_00201] The job of the function NvM_ReadBlock shall request
         * a CRC recalculation over the RAM block data after the copy process
         * [SWS_NvM_00180] if the NV block is configured with CRC, i.e.
         * if NvMCalRamBlockCrC == TRUE for the NV block.
         */

        if(CalculateCRC(Current_Job.Block_Id,Current_Job.RAM_Ptr) == E_OK)
        {
            if(AdministrativeBlock[Current_Job.Block_Id].PrevCRCVal != CRCTempbuffer )
            {
                /* [SWS_NvM_00202]The job of the function NvM_ReadBlock shall load the default values according to
                 * processing of NvM_RestoreBlockDefaults (also set the request result to
                 * NVM_REQ_RESTORED_FROM_ROM) if the recalculated CRC is not equal to the
                 * CRC stored in NV memory.
                 */
                Current_State = READ_ROM_BLOCK ;
                AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_INTEGRITY_FAILED ;
            }
            else
            {
                Current_State  = END_JOB ;
            }
        }
    break;

/********************case : END_JOB*******************/
    case END_JOB:
        /*Check if single block call back is configured */
        if(NvMBlockDescriptor[Current_Job.Block_Id].NvMSingleBlockCallback != NULL)
        {
            NvMBlockDescriptor[Current_Job.Block_Id].\
            NvMSingleBlockCallback(Current_Job.ServiceId,\
            AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus);
        }
  /*remove job request from queue*/
            Job_Dequeue() ;
           Get_SingleJob( &Current_Job ) ;
           Current_State = GET_MEMIF_BLOCK_ID ;
        break ;
    }
}


/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_WriteBlock                              */
/*    Function Description    : Local Service to to execute write jobs                  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00455 & SWS_NvM_00464                           */
/*    Notes                   :                                                         */
/****************************************************************************************/

static void NvM_MainFunction_WriteBlock(void)
{
    static uint8 Current_State = CALC_CRC;
    static uint8 Retry_Counter = 0;
    static uint8 redundant_block_Num = 0;

    uint16 counter = 0 ;
    uint16 Fee_Ea_Block_Num ;

    switch (Current_State){

     /* Case CALC_CRC :
      * Initial state
      */
     case CALC_CRC :

         /* [SWS_NvM_00212]
          * The job of the function NvM_WriteBlock shall request a CRC recalculation
          * before the RAM block will be copied to NV memory if the NV block is configured with CRC
          */
         if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockUseCrc == TRUE){

             /* if the RAM block is not permanent,
              * OR the RAM block is permanent and NvMCalcRamBlockCrc is true
              * So , Calculate CRC
              */
             /* ECUC_NvM_00119 */
             if((NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockUseCrc == TRUE))
             {

                  CalculateCRC(Current_Job.Block_Id, Current_Job.RAM_Ptr) ;

                 /* [SWS_NvM_00852]
                  * The job of the function NvM_WriteBlock shall skip writing and consider the job as
                  * successfully finished if the NvMBlockUseCRCCompMechanism attribute of the NVRAM Block
                  * is set to true and the RAM block CRC calculated by the write job is equal to the CRC
                  * calculated during the last successful read or write job.
                  * This mechanism shall not be applied to blocks for which a loss of redundancy has been detected.
                  */
                if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockUseCRCCompMechanism == TRUE){

                   if(CRCTempbuffer == AdministrativeBlock[Current_Job.Block_Id].PrevCRCVal){

                        redundant_block_Num = 1 ;
                        Current_State = WRITE_OK ;
                        break ;

                    }
                }

             }

             if(CRCTempbuffer != 0){

                for (counter = 0 ; counter < NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength ; counter++){

                      TempBuffer[counter] = *((uint8 *)(Current_Job.RAM_Ptr)) ;
                      Current_Job.RAM_Ptr = ((uint8 *)(Current_Job.RAM_Ptr) + 1) ;

                }

                /*Add CRC Value to the buffer*/
                TempBuffer[NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength] = *((uint8 *)&CRCTempbuffer) ;
                TempBuffer[NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength +1] = *(((uint8 *)&CRCTempbuffer) + 1) ;
                TempBuffer[NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength +2] = *(((uint8 *)&CRCTempbuffer) + 2) ;
                TempBuffer[NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength +3] = *(((uint8 *)&CRCTempbuffer) + 3) ;
                Current_State = WRITE_NV_BLOCK ;
             }
         }
      break;
     /* case WRITE_NV_BLOCK */
     case WRITE_NV_BLOCK :

         if(MemIf_GetStatus(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_IDLE\
                 && EndJobStatus.EndJobSuccess ==0 && EndJobStatus.EndJobFailed==0)
         {


            /* [SWS_NvM_00303]
             * The job of the function NvM_WriteBlock shall assume a referenced permanent RAM block or the RAM mirror
             * in the NvM module in case of explicit synchronization to be valid when the request is passed to the NvM module.
             * If the permanent RAM block is still in an invalid state, the function NvM_WriteBlock shall validate it automatically
             * before copying the RAM block contents to NV memory or after calling explicit synchronization callback
             */
            if((NvMBlockDescriptor[Current_Job.Block_Id].NvMRamBlockDataAddress == Current_Job.RAM_Ptr) && (AdministrativeBlock[Current_Job.Block_Id].PRAMStatus != VALID_CHANGED)){

                AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = VALID_CHANGED ;

            }

            /*Calculate FEE/EA Block Number to send to the MemIf Module*/
            /*Native Block*/
            if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_NATIVE){

               Fee_Ea_Block_Num = NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS ;

            }

            /* [SWS_NvM_00338]
             * The job of the function NvM_WriteBlock shall copy the RAM block to the corresponding NV block
             * which is selected via the data index in the administrative block
             * if the NVRAM block management type of the given NVRAM block is NVM_BLOCK_DATASET.
             */
            /*DataSet Block*/
            else if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_DATASET){

                Fee_Ea_Block_Num = (NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + AdministrativeBlock[Current_Job.Block_Id].DataSetIndex ;

            }
            /* [SWS_NvM_00760] The job of the function NvM_WriteBlock shall copy the data content of the RAM block
             * to both corresponding NV blocks if the NVRAM block management type of the processed NVRAM block
             * is NVM_BLOCK_REDUNDANT.
             */
            /*Redundant Block*/
            else if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_REDUNDANT){
                Fee_Ea_Block_Num = (NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + redundant_block_Num ;
            }

            Std_ReturnType InitWrite ;

            /*Call MemIf_Write function*/
            if(CRCTempbuffer != 0){

                InitWrite = MemIf_Write(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId, Fee_Ea_Block_Num, TempBuffer) ;

            }
            else{

                InitWrite = MemIf_Write(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId, Fee_Ea_Block_Num, (const uint8 *)Current_Job.RAM_Ptr) ;

            }


            AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_PENDING ;

            if(InitWrite == E_NOT_OK){
                /*Job Failed*/
                Current_State = WRITE_FAILED ;
                break ;
            }
         }

         /* If the job is done and result is OK */
         if(MemIf_GetJobResult(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_JOB_OK){
             Current_State = WRITE_OK ;
         }
         /* If the job failed */
         else if (MemIf_GetJobResult(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_JOB_FAILED)
         {
             Current_State = WRITE_FAILED ;
         }

         break ;

     /*Case WRITE_OK :
      * If the job is done and result is OK
      */
     case WRITE_OK :

         /* [SWS_NvM_00284]
          * The job of the function NvM_WriteBlock shall set NVM_REQ_OK as request result
          * if the passed BlockId references a NVRAM block of type NVM_BLOCK_REDUNDANT
          * and at least one of the NV blocks has been written successfully.
          */
         if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_REDUNDANT && redundant_block_Num == 0){

            redundant_block_Num = 1 ;
            AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_OK ;
            Retry_Counter = 0 ;
            /* return to the last state again to write the other NV block */
            Current_State = WRITE_NV_BLOCK ;

         }

         else{
             redundant_block_Num = 0;
             AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_OK ;
             AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = VALID_UNCHANGED ;

             if(CRCTempbuffer != 0){
                 AdministrativeBlock[Current_Job.Block_Id].PrevCRCVal = CRCTempbuffer ;
                 CRCTempbuffer = 0 ;
             }

             Current_State = WRITE_END ;

         }

         break ;

     /* Case WRITE_FAILED :
      * If the job failed
      */
     case WRITE_FAILED :

         /* [SWS_NvM_00213]
          * The job of the function NvM_WriteBlock shall check the number of write retries using a write retry counter
          * to avoid infinite loops. Each negative result reported by the memory interface shall be followed by
          * an increment of the retry counter. In case of a retry counter overrun,
          * the job of the function NvM_WriteBlock shall set the request result to NVM_REQ_NOT_OK.
          */
         Retry_Counter++ ;

         if(Retry_Counter > NvMBlockDescriptor[Current_Job.Block_Id].NvMMaxNumOfWriteRetries){

             AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_NOT_OK ;
             AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = INVALID_UNCHANGED ;

             /*report NVM_E_REQ_FAILED to the DEM.*/

             Retry_Counter = 0 ;
             redundant_block_Num = 0 ;
             Current_State = WRITE_END ;
         }
         else {
             Current_State = WRITE_NV_BLOCK ;
         }

         break ;

     /* Case WRITE_END :
      * the write job has been finished
      */
     case WRITE_END :

         CRCTempbuffer = 0 ;
         Retry_Counter = 0;
         redundant_block_Num = 0;

         /* [SWS_NvM_00347]
          * If callback routines are configured, the function NvM_MainFunction shall call
          * callback routines to the upper layer after completion of an asynchronous service.
          */
         NvMBlockDescriptor[Current_Job.Block_Id].NvMSingleBlockCallback(NVM_WRITEBLOCK_API_ID, AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus) ;

         Job_Dequeue() ;
         Get_SingleJob( &Current_Job ) ;

         Current_State = CALC_CRC ;
         break ;
    }
}

/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_RestoreBlockDefaults                   */
/*    Function Description    : Asynchronous processing of restore block  defaults job  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
static void NvM_MainFunction_RestoreBlockDefaults(void)
{
    static uint8 Current_State = RESTORE_FROM_ROM;
    uint16 BlockId ;
    uint8* RomAddress ;
    BlockId = Current_Job.Block_Id ;
    switch(Current_State)
    {
        case RESTORE_FROM_ROM :
            if(NULL != NvMBlockDescriptor[BlockId].NvMRomBlockDataAddress)
            {
                /*Get Rom block address
                 *RomAddress = First blockAdress +(index*Block Length)
                 */
                RomAddress = NvMBlockDescriptor[BlockId].NvMRomBlockDataAddress + \
                        (AdministrativeBlock[BlockId].DataSetIndex * NvMBlockDescriptor[BlockId].NvMNvBlockLength);
                /*Read data*/
                ReadRomBlock(RomAddress ,Current_Job.RAM_Ptr,NvMBlockDescriptor[BlockId].NvMNvBlockLength);

                /*Set result to restored from ROM*/
                AdministrativeBlock[BlockId].BlockResultStatus = NVM_REQ_RESTORED_FROM_ROM ;
                /*Set RAM block status to valid changed*/
                AdministrativeBlock[BlockId].PRAMStatus = VALID_CHANGED ;
                Current_State = END_JOB ;
            }
            else
            {
                if(NvMBlockDescriptor[BlockId].NvMInitBlockCallback()== E_OK)
                {
                    /*Set result to restored from ROM*/
                    AdministrativeBlock[BlockId].BlockResultStatus = NVM_REQ_RESTORED_FROM_ROM ;

                    /*Set RAM block status to valid changed*/
                    AdministrativeBlock[BlockId].PRAMStatus = VALID_CHANGED ;

                    Current_State = END_JOB ;
                }
                else
                {
                    AdministrativeBlock[BlockId].BlockResultStatus = NVM_REQ_NOT_OK ;
                }
            }
        break;
        case END_JOB :
            NvMBlockDescriptor[BlockId].NvMSingleBlockCallback(Current_Job.ServiceId,\
                    AdministrativeBlock[BlockId].BlockResultStatus);
            Job_Dequeue() ;
            Get_SingleJob( &Current_Job ) ;
            Current_State = RESTORE_FROM_ROM ;
        break;
    }
}

/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_WriteAll                               */
/*    Function Description    : Asynchronous processing of Write All job                */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00461                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/

static void NvM_MainFunction_WriteAll( void )
{
    static NvM_BlockIdType IdCounter = 2 ;
    static uint8 redundant_block_Num = 0 ;
    static uint8 Retry_Counter ;
    static boolean FailedJob = FALSE ;

    uint32 CrcCounter = 0 ;
    uint32 Fee_Ea_Block_Num ;
    Std_ReturnType InitWrite ;

    switch(MultiBlcokRequest.Internal_state){

      case WRITE_NV_BLOCK :

          /*[SWS_NvM_00252]
           * The job of the function NvM_WriteAll shall process only the permanent RAM blocks
           * or call explicit synchronization callback (NvM_WriteRamBlockToNvm) for all blocks
           * for which the corresponding NVRAM block parameter NvMSelectBlockForWriteAll is configured to true.
           */
          /*[SWS_NvM_00682]
           * The job of the function NvM_WriteAll shall check the
           * “valid/modified” state for each RAM block in advance.
           */
          if(NvMBlockDescriptor[IdCounter].NvMSelectBlockForWriteAll == TRUE && AdministrativeBlock[IdCounter].PRAMStatus == VALID_CHANGED){

              /* [SWS_NvM_00549]
               * The job of the function NvM_ WriteAll shall set each proceeding block specific
               * request result for NVRAM blocks and the multi block request
               * result to NVM_REQ_PENDING in advance.
               */
              AdministrativeBlock[IdCounter].BlockResultStatus = NVM_REQ_PENDING ;

              if(MemIf_GetStatus(NvMBlockDescriptor[IdCounter].NvMNvramDeviceId) == MEMIF_IDLE){

                  uint8* PRamPtr = (uint8 *)NvMBlockDescriptor[IdCounter].NvMRamBlockDataAddress ;

                  /*Calculate FEE/EA Block Number to send to the MemIf Module*/
                  /*if Native Block*/
                  if(NvMBlockDescriptor[IdCounter].NvMBlockManagement == NVM_BLOCK_NATIVE){

                     Fee_Ea_Block_Num = NvMBlockDescriptor[IdCounter].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS ;

                  }

                  /*DataSet Block*/
                  /*[SWS_NvM_00339]
                   * In case of NVRAM block management type NVM_BLOCK_DATASET,
                   * the job of the function NvM_WriteAll shall copy only the RAM block to the corresponding
                   * NV block which is selected via the data index in the administrative block.
                   */
                  else if(NvMBlockDescriptor[IdCounter].NvMBlockManagement == NVM_BLOCK_DATASET){

                      Fee_Ea_Block_Num = (NvMBlockDescriptor[IdCounter].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + AdministrativeBlock[IdCounter].DataSetIndex ;

                  }

                  /*Redundant Block*/
                  else if(NvMBlockDescriptor[IdCounter].NvMBlockManagement == NVM_BLOCK_REDUNDANT){
                      Fee_Ea_Block_Num = (NvMBlockDescriptor[IdCounter].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + redundant_block_Num ;
                  }

                  if(NvMBlockDescriptor[IdCounter].NvMBlockUseCrc == TRUE && NvMBlockDescriptor[IdCounter].NvMCalcRamBlockCrc == TRUE){

                     for (CrcCounter = 0 ; CrcCounter < NvMBlockDescriptor[IdCounter].NvMNvBlockLength - 4 ; CrcCounter++){
                          TempBuffer[CrcCounter] = *(PRamPtr) ;
                          PRamPtr += 1 ;
                     }

                     /*Add CRC Value to the buffer*/
                     TempBuffer[NvMBlockDescriptor[IdCounter].NvMNvBlockLength - 4] = *((uint8 *)(&AdministrativeBlock[IdCounter].PrevCRCVal)) ;
                     TempBuffer[NvMBlockDescriptor[IdCounter].NvMNvBlockLength - 3] = *(((uint8 *)(&AdministrativeBlock[IdCounter].PrevCRCVal)) + 1) ;
                     TempBuffer[NvMBlockDescriptor[IdCounter].NvMNvBlockLength - 2] = *(((uint8 *)(&AdministrativeBlock[IdCounter].PrevCRCVal)) + 2) ;
                     TempBuffer[NvMBlockDescriptor[IdCounter].NvMNvBlockLength - 1] = *(((uint8 *)(&AdministrativeBlock[IdCounter].PrevCRCVal)) + 3) ;

                     InitWrite = MemIf_Write(NvMBlockDescriptor[IdCounter].NvMNvramDeviceId, Fee_Ea_Block_Num, TempBuffer) ;
                  }
                  else {

                      InitWrite = MemIf_Write(NvMBlockDescriptor[IdCounter].NvMNvramDeviceId, Fee_Ea_Block_Num, (const uint8 *)PRamPtr) ;
                  }

                  /* [SWS_NvM_00549]
                   * The job of the function NvM_ WriteAll shall set each proceeding block
                   * specific request result for NVRAM blocks and the multi block request
                   * result to NVM_REQ_PENDING in advance.
                   */
                  AdministrativeBlock[IdCounter].BlockResultStatus = NVM_REQ_PENDING ;

                  if(InitWrite == E_NOT_OK){
                     /*Job Failed*/
                     MultiBlcokRequest.Internal_state = WRITE_FAILED ;
                     break ;
                  }

                  /* If the job is done and result is OK */
                  if(MemIf_GetJobResult(NvMBlockDescriptor[IdCounter].NvMNvramDeviceId) == MEMIF_JOB_OK){
                      MultiBlcokRequest.Internal_state = WRITE_OK ;
                  }

                  /* If the job failed */
                  else if (MemIf_GetJobResult(NvMBlockDescriptor[IdCounter].NvMNvramDeviceId) == MEMIF_JOB_FAILED){
                      MultiBlcokRequest.Internal_state = WRITE_FAILED ;
                  }

                  break ;

             }

          }
          else {

              /* [SWS_NvM_00298]
               * The job of the function NvM_WriteAll shall set the request result to
               * NVM_REQ_BLOCK_SKIPPED for each NVRAM block configured to be processed
               * by the job of the function NvM_WriteAll (NvMSelectBlockForWriteAll is checked)
               * and which has not been written during processing of the NvM_WriteAll job.
               */
              if(NvMBlockDescriptor[IdCounter].NvMSelectBlockForWriteAll)
              {
                  AdministrativeBlock[IdCounter].BlockResultStatus = NVM_REQ_BLOCK_SKIPPED ;
              }
              MultiBlcokRequest.Internal_state = WRITE_END ;
          }

      /*Case WRITE_OK :
       * If a single block write job is done and result is OK
       */
      case WRITE_OK :

          /*[SWS_NvM_00337]
           * The job of the function NvM_WriteAll shall set the single block request result to
           * NVM_REQ_OK if the processed NVRAM block is of type NVM_BLOCK_REDUNDANT and at least
           * one of the NV blocks has been written successfully.
           */
          if(NvMBlockDescriptor[IdCounter].NvMBlockManagement == NVM_BLOCK_REDUNDANT && redundant_block_Num == 0){

             redundant_block_Num = 1 ;
             AdministrativeBlock[IdCounter].BlockResultStatus = NVM_REQ_OK ;
             Retry_Counter = 0 ;

             /* [SWS_NvM_00762]
              * The job of the function NvM_WriteAll shall copy the data content of the RAM block to
              * both corresponding NV blocks if the NVRAM block management type of the processed NVRAM
              * block is NVM_BLOCK_REDUNDANT.
              */
             /* return to WRITE_NV_BLOCK state again to write the other NV block */
             MultiBlcokRequest.Internal_state = WRITE_NV_BLOCK ;

          }
          else {

              redundant_block_Num = 0;
              AdministrativeBlock[IdCounter].BlockResultStatus = NVM_REQ_OK ;
              AdministrativeBlock[IdCounter].PRAMStatus = VALID_UNCHANGED ;

              MultiBlcokRequest.Internal_state = WRITE_END ;
          }

          break ;

      /* Case WRITE_FAILED :
       * If a single block write job failed
       */
      case WRITE_FAILED :

          /*[SWS_NvM_00296]
           * The job of the function NvM_WriteAll shall check the number of write retries
           * by a write retry counter to avoid infinite loops. Each unsuccessful result
           * reported by the MemIf module shall be followed by an increment of the retry counter.
           */
          Retry_Counter++ ;

          /*[SWS_NvM_00683]
           * The job of the function NvM_WriteAll shall set the block specific request
           * result to NVM_REQ_NOT_OK if the write retry counter becomes greater than the
           * configured NVM_MAX_NUM_OF_WRITE_RETRIES.
           */
          if(Retry_Counter >= NvMBlockDescriptor[IdCounter].NvMMaxNumOfWriteRetries){

             AdministrativeBlock[IdCounter].BlockResultStatus = NVM_REQ_NOT_OK ;
             AdministrativeBlock[IdCounter].PRAMStatus = INVALID_UNCHANGED ;

             /*report error to the DEM.*/

             FailedJob = TRUE ;
             Retry_Counter = 0 ;
             redundant_block_Num = 0 ;
             MultiBlcokRequest.Internal_state = WRITE_END ;
          }
          else {

             MultiBlcokRequest.Internal_state = WRITE_NV_BLOCK ;
          }

          break ;

      /* Case WRITE_END :
       * If a single block write job ended
       */
      case WRITE_END :

          redundant_block_Num = 0 ;
          Retry_Counter = 0 ;
          CrcCounter = 0 ;
          Fee_Ea_Block_Num = 0 ;
          IdCounter++ ;

          /*if the blocks IDs finishes*/
          if(IdCounter >= NUMBER_OF_NVM_BLOCKS){

              IdCounter = 2 ;

              /* [SWS_NvM_00318]
               * The job of the function NvM_WriteAll shall set the multi block request result
               * to NVM_REQ_NOT_OK if processing of one or even more NVRAM blocks fails.
               */
              if(FailedJob == TRUE){

                  MultiBlcokRequest.ResultStatus = NVM_REQ_NOT_OK ;
                  FailedJob = FALSE ;
              }

              /*[SWS_NvM_00896]
               * The job of the function NvM_WriteAll shall set the multi block request result
               * to NVM_REQ_OK if no job fails with the processing of the NVRAM blocks.
               */
              else {

                  MultiBlcokRequest.ResultStatus = NVM_REQ_OK ;
              }
          }
          else {

              MultiBlcokRequest.Internal_state = WRITE_NV_BLOCK ;
          }

          break ;
    }
}


/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_InvalidateBlock                        */
/*    Function Description    : Asynchronous processing of InvalidateBlock job          */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00459                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/
static void NvM_MainFunction_InvalidateBlock(void)
{
    static uint8 Current_State = INVALIDATE_NV_BLOCK;
    static uint8 redundant_block_Num = 0;
    uint16 Fee_Ea_Block_Num ;
    Std_ReturnType InitInvalidate ;

    switch(Current_State){

      /*case INVALIDATE_NV_BLOCK : Initial state*/
      case INVALIDATE_NV_BLOCK :

          if(MemIf_GetStatus(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_IDLE){


              /*Calculate FEE/EA Block Number to send to the MemIf Module*/
              /*Native Block*/
              if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_NATIVE){

                 Fee_Ea_Block_Num = NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS ;

              }

              /*DataSet Block*/
              else if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_DATASET){

                  Fee_Ea_Block_Num = (NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + AdministrativeBlock[Current_Job.Block_Id].DataSetIndex ;

              }

              /*Redundant Block*/
              else if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_REDUNDANT){
                  Fee_Ea_Block_Num = (NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + redundant_block_Num ;
              }

              InitInvalidate = MemIf_InvalidateBlock(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId, Fee_Ea_Block_Num) ;

              if(InitInvalidate == E_NOT_OK){

                  Current_State = INVALIDATE_FAILED ;
                  break ;

              }
              else {

                  /* If the job is done and result is OK */
                  if(MemIf_GetJobResult(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_JOB_OK){
                      Current_State = INVALIDATE_OK ;
                  }
                  /* If the job failed */
                  else if (MemIf_GetJobResult(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_JOB_FAILED){
                           Current_State = INVALIDATE_FAILED ;
                  }

              }
          }

          break ;

      /* case INVALIDATE_OK : when the invalidation done successfully */
      case INVALIDATE_OK :

          /* [SWS_NvM_00274]
           * If the referenced NVRAM block is of type NVM_BLOCK_REDUNDANT,
           * the function NvM_InvalidateNvBlock shall only set
           * the request result NvM_RequestResultType to NVM_REQ_OK
           * when both NV blocks have been invalidated.
           */
          if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_REDUNDANT && redundant_block_Num == 0){

              redundant_block_Num = 1 ;
              /* return to the last state again to invalidate the other NV block */
              Current_State = INVALIDATE_NV_BLOCK ;

          }

          else {

              AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_OK ;
              AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = INVALID_UNCHANGED ;

              Current_State = INVALIDATE_END ;

          }

          break ;

      /* case INVALIDATE_FAILED : when the invalidation failed */
      case INVALIDATE_FAILED :

          /* [SWS_NvM_00275]
           * The function NvM_InvalidateNvBlock shall set the request result to
           * NVM_REQ_NOT_OK if the processing of this service fails
           */
          AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus = NVM_REQ_NOT_OK ;

          /* [SWS_NvM_00666]
           * The function NvM_InvalidateNvBlock shall report NVM_E_REQ_FAILED
           * to the DEM if the processing of this service fails.
           */
          /*report NVM_E_REQ_FAILED to the DEM.*/

          Current_State = INVALIDATE_END ;

      break ;

      /* case INVALIDATE_END : when the invalidation ends, dequeue the job from the queue */
      case INVALIDATE_END :

           redundant_block_Num = 0 ;

           /* [SWS_NvM_00347]
            * If callback routines are configured, the function NvM_MainFunction shall call
            * callback routines to the upper layer after completion of an asynchronous service.
            */
           NvMBlockDescriptor[Current_Job.Block_Id].NvMSingleBlockCallback(NVM_INVALIDATEBLOCK_API_ID, AdministrativeBlock[Current_Job.Block_Id].BlockResultStatus) ;

           Job_Dequeue() ;
           Get_SingleJob( &Current_Job ) ;

           Current_State = INVALIDATE_NV_BLOCK ;
       break ;
    }
}

/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_ReadAll                                */
/*    Function Description    : Asynchronous processing of ReadAll job                  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
static void NvM_MainFunction_ReadAll(void)
{
    static uint16 block_counter = 2;
    MemIf_StatusType  MemIf_Status  ;
    uint16 Length=0;
    uint8  DeviceId =0 ;
    Std_ReturnType rtn_val ;

    if(block_counter < NUMBER_OF_NVM_BLOCKS)
    {
        /*Check if block is configured to be used in the ReadAll Request*/
        if(NvMBlockDescriptor[block_counter].NvMSelectBlockForReadAll &&\
           NvMBlockDescriptor[block_counter].NvMRamBlockDataAddress != NULL &&
           NvMBlockDescriptor[block_counter].NvMBlockManagement != NVM_BLOCK_DATASET)
        {
            switch(MultiBlcokRequest.Internal_state)
            {
                case READ_NV_BLOCK :
                    DeviceId = NvMBlockDescriptor[block_counter].NvMNvramDeviceId ;

                    /*Read underlying device status*/
                    MemIf_Status    = MemIf_GetStatus(DeviceId);


                    /*Initiate a request if underlying device not busy*/
                    if(MemIf_Status != MEMIF_BUSY && EndJobStatus.EndJobSuccess == 0&&EndJobStatus.EndJobFailed ==0)
                    {
                        /*Get block number*/
                        BlockNumber = (NvMBlockDescriptor[block_counter].NvMNvBlockBaseNumber <<\
                                                    NVM_DATASET_SELECTION_BITS ) ;
                        Length = NvMBlockDescriptor[block_counter].NvMNvBlockLength ;
                        MemIf_Read(DeviceId , BlockNumber ,TempBuffer ,Length+CRC_SIZE) ;
                    }

                     /*Read Job result*/
                     MemIf_JobResult = MemIf_GetJobResult(DeviceId)  ;

                    /*Check if Job result is no longer pending*/
                    if(MemIf_JobResult != MEMIF_JOB_PENDING)
                    {
                        Length = NvMBlockDescriptor[block_counter].NvMNvBlockLength ;
                        EmptyTempBuffer(Length,NvMBlockDescriptor[block_counter].NvMRamBlockDataAddress,\
                                &AdministrativeBlock[block_counter].PrevCRCVal) ;
                        if(MemIf_JobResult == MEMIF_JOB_OK)
                        {
                             /*Move data and CRC to their places from temp buffer*/

                                MultiBlcokRequest.Internal_state  = END_JOB ;
                                AdministrativeBlock[block_counter].BlockResultStatus = NVM_REQ_OK ;
                                /*[SWS_NvM_00251] The job of the function NvM_ReadAll shall mark a NVRAM block
                                 * as “valid/unmodified” if NV data has been successfully loaded to the RAM
                                 * Block.
                                 */
                                AdministrativeBlock[block_counter].PRAMStatus = VALID_UNCHANGED ;
                        }

                     else if(MemIf_JobResult == MEMIF_JOB_FAILED)
                     {
                         /*Job failed , the get default data*/
                         MultiBlcokRequest.Internal_state = READ_ROM_BLOCK ;
                         AdministrativeBlock[block_counter].BlockResultStatus = NVM_REQ_NOT_OK ;
                     }
                     else if(MemIf_JobResult == MEMIF_BLOCK_INCONSISTENT)
                     {
                         /*Job failed , the get default data*/
                         MultiBlcokRequest.Internal_state = READ_ROM_BLOCK ;
                         AdministrativeBlock[block_counter].BlockResultStatus = NVM_REQ_INTEGRITY_FAILED ;
                     }
                     else
                     {
                         AdministrativeBlock[block_counter].BlockResultStatus = NVM_REQ_NV_INVALIDATED ;
                         MultiBlcokRequest.Internal_state = END_JOB ;
                     }
                 }
                break ;
            /********************case : READ_ROM_BLOCK*******************/
                case READ_ROM_BLOCK :
                    /*Check if ROM block address is  configured*/
                    if(NvMBlockDescriptor[block_counter].NvMRomBlockDataAddress != NULL )
                    {
                        ReadRomBlock(NvMBlockDescriptor[block_counter].NvMRomBlockDataAddress ,\
                                     NvMBlockDescriptor[block_counter].NvMRamBlockDataAddress , \
                                     NvMBlockDescriptor[block_counter].NvMNvBlockLength);
                         AdministrativeBlock[block_counter].BlockResultStatus = NVM_REQ_RESTORED_FROM_ROM ;

                         /*[SWS_NvM_00367] The job of the function NvM_ReadAll shall set a RAM
                          * block to valid and assume it to be changed if the job has successfully
                          * copied default values to the corresponding RAM.
                          */
                         AdministrativeBlock[block_counter].PRAMStatus = VALID_CHANGED ;
                    }
                    /*Check if NvMInitBlockCallback is  configured */
                    else if(NvMBlockDescriptor[block_counter].NvMInitBlockCallback != NULL)
                    {
                        rtn_val = NvMBlockDescriptor[block_counter].NvMInitBlockCallback();

                        /*Check return value*/
                        if(rtn_val == E_OK )
                        {
                             MultiBlcokRequest.Internal_state = END_JOB ;

                             /*[SWS_NvM_00202] The job of the function NvM_ReadBlock shall load the default
                                values according to processing of NvM_RestoreBlockDefaults (also set the request
                                result to NVM_REQ_RESTORED_FROM_ROM) if the recalculated CRC is not equal
                                to the CRC stored in NV memory.
                              */
                             AdministrativeBlock[block_counter].BlockResultStatus = NVM_REQ_RESTORED_FROM_ROM ;

                             /*[SWS_NvM_00367] The job of the function NvM_ReadAll shall set a RAM
                              * block to valid and assume it to be changed if the job has successfully
                              * copied default values to the corresponding RAM.
                              */
                             AdministrativeBlock[block_counter].PRAMStatus = VALID_CHANGED ;
                        }
                        else
                        {
                             MultiBlcokRequest.Internal_state = END_JOB;
                        }
                    }
                    else
                    {
                         MultiBlcokRequest.Internal_state  = END_JOB ;
                    }
                break;

/********************case : END_JOB*******************/
                case END_JOB:
                    /*Check if single block call back is configured */
                    if(NvMBlockDescriptor[block_counter].NvMSingleBlockCallback != NULL)
                    {
                        NvMBlockDescriptor[block_counter].\
                        NvMSingleBlockCallback(MultiBlcokRequest.request,\
                        AdministrativeBlock[block_counter].BlockResultStatus);
                    }
                    /*Increment block counter*/
                    block_counter++;
                    EndJobStatus.EndJobSuccess = 0 ;
                    EndJobStatus.EndJobFailed = 0 ;
                    MultiBlcokRequest.Internal_state = READ_NV_BLOCK ;
                break ;
            }
        }
        else
        {
            block_counter++ ;
        }
    }

    /*Check if we reached end of multi block request function*/
    if(block_counter == NUMBER_OF_NVM_BLOCKS)
    {
        block_counter =2 ;

        MultiBlcokRequest.ResultStatus = NVM_REQ_OK;

#if(NvMMultiBlockCallback != STD_OFF)

            NvM_MultiBlockCallbackFunction(MultiBlcokRequest.request,MultiBlcokRequest.ResultStatus);
#endif
        MultiBlcokRequest.request = IDLE_REQUEST ;
    }
}
/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_CalcCRC                                */
/*    Function Description    : Calculate CRC for for queued CRC calculations requests  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnTyp                                           */
/****************************************************************************************/
static void NvM_MainFunction_CalcCRC(void)
{
    uint16 blockId = CRC_Job_Queue[CRC_Queue_Indeces.Head];

    if(CalculateCRC(blockId,NvMBlockDescriptor[blockId].NvMRamBlockDataAddress)== E_OK)
    {
        AdministrativeBlock[blockId].PrevCRCVal = CRCTempbuffer ;
        /*Remove current CRC job from queue*/
        CRCJob_Dequeue();

        if(NvMBlockDescriptor[blockId].NvMSingleBlockCallback != NULL )
        {
            NvMBlockDescriptor[blockId].NvMSingleBlockCallback(NVM_SET_RAMBLOCK_STATUS_API_ID,NVM_REQ_OK) ;
        }

    }
}

/****************************************************************************************/
/*    Function Name           : CalculateCRC                                            */
/*    Function Description    : Calculate CRC for RAM block                             */
/*    Parameter in            : blockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnTyp                                           */
/****************************************************************************************/
static Std_ReturnType CalculateCRC(uint16 blockId ,uint8* RamAddress)
{
    Std_ReturnType rtn_val;
    uint16 Length = NvMBlockDescriptor[blockId].NvMNvBlockLength;

    CRCTempbuffer = Compute_CRC32_Simple(RamAddress,( Length));
    *((uint32*)(TempBuffer+Length)) = CRCTempbuffer;
    rtn_val = E_OK ;
    return rtn_val ;
}
/****************************************************************************************/
/*    Function Name           : ReadRomBlock                                            */
/*    Function Description    : Read default value from ROM block                       */
/*    Parameter in            : SourceAddress, TargetAddress , Length                   */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
static void ReadRomBlock(uint8* SourceAddress , uint8* TargetAddress,uint16 Length)
{
    uint16 counter =0;
    for(counter = 0 ; counter<Length ; counter++)
    {
        TargetAddress[counter] = SourceAddress[counter] ;
    }
}

/****************************************************************************************/
/*    Function Name           : FillTempBuffer                                          */
/*    Function Description    : Add crc to ram block in one buffer                      */
/*    Parameter in            : Length, RAMBlock , crc                                  */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
void FillTempBuffer(uint16 Length,uint8* RAMBlock,uint32* crc)
{
    uint16 counter=0;
    for(counter =0;counter < Length ; counter++)
    {
        TempBuffer[counter]= RAMBlock[counter];
    }

    for(counter =Length;counter < Length+CRC_SIZE ; counter++)
    {
        TempBuffer[counter]= *(((uint8*) crc)+counter-Length);
    }
}

/****************************************************************************************/
/*    Function Name           : EmptyTempBuffer                                         */
/*    Function Description    : Move data from Tempbuffer to ram block                  */
/*    Parameter in            : Length, RAMBlock , crc                                  */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
static void EmptyTempBuffer(uint16 Length,uint8* RAMBlock,uint32* crc)
{
    uint16 counter=0;
    for(counter =0;counter < Length ; counter++)
    {
        RAMBlock[counter] = TempBuffer[counter];
    }

    for(counter =Length;counter < Length+4 ; counter++)
    {
        *((((uint8*)crc)+counter-Length)) = TempBuffer[counter];
    }
}

/****************************************************************************************/
/*    Function Name           : Compute_CRC32_Simple                                    */
/*    Function Description    : Calculate crc for a specific length of data             */
/*    Parameter in            : bytes, len                                              */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : uint32                                                  */
/****************************************************************************************/
static uint32 Compute_CRC32_Simple(uint8* bytes,uint16 len)
{
    uint32 crc = 0xFFFFFFFF; /* CRC value is 32bit */
    uint32 counter , ByteCounter;

    for ( ByteCounter=0  ; ByteCounter<len ; ByteCounter++)
    {
        uint32 x = (bytes[ByteCounter]);

        crc ^= (uint32)(x << 24); /* move byte counternto MSB of 32bcountert CRC */

        for (counter = 0; counter < 8; counter++)
        {
            if ((crc & 0x80000000) != 0) /* test for MSB = bit 31 */
            {
                crc = (uint32)((crc << 1) ^ CRC_POLYNOMIAL);
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}
