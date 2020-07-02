
/*******************************************************************************
**                                                                            **
**  FILENAME     : Eep.c                                                      **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2019-11-22                                                 **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Sahar Elnagar                                              **
**                                                                            **
**  Description  :  Device deiver for Internal EEPROM                         **
**                                                                            **
**  Notes        :  Built-in wear leveling                                    **
*******************************************************************************/






/*****************************************************************************************/
/*                                   Include Common headres                              */
/*****************************************************************************************/
#include "hw_types.h"
#include "hw_eeprom.h"
#include "sysctl.h"
/*****************************************************************************************/
/*                                   Include Other  headres                              */
/*****************************************************************************************/
#include "Det.h"

/*****************************************************************************************/
/*                                   Include Component headres                           */
/*****************************************************************************************/
#include "Eep.h"
/*****************************************************************************************/
/*                                   Local Macro Definition                              */
/*****************************************************************************************/

/*Used compiler*/
#define gcc

//*****************************************************************************
//
//! Returns the EEPROM block number containing a given offset address.
//!
//! \param ui32Addr is the linear, byte address of the EEPROM location whose
//! block number is to be returned.  This is a zero-based offset from the start
//! of the EEPROM storage.
//!
//! This macro may be used to translate an EEPROM address offset into a
//! block number suitable for use in any of the driver's block protection
//! functions.  The address provided is expressed as a byte offset from the
//! base of the EEPROM.
//!
//! \return Returns the zero-based block number which contains the passed
//! address.
//
//*****************************************************************************
#define EEPROMBlockFromAddr(ui32Addr) ((ui32Addr) >> 6)


//*****************************************************************************
//
// Useful macro to extract the offset from a linear address.
//
//*****************************************************************************
#define OFFSET_FROM_ADDR(x) (((x) >> 2) & 0x0F)


//*****************************************************************************
//
//  Define word size in EEPROM
//
//*****************************************************************************
#define WORD_SIZE       4

//*****************************************************************************
//
// Macro to represent a pattern ofAttempts to read from a block
// for which the application does not have permission
//
//*****************************************************************************
#define NO_READ_PERMISSION_PATTERN     0xFFFFFFFF

/*****************************************************************************************/
/*                                   Local Definition                                    */
/*****************************************************************************************/
/*    Description        :
 */

/*****************************************************************************************/
/*                                   Local types Definition                              */
/*****************************************************************************************/
/*****************************************************************************************/
/*                                   Local Function Declaration                          */
/*****************************************************************************************/
static void _EEPROMSectorMaskClear(void) ;
static void _EEPROMSectorMaskSet(uint32 ui32Address);
static void Eep_MainFunction_Read(void) ;
static void Eep_MainFunction_Write(void) ;
static void Eep_MainFunction_Erase(void) ;
static void Eep_MainFunction_Compare(void) ;

/*****************************************************************************************/
/*                                   Local Function Definition                           */
/*****************************************************************************************/

//*****************************************************************************
//
// This function implements a workaround for a bug in Blizzard rev A silicon.
// It ensures that only the 1KB flash sector containing a given EEPROM address
// is erased if an erase/copy operation is required as a result of a following
// EEPROM write.
//
//*****************************************************************************
static void _EEPROMSectorMaskSet(uint32 ui32Address)
{
    uint32 ui32Mask;

    //
    // Determine which page contains the passed EEPROM address.  The 2KB EEPROM
    // is implemented in 16KB of flash with each 1KB sector of flash holding
    // values for 32 consecutive EEPROM words (or 128 bytes).
    //
    ui32Mask = ~(1 << (ui32Address >> 7));

    SysCtlDelay(10);
    HWREG(0x400FD0FC) = 3;
    SysCtlDelay(10);
    HWREG(0x400AE2C0) = ui32Mask;
    SysCtlDelay(10);
    HWREG(0x400FD0FC) = 0;
    SysCtlDelay(10);
}

//*****************************************************************************
//
// Clear the FSM sector erase mask to ensure that any following main array
// flash erase operations operate as expected.
//
//*****************************************************************************
static void
_EEPROMSectorMaskClear(void)
{
    SysCtlDelay(10);
    HWREG(0x400FD0FC) = 3;
    SysCtlDelay(10);
    HWREG(0x400AE2C0) = 0;
    SysCtlDelay(10);
    HWREG(0x400FD0FC) = 0;
    SysCtlDelay(10);
}

