/*******************************************************************************
**                                                                            **
**  FILENAME     : Fee.c                                                      **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2020-3-12                                                  **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Yomna Mokhtar  , Sahar Elnagar                             **
*******************************************************************************/



/*****************************************************************************************/
/*                                   Include Component headres                           */
/*****************************************************************************************/
#include "Fee.h"
#include "Fee_Sector.h"

/*****************************************************************************************/
/*                                   Include Other  headres                              */
/*****************************************************************************************/
#include "Det.h"

/*****************************************************************************************/
/*                                   Local Macro Definitions                             */
/*****************************************************************************************/

//*****************************************************************************
//   internal processing states
//*****************************************************************************
#define IDLE_STATE                              (0U)
#define CHECK_SECTOR_AVAILABLE_SPACE            (1U)
#define TRANSFER_SECTOR                         (2U)
#define ERASE_SECTOR                            (3U)
#define WRITE_NEW_BLOCK                         (4U)
#define SET_NEXT_ADDRESS                        (5U)
#define FIND_BLOCK_IN_LOOKUP_TABLE              (6U)
#define FIND_BLOCK_IN_ACTIVE_SECTOR             (7U)
#define READ_BLOCK                              (8U)
#define END_JOB                                 (9U)
#define FIND_BLOCK                              (10U)
#define INVALIDATE_BLOCK                        (11U)
#define ERROR_STATE                             (12U)
#define READ_ACTIVE_SECTOR                      (13U)
#define SEARCH_SECTOR                           (14U)
#define CHECK_BLOCK_SIZE                        (15U)


/*****************************************************************************************/
/*             Local Type Definitions                                                    */
/*****************************************************************************************/

/*pending job type definition*/
typedef uint8 Fee_JobPendingType;
#define FEE_IDLE_JOB                            ((Fee_JobPendingType)0U)
#define FEE_READ_JOB                            ((Fee_JobPendingType)1U)
#define FEE_WRITE_JOB                           ((Fee_JobPendingType)2U)
#define FEE_INVALIDATE_JOB                      ((Fee_JobPendingType)4U)
#define FEE_INIT_JOB                            ((Fee_JobPendingType)5U)

typedef struct
{
    Fee_JobPendingType JobProcessing_State ;
     MemIf_StatusType  ModuleStatus ;
     MemIf_JobResultType JobResult ;
     uint8 InternalProcessingState ;
}str_ModuleInternalManagement ;

/*****************************************************************************************/
/*                                   Local Variable Definitions                          */
/*****************************************************************************************/

/*Save current Active sector information*/
str_ActiveSectorInfoType ActiveSectorInfo ;
str_ValidBlocksInfoType  ValidBlocksInfo[BLOCKS_NUM] = {0} ;
str_JobParametersCopy           JobParametersCopy ;
str_ModuleInternalManagement    ModuleInternalManagement ;
uint8 BlockData[MAX_CONFIGURED_BLOCK_SIZE + DATA_BLOCK_HEADER_SIZE] ;
static uint16 BlockIndex ;
static Std_ReturnType rtn_val1, rtn_val2 , rtn_val3, rtn_val4 , rtn_val5 ;
uint8 JobDone = 0 ;
uint8 JobError = 0 ;
/*Variable to save sector Status */
static SectorStatusType Status = 0  ;

/*****************************************************************************************/
/*                                Global Variables                                       */
/*****************************************************************************************/
extern Fee_BlockConfigType Fee_BlockConfig[BLOCKS_NUM] ;

/*****************************************************************************************/
/*                                   Local Function Declaration                          */
/*****************************************************************************************/

static void Fee_MainFunction_Write(void) ;
static void Fee_MainFunction_Read(void) ;
static void Fee_MainFunction_Invalidate(void) ;
static void Fee_MainFunction_Init(void) ;

/*****************************************************************************************/
/*                                   Global Function Definition                          */
/*****************************************************************************************/


/****************************************************************************************/
/*    Function Name           : Fee_Init                                                */
/*    Function Description    : Initializes the module                                  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Fee_Errors                                              */
/*    Requirment              : SWS_Fee_00085                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/

void Fee_Init( const Fee_ConfigType* ConfigPtr )
{

    /*Check if the module isn't in the UNINIT state */
    if(ModuleInternalManagement.ModuleStatus != MEMIF_UNINIT)
    {
        #if (FeeDevErrorDetect == STD_ON)
           Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_INIT_API_ID, FEE_E_INIT_FAILED);
        #endif
    }
    else
    {
      /*Initialize Active Sector with Valid Blocks array*/
      ActiveSectorInfo.SectorValidBlocksInfo = ValidBlocksInfo ;

      /*Set current job to reading*/
      ModuleInternalManagement.JobProcessing_State = FEE_INIT_JOB ;
      /*Set module state to busy*/
      ModuleInternalManagement.ModuleStatus = MEMIF_BUSY ;
      /*Set current job result to pending*/
      ModuleInternalManagement.JobResult = MEMIF_JOB_PENDING ;
      ModuleInternalManagement.InternalProcessingState = READ_ACTIVE_SECTOR ;

      /*Initialize return values for next state*/
      rtn_val1= E_PENDING;
      rtn_val2= E_PENDING;
      rtn_val3= E_PENDING;
      rtn_val4= E_PENDING ;
      /*Reset Job done flag and Error flag*/
      JobDone  = 0 ;
      JobError = 0 ;

      /*Clear all internal variables*/
      JobParametersCopy.BlockNum = 0 ;
      JobParametersCopy.DataBufPtr = NULL ;
      JobParametersCopy.Len = 0 ;

      BlockIndex = 0 ;
    }

}

