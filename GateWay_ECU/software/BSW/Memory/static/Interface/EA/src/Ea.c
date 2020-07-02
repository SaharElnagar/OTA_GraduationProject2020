
/*******************************************************************************
**                                                                            **
**  FILENAME     : Ea.c                                                       **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2020-2-17                                                  **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Sahar Elnagar                                              **
**                                                                            **
**  Description  :   EEPROM Abstraction Interface                             **
*******************************************************************************/

/*  Each device should work on range of blocks so we can get
 *  the start address and the device from the logical block number
 *  [SRS_MemHwAb_14009]
 *  The physical device and the start address of a logical block shall
 *  be derived from the logical block identifier.
 * */

/*****************************************************************************************/
/*                                   Include Common headres                              */
/*****************************************************************************************/

/*****************************************************************************************/
/*                                   Include Other  headres                              */
/*****************************************************************************************/
#include "Det.h"

/*****************************************************************************************/
/*                                   Include Component headres                           */
/*****************************************************************************************/
#include "Ea.h"
#include "Ea_PrivateTypes.h"


/*****************************************************************************************/
/*                                   Local Function Definitions                          */
/*****************************************************************************************/
static void Ea_MainFunction_Read(void)    ;
static void Ea_MainFunction_Write(void)   ;
static void Ea_MainFunction_Invalidate(void) ;
static Std_ReturnType Find_Block(uint16 BlockNum );
static Std_ReturnType Find_Blocksize(uint16 BlockNum,uint16* Blocksize) ;
static Std_ReturnType Set_BlockStatus(uint16 BlockIndex , uint32* Status) ;
static Std_ReturnType Get_BlockStatus(uint16 BlockIndex , uint32* Status) ;
static Std_ReturnType RequestEepJob(uint16 BlockIndex , uint8* DataPtr, uint8 JobReq,uint16 Len) ;
/*****************************************************************************************/
/*                                   Local Macro Definition                              */
/*****************************************************************************************/


/*Macro to calculate physical Writing address*/
#define CALC_PHSICAL_W_ADD(LogialBlockNum)          ((LogialBlockNum - MODULE_LOGICAL_START_ADDRESS)*\
                                                        EA_VIRTUAL_PAGE_SIZE)
/*Macro to calculate physical Reading address*/
#define CALC_PHSICAL_R_ADD(LogialBlockNum,Offset)   (((LogialBlockNum - MODULE_LOGICAL_START_ADDRESS)*\
                                                        EA_VIRTUAL_PAGE_SIZE) + Offset )
/*States of current job processing*/
#define IDLE_JOB        0x00
#define READ_JOB        0x01
#define WRITE_JOB       0x02
#define ERASE_JOB       0x03
#define INVALIDATE_JOB  0x04

//*****************************************************************************
//  BLOCK header states
//  1-[SRS_MemHwAb_14014] The FEE and EA modules shall detect possible
//  data inconsistencies due to aborted / interrupted write operations
//  2-[SWS_Ea_00091] [SWS_Ea_00074] each block can be requested
//  by the upper layer module to invalid
//*****************************************************************************
#define EA_BLOCK_HEADER_SIZE        (0x04U)
#define EA_INVALID_BLOCk            (0xFFFFFFFF)
#define EA_INCONSISTANT_BLOCK       (0xFFFFFF00)
#define EA_VALID_BLOCK              (0xFFFF0000)


//*****************************************************************************
//  Module internal states
//*****************************************************************************


/*****************************************************************************************/
/*                                   Local types Definition                              */
/*****************************************************************************************/

//*****************************************************************************
//  Strut to save the required data for the reading process
//  BlockID : the logical Block ID in this module
//  PhysicalStartAddress: Start address of data to be read must be divisible by 4
//  Length : the number of bytes to read must be divisible by 4
//  DataPtr: pointer  that points to the place where data will be saved
//*****************************************************************************
typedef struct
{
    uint16 BlockIndex ;
    uint16 Len;
    uint8* DataBufPtr;
}str_ParametersCopy;

//*****************************************************************************
//  Internal Main_function states to handle a job request
//*****************************************************************************
typedef enum
{
    Read_block_status   ,
    Read_block          ,
    Write_block_Status  ,
    Write_block         ,
    End_job             ,
    Failed_job          ,
}InternalStatesType;
/*****************************************************************************************/
/*                                Local Variables Definition                             */
/*****************************************************************************************/