/***********************************************************************************/


//*****************************************************************************
//  Strut to save the required data for the reading process
//  Address: Start address of data to be read must be divisible by 4
//  DataPtr: pointer  that points to the place where data will be saved
//  Length : the number of bytes to read must be divisible by 4
//*****************************************************************************
typedef struct
{
    Eep_AddressType Address;
    uint8*          DataPtr;
    Eep_LengthType  Length ;
    uint8_t         Eep_InternalState ;
}str_ParametersCopy;

static str_ParametersCopy  ParametersCopyObj;

//*****************************************************************************
//  Variable to save the MainFunction state of current job processing
//  of read/write/erase/compare
//*****************************************************************************
#define IDLE_JOB        0x00
#define READ_JOB        0x01
#define WRITE_JOB       0x02
#define ERASE_JOB       0x03
#define COPMARE_JOB     0x04

//*****************************************************************************
//  EEPROM Main functions  Internal states
//*****************************************************************************
#define SET_EEPROM_ADDRESS              0x00
#define WRITE_EEPROM_WORD               0x01
#define END_JOB_SUCCESS                 0x02
#define END_JOB_FAILED                  0x03
#define READ_EEPROM_WORD                0x04

static uint8 JobProcessing_State     = IDLE_JOB;

static MemIf_StatusType   StatusType = MEMIF_UNINIT;

static MemIf_JobResultType JobResult = MEMIF_JOB_OK;

static Eep_ConfigType* Global_Config = NULL_PTR ;

/****************************************************************************************/
/*    Function Name           : Eep_Init                                                */
/*    Function Description    : Initializes the module                                  */
/*    Parameter in            : const Ea_ConfigType* ConfigPtr                          */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Eep_00143                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/
void Eep_Init( const Eep_ConfigType* ConfigPtr )
{
    uint32 Status;
   /*Report error to DET module if pointer parameter has a NULL value*/
    #if(EepDevErrorDetect == STD_ON)
        if(ConfigPtr == NULL_PTR)
        {
            Det_ReportError(EEPROM_DRIVER_ID, EEP_INSTANCE_ID,EEP_INIT_API_ID,EEP_E_PARAM_POINTER);
        }
    #endif

   /*Save parameter ConfigPtr to be used in other functions */
    Global_Config = (Eep_ConfigType*) ConfigPtr ;

   /*
    * Insert a small delay (6 cycles + call overhead) to guard against the
    * possibility that this function is called immediately after the EEPROM
    * peripheral is enabled.  Without this delay, there is a slight chance
    * that the first EEPROM register read will fault if you are using a
    * compiler with a ridiculously good optimizer!
    */
    SysCtlDelay(2);

    /*
     * Make sure the EEPROM has finished any ongoing processing.
     */
    while(HWREG(EEPROM_EEDONE) & EEPROM_EEDONE_WORKING)
      {
          /* Spin while EEPROM is busy.*/
      }

     /* Read the EESUPP register to see if any errors have been reported.*/
      Status = HWREG(EEPROM_EESUPP);

      /*Did an error of some sort occur during initialization?*/
      if(Status & (EEPROM_EESUPP_PRETRY | EEPROM_EESUPP_ERETRY))
      {
          /* Report error to Det Init failed */
        #if(EepDevErrorDetect == STD_ON)
           Det_ReportError(EEPROM_DRIVER_ID, EEP_INSTANCE_ID,EEP_INIT_API_ID,EEP_E_INIT_FAILED);
        #endif
      }

      /*Perform a second EEPROM reset.*/
      SysCtlPeripheralReset(SYSCTL_PERIPH_EEPROM0);

      /* Wait for the EEPROM to complete its reset processing once again.*/
      SysCtlDelay(2);
      /*
       * Make sure the EEPROM has finished any ongoing processing.
       */
       while(HWREG(EEPROM_EEDONE) & EEPROM_EEDONE_WORKING)
       {
           /* Spin while EEPROM is busy.*/
       }

      /* Read EESUPP once again to determine if any error occurred.*/
       Status = HWREG(EEPROM_EESUPP);

      /* Was an error reported following the second reset?*/
       if(Status & (EEPROM_EESUPP_PRETRY | EEPROM_EESUPP_ERETRY))
       {
          /* Report error to Det Init failed */
          #if(EepDevErrorDetect == STD_ON)
             Det_ReportError(EEPROM_DRIVER_ID, EEP_INSTANCE_ID,EEP_INIT_API_ID,EEP_E_INIT_FAILED);
          #endif
       }

      /* [SWS_Eep_00006] After having finished the module initialization, the function
       *  Eep_Init shall set the EEPROM state to MEMIF_IDLE and shall set the job result
       *  to MEMIF_JOB_OK.(SRS_BSW_00406)*/
       StatusType = MEMIF_IDLE ;
       JobResult  = MEMIF_JOB_OK ;

}