/****************************************************************************************/
/*    Function Name           : Fee_Read                                                */
/*    Function Description    : Reads Length bytes of block Blocknumber at offset       */
/*                              BlockOffset into the buffer DataBufferPtr               */
/*    Parameter in            : BlockNumber , BlockOffset , DataBufferPtr , Length      */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : DataBufferPtr                                           */
/*    Return value            : Fee_ErrorsType                                          */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType Fee_Read( uint16 BlockNumber, uint16 BlockOffset, uint8* DataBufferPtr, uint16 Length )
{
  Std_ReturnType Return_Val = E_OK ;

  /*[SWS_Fee_00172]
   * If the current module status is MEMIF_UNINIT or MEMIF_BUSY, the function Fee_Read shall reject
   * the job request and return with E_NOT_OK.
   */
  if(ModuleInternalManagement.ModuleStatus == MEMIF_UNINIT)
  {
       #if (FeeDevErrorDetect == STD_ON)
            Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_READ_API_ID, FEE_E_UNINIT);
       #endif
       Return_Val = E_NOT_OK;
  }

  else if(ModuleInternalManagement.ModuleStatus == MEMIF_BUSY)
  {
    #if (FeeDevErrorDetect == STD_ON)
        Det_ReportRuntimeError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_READ_API_ID, FEE_E_BUSY);
    #endif
        Return_Val = E_NOT_OK;
  }

  /*[SWS_Fee_00134]
   * If development error detection is enabled for the module: the function Fee_Read shall check that the given
   * block number is valid (i.e. it has been configured). If this is not the case, the function Fee_Read
   * shall reject the read request, raise the development error FEE_E_INVALID_BLOCK_NO and return with E_NOT_OK.
   */
  else if(GetIndexFromBlockNum(BlockNumber, &BlockIndex) != E_OK)
  {
        #if (FeeDevErrorDetect == STD_ON)
          Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_READ_API_ID, FEE_E_INVALID_BLOCK_NO);
      #endif
      Return_Val = E_NOT_OK ;
  }

  /*[SWS_Fee_00135]
   * If development error detection is enabled for the module: the function Fee_Read shall check
   * that the given block offset is valid (i.e. that it is less than the block length configured for this block).
   * If this is not the case, the function Fee_Read shall reject the read request, raise the development error
   * FEE_E_INVALID_BLOCK_OFS and return with E_NOT_OK.
   */
  else if(BlockOffset > Fee_BlockConfig[BlockIndex].FeeBlockSize)
  {
      #if (FeeDevErrorDetect == STD_ON)
          Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_READ_API_ID, FEE_E_INVALID_BLOCK_OFS);
      #endif
      Return_Val = E_NOT_OK;
  }

  /*[SWS_Fee_00136]
   * If development error detection is enabled for the module: the function Fee_Read shall
   * check that the given data pointer is valid (i.e. that it is not NULL). If this is not the case,
   * the function Fee_Read shall reject the read request, raise the development error FEE_E_PARAM_POINTER
   * and return with E_NOT_OK.
   */
  else if(DataBufferPtr == NULL){
      #if (FeeDevErrorDetect == STD_ON)
           Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_READ_API_ID, FEE_E_PARAM_POINTER);
      #endif
      Return_Val = E_NOT_OK;
  }

  /*[SWS_Fee_00137]
   * If development error detection is enabled for the module: the function Fee_Read shall check
   * that the given length information is valid, i.e. that the requested length information plus
   * the block offset do not exceed the block end address. If this is not the case, the function
   * Fee_Read shall reject the read request, raise the development error FEE_E_INVALID_BLOCK_LEN
   * and return with E_NOT_OK.
   */
  else if((Length + BlockOffset) > Fee_BlockConfig[BlockIndex].FeeBlockSize){
      #if (FeeDevErrorDetect == STD_ON)
           Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_READ_API_ID, FEE_E_INVALID_BLOCK_LEN);
      #endif
      Return_Val = E_NOT_OK;
  }

  /*[SWS_Fee_00022]
   * If the current module status is MEMIF_IDLE or if the current module status is MEMIF_BUSY INTERNAL,
   * the function Fee_Read shall accept the read request, copy the given / computed parameters to module
   * internal variables, initiate a read job, set the FEE module status to MEMIF_BUSY, set the job result
   * to MEMIF_JOB_PENDING and return with E_OK.
  */
  else if(ModuleInternalManagement.ModuleStatus == MEMIF_IDLE || ModuleInternalManagement.ModuleStatus == MEMIF_BUSY_INTERNAL){

      /*Save Parameters to be used in the main function*/
      JobParametersCopy.BlockNum = BlockNumber ;
      JobParametersCopy.Len        = Length ;
      JobParametersCopy.DataBufPtr = DataBufferPtr ;

      /*Set current job to reading*/
      ModuleInternalManagement.JobProcessing_State = FEE_READ_JOB ;
      /*Set module state to busy*/
      ModuleInternalManagement.ModuleStatus = MEMIF_BUSY ;
      /*Set current job result to pending*/
      ModuleInternalManagement.JobResult = MEMIF_JOB_PENDING ;
      ModuleInternalManagement.InternalProcessingState = FIND_BLOCK_IN_LOOKUP_TABLE ;

      /*Initialize return values for next state*/
      rtn_val1= E_PENDING;
      rtn_val2= E_PENDING;
      rtn_val3= E_PENDING;
      rtn_val4= E_PENDING ;
      /*Reset Job done flag and Error flag*/
      JobDone  = 0;
      JobError = 0;

      Return_Val = E_OK;
  }

    return Return_Val;
}

/****************************************************************************************/
/*    Function Name           : Fee_Write                                               */
/*    Function Description    : Writes the contents of the DataBufferPtr to             */
/*                              the block BlockNumber.                                  */
/*    Parameter in            : BlockNumber ,DataBufferPtr                              */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType Fee_Write( uint16 BlockNumber, const uint8* DataBufferPtr )
{
  Std_ReturnType Return_Val = E_OK ;

  /*[SWS_Fee_00174]
   * If the current module status is MEMIF_UNINIT or MEMIF_BUSY, the function Fee_Write shall reject
   * the job request and return with E_NOT_OK.
   */
  if(ModuleInternalManagement.ModuleStatus == MEMIF_UNINIT){
     #if (FeeDevErrorDetect == STD_ON)
          Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_WRITE_API_ID, FEE_E_UNINIT);
     #endif
     Return_Val = E_NOT_OK;
  }

  else if(ModuleInternalManagement.ModuleStatus == MEMIF_BUSY){
     #if (FeeDevErrorDetect == STD_ON)
         Det_ReportRuntimeError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_WRITE_API_ID, FEE_E_BUSY);
     #endif
     Return_Val = E_NOT_OK;
  }

  /* [SWS_Fee_00138]
   * If development error detection is enabled for the module: the function Fee_Write shall check
   * that the given block number is valid (i.e. it has been configured). If this is not the case,
   * the function Fee_Write shall reject the write request, raise the development error
   * FEE_E_INVALID_BLOCK_NO and return with E_NOT_OK.
   */
  else if(GetIndexFromBlockNum(BlockNumber, &BlockIndex) != E_OK)
  {
      #if (FeeDevErrorDetect == STD_ON)
          Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_WRITE_API_ID, FEE_E_INVALID_BLOCK_NO);
      #endif
      Return_Val = E_NOT_OK ;
  }

  /* [SWS_Fee_00139]
   * If development error detection is enabled for the module: the function Fee_Write shall check
   * that the given data pointer is valid (i.e. that it is not NULL). If this is not the case,
   * the function Fee_Write shall reject the write request, raise the development error FEE_E_PARAM_POINTER
   * and return with E_NOT_OK.
   */
  else if(DataBufferPtr == NULL)
  {
      #if (FeeDevErrorDetect == STD_ON)
          Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_WRITE_API_ID, FEE_E_PARAM_POINTER);
      #endif
      Return_Val = E_NOT_OK ;
  }

  /*[SWS_Fee_00025]
   * If the current module status is MEMIF_IDLE or if the current module status is MEMIF_BUSY INTERNAL,
   *  the function Fee_Write shall accept the write request, copy the given / computed parameters to
   *  module internal variables, initiate a write job, set the FEE module status to MEMIF_BUSY,
   *  set the job result to MEMIF_JOB_PENDING and return with E_OK.
   */
  else if(ModuleInternalManagement.ModuleStatus == MEMIF_IDLE || ModuleInternalManagement.ModuleStatus == MEMIF_BUSY_INTERNAL){

     /*Save Parameters to be used in the main function*/
     JobParametersCopy.BlockNum   = BlockNumber ;
     JobParametersCopy.Len        = Fee_BlockConfig[BlockIndex].FeeBlockSize ;
     JobParametersCopy.DataBufPtr = (uint8*)DataBufferPtr ;

     /*Set current job to reading*/
     ModuleInternalManagement.JobProcessing_State = FEE_WRITE_JOB ;
     /*Set module state to busy*/
     ModuleInternalManagement.ModuleStatus = MEMIF_BUSY ;
     /*Set current job result to pending*/
     ModuleInternalManagement.JobResult = MEMIF_JOB_PENDING ;
     ModuleInternalManagement.InternalProcessingState = FIND_BLOCK;

     /*Initialize return values for next state*/
     rtn_val1= E_PENDING;
     rtn_val2= E_PENDING;
     rtn_val3= E_PENDING;
     rtn_val4= E_PENDING;
     /*Reset Job done flag and Error flag*/
     JobDone  = 0;
     JobError = 0;

     Return_Val = E_OK ;
    }
    return Return_Val;
}