/*Variable to save the module state*/
static MemIf_StatusType EA_ModuleState = MEMIF_UNINIT ;

/*Struct to save the Read and Write functions parameters*/
static str_ParametersCopy ParametersCopy ;

/*Variable to save current job result  */
static MemIf_JobResultType JobResult = MEMIF_JOB_OK;

/*Variable to current processing job*/
static uint8 JobProcessing_State     = IDLE_JOB;

static uint8 Eep_JobDone =0;

static uint8 Eep_JobError =0;

static Std_ReturnType ret_val1 ;

static uint32 BlockStatus ;

static InternalStatesType InternalStates;
/*****************************************************************************************/
/*                                extern Variables                                       */
/*****************************************************************************************/
extern Ea_BlockConfigType Ea_BlockSConfig[EA_BLOCKS_NUM];


/*****************************************************************************************/
/*                                   Global Function Definition                          */
/*****************************************************************************************/

/****************************************************************************************/
/*    Function Name           : Ea_Init                                                 */
/*    Function Description    : Initializes the module                                  */
/*    Parameter in            : const Ea_ConfigType* ConfigPtr                          */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Ea_00084                                            */
/*    Notes                   :                                                         */
/****************************************************************************************/

void Ea_Init(const Ea_ConfigType* ConfigPtr )
{

uint16 BlockCounter = 0;

#if(EA_DEV_ERROR_DETECT == STD_ON)
    /*Report development error if module is already initialized  */
    if(EA_ModuleState != MEMIF_UNINIT)
    {
        Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_INIT_API_ID, EA_E_INIT_FAILED);
    }
#endif


    /*[SWS_Ea_00017] The function Ea_Init shall shall set the module state from MEMIF_UNINIT
     * to MEMIF_BUSY_INTERNAL once it starts the module’s initialization. (SRS_BSW_00101)
     */
    EA_ModuleState = MEMIF_BUSY_INTERNAL ;

    /*First Block address = first address in EEPROM*/
    Ea_BlockSConfig[0].PhysicalStartAddress = EEP_START_ADDRESS;
    /*calculate each block physical start address*/
    for(BlockCounter= 1 ; BlockCounter<EA_BLOCKS_NUM ; BlockCounter++)
    {
        /*Each block start address = previous block address + header size + previous block size*/
        Ea_BlockSConfig[BlockCounter].PhysicalStartAddress =\
        Ea_BlockSConfig[BlockCounter-1].PhysicalStartAddress + EA_BLOCK_HEADER_SIZE + Ea_BlockSConfig[BlockCounter-1].BlockSize;
    }


    /*[SWS_Ea_00128]  If initialization is finished within Ea_Init, the function Ea_Init
     *  shall set the module state from MEMIF_BUSY_INTERNAL to MEMIF_IDLE once initialization
     *   has been successfully finished. (SRS_BSW_00406
     */
    EA_ModuleState = MEMIF_IDLE ;
}

/****************************************************************************************/
/*    Function Name           : Ea_Read                                                 */
/*    Function Description    : Reads Length bytes of block Blocknumber at offset       */
/*                              BlockOffset into the buffer DataBufferPtr               */
/*    Parameter in            : BlockNumber , BlockOffset , Length                      */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : DataBufferPtr                                           */
/*    Return value            : Std_ReturnType                                          */
/*    Requirment              : SWS_Ea_00086                                            */
/*    Notes                   :                                                         */
/****************************************************************************************/