/****************************************************************************************/
/*    Function Name           : Eep_Read                                                */
/*    Function Description    : Request Read job from EEPROM                            */
/*    Parameter in            : EepromAddress , Length                                  */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : DataBufferPtr                                           */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Eep_00145                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType Eep_Read(Eep_AddressType EepromAddress,uint8* DataBufferPtr,Eep_LengthType Length )
{
    /*
     * 1- Check function parameters
     */
    if(EepromAddress > EEP_END_ADDRESS)
    {
        #if (EepDevErrorDetect==STD_ON)
            Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_READ_API_ID,EEP_E_PARAM_ADDRESS);
        #else
            return E_NOT_OK;
        #endif
    }
    else if(DataBufferPtr == NULL_PTR)
    {
        #if(EepDevErrorDetect==STD_ON)
            Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_READ_API_ID,EEP_E_PARAM_POINTER);
        #else
            return E_NOT_OK;
        #endif
    }
    else if(Length < MIN_LENGTH || Length > (EEP_SIZE - EepromAddress))
    {
        #if(EepDevErrorDetect==STD_ON)
            Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_READ_API_ID,EEP_E_PARAM_LENGTH);
        #else
            return E_NOT_OK;
        #endif
    }
    /*
     *Check Module state initialized or not
     */
    else if(StatusType != MEMIF_IDLE)
    {
        #if(EepDevErrorDetect==STD_ON)
            Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_READ_API_ID,EEP_E_UNINIT);
        #else
            return E_NOT_OK;
        #endif
    }
    /*
     * Check if the device is busy
     */
    else if(StatusType == MEMIF_BUSY ||JobResult == MEMIF_JOB_PENDING)
    {
        #if(EepDevErrorDetect==STD_ON)
            Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_READ_API_ID,EEP_E_BUSY);
        #else
            return E_NOT_OK;
        #endif
    }
    else
    {
        /*
         *  2- Copy parameters to be used by Main_Process Function
         */
         ParametersCopyObj.Address = EepromAddress;
         ParametersCopyObj.DataPtr = DataBufferPtr;
         ParametersCopyObj.Length  = Length;
         ParametersCopyObj.Eep_InternalState = SET_EEPROM_ADDRESS ;

        /*
         *  3- Initiate Reading Job
         */
         JobProcessing_State = READ_JOB ;

        /*
         *  4- Set status to Busy
         */
         StatusType = MEMIF_BUSY;

         /*
          * 5- Set Job Result Pending
          */
          JobResult  = MEMIF_JOB_PENDING ;

    }
        return E_OK;
}