/****************************************************************************************/
/*    Function Name           : Fee_Cancel                                              */
/*    Function Description    : Service to call the cancel function of the underlying   */
/*                              flash driver.                                           */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Notes                   :                                                         */
/****************************************************************************************/
void Fee_Cancel( void )
{
  /* [SWS_Fee_00124]
   * If development error detection is enabled for the module: the function Fee_Cancel shall check
   * if the module state is MEMIF_UNINIT. If this is the case the function Fee_Cancel shall raise the
   * development error FEE_E_UNINIT and return to the caller without changing any internal variables.
   */
  if(ModuleInternalManagement.ModuleStatus == MEMIF_UNINIT)
  {
     #if (FeeDevErrorDetect == STD_ON)
        Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_CANCEL_API_ID, FEE_E_UNINIT);
     #endif
  }

  /* [SWS_Fee_00080]
   * If the current module status is MEMIF_BUSY (i.e. the request to cancel a pending job is accepted by the
   * function Fee_Cancel), the function Fee_Cancel shall call the cancel function of the underlying flash driver.
   */
  else if(ModuleInternalManagement.ModuleStatus == MEMIF_BUSY)
  {
     Fls_Cancel();

     /* [SWS_Fee_00081]
      * If the current module status is MEMIF_BUSY (i.e. the request to cancel a pending job is accepted by the
      * function Fee_Cancel), the function Fee_Cancel shall reset the FEE module’s internal variables to make
      * the module ready for a new job request from the upper layer, i.e. it shall set the module status to
      * MEMIF_IDLE.
      */
     JobParametersCopy.BlockNum   = 0 ;
     JobParametersCopy.Len        = 0 ;
     JobParametersCopy.DataBufPtr = NULL ;

     /*Set current job to reading*/
     ModuleInternalManagement.JobProcessing_State = FEE_IDLE_JOB ;
     /*Set module state to busy*/
     ModuleInternalManagement.ModuleStatus = MEMIF_IDLE ;
     /*Set current job result to pending*/
     ModuleInternalManagement.JobResult = MEMIF_JOB_CANCELED ;
     ModuleInternalManagement.InternalProcessingState = IDLE_STATE;

     /*Initialize return values for next state*/
     rtn_val1= E_PENDING;
     rtn_val2= E_PENDING;
     rtn_val3= E_PENDING;
     rtn_val4= E_PENDING;
     /*Reset Job done flag and Error flag*/
     JobDone  = 0;
     JobError = 0;
  }
  else
  {
     /* [SWS_Fee_00184]
      * If the current module status is not MEMIF_BUSY (i.e. there is no job to cancel and therefore the request
      * to cancel a pending job is rejected by the function Fee_Cancel), the function Fee_Cancel shall raise the
      * runtime error FEE_E_INVALID_CANCEL.
      */
     Det_ReportRuntimeError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_CANCEL_API_ID, FEE_E_INVALID_CANCEL);
  }

}

/****************************************************************************************/
/*    Function Name           : Fee_GetStatus                                           */
/*    Function Description    : Service to return the Status.                           */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : MemIf_StatusType                                        */
/*    Notes                   : none                                                    */
/****************************************************************************************/
MemIf_StatusType Fee_GetStatus( void )
{
    return ModuleInternalManagement.ModuleStatus ;
}

/****************************************************************************************/
/*    Function Name           : Fee_GetJobResult                                        */
/*    Function Description    : Service to return the JobResult.                        */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : MemIf_JobResultType                                     */
/*    Notes                   : none                                                    */
/****************************************************************************************/
MemIf_JobResultType Fee_GetJobResult( void )
{
    MemIf_JobResultType Return_Val ;

    /* [SWS_Fee_00125]
     * If development error detection is enabled for the module: the function Fee_GetJobResult shall check
     * if the module state is MEMIF_UNINIT. If this is the case, the function Fee_GetJobResult shall raise
     * the development error FEE_E_UNINIT and return with MEMIF_JOB_FAILED.
     */
    if(ModuleInternalManagement.ModuleStatus == MEMIF_UNINIT){
       #if (FeeDevErrorDetect == STD_ON)
          Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_GETJOBRESULT_API_ID, FEE_E_UNINIT) ;
       #endif
       Return_Val = MEMIF_JOB_FAILED ;
    }
    else
    {
        Return_Val =  ModuleInternalManagement.JobResult ;
    }

    return Return_Val ;
}