Std_ReturnType Ea_Read(uint16 BlockNumber,uint16 BlockOffset,uint8* DataBufferPtr, uint16 Length )
{
    uint16 BlockSize  = 0;
    Std_ReturnType rtn_val = E_OK ;

    Find_Blocksize(BlockNumber,&BlockSize);
    /*
     * [SWS_Ea_00130] If development error detection for the module EA is enabled:
     *  the function Ea_Read shall check if the module state is MEMIF_UNINIT.
     *  If this is the case, the function Ea_Read shall reject the read request,
     *  raise the development error EA_E_UNINIT and return with E_NOT_OK.  (SRS_BSW_00406)
     */
    if(EA_ModuleState == MEMIF_UNINIT)
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_READ_API_ID, EA_E_UNINIT);
        #endif

        rtn_val =  E_NOT_OK ;
    }
    /*Check parameter pointer is NULL*/
    else if(DataBufferPtr == NULL_PTR)
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_READ_API_ID, EA_E_PARAM_POINTER);
        #endif

        rtn_val =  E_NOT_OK ;
    }

    /*
     * [SWS_Ea_00147]If development error detection is enabled for the module:
     * the function Ea_Read shall check whether the given block number is valid
     *  (i.e. inside the configured range)
     */
    else if((Find_Block(BlockNumber)== E_NOT_OK) ||(BlockNumber == 0) ||( BlockNumber == 0xFFFF))
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_READ_API_ID, EA_E_INVALID_BLOCK_NO);
        #endif

        rtn_val =  E_NOT_OK ;
    }
    /*[SWS_Ea_00167] The function Ea_Read shall check if the module state is MEMIF_BUSY.
     * If this is the case, the function Ea_Read shall reject the read request,
     * raise the runtime error EA_E_BUSY and return with E_NOT_OK.
     */
    else if(EA_ModuleState == MEMIF_BUSY)
    {
        /*report error to DEM Module to report runtime error*/

        rtn_val =  E_NOT_OK ;
    }

    /*
     * [SWS_Ea_00168] If development error detection is enabled for the module:
     *  the function Ea_Read shall check that the given block offset is valid
     *  (i.e. that it is less than the block length configured for this block)
     */
    else if(BlockOffset >= BlockSize)
    {


            #if(EA_DEV_ERROR_DETECT == STD_ON)
                Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_READ_API_ID, EA_E_INVALID_BLOCK_OFS);
            #endif

            rtn_val =  E_NOT_OK ;
    }

    /*
     * [SWS_Ea_00169]  If development error detection is enabled for the module:
     *  the function Ea_Read shall check that the given length information is valid,
     *  i.e. that the requested length information plus the block offset do not exceed
     *  the block end address (block start address plus configured block length).
     *  If this is not the case, the function Ea_Read shall reject the read request,
     *  raise the development error EA_E_INVALID_BLOCK_LEN and return with E_NOT_OK.
     */
    else if((Length + BlockOffset)  >  BlockSize )
    {
    #if(EA_DEV_ERROR_DETECT == STD_ON)
        Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_READ_API_ID, EA_E_INVALID_BLOCK_LEN);
    #endif

    rtn_val =  E_NOT_OK ;
    }
    else
    {
        /*Save Parameters to be used in the main function*/
        ParametersCopy.Len        = Length ;
        ParametersCopy.DataBufPtr = DataBufferPtr ;

        /*Set current job to reading*/
        JobProcessing_State = READ_JOB ;

        /*Set module state to busy*/
        EA_ModuleState = MEMIF_BUSY ;

        /*Reset Job done flag and Error flag*/
        Eep_JobDone = 0 ;
        Eep_JobError = 0 ;

        /*Set Internal state*/
        InternalStates = Read_block_status ;
        ret_val1  = E_PENDING;
        /*Set current job result to pending*/
        JobResult = MEMIF_JOB_PENDING ;
    }
    return rtn_val ;
}