/****************************************************************************************/
/*    Function Name           : Eep_Write                                               */
/*    Function Description    : Request Write job from EEPROM                           */
/*    Parameter in            : EepromAddress.DataBufferPtr , Length                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : DataBufferPtr                                           */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Eep_00146                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType Eep_Write(Eep_AddressType EepromAddress, const uint8* DataBufferPtr,Eep_LengthType Length )
{
   /*
    * 1- Check function parameters
    */
   if(EepromAddress > EEP_END_ADDRESS)
   {
       #if (EepDevErrorDetect==STD_ON)
           Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_WRITE_API_ID,EEP_E_PARAM_ADDRESS);
       #endif
           return E_NOT_OK;

   }
   else if(DataBufferPtr == NULL_PTR)
   {
       #if(EepDevErrorDetect==STD_ON)
           Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_WRITE_API_ID,EEP_E_PARAM_POINTER);
       #endif
           return E_NOT_OK;

   }
   else if(Length < MIN_LENGTH || Length > (EEP_SIZE - EepromAddress))
   {
       #if(EepDevErrorDetect==STD_ON)
           Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_WRITE_API_ID,EEP_E_PARAM_LENGTH);
       #endif
           return E_NOT_OK;

   }
   /*
    *Check Module state initialized or not
    */
   else if(StatusType != MEMIF_IDLE)
   {
       #if(EepDevErrorDetect==STD_ON)
           Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_WRITE_API_ID,EEP_E_UNINIT);
       #endif
           return E_NOT_OK;

   }
   /*
    * Check if the device is busy
    */
   else if(StatusType == MEMIF_BUSY ||JobResult == MEMIF_JOB_PENDING)
   {
       #if(EepDevErrorDetect==STD_ON)
           Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_WRITE_API_ID,EEP_E_BUSY);
       #endif
           return E_NOT_OK;
   }
   else
   {
       /*
        *  2- Copy parameters to be used by Main_Process Function
        */
        ParametersCopyObj.Address = EepromAddress;
        ParametersCopyObj.DataPtr = (uint8*)DataBufferPtr;
        ParametersCopyObj.Length  = Length;
        ParametersCopyObj.Eep_InternalState = SET_EEPROM_ADDRESS ;
       /*
        *  3- Initiate Writing Job
        */
        JobProcessing_State = WRITE_JOB ;

       /*
        *  4- Set status to Busy
        */
        StatusType = MEMIF_BUSY;

       /*
        * 5- Set Job Result Pending
        */
        JobResult  = MEMIF_JOB_PENDING ;
   }
       return E_OK;
}

/****************************************************************************************/
/*    Function Name           : Eep_Erase                                               */
/*    Function Description    : Request  EEPROM erase                                   */
/*    Parameter in            : EepromAddress.DataBufferPtr , Length                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : DataBufferPtr                                           */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Eep_00147                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType Eep_Erase(Eep_AddressType EepromAddress,Eep_LengthType Length )
{
      /*
       * 1- Check function parameters
       */
      if(EepromAddress > EEP_END_ADDRESS)
      {
          #if (EepDevErrorDetect==STD_ON)
              Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_ERASE_API_ID,EEP_E_PARAM_ADDRESS);
          #endif
              return E_NOT_OK;
      }
      /*Mass erase is the only possible erase in tm4c123g*/
      else if(Length !=EepEraseUnitSize)
      {
          #if(EepDevErrorDetect==STD_ON)
              Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_ERASE_API_ID,EEP_E_PARAM_LENGTH);
          #endif
              return E_NOT_OK;
      }
      /*
       *Check Module state initialized or not
       */
      else if(StatusType != MEMIF_IDLE)
      {
          #if(EepDevErrorDetect==STD_ON)
              Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_ERASE_API_ID,EEP_E_UNINIT);
          #endif
              return E_NOT_OK;
      }
      /*
       * Check if the device is busy
       */
      else if(StatusType == MEMIF_BUSY ||JobResult == MEMIF_JOB_PENDING )
      {
          #if(EepDevErrorDetect==STD_ON)
              Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_ERASE_API_ID,EEP_E_BUSY);
          #endif
              return E_NOT_OK;
      }
      else
      {
          /*
           *  2- Copy parameters to be used by Main_Process Function
           */
           ParametersCopyObj.Address = EepromAddress;
           ParametersCopyObj.DataPtr = NULL_PTR;
           ParametersCopyObj.Length  = Length;

          /*
           *  3- Initiate Erase Job
           */
           JobProcessing_State = ERASE_JOB ;

          /*
           *  4- Set status to Busy
           */
           StatusType = MEMIF_BUSY;

          /*
           * 5- Set Job Result Pending
           */
           JobResult  = MEMIF_JOB_PENDING ;
      }
          return E_OK;
}