/****************************************************************************************/
/*    Function Name           : Fee_InvalidateBlock                                     */
/*    Function Description    : Invalidates the block BlockNumber.                      */
/*    Parameter in            : BlockNumber                                             */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/
Std_ReturnType Fee_InvalidateBlock( uint16 BlockNumber )
{
    Std_ReturnType Return_Val = E_OK ;

    /* [SWS_Fee_00126]
     * If development error detection is enabled for the module: the function Fee_InvalidateBlock shall check
     * if the module status is MEMIF_UNINIT. If this is the case, the function Fee_InvalidateBlock shall reject
     * the invalidation request, raise the development error FEE_E_UNINIT and return with E_NOT_OK.
     */
    if(ModuleInternalManagement.ModuleStatus == MEMIF_UNINIT)
    {
       #if (FeeDevErrorDetect == STD_ON)
            Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_INVALIDATEBLOCK_API_ID, FEE_E_UNINIT);
       #endif
       Return_Val = E_NOT_OK ;
    }

    /* [SWS_Fee_00145]
     * The function Fee_InvalidateBlock shall check if the module status is MEMIF_BUSY. If this is the case,
     * the function Fee_InvalidateBlock shall reject the request, raise the runtime error FEE_E_BUSY and return
     * with E_NOT_OK.
     */
    else if(ModuleInternalManagement.ModuleStatus == MEMIF_BUSY)
    {
        #if(FeeDevErrorDetect == STD_ON)
            Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_INVALIDATEBLOCK_API_ID, FEE_E_BUSY);
        #endif
        Return_Val = E_NOT_OK ;
    }

    /* [SWS_Fee_00140]
     * If development error detection is enabled for the module: the function Fee_InvalidateBlock shall check
     * that the given block number is valid (i.e. it has been configured). If this is not the case, the function
     * Fee_InvalidateBlock shall reject the request, raise the development error FEE_E_INVALID_BLOCK_NO and return
     * with E_NOT_OK.
     */
    else if(GetIndexFromBlockNum(BlockNumber, &BlockIndex) != E_OK)
    {
        #if (FeeDevErrorDetect == STD_ON)
            Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_INVALIDATEBLOCK_API_ID, FEE_E_INVALID_BLOCK_NO);
        #endif
        Return_Val = E_NOT_OK ;
    }

    /*[SWS_Fee_00192]
     * The function Fee_InvalidateBlock shall check if the module state is MEMIF_IDLE or
     * MEMIF_BUSY_INTERNAL. If this is the case the module shall accept the invalidation request
     * and shall return E_OK to the caller. The block invalidation shall be executed asynchronously
     * in the module's main function as soon as the module has finished the internal management operation.
     */
    else if(ModuleInternalManagement.ModuleStatus == MEMIF_IDLE || ModuleInternalManagement.ModuleStatus == MEMIF_BUSY_INTERNAL)
    {
        /*Save Parameters to be used in the main function*/
        JobParametersCopy.BlockNum   = BlockNumber ;

        /*Set current job to Invalidate*/
        ModuleInternalManagement.JobProcessing_State = FEE_INVALIDATE_JOB ;
        /*Set module state to busy*/
        ModuleInternalManagement.ModuleStatus = MEMIF_BUSY ;
        /*Set current job result to pending*/
        ModuleInternalManagement.JobResult = MEMIF_JOB_PENDING ;
        ModuleInternalManagement.InternalProcessingState = INVALIDATE_BLOCK;

        /*Initialize return values for next state*/
        rtn_val1= E_PENDING;
        rtn_val2= E_PENDING;
        rtn_val3= E_PENDING;
        rtn_val4= E_PENDING ;
        /*Reset Job done flag and Error flag*/
        JobDone  = 0;
        JobError = 0;

        Return_Val = E_OK ;
    }
    return Return_Val ;
}

/****************************************************************************************/
/*    Function Name           : Fee_GetVersionInfo                                      */
/*    Function Description    : Service to return version information of the FEE module.*/
/*    Parameter in            : BlockNumber                                             */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/
#if (FeeVersionInfoApi == STD_ON)
   void Fee_GetVersionInfo( Std_VersionInfoType* VersionInfoPtr )
   {

      if(VersionInfoPtr == NULL){
          #if (FeeDevErrorDetect == STD_ON)
              Det_ReportError(FEE_MODULE_ID, FEE_0_INSTANCE_ID, FEE_GETVERSIONINFO_API_ID, FEE_E_PARAM_POINTER);
          #endif
      }

      VersionInfoPtr->moduleID = FEE_MODULE_ID;
      VersionInfoPtr->instanceID = FEE_0_INSTANCE_ID;
      /*******************************************************************************/
      //  Rest of version info
      /*******************************************************************************/

   }
#endif

/****************************************************************************************/
/*    Function Name           : Fee_JobEndNotification                                  */
/*    Function Description    : Requested job ended successfully                        */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Notes                   :                                                         */
/****************************************************************************************/
void Fee_JobEndNotification( void )
{
    /*Set JobDone global variable to 1*/
    JobDone = 1 ;
}

/****************************************************************************************/
/*    Function Name           : Fee_JobErrorNotification                                */
/*    Function Description    : Requested job ended with error                          */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Notes                   :                                                         */
/****************************************************************************************/
void Fee_JobErrorNotification( void )
{
    /*Set JobError global variable to 1*/
    JobError = 1 ;
}

/****************************************************************************************/
/*    Function Name           : Fee_MainFunction                                        */
/*    Function Description    : Service to handle the requested jobs and the internal   */
/*                              management operations                                   */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Notes                   : none                                                    */
/****************************************************************************************/
void Fee_MainFunction( void )
{
    if(ModuleInternalManagement.JobResult == MEMIF_JOB_PENDING)
    {
       switch (ModuleInternalManagement.JobProcessing_State)
       {
           /*Current pending job is Initialization job*/
           case FEE_INIT_JOB :
             Fee_MainFunction_Init();
           break;
           /*Current pending job is reading job*/
           case FEE_READ_JOB :
              Fee_MainFunction_Read();
           break;

           /*Current pending job is writing job*/
           case FEE_WRITE_JOB :
               Fee_MainFunction_Write() ;
           break ;

           /*Current pending job is Invalidation job*/
           case FEE_INVALIDATE_JOB :
               Fee_MainFunction_Invalidate() ;
           break ;
       }
    }
}

/*****************************************************************************************/
/*                                   Local Function Definitions                          */
/*****************************************************************************************/