/****************************************************************************************/
/*    Function Name           : Ea_Write                                                */
/*    Function Description    : Writes the contents of the DataBufferPtr to             */
/*                              the block BlockNumber.                                  */
/*    Parameter in            : BlockNumber ,DataBufferPtr                              */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirment              : SWS_Ea_00087                                            */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType Ea_Write( uint16 BlockNumber,const uint8* DataBufferPtr )
{
    Std_ReturnType rtn_val = E_OK ;

    /*[SWS_Ea_00181] If the current module status is MEMIF_UNINIT or MEMIF_BUSY,
     * the function Ea_Write shall reject the job request and return with E_NOT_OK.
     *(SRS_MemHwAb_14010)
     */
    if(EA_ModuleState == MEMIF_UNINIT)
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_WRITE_API_ID, EA_E_UNINIT);
        #endif

        rtn_val =E_NOT_OK ;
    }

    /*[SWS_Ea_00172] If development error detection is enabled for the module:
     * the function Ea_Write shall check that the given data pointer is valid
     * (i.e. that it is not NULL). If this is not the case, the function
     * Ea_Write shall reject the write request, raise the development error
     * EA_E_PARAM_POINTER and return with E_NOT_OK.  (SRS_BSW_00323)
     */
    else if(DataBufferPtr == NULL_PTR)
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_WRITE_API_ID, EA_E_PARAM_POINTER);
        #endif

        rtn_val =E_NOT_OK ;
    }
    /*[SWS_Ea_00181] If the current module status is MEMIF_UNINIT or MEMIF_BUSY,
     * the function Ea_Write shall reject the job request and return with E_NOT_OK.
     *(SRS_MemHwAb_14010)
     */
    else if(EA_ModuleState == MEMIF_BUSY)
    {
        /*Call DEM module to report runtime error*/

        rtn_val =E_NOT_OK ;
    }

    /*[SWS_Ea_00148]  If development error detection for the module EA is enabled: the
      function Ea_Write shall check whether the given block number is valid (i.e. inside
      the configured range). If this is not the case, the function Ea_Write shall reject the
      write request, raise the development error EA_E_INVALID_BLOCK_NO and return
      with E_NOT_OK.  (SRS_BSW_00323)
     * */
    else if((Find_Block(BlockNumber)== E_NOT_OK) ||(BlockNumber == 0 )||( BlockNumber == 0xFFFF))
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_WRITE_API_ID, EA_E_INVALID_BLOCK_NO);
        #endif

        rtn_val =E_NOT_OK ;
    }
    /*
     * NO Errors Then:
     * Copy Parameters to be used in the main function
     */
    else
    {

        /*Save Parameters to be used in the main function*/
        ParametersCopy.Len        = Ea_BlockSConfig[ParametersCopy.BlockIndex].BlockSize ;
        ParametersCopy.DataBufPtr =(uint8*) DataBufferPtr ;
        /*Set current job to writing*/
        JobProcessing_State = WRITE_JOB ;

        /*Set module state to busy*/
        EA_ModuleState = MEMIF_BUSY ;

        /*Reset Job done flag and Error flag*/
        Eep_JobDone = 0 ;
        Eep_JobError = 0 ;

        /*Set Internal state*/
        InternalStates = Write_block_Status ;
        BlockStatus = EA_INCONSISTANT_BLOCK ;
        ret_val1  = E_PENDING;
        /*Set current job result to pending*/
        JobResult = MEMIF_JOB_PENDING ;
    }
    return rtn_val ;
}

/****************************************************************************************/
/*    Function Name           : Ea_JobEndNotification                                  */
/*    Function Description    : Requested job ended successfully                        */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Notes                   :                                                         */
/****************************************************************************************/
void Ea_JobEndNotification(void)
{
    Eep_JobDone = 1;
}


/****************************************************************************************/
/*    Function Name           : Ea_JobErrorNotification                                 */
/*    Function Description    : Requested job ended with error                          */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Notes                   :                                                         */
/****************************************************************************************/
void Ea_JobErrorNotification(void)
{
    Eep_JobError = 1;
}

/****************************************************************************************/
/*    Function Name           : Ea_Cancel                                               */
/*    Function Description    : Cancels the ongoing asynchronous operation              */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Ea_00088                                            */
/*    Notes                   : The cancel functions shall only reset their modules     */
/*                              internal variables so that a new job can be accepted    */
/*                              by the modules.                                         */
/****************************************************************************************/
void Ea_Cancel(void)
{
    /*[SWS_Ea_00132] If development error detection for the module EA is enabled:
     * the function Ea_Cancel shall check if the module state is MEMIF_UNINIT.
     * If this is the case, the function Ea_Cancel shall raise the development
     * error EA_E_UNINIT and return to the caller without changing any internal variables.
     *  (SRS_BSW_00406)
     *  */
    if(EA_ModuleState == MEMIF_UNINIT)
     {
         #if(EA_DEV_ERROR_DETECT == STD_ON)
             Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_CANCEL_API_ID, EA_E_UNINIT);
         #endif
     }

    /*[SWS_Ea_00173] If the current module status is not MEMIF_BUSY
     * (i.e. there is no job to cancel and therefore the request to cancel
     *  a pending job is rejected by the function Ea_Cancel), the function
     *  Ea_Cancel shall raise the runtime error EA_E_INVALID_CANCEL.
     *(SRS_BSW_00323)
     */
    else if(EA_ModuleState != MEMIF_BUSY)
    {
         /*Report Runtime error to DEM Module EA_E_INVALID_CANCEL*/
    }

    /*
     * [SWS_Ea_00077] If the current module status is MEMIF_BUSY
     *  (i.e. the request to cancel a pending job is accepted by the function Ea_Cancel),
     *  the function Ea_Cancel shall call the cancel function of the underlying EEPROM driver.
     * (SRS_MemHwAb_14031)
     */
    else
    {
        /*[SWS_Ea_00078] If the current module status is MEMIF_BUSY
         * (i.e. the request to cancel a pending job is accepted by the function Ea_Cancel),
         * the function Ea_Cancel shall reset the EA module’s internal variables to make
         * the module ready for a new job request. I.e. the function Ea_Cancel shall set
         * the job result to MEMIF_JOB_CANCELED and the module status to MEMIF_IDLE.
         * (SRS_MemHwAb_14031)
         */

        /*Set current job to writing*/
        JobProcessing_State = IDLE_JOB ;

        /*Set module state to busy*/
        EA_ModuleState = MEMIF_IDLE ;

        /*Set current job result to pending*/
        JobResult = MEMIF_JOB_CANCELED ;

        /*Reset Job done flag and Error flag*/
        Eep_JobDone = 0 ;
        Eep_JobError = 0 ;

        /*call the cancel function of the underlying EEPROM driver*/
        Eep_Cancel();
    }
}