/****************************************************************************************/
/*    Function Name           : Eep_Compare                                             */
/*    Function Description    : Request  compare job                                    */
/*    Parameter in            : EepromAddress.DataBufferPtr , Length                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : DataBufferPtr                                           */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Eep_00148                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType Eep_Compare(Eep_AddressType EepromAddress,const uint8* DataBufferPtr,Eep_LengthType Length )
{
    /*
     * 1- Check function parameters
     */
    if(EepromAddress > EEP_END_ADDRESS)
    {
        #if (EepDevErrorDetect==STD_ON)
            Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_COMPARE_API_ID,EEP_E_PARAM_ADDRESS);
        #endif
            return E_NOT_OK;
    }
    else if(DataBufferPtr == NULL_PTR)
    {
        #if(EepDevErrorDetect==STD_ON)
            Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_COMPARE_API_ID,EEP_E_PARAM_POINTER);
        #endif
            return E_NOT_OK;
    }
    else if(Length < MIN_LENGTH || Length > (EEP_SIZE - EepromAddress))
    {
        #if(EepDevErrorDetect==STD_ON)
            Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_COMPARE_API_ID,EEP_E_PARAM_LENGTH);
        #endif
            return E_NOT_OK;

    }
    /*
     *Check Module state initialized or not
     */
    else if(StatusType != MEMIF_IDLE)
    {
        #if(EepDevErrorDetect==STD_ON)
            Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_COMPARE_API_ID,EEP_E_UNINIT);
        #endif
            return E_NOT_OK;

    }
    /*
     * Check if the device is busy
     */
    else if(StatusType == MEMIF_BUSY ||JobResult == MEMIF_JOB_PENDING)
    {
        #if(EepDevErrorDetect==STD_ON)
            Det_ReportError(EEPROM_DRIVER_ID,EEP_INSTANCE_ID,EEP_COMPARE_API_ID,EEP_E_BUSY);
        #endif
        return E_NOT_OK;
    }
    else
    {
        /*
         *  2- Copy parameters to be used by Main_Process Function
         */
         ParametersCopyObj.Address = EepromAddress;
         ParametersCopyObj.DataPtr = (uint8*)DataBufferPtr;
         ParametersCopyObj.Length  = Length;

        /*
         *  3- Initiate Compare Job
         */
         JobProcessing_State = COPMARE_JOB ;

        /*
         *  4- Set status to Busy
         */
         StatusType = MEMIF_BUSY;

        /*
         * 5- Set Job Result Pending
         */
         JobResult  = MEMIF_JOB_PENDING ;
    }

    return E_OK;
}

/*****************************************************************************************
 * Function Description : Cancels a running job.                                         *
 *                        The EEPROM driver shall provide a synchronous cancel function  *
 *                        that stops the currently processed job. The states and data    *
 *                        of the affected EEPROM cells are undefined! The EEPROM         *
 *                        driver and controller itself shall be ready for valid jobs     *
 *****************************************************************************************/
void Eep_Cancel(void)
{
   /*
    * [SWS_Eep_00027] The function Eep_Cancel shall set the EEP module
    *  state to MEMIF_IDLE. (SRS_Eep_00090)
    */
    StatusType = MEMIF_IDLE;

   /*
    *  Set current processing job to idle
    */
    JobProcessing_State = IDLE_JOB ;


   /*
    *  [SWS_Eep_00028] The function Eep_Cancel shall set the job result
    *   to MEMIF_JOB_CANCELED if the job result currently has the value
    *   MEMIF_JOB_PENDING. Otherwise it shall leave the job result unchanged.
    *   (SRS_Eep_00090)
    */
    if(JobResult == MEMIF_JOB_PENDING)
    {
        JobResult = MEMIF_JOB_CANCELED ;
    }

    /*
     * [SWS_Eep_00216] If configured, Eep_Cancel shall call the error notification
     *  function defined in EepJobErrorNotification in order to inform the caller
     *  about the cancelation of a job.  (SRS_Eep_00090)
     */
      Global_Config->EepInitConfigurationRef->EepJobErrorNotification();
}

/*****************************************************************************************
 * Function Description : [SWS_Eep_00029] The function Eep_GetStatus shall return        *
 * the EEPROM status synchronously.                                                      *
 *****************************************************************************************/
MemIf_StatusType Eep_GetStatus(void)
{
    return StatusType ;
}

/*****************************************************************************************
 * Function Description : [SWS_Eep_00024] The function Eep_GetJobResult shall            *
 *  synchronously return the result of the last job that has been                        *
 *  accepted by the Eep module.                                                          *
 *****************************************************************************************/
MemIf_JobResultType Eep_GetJobResult(void)
{
    return JobResult ;
}


/*****************************************************************************************
 * Function Description : [SWS_Eep_00030] The function Eep_MainFunction shall perform    *
 *  the processing of the EEPROM read, write, erase and compare jobs.  (SRS_Eep_12047)   *                                                       *
 *****************************************************************************************/