/****************************************************************************************/
/*    Function Name           : Fee_MainFunction_Write                                  */
/*    Function Description    : perform the processing to Write a block                 */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
static void Fee_MainFunction_Write(void)
{
    static uint32 SectorEraseCount= 0 ;

    switch(ModuleInternalManagement.InternalProcessingState)
    {
        case SEARCH_SECTOR :
            /*search sector for next available space to write new block*/
            if(rtn_val1 == E_PENDING)
            {
                /*call search sector with current sector address*/
                rtn_val1 = SearchSectorValidBlocks(ActiveSectorInfo.StartAddress);
            }
            if(rtn_val1 == E_OK)
            {
                /*Go to check length state*/
                ModuleInternalManagement.InternalProcessingState = CHECK_SECTOR_AVAILABLE_SPACE ;
                rtn_val1= E_PENDING;
                rtn_val2= E_PENDING;
                rtn_val3= E_PENDING;
                rtn_val4= E_PENDING;
            }
            else if(rtn_val1 == E_NOT_OK)
            {
                /*report error*/
                ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
            }
            else
            {}
        break;
    /*******************case: CHECK_SECTOR_AVAILABLE_SPACE********************/
        case CHECK_SECTOR_AVAILABLE_SPACE:

            /*Compare required space to available space */
            if(JobParametersCopy.Len + DATA_BLOCK_HEADER_SIZE > GET_AVAILABLE_SPACE(ActiveSectorInfo.StartAddress, ActiveSectorInfo.InternalNextAvailableAddress))
            {
                /*Check if next sector Empty*/
                if(rtn_val1 == E_PENDING)
                {
                    rtn_val1 = GetSectorStatus(((ActiveSectorInfo.StartAddress + VIRTUAL_SECTOR_SIZE)% FLASH_SIZE) ,&Status) ;
                }
                if(rtn_val1 == E_OK && (Status == EMPTY_VIRTUAL_SECTOR) && rtn_val2==E_PENDING )
                {
                    rtn_val2 = E_OK ;
                }
                if(rtn_val1 == E_OK && rtn_val2!= E_OK)
                {
                    /*sector not erased before ,request erase for this sector*/
                    /*Go to erase state*/
                     ModuleInternalManagement.InternalProcessingState = ERASE_SECTOR ;
                }
                else
                {}
                if(rtn_val2 == E_OK)
                {
                    Status = COPY_VIRTUAL_SECTOR ;
                    /*Set sector status to COPY_VIRTUAL_SECTOR*/
                    if(rtn_val3 == E_PENDING)
                    {
                        rtn_val3 = SetSectorStatus(((ActiveSectorInfo.StartAddress + VIRTUAL_SECTOR_SIZE)% FLASH_SIZE),&Status);
                    }
                    if(rtn_val3 == E_OK)
                    {
                        /***start transferring***/
                        ModuleInternalManagement.InternalProcessingState =TRANSFER_SECTOR ;
                        rtn_val1= E_PENDING;
                        rtn_val2= E_PENDING;
                        rtn_val3= E_PENDING;
                        rtn_val4= E_PENDING;
                    }
                    else if(rtn_val3== E_NOT_OK)
                    {
                        ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                    }
                    else
                    {}
                }
                else if(rtn_val2== E_NOT_OK)
                {
                    ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                }
                else
                {}
            }
            else
            {
                /*proceed writing*/
                ModuleInternalManagement.InternalProcessingState = WRITE_NEW_BLOCK ;
                /*Initialize return values for next state*/
                rtn_val1= E_PENDING;
                rtn_val2= E_PENDING;
                rtn_val3= E_PENDING;
                rtn_val4= E_PENDING;
                rtn_val5= E_PENDING;
            }
        break;

    /*******************case: TRANSFER_SECTOR********************/
        case TRANSFER_SECTOR :

            /*Request transfer*/
                if(rtn_val1!=E_OK)
                {
                    rtn_val1 = TransferSector(ActiveSectorInfo.StartAddress , ((ActiveSectorInfo.StartAddress + VIRTUAL_SECTOR_SIZE)% FLASH_SIZE));
                }

                if(rtn_val1 == E_OK)
                {
                    Status = ACTIVE_VIRTUAL_SECTOR ;
                    /*Set new sector state to active*/
                    if(rtn_val2==E_PENDING)
                    {
                        rtn_val2=SetSectorStatus(((ActiveSectorInfo.StartAddress + VIRTUAL_SECTOR_SIZE)% FLASH_SIZE), &Status);
                    }

                    if(rtn_val2 == E_OK)
                    {
                        /*Set current block physical address */
                        ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].PhsicalAddressStart = \
                                ActiveSectorInfo.InternalNextAvailableAddress ;

                        /*Set previous sector to READY_FOR_ERASE*/
                        Status = READY_FOR_ERASE;
                        if(rtn_val3==E_PENDING)
                        {
                            rtn_val3=SetSectorStatus(ActiveSectorInfo.StartAddress , &Status);
                        }
                        if(rtn_val3 == E_OK)
                        {
                            ActiveSectorInfo.StartAddress = ((ActiveSectorInfo.StartAddress + VIRTUAL_SECTOR_SIZE)% FLASH_SIZE);
                            /*Write required block*/
                            ModuleInternalManagement.InternalProcessingState = WRITE_NEW_BLOCK ;
                            /*Initialize return values for next state*/
                            rtn_val1= E_PENDING;
                            rtn_val2= E_PENDING;
                            rtn_val3= E_PENDING;
                            rtn_val4= E_PENDING ;
                            rtn_val5= E_PENDING;
                        }
                        else if(rtn_val3== E_NOT_OK)
                        {
                            ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                        }
                        else
                        {}
                    }
                    else if(rtn_val2== E_NOT_OK)
                    {
                        ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                    }
                    else
                    {}
                }
                else if(rtn_val1== E_NOT_OK)
                {
                    ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                }
                else
                {}
        break;

/*******************case: ERASE_SECTOR********************/
        /*Erase the sector we are moving to */
        case ERASE_SECTOR :
            /*First read the times the sector has been erased */
            if(rtn_val1==E_PENDING)
            rtn_val1=ReadVariable(((ActiveSectorInfo.StartAddress + VIRTUAL_SECTOR_SIZE)% FLASH_SIZE)+8,(uint8*)&SectorEraseCount, 4);

            if(rtn_val1 == E_OK)
            {
                if(rtn_val2==E_PENDING)
                rtn_val2 = Erase_Virtualsector((ActiveSectorInfo.StartAddress + VIRTUAL_SECTOR_SIZE)% FLASH_SIZE);
                if(rtn_val2 == E_OK)
                {
                    /*Check if sector has not been erased before*/
                    if(SectorEraseCount == ERASED_WORD )
                    {
                        SectorEraseCount = 1;
                        rtn_val5 = E_OK;
                    }
                    else if(rtn_val5 == E_PENDING)
                    {
                        /*Increment Erase count*/
                        SectorEraseCount++;
                        rtn_val5 = E_OK;
                    }
                    else
                    {}
                    /*Check if we reached maximum erase times*/
                    if(SECTOR_MAX_ERASE_CYCLES == SectorEraseCount)
                    {
                        /*Set to E_NOT_OK , To go to error state*/
                        rtn_val3 = E_NOT_OK ;
                    }
                    /*Save erase count in sector header*/
                    if(rtn_val3== E_PENDING)
                    rtn_val3 = WriteVariable(((ActiveSectorInfo.StartAddress + VIRTUAL_SECTOR_SIZE)% FLASH_SIZE)+8,(uint8*)&SectorEraseCount, 4);

                    if(rtn_val3 == E_OK)
                    {
                        /*Set sector to EMPTY_VIRTUAL_SECTOR*/
                        Status = COPY_VIRTUAL_SECTOR;
                        if(rtn_val4 == E_PENDING)
                        {
                            rtn_val4 = SetSectorStatus(ActiveSectorInfo.StartAddress , &Status);
                        }
                        if(rtn_val4 == E_OK)
                        {
                            ModuleInternalManagement.InternalProcessingState = TRANSFER_SECTOR ;
                            /*Initialize return values for next state*/
                            rtn_val1= E_PENDING;
                            rtn_val2= E_PENDING;
                            rtn_val3= E_PENDING;
                            rtn_val4= E_PENDING;
                            rtn_val5= E_PENDING;
                        }
                        else if(rtn_val4 == E_NOT_OK)
                        {
                            ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                        }
                        else
                        {}
                    }
                    else if(rtn_val3 == E_NOT_OK)
                    {
                        ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                    }
                    else
                    {}
                }
                else if(rtn_val2 == E_NOT_OK)
                {
                    ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                }
                else
                {}
            }
            else if(rtn_val1 == E_NOT_OK)
            {
                ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
            }
            else
            {}
        break;

    /*******************case: FIND_BLOCK********************/
        case FIND_BLOCK :

                /*check if block was valid in Lookup table*/
                if(ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].Valid == TRUE)
                {
                    /*Change block status to Invalid*/
                    Status = INVALID_BLOCK ;
                    if(rtn_val1==E_PENDING)
                    {
                        rtn_val1 =WriteVariable(ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].PhsicalAddressStart ,\
                                           (uint8*)&Status , STATUS_LENGTH);
                    }
                    if(rtn_val1 == E_OK)
                    {
                        //rtn_val4 = ReadVariable()
                        /*Invalidate block in lookup table */
                        ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].Valid = FALSE;
                        /*decrement valid blocks number*/
                        ActiveSectorInfo.ValidBlocksNumber--;

                        /*Go to SEARCH_SECTOR ,
                         *To fill lookup table
                         */
                        ModuleInternalManagement.InternalProcessingState = SEARCH_SECTOR ;
                        /*Initialize return values for next state*/
                        rtn_val1= E_PENDING;
                        rtn_val2= E_PENDING;
                        rtn_val3= E_PENDING;
                        rtn_val4= E_PENDING;
                    }
                    else if(rtn_val1 == E_NOT_OK)
                    {
                        ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                    }
                    else
                    {}
                }
                /*Not saved in lookup table Valid , search in Active sector*/
                else
                {
                    if(rtn_val2 == E_PENDING)
                    {
                        /*Find required block in the sector*/
                        rtn_val2 = FindValidBlocks(JobParametersCopy.BlockNum);
                    }
                    if(rtn_val2 == E_OK)
                    {
                        /*Found block , repeat FIND_BLOCK state to invalidate the block */
                        ModuleInternalManagement.InternalProcessingState = FIND_BLOCK ;
                        /*Reset return value for next state*/
                        rtn_val1 =E_PENDING;
                        rtn_val2 =E_PENDING;
                    }
                    /*Block not found in active sector */
                    else if(rtn_val2 == E_NOT_OK)
                    {
                        /*Save in the lookup table*/
                        ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].BlockSize = JobParametersCopy.Len ;
                        ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].LogicalAddress = JobParametersCopy.BlockNum;
                        ModuleInternalManagement.InternalProcessingState = SEARCH_SECTOR ;
                       /*Initialize return values for next state*/
                        rtn_val1= E_PENDING;
                        rtn_val2= E_PENDING;
                        rtn_val3= E_PENDING;
                        rtn_val4= E_PENDING;
                    }
                    else
                    {}
                }
        break;