/****************************************************************************************/
/*    Function Name           : Ea_GetStatus                                            */
/*    Function Description    : Service to return the Status.                           */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : MemIf_StatusType                                        */
/*    Requirment              : SWS_Ea_00089                                            */
/*    Notes                   : none                                                    */
/****************************************************************************************/
MemIf_StatusType Ea_GetStatus(void)
{
    return EA_ModuleState ;
}

/****************************************************************************************/
/*    Function Name           : Ea_GetJobResult                                         */
/*    Function Description    : Service to return the JobResult.                        */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : MemIf_StatusType                                        */
/*    Requirment              : SWS_Ea_00090                                            */
/*    Notes                   : none                                                    */
/****************************************************************************************/
MemIf_JobResultType Ea_GetJobResult(void)
{
    return JobResult ;
}

/****************************************************************************************/
/*    Function Name           : Ea_InvalidateBlock                                      */
/*    Function Description    : Invalidates the block BlockNumber.                      */
/*    Parameter in            : BlockNumber                                             */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirment              : SWS_Ea_00091                                            */
/*    Notes                   : SRS_MemHwAb_14028                                       */
/****************************************************************************************/
Std_ReturnType Ea_InvalidateBlock(uint16 BlockNumber)
{
    Std_ReturnType rtn_val = E_OK ;

    /*[SWS_Ea_00135]  If development error detection for the module Ea is enabled: the
      function Ea_InvalidateBlock shall check if the module state is MEMIF_UNINIT.
      If this is the case, the function Ea_InvalidateBlock shall reject the invalidation
      request, raise the development error EA_E_UNINIT and return with E_NOT_OK.(SRS_BSW_00323)
     */
    if(EA_ModuleState == MEMIF_UNINIT)
     {
         #if(EA_DEV_ERROR_DETECT == STD_ON)
             Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_INVALIDATE_BLOCK_API, EA_E_UNINIT);
         #endif
         rtn_val = E_NOT_OK ;
     }
    /*[SWS_Ea_00175] The function Ea_InvalidateBlock shall check if the module
      state is MEMIF_BUSY. If this is the case, the function Ea_InvalidateBlock shall
      reject the invalidation request, raise the runtime error EA_E_BUSY and return with
      E_NOT_OK
       */
     else if(EA_ModuleState == MEMIF_BUSY)
     {
          /*Call DEM module to report runtime error*/
          rtn_val =E_NOT_OK ;
      }
     else
     {
         /*Get block index */
         Find_Block(BlockNumber);

         /*reset job flags*/
         Eep_JobDone  = 0;
         Eep_JobError = 0;

         JobResult = MEMIF_JOB_PENDING ;
         ret_val1  = E_PENDING;
         /*Set Internal state*/
         InternalStates = Write_block_Status ;
         BlockStatus = EA_INCONSISTANT_BLOCK ;
         EA_ModuleState = MEMIF_BUSY ;
     }

    return rtn_val ;
}
/****************************************************************************************/
/*    Function Name           : Ea_MainFunction                                         */
/*     Function Description    : Service to handle the requested jobs and the internal   */
/*                              management operations                                   */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Ea_00096                                            */
/*    Notes                   : none                                                    */
/****************************************************************************************/
void Ea_MainFunction(void)
{
  switch (JobProcessing_State)
   {
       /*Current pending job is reading job*/
       case READ_JOB :
          Ea_MainFunction_Read();
       break;

       /*Current pending job is writing job*/
       case WRITE_JOB :
           Ea_MainFunction_Write() ;
       break ;

       /*Current pending job is compare job*/
       case INVALIDATE_JOB :
           Ea_MainFunction_Invalidate();
       break ;
   }

}