void Eep_MainFunction(void)
{
    if(JobResult == MEMIF_JOB_PENDING)
    {
        switch (JobProcessing_State)
        {
            /*Current pending job is reading job*/
            case READ_JOB :
               Eep_MainFunction_Read();
            break;

            /*Current pending job is writing job*/
            case WRITE_JOB :
                Eep_MainFunction_Write() ;
            break ;

            /*Current pending job is erase job*/
            case ERASE_JOB :
                Eep_MainFunction_Erase() ;
            break ;

            /*Current pending job is compare job*/
            case COPMARE_JOB :
                Eep_MainFunction_Compare();
            break ;
        }
    }
}



/*****************************************************************************************
 * Function Description : The function Eep_MainFunction_Read shall perform               *
 *  the processing of the EEPROM read job.  (SRS_Eep_12047)                              *
 *****************************************************************************************/
static void Eep_MainFunction_Read(void)
{
    switch(ParametersCopyObj.Eep_InternalState)
    {
        case SET_EEPROM_ADDRESS :
            /* Make sure the EEPROM is idle before we start.*/
           while(HWREG(EEPROM_EEDONE) & EEPROM_EEDONE_WORKING)
           {
           }
          /*
           * Set the block and offset appropriately to read the first word.
           */
           HWREG(EEPROM_EEBLOCK)  = EEPROMBlockFromAddr(ParametersCopyObj.Address);
           HWREG(EEPROM_EEOFFSET) = OFFSET_FROM_ADDR(ParametersCopyObj.Address)   ;

           ParametersCopyObj.Length /= 4 ;
           ParametersCopyObj.Eep_InternalState = READ_EEPROM_WORD ;
        break;
        case READ_EEPROM_WORD :
            if(ParametersCopyObj.Length)
            {
                   /* Read the next word through the autoincrementing register.*/
                 *((uint32*)(ParametersCopyObj.DataPtr)) = HWREG(EEPROM_EERDWRINC);
                     ParametersCopyObj.DataPtr +=4;
                     ParametersCopyObj.Length --;
                /*
                 * Do we need to move to the next block?  This is the case if the
                 * offset register has just wrapped back to 0.  Note that we only
                 * write the block register if we have more data to read.  If this
                 * register is written, the hardware expects a read or write operation
                 * next.  If a mass erase is requested instead, the mass erase will
                 * fail.
                 */
                 if(ParametersCopyObj.Length && (HWREG(EEPROM_EEOFFSET) == 0))
                 {
                     HWREG(EEPROM_EEBLOCK) += 1;
                 }
            }
            else
            {
                ParametersCopyObj.Eep_InternalState = END_JOB_SUCCESS ;
            }
        break;

        case END_JOB_SUCCESS :
            /*Job done correctly*/
            JobResult  = MEMIF_JOB_OK ;
            StatusType = MEMIF_IDLE   ;
            /*End job notification for the upper layer if configured*/
            if(Global_Config->EepInitConfigurationRef->EepJobEndNotification != NULL_PTR)
            {
              Global_Config->EepInitConfigurationRef->EepJobEndNotification();
            }
        break;
    }
}

/*****************************************************************************************
 * Function Description : The function Eep_MainFunction_Write shall perform              *
 *  the processing of the EEPROM write job.  (SRS_Eep_12047)                             *
 *****************************************************************************************/