/*******************case: WRITE_NEW_BLOCK********************/
        case WRITE_NEW_BLOCK :
            if(rtn_val1==E_PENDING)
            {
                /*Read block header before writing ,make sure it's empty */
                rtn_val1 = ReadVariable(ActiveSectorInfo.InternalNextAvailableAddress, BlockData, DATA_BLOCK_HEADER_SIZE);
            }
            if(rtn_val1 == E_OK && rtn_val2 !=E_OK)
            {
                if(*((BlockStatusType*)BlockData) == EMPTY_BLOCK )
                {
                    rtn_val2 = E_OK ;
                }
                else
                {
                    /*Shift current available address with this block size saved in the header */
                    ActiveSectorInfo.InternalNextAvailableAddress += (*((uint16*)(BlockData+18)))+ DATA_BLOCK_HEADER_SIZE ;
                    rtn_val2 = E_OK ;
                }
            }
            /*Write block header*/
            if(rtn_val2 == E_OK)
            {
                if(rtn_val3 == E_PENDING)
                {
                    /*Save block status*/
                    *((BlockStatusType*)BlockData) = START_PROGRAM_BLOCK ;
                    /*block write cycles count*/
                     ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].WriteCycles++ ;
                    *((uint32*)(BlockData+8))    = ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].WriteCycles ;
                    /*Next block address */
                    *((uint32*)(BlockData+12))   = ERASED_WORD ;
                   /*Save block number*/
                    *((uint16*)(BlockData+16)) = JobParametersCopy.BlockNum ;
                    /*Save block size in header*/
                    *((uint16*)(BlockData+18)) = JobParametersCopy.Len ;
                    /*Write block header*/
                    rtn_val3 = WriteVariable(ActiveSectorInfo.InternalNextAvailableAddress\
                            ,BlockData,DATA_BLOCK_HEADER_SIZE);
                }
                if(rtn_val3 == E_OK)
                {

                    if(rtn_val4==E_PENDING)
                    {
                        /*Write data*/
                        rtn_val4 = WriteVariable(ActiveSectorInfo.InternalNextAvailableAddress+ DATA_BLOCK_HEADER_SIZE,\
                                  JobParametersCopy.DataBufPtr,JobParametersCopy.Len );
                    }
                     if(rtn_val4 == E_OK)
                    {

                       if(rtn_val5 == E_PENDING)
                       {
                           *((BlockStatusType*)BlockData) = VALID_BLOCK ;
                           /*Set block status as valid*/
                           rtn_val5 = WriteVariable(ActiveSectorInfo.InternalNextAvailableAddress\
                                    ,BlockData,STATUS_LENGTH);
                       }
                       if(rtn_val5 == E_OK)
                       {
                            /*Save in the lookup table as valid*/
                            ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].Valid = TRUE ;
                            /*Increment valid blocks*/
                            ActiveSectorInfo.ValidBlocksNumber++;
                            /*Save block physical address*/
                            ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].PhsicalAddressStart\
                                = ActiveSectorInfo.InternalNextAvailableAddress ;
                            /*Go to End Job state*/
                            ModuleInternalManagement.InternalProcessingState = SET_NEXT_ADDRESS ;
                            rtn_val1 = E_PENDING ;
                       }
                       else if(rtn_val5 == E_NOT_OK)
                       {
                           ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                       }
                       else
                       {}
                    }
                    else if(rtn_val4 == E_NOT_OK)
                    {
                        ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                    }
                    else
                    {}
            }
            else if(rtn_val3 == E_NOT_OK)
            {
                ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
            }
            else
            {}
        }
        break ;

    /*******************case: SET_NEXT_ADDRESS********************/
        case SET_NEXT_ADDRESS :
            /*Check if it's not the first block in the sector*/
            if(ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].PhsicalAddressStart != ActiveSectorInfo.StartAddress + SECTOR_HEADER_SIZE)
            {
                if(rtn_val1 == E_PENDING)
                {
                    /*Save current address as the previous's next block address to make linked list*/
                    rtn_val1 = WriteVariable(ActiveSectorInfo.PreviousBlockAddress + 12, (uint8*)(&ActiveSectorInfo.InternalNextAvailableAddress), 4);
                }
                if(rtn_val1 == E_OK)
                {
                    ActiveSectorInfo.PreviousBlockAddress = ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].PhsicalAddressStart;
                    /*Change next start address in the module */
                    ActiveSectorInfo.InternalNextAvailableAddress += JobParametersCopy.Len + DATA_BLOCK_HEADER_SIZE ;
                    /*Go to End state*/

                    ModuleInternalManagement.InternalProcessingState = END_JOB ;
                }
                else if(rtn_val1 == E_NOT_OK)
                {
                    ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                }
                else
                {}
            }
            else
            {
                ActiveSectorInfo.PreviousBlockAddress = ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].PhsicalAddressStart;
                /*Change next start address in the module */
                ActiveSectorInfo.InternalNextAvailableAddress += JobParametersCopy.Len + DATA_BLOCK_HEADER_SIZE ;
                /*Go to End state*/
                ModuleInternalManagement.InternalProcessingState = END_JOB ;
            }

            break;