/****************************************************************************************/
/*    Function Name           : Ea_MainFunction_Read                                    */
/*     Function Description    : Service to handle internal processing of a read job    */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : none                                                    */
/*    Notes                   : none                                                    */
/****************************************************************************************/
void Ea_MainFunction_Read(void)
{

    switch(InternalStates)
    {
/***********************case :Read_block*******************/
        case Read_block_status :
            /* [SWS_Ea_00074]  The function Ea_MainFunction shall check,
             * whether the block requested for reading has been invalidated by the upper layer module.
             */

             if(ret_val1 == E_PENDING )
             {
                 /*Request to read block header*/
                 ret_val1= Get_BlockStatus(ParametersCopy.BlockIndex, &BlockStatus) ;
             }
             else if(ret_val1 == E_OK)
             {
                 if(BlockStatus == EA_VALID_BLOCK)
                 {
                    InternalStates = Read_block ;
                    ret_val1 = E_PENDING ;
                 }
                /* [SWS_Ea_00104]   The function Ea_MainFunction shall check the consistency of the
                 * logical block being read before notifying the caller
                 */
                 else if(BlockStatus == EA_INCONSISTANT_BLOCK )
                 {
                     InternalStates = End_job ;
                     JobResult = MEMIF_BLOCK_INCONSISTENT ;
                 }
                 /*Check if the Block status not Valid*/
                 /* If so, the function Ea_MainFunction shall set the job result to MEMIF_BLOCK_INVALID
                  * and call the job error notification function if configured
                  */
                 else
                 {
                     InternalStates = End_job ;
                     JobResult = MEMIF_BLOCK_INVALID ;
                 }
             }
             else
             {
                 InternalStates = End_job ;
                 JobResult      = MEMIF_JOB_FAILED ;
             }
         break ;
 /***********************case :Read_block*******************/
         case Read_block :
            /*Call Eep_Read if block is valid and consistent*/
            if(ret_val1 == E_PENDING)
            {
                /*Request read job from Eep module*/
                ret_val1 = RequestEepJob(ParametersCopy.BlockIndex, ParametersCopy.DataBufPtr,JobProcessing_State,ParametersCopy.Len);
            }
            else if(ret_val1 == E_NOT_OK)
            {
                /*Report job failed if Eep return not OK*/
                JobResult   = MEMIF_JOB_FAILED ;
                InternalStates = End_job ;
            }
            else
            {
                JobResult = MEMIF_JOB_OK ;
                InternalStates = End_job ;
            }
        break ;
/***********************case :End_job*******************/
        case End_job :
            if(JobResult == MEMIF_JOB_OK)
            {
                EaNvmJobEndNotification();
            }
            else
            {
                EaNvmJobErrorNotification();
            }
            EA_ModuleState = MEMIF_IDLE ;
            ret_val1 = E_PENDING ;
            JobProcessing_State = IDLE_JOB ;
        break;
    }
}