static void Eep_MainFunction_Write(void)
{

    switch(ParametersCopyObj.Eep_InternalState)
    {
/*******************case : SET_EEPROM_ADDRESS **********************/
        case SET_EEPROM_ADDRESS :
            /* Make sure the EEPROM is idle before we start.*/
            while(HWREG(EEPROM_EEDONE) & EEPROM_EEDONE_WORKING)
            {
            }

           /*
            * Set the block and offset appropriately to read the first word.
            */
            HWREG(EEPROM_EEBLOCK)  = EEPROMBlockFromAddr(ParametersCopyObj.Address);
            HWREG(EEPROM_EEOFFSET) = OFFSET_FROM_ADDR(ParametersCopyObj.Address)   ;
            ParametersCopyObj.Eep_InternalState = WRITE_EEPROM_WORD;
            ParametersCopyObj.Length/=4;
        break;


/*****************case : WRITE_EEPROM_WORD*****************/
        case WRITE_EEPROM_WORD :
         if(ParametersCopyObj.Length)
         {

               /*
                * This is a workaround for a silicon problem on Blizzard rev A.  We
                * need to do this before every word write to ensure that we don't
                * have problems in multi-word writes that span multiple flash sectors.
                */
                _EEPROMSectorMaskSet(ParametersCopyObj.Address);
                /*
                 * Write the next word through the autoincrementing register.
                 */
                 HWREG(EEPROM_EERDWRINC) = *((uint32_t*)ParametersCopyObj.DataPtr) ;
                 /*
                 * Wait a few cycles.  In some cases, the WRBUSY bit is not set
                 * immediately and this prevents us from dropping through the polling
                 * loop before the bit is set.
                 */
                 SysCtlDelay(10);

                 /* Wait for the write to complete.*/
                 while(HWREG(EEPROM_EEDONE) & EEPROM_EEDONE_WORKING)
                 {}
                 /*
                  * Make sure we completed the write without errors.  Note that we
                  * must check this per-word because write permission can be set per
                  * block resulting in only a section of the write not being performed.
                  */
                 if(HWREG(EEPROM_EEDONE) & EEPROM_EEDONE_NOPERM)
                 {
                     /* An error was reported that would prevent the values from
                      * being written correctly.
                      */
                      _EEPROMSectorMaskClear();
                       ParametersCopyObj.Eep_InternalState = END_JOB_FAILED ;
                 }

               /*Decrement the number of bytes to write*/
                ParametersCopyObj.Length-- ;
                ParametersCopyObj.DataPtr+=4;
            /*
             * Do we need to move to the next block?  This is the case if the
             * offset register has just wrapped back to 0.  Note that we only
             * write the block register if we have more data to read.  If this
             * register is written, the hardware expects a read or write operation
             * next.  If a mass erase is requested instead, the mass erase will
             * fail.
             */
             if(ParametersCopyObj.Length && (HWREG(EEPROM_EEOFFSET) == 0))
             {
                 HWREG(EEPROM_EEBLOCK) += 1;
             }

             if(ParametersCopyObj.Length == 0)
             {
                 ParametersCopyObj.Eep_InternalState = END_JOB_SUCCESS ;
             }
        }
     break;
/******************case :  END_JOB_FAILED********************/
        case END_JOB_FAILED :
            /*Report Production Error to DEM :  EEP_E_WRITE_FAILED */
            JobResult  = MEMIF_JOB_FAILED ;
            StatusType = MEMIF_IDLE   ;
            if(Global_Config->EepInitConfigurationRef->EepJobErrorNotification!= NULL_PTR)
            {
                Global_Config->EepInitConfigurationRef->EepJobErrorNotification();
            }
        break;
/******************case END_JOB_SUCCESS :*******************/
        case END_JOB_SUCCESS :
            /* Clear the sector protection bits to prevent possible problems when
             * programming the main flash array later.
             */
            _EEPROMSectorMaskClear();
            /*Job done correctly*/
             JobResult  = MEMIF_JOB_OK ;
             StatusType = MEMIF_IDLE   ;
            /*End job notification for the upper layer if configured*/
             if(Global_Config->EepInitConfigurationRef->EepJobEndNotification != NULL_PTR)
             {
                 Global_Config->EepInitConfigurationRef->EepJobEndNotification();
             }
         break;
    }
}

/*****************************************************************************************
 * Function Description : The function Eep_MainFunction_Erase shall perform              *
 *  the processing of the EEPROM erase job.  (SRS_Eep_12047)                             *
 *****************************************************************************************/