/*******************case: ERROR_STATE********************/
        case ERROR_STATE :
            ModuleInternalManagement.JobProcessing_State = FEE_IDLE_JOB ;
            ModuleInternalManagement.JobResult           = MEMIF_JOB_FAILED ;
            ModuleInternalManagement.ModuleStatus        = MEMIF_IDLE ;
            /*Call error notification routine of the upper layer if configured*/
            FeeNvmJobErrorNotification() ;
        break ;
/*******************case: END_JOB********************/
    case END_JOB :
        ModuleInternalManagement.JobProcessing_State = FEE_IDLE_JOB ;
        ModuleInternalManagement.JobResult           = MEMIF_JOB_OK;
        ModuleInternalManagement.ModuleStatus        = MEMIF_IDLE ;
        /*Call end job notification routine of the upper layer if configured*/
        FeeNvmJobEndNotification() ;
    break ;
    }
}

/****************************************************************************************/
/*    Function Name           : Fee_MainFunction_Read                                   */
/*    Function Description    : perform the processing to read a block                  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
static void Fee_MainFunction_Read(void)
{
    switch(ModuleInternalManagement.InternalProcessingState)
    {
    /*******************case: FIND_BLOCK_IN_LOOKUP_TABLE********************/
    case FIND_BLOCK_IN_LOOKUP_TABLE :

        /*Check if block saved in lookup table */
        if(ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].Valid == TRUE)
        {
            ModuleInternalManagement.InternalProcessingState = READ_BLOCK ;
            /*Reset return value for next state*/
            rtn_val1 =E_PENDING;
        }
        else
        {
            /*Block wans't saved before , so search in it in the physical active sector */
            ModuleInternalManagement.InternalProcessingState = FIND_BLOCK_IN_ACTIVE_SECTOR ;
            /*Reset return value for next state*/
            rtn_val1 =E_PENDING;
        }
    break;
    /*******************case: READ_BLOCK********************/
    case READ_BLOCK :

         if(rtn_val1 == E_PENDING)
         {
             rtn_val1 = ReadVariable(ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].PhsicalAddressStart + DATA_BLOCK_HEADER_SIZE,
             JobParametersCopy.DataBufPtr,JobParametersCopy.Len);
         }
         if(rtn_val1 == E_OK)
         {
             ModuleInternalManagement.InternalProcessingState = END_JOB ;
         }
         else if(rtn_val1 == E_NOT_OK)
         {
             ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
         }
         else
         {}
    break;
    /*******************case: FIND_BLOCK_IN_ACTIVE_SECTOR********************/
    case FIND_BLOCK_IN_ACTIVE_SECTOR :

        if(rtn_val1 == E_PENDING)
        {
            /*Find required block in the sector*/
            rtn_val1=FindValidBlocks(JobParametersCopy.BlockNum);
        }
        if(rtn_val1 == E_OK)
        {
            /*Found block, go to read state*/
            ModuleInternalManagement.InternalProcessingState = CHECK_BLOCK_SIZE ;
            /*Reset return value for next state*/
            rtn_val1 =E_PENDING;
            rtn_val2 =E_PENDING;
        }
        else if(rtn_val1 == E_NOT_OK)
        {
            ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
        }
        else
        {}
    break ;
    /*******************case: CHECK_BLOCK_SIZE********************/
    case CHECK_BLOCK_SIZE :

        if(ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].BlockSize != Fee_BlockConfig[BlockIndex].FeeBlockSize)
        {
            /*Change block status physically to Invalid*/
            Status = INVALID_BLOCK ;
            if(rtn_val2==E_PENDING)
            {
                rtn_val2 = WriteVariable(ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].PhsicalAddressStart , (uint8*)&Status , STATUS_LENGTH);
            }
            if(rtn_val2 != E_PENDING)
            {
                ActiveSectorInfo.ValidBlocksNumber--;
                /*Change block status logically*/
                ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].Valid = FALSE;
                /*Go to Error Job state*/
                ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
            }

        }
        else
        {
            /*Block configurations not changed , go to read state*/
            ModuleInternalManagement.InternalProcessingState = READ_BLOCK ;
        }
    break;