/****************************************************************************************/
/*    Function Name           : Ea_MainFunction_Write                                   */
/*     Function Description   : Service to handle internal processing of a write job    */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : none                                                    */
/*    Notes                   : none                                                    */
/****************************************************************************************/
void Ea_MainFunction_Write(void)
{
    switch(InternalStates)
    {
/***********************case :Write_block_Status*******************/
        case Write_block_Status :
            if(ret_val1 == E_PENDING)
            {
                ret_val1= Set_BlockStatus(ParametersCopy.BlockIndex, &BlockStatus);
            }
            else if(ret_val1 == E_OK)
            {
                if(BlockStatus == EA_INCONSISTANT_BLOCK)
                {
                    InternalStates = Write_block ;
                    ret_val1 = E_PENDING ;
                }
                else
                {
                    JobResult = MEMIF_JOB_OK;
                    InternalStates = End_job ;
                    ret_val1 = E_PENDING ;

                }
            }
            else
            {
                JobResult = MEMIF_JOB_FAILED ;
                InternalStates = End_job ;
                ret_val1 = E_PENDING ;
            }
        break;
/***********************case :Write_block*******************/
        case Write_block :
            if(ret_val1 == E_PENDING)
               {
                /*Request write job from Eep module*/
                ret_val1 = RequestEepJob(ParametersCopy.BlockIndex, ParametersCopy.DataBufPtr,JobProcessing_State,ParametersCopy.Len);
               }
               else if(ret_val1 == E_OK)
               {
                   InternalStates = Write_block_Status ;
                   BlockStatus = EA_VALID_BLOCK ;
                   ret_val1 = E_PENDING ;
               }
               else
               {
                   JobResult = MEMIF_JOB_FAILED ;
                   InternalStates = End_job ;
                   ret_val1 = E_PENDING ;
               }
        break;
/***********************case :End_job*******************/
        case End_job :
            if(JobResult == MEMIF_JOB_OK)
            {
                EaNvmJobEndNotification();
            }
            else
            {
                EaNvmJobErrorNotification();
            }
            EA_ModuleState = MEMIF_IDLE ;
            ret_val1 = E_PENDING ;
            JobProcessing_State = IDLE_JOB ;
        break;
    }
}

/****************************************************************************************/
/*    Function Name           : Ea_MainFunction_Invalidate                              */
/*     Function Description   : Internal processing for Invalidate block request        */
/*    Parameter in            : BlockNum                                                */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : none                                                    */
/*    Notes                   : none                                                    */
/****************************************************************************************/
void Ea_MainFunction_Invalidate(void)
{
    switch(InternalStates)
    {
        case Write_block_Status :
            if(ret_val1 == E_PENDING)
           {
               BlockStatus = EA_INVALID_BLOCk ;
               ret_val1= Set_BlockStatus(ParametersCopy.BlockIndex, &BlockStatus);
           }
           else if(ret_val1 == E_OK)
           {
               JobResult = MEMIF_JOB_OK;
               InternalStates = End_job ;
               ret_val1 = E_PENDING ;
           }
           else
           {
               JobResult = MEMIF_JOB_FAILED ;
               InternalStates = End_job ;
               ret_val1 = E_PENDING ;
           }
        break;
        case End_job :
            if(JobResult == MEMIF_JOB_OK)
           {
               EaNvmJobEndNotification();
           }
           else
           {
               EaNvmJobErrorNotification();
           }
           EA_ModuleState = MEMIF_IDLE ;
           ret_val1 = E_PENDING ;
           JobProcessing_State = IDLE_JOB ;
        break;
    }

}


/****************************************************************************************/
/*    Function Name           : Find_Block                                              */
/*     Function Description   : Search configured blocks                                */
/*    Parameter in            : BlockNum                                                */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : none                                                    */
/*    Notes                   : none                                                    */
/****************************************************************************************/
static Std_ReturnType Find_Block(uint16 BlockNum )
{
    uint16 counter = 0 ;
    Std_ReturnType rtn_val = E_NOT_OK ;

    for(counter =0 ;counter < EA_BLOCKS_NUM ;counter ++)
    {
        if(Ea_BlockSConfig[counter].BlockNumber == BlockNum)
        {
            rtn_val = E_OK ;
            ParametersCopy.BlockIndex = counter ;
            break;
        }
    }
    return rtn_val ;
  }

/****************************************************************************************/
/*    Function Name           : Find_Block                                              */
/*     Function Description   : Search configured blocks                                */
/*    Parameter in            : BlockNum                                                */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : none                                                    */
/*    Notes                   : none                                                    */
/****************************************************************************************/
static Std_ReturnType Find_Blocksize(uint16 BlockNum,uint16* Blocksize)
{
    uint16 counter = 0 ;
    Std_ReturnType rtn_val = E_NOT_OK ;

    for(counter =0 ;counter < EA_BLOCKS_NUM ;counter ++)
    {
        if(Ea_BlockSConfig[counter].BlockNumber == BlockNum)
        {
            rtn_val = E_OK ;
            *Blocksize = Ea_BlockSConfig[counter].BlockSize ;
            break;
        }
    }
    return rtn_val ;
  }