static void Eep_MainFunction_Erase(void)
{
    /*
     * This is a workaround for a silicon problem on Blizzard rev A.
     */
     _EEPROMSectorMaskClear();
    /*
     * Start the mass erase processing
     */
    HWREG(EEPROM_EEDBGME) = EEPROM_MASS_ERASE_KEY | EEPROM_EEDBGME_ME;
    /* Wait for completion.*/
    while(HWREG(EEPROM_EEDONE) & EEPROM_EEDONE_WORKING)
    {
        /* Spin while EEPROM is busy.*/
    }
    /* Reset the peripheral.  This is required so that all protection
     * mechanisms and passwords are reset now that the EEPROM data has been scrubbed.
     */
    SysCtlPeripheralReset(SYSCTL_PERIPH_EEPROM0);

    /*Wait for completion again.*/
    SysCtlDelay(2);
    /* Wait for completion.*/
    while(HWREG(EEPROM_EEDONE) & EEPROM_EEDONE_WORKING)
    {
        /* Spin while EEPROM is busy.*/
    }
    if(HWREG(EEPROM_EEDONE)!= 0)
    {
        /*Report Production Error to DEM :  EEP_E_ERASE_FAILED */
        JobResult  = MEMIF_JOB_FAILED ;
        StatusType = MEMIF_IDLE   ;
    }
    else
    {
        /*Job done correctly*/
         JobResult  = MEMIF_JOB_OK ;
         StatusType = MEMIF_IDLE   ;
        /*End job notification for the upper layer if configured*/
         if(Global_Config->EepInitConfigurationRef->EepJobEndNotification != NULL_PTR)
         {
             Global_Config->EepInitConfigurationRef->EepJobEndNotification();
         }
    }
}

/*****************************************************************************************
 * Function Description : The function Eep_MainFunction_Compare shall perform            *
 *  the processing of the EEPROM compare job.  (SRS_Eep_12047)                           *
 *****************************************************************************************/
static void Eep_MainFunction_Compare(void)
{

    /*Variable to save the start byte to access in a word*/
     uint8  ByteNum  = 0 ;
    /*Variable to save the value of the word*/
     uint32 DataWord = 0;

     /* Make sure the EEPROM is idle before we start.*/
      while(HWREG(EEPROM_EEDONE) & EEPROM_EEDONE_WORKING)
      {
      }
     /*
      * Set the block and offset appropriately to read the first word.
      */
      HWREG(EEPROM_EEBLOCK)  = EEPROMBlockFromAddr(ParametersCopyObj.Address);
      HWREG(EEPROM_EEOFFSET) = OFFSET_FROM_ADDR(ParametersCopyObj.Address)   ;

     /*Check if the current address is not word aligned*/
      if(ParametersCopyObj.Address % WORD_SIZE != 0)
      {
          /*Get the start byte number to read in the word located in the required address */
          ByteNum = ParametersCopyObj.Address % WORD_SIZE ;

          /*Read the word located at the current address*/
          DataWord = HWREG(EEPROM_EERDWRINC) ;
      }
      while(ParametersCopyObj.Length)
      {
          if(ByteNum == WORD_SIZE)
          {
            /* Read the next word through the autoincrementing register.*/
              DataWord = HWREG(EEPROM_EERDWRINC);
              ByteNum = 0;
          }
          else
          {
             /*Compare current byte in buer with the current byte in EEPORM */
             if(*(ParametersCopyObj.DataPtr) == *(((uint8*)&DataWord) + ByteNum) )
             {
                 /*Increment the data pointer */
                  ParametersCopyObj.DataPtr++;

                 /*Increment to the next byte within the word*/
                  ByteNum ++ ;

                 /*Decrement the number of bytes to read*/
                  ParametersCopyObj.Length-- ;
             }
             else
             {
                 /*Report Production Error to DEM :  EEP_E_Compare_FAILED */
                  JobResult  = MEMIF_JOB_FAILED ;
                  StatusType = MEMIF_IDLE   ;
                  break;
             }
          }
         /*
          * Do we need to move to the next block?  This is the case if the
          * offset register has just wrapped back to 0.  Note that we only
          * write the block register if we have more data to read.  If this
          * register is written, the hardware expects a read or write operation
          * next.  If a mass erase is requested instead, the mass erase will
          * fail.
          */
          if(ParametersCopyObj.Length && (HWREG(EEPROM_EEOFFSET) == 0))
          {
              HWREG(EEPROM_EEBLOCK) += 1;
          }
      }
      if(JobResult != MEMIF_JOB_FAILED)
      {
         /*Job done correctly*/
          JobResult  = MEMIF_JOB_OK ;
          StatusType = MEMIF_IDLE   ;
         /*End job notification for the upper layer if configured*/
          if(Global_Config->EepInitConfigurationRef->EepJobEndNotification != NULL_PTR)
          {
              Global_Config->EepInitConfigurationRef->EepJobEndNotification();
          }
      }
}