/*******************case: END_JOB********************/
    case END_JOB :

        ModuleInternalManagement.JobProcessing_State = FEE_IDLE_JOB ;
        ModuleInternalManagement.JobResult           = MEMIF_JOB_OK;
        ModuleInternalManagement.ModuleStatus        = MEMIF_IDLE ;
        /*Call end job notification routine of the upper layer if configured*/
        FeeNvmJobEndNotification();
    break ;

/*******************case: ERROR_STATE********************/
    case ERROR_STATE :

        ModuleInternalManagement.JobProcessing_State = FEE_IDLE_JOB ;
        ModuleInternalManagement.JobResult           = MEMIF_BLOCK_INVALID;
        ModuleInternalManagement.ModuleStatus        = MEMIF_IDLE ;
        /*Call error notification routine of the upper layer if configured*/
        FeeNvmJobErrorNotification();
    break ;
    }
}

/****************************************************************************************/
/*    Function Name           : Fee_MainFunction_Invalidate                             */
/*    Function Description    : perform the processing to Invalidate a block            */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
static void Fee_MainFunction_Invalidate(void)
{
    switch(ModuleInternalManagement.InternalProcessingState)
    {
/*******************case: INVALIDATE_BLOCK********************/
    case INVALIDATE_BLOCK :

        /* Block Found in Lookup table */
        if(ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].Valid == TRUE)
        {
            /*Change block status physically to Invalid*/
            Status = INVALID_BLOCK ;
            if(rtn_val2 == E_PENDING)
            {
                rtn_val2 = WriteVariable(ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].PhsicalAddressStart , (uint8*)&Status , STATUS_LENGTH) ;
            }
            if(rtn_val2 == E_OK)
            {
                 ActiveSectorInfo.ValidBlocksNumber--;
                /*Change block status logically*/
                ActiveSectorInfo.SectorValidBlocksInfo[BlockIndex].Valid = FALSE;
                /*Go to End Job state*/
                ModuleInternalManagement.InternalProcessingState = END_JOB ;
            }
            else if(rtn_val2 == E_NOT_OK)
            {
                ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
            }
            else
            {}
        }
        else
        {
            ModuleInternalManagement.InternalProcessingState = FIND_BLOCK_IN_ACTIVE_SECTOR ;
            rtn_val1 = E_PENDING ;
            rtn_val2 = E_PENDING ;
        }
    break;
/*******************case: FIND_BLOCK_IN_ACTIVE_SECTOR********************/
    case FIND_BLOCK_IN_ACTIVE_SECTOR :
            if(rtn_val1 == E_PENDING)
            {
                /*Find required block in the sector*/
                rtn_val1 = FindValidBlocks(JobParametersCopy.BlockNum);
            }
            if(rtn_val1 == E_OK)
            {
                /*Found block, go to read state*/
                ModuleInternalManagement.InternalProcessingState = INVALIDATE_BLOCK ;
                /*Reset return value for next state*/
                rtn_val1 = E_PENDING;
                rtn_val2 = E_PENDING;
            }
            else if(rtn_val1 == E_NOT_OK)
            {
                ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
            }
            else
            {}
        break ;
/*******************case: END_JOB********************/
    case END_JOB :
        ModuleInternalManagement.JobProcessing_State = FEE_IDLE_JOB ;
        ModuleInternalManagement.JobResult           = MEMIF_JOB_OK;
        ModuleInternalManagement.ModuleStatus        = MEMIF_IDLE ;
        /*Call end job notification routine of the upper layer if configured*/
        FeeNvmJobEndNotification() ;
    break ;

/*******************case: ERROR_STATE********************/
    case ERROR_STATE :
        ModuleInternalManagement.JobProcessing_State = FEE_IDLE_JOB ;
        ModuleInternalManagement.JobResult           = MEMIF_JOB_FAILED;
        ModuleInternalManagement.ModuleStatus        = MEMIF_IDLE ;
        /*Call error notification routine of the upper layer if configured*/
        FeeNvmJobErrorNotification() ;
    break ;
    }
}

/****************************************************************************************/
/*    Function Name           : Fee_MainFunction_Init                                   */
/*    Function Description    : perform the processing of reading an active sector      */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
void  Fee_MainFunction_Init(void)
{
    switch(ModuleInternalManagement.InternalProcessingState)
    {
/*******************case: READ_ACTIVE_SECTOR********************/
    case READ_ACTIVE_SECTOR :
        /*Get Valid Sector */
        if(rtn_val1 == E_PENDING)
        {
            rtn_val1= FindActiveSector();
        }
        /*Sector Found*/
        if(rtn_val1 == E_OK)
        {
            /*Go to End job*/
            ModuleInternalManagement.InternalProcessingState = END_JOB ;
        }
        /*Found no Active sectors*/
        else if(rtn_val1 == E_NOT_OK)
        {
            /*Set first sector to the active one */
            if(rtn_val2==E_PENDING)
            {
                /*First erase sector*/
                rtn_val2 = Erase_Virtualsector(FLASH_BASE_ADDRESS);
            }
            if(rtn_val2 == E_OK)
            {
                if(rtn_val3 == E_PENDING)
                {
                    Status = ACTIVE_VIRTUAL_SECTOR ;
                    rtn_val3 = SetSectorStatus(FLASH_BASE_ADDRESS, &Status);
                }
                if(rtn_val3 == E_OK)
                {
                    /*Save sector start address*/
                    ActiveSectorInfo.StartAddress = FLASH_BASE_ADDRESS ;
                    /*First block address*/
                    ActiveSectorInfo.InternalNextAvailableAddress = ActiveSectorInfo.StartAddress + SECTOR_HEADER_SIZE ;
                    /*Go to End job*/
                    ModuleInternalManagement.InternalProcessingState = END_JOB ;
                }
                else if(rtn_val3 == E_NOT_OK)
                {
                    /*Go to Error state*/
                    ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
                }
                else
                {}
            }
            else if(rtn_val2 == E_NOT_OK)
            {
                /*Go to Error state*/
                ModuleInternalManagement.InternalProcessingState = ERROR_STATE ;
            }
            else
            {}
        }
        else
        {}
    break ;
/*******************case: END_JOB********************/
    case END_JOB :
        ModuleInternalManagement.JobProcessing_State = FEE_IDLE_JOB ;
        ModuleInternalManagement.JobResult           = MEMIF_JOB_OK;
        ModuleInternalManagement.ModuleStatus        = MEMIF_IDLE ;
    break ;

/*******************case: ERROR_STATE********************/
    case ERROR_STATE :
        ModuleInternalManagement.JobProcessing_State = FEE_IDLE_JOB ;
        ModuleInternalManagement.JobResult           = MEMIF_JOB_FAILED;
        ModuleInternalManagement.ModuleStatus        = MEMIF_IDLE ;
        FeeNvmJobErrorNotification();
    break ;
}
}