/****************************************************************************************/
/*    Function Name           : Set_BlockStatus                                         */
/*     Function Description   : service to write block header                           */
/*    Parameter in            : BlockIndex  , Status                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirment              : none                                                    */
/*    Notes                   : none                                                    */
/****************************************************************************************/
static Std_ReturnType Set_BlockStatus(uint16 BlockIndex , uint32* Status)
{
    Std_ReturnType rtn_val = E_PENDING;
    uint32 BlockAdrs = 0 ;

    if(Eep_GetStatus()!= MEMIF_BUSY && Eep_JobDone != 1)
    {
        /*Reset Job done flag and Error flag*/
        Eep_JobDone = 0 ;
        Eep_JobError = 0 ;

        /*Request Write job*/
        BlockAdrs = Ea_BlockSConfig[BlockIndex].PhysicalStartAddress ;
        Eep_Write(BlockAdrs,(uint8*)Status, EA_BLOCK_HEADER_SIZE) ;
    }
    else
    {
        /*Do nothing if underlying module busy*/
    }

    /*Check if requested job done OK*/
    if(Eep_JobDone)
    {
        /*Reset Job done flag */
        Eep_JobDone  = 0;
        rtn_val = E_OK ;
    }

    /*Check if error occurred */
    if(Eep_JobError)
    {
        /*Reset Error flag*/
        Eep_JobError = 0;
        rtn_val = E_NOT_OK ;
    }
    return rtn_val ;
}

/****************************************************************************************/
/*    Function Name           : Get_BlockStatus                                         */
/*     Function Description   : service to write block header                           */
/*    Parameter in            : BlockIndex  , Status                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirment              : none                                                    */
/*    Notes                   : none                                                    */
/****************************************************************************************/
static Std_ReturnType Get_BlockStatus(uint16 BlockIndex , uint32* Status)
{
    Std_ReturnType rtn_val = E_PENDING;
    uint32 BlockAdrs = 0 ;

    if(Eep_GetStatus()!= MEMIF_BUSY && Eep_JobDone != 1)
    {
        /*Reset Job done flag and Error flag*/
        Eep_JobDone = 0 ;
        Eep_JobError = 0 ;

        /*Request Write job*/
        BlockAdrs = Ea_BlockSConfig[BlockIndex].PhysicalStartAddress ;
        Eep_Read(BlockAdrs,(uint8*)Status, EA_BLOCK_HEADER_SIZE) ;
    }
    else
    {
        /*Do nothing if underlying module busy*/
    }

    /*Check if requested job done OK*/
    if(Eep_JobDone)
    {
        /*Reset Job done flag */
        Eep_JobDone  = 0;
        rtn_val = E_OK ;
    }

    /*Check if error occurred */
    if(Eep_JobError)
    {
        /*Reset Error flag*/
        Eep_JobError = 0;
        rtn_val = E_NOT_OK ;
    }
    return rtn_val ;
}

/****************************************************************************************/
/*    Function Name           : RequestEepJob                                           */
/*     Function Description   : service to write or read a block                        */
/*    Parameter in            : BlockIndex  , DataPtr ,JobReq , Len                     */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirment              : none                                                    */
/*    Notes                   : none                                                    */
/****************************************************************************************/
static Std_ReturnType RequestEepJob(uint16 BlockIndex , uint8* DataPtr, uint8 JobReq,uint16 Len)
{
    Std_ReturnType rtn_val = E_PENDING;
    uint32 BlockAdrs = 0 ;

    if(Eep_GetStatus()!= MEMIF_BUSY && Eep_JobDone != 1)
    {
        /*Reset Job done flag and Error flag*/
        Eep_JobDone = 0 ;
        Eep_JobError = 0 ;

        /*Request  job*/
        BlockAdrs = Ea_BlockSConfig[BlockIndex].PhysicalStartAddress + EA_BLOCK_HEADER_SIZE;
        if(JobReq == READ_JOB)
        {
            Eep_Read(BlockAdrs,DataPtr, Len) ;
        }
        else if(JobReq == WRITE_JOB)
        {
            Eep_Write(BlockAdrs,DataPtr, Len) ;
        }
        else
        {}
    }
    else
    {
        /*Do nothing if underlying module busy*/
    }

    /*Check if requested job done OK*/
    if(Eep_JobDone)
    {
        /*Reset Job done flag */
        Eep_JobDone  = 0;
        rtn_val = E_OK ;
    }

    /*Check if error occurred */
    if(Eep_JobError)
    {
        /*Reset Error flag*/
        Eep_JobError = 0;
        rtn_val = E_NOT_OK ;
    }
    return rtn_val ;
}
