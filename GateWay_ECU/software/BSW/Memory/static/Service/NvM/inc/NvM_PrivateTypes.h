/*
 * NvM_PrivateTypes.h
 *
 *
 *      Author: Sahar
 */

#ifndef BSW_STATIC_SERVICE_NVM_INC_NVM_PRIVATETYPES_H_
#define BSW_STATIC_SERVICE_NVM_INC_NVM_PRIVATETYPES_H_

#include "Std_Types.h"
#include "NvM_Types.h"
/*****************************************************************************************/
/*                                   Local types Definition                              */
/*****************************************************************************************/

/*Module state type*/
typedef uint8 ModuleStateType ;

/*Permanent RAM Status type*/
typedef uint8 PRamStatusType ;

/* [SWS_NvM_00134]
 * Administrative block Type
 * The Administrative block shall be located in RAM
 * shall contain a block index which is used in association with Data set NV blocks.
 * error/status information of the corresponding NVRAM block shall be contained
 * use state information of the permanent RAM block (valid / Invalid)
 * The Administrative block shall be invisible for the application
 * shall use an error/status field to manage the error/status value of the last request
 * shall use an attribute field to manage the NV block write protection in order to
 * protect/unprotect a NV block data field
 */
typedef struct
{
    uint8 DataSetIndex ;
    NvM_RequestResultType BlockResultStatus ;
    PRamStatusType PRAMStatus ;
    uint32 PrevCRCVal ;

}AdministrativeBlockType ;

/* struct to hold the parameters for the job request*/
typedef struct
{
   uint8 ServiceId ;
   NvM_BlockIdType Block_Id ;
   uint8* RAM_Ptr ;
   uint8  Job_InternalState ;
}Job_Parameters ;

/*struct to hold the indices which point to the queue
 *Used for queue implementation
 */
typedef struct{
  uint16 Head ;
  uint16 Tail ;
}Queue_Indices_Struct ;

/*Multi  block request Information */
typedef struct
{
    NvM_RequestResultType ResultStatus ;
    uint8 request ;
    uint8 Internal_state ;
}MultiBlockRequestType;

#if(NVM_POLLING_MODE == STD_OF)
/*Struct to save end job status from underlying layer Successful or failed*/
typedef struct
{
    uint8 EndJobSuccess : 1;
    uint8 EndJobFailed  : 1;
}EndJobStatusType;
#endif


#endif /* BSW_STATIC_SERVICE_NVM_INC_NVM_PRIVATETYPES_H_ */
