/*
 * App_Cbk.c
 *
 *  Created on: Jul 24, 2020
 *      Author: Sahar
 */

#include "NvM.h"


/*Storing in memory result variable*/
extern Std_ReturnType StoringRequestResult  ;
extern Std_ReturnType ReadingRequestResult ;



/****************************************************************************/
//    Function Name           : NvMSingleBlockCallback
//    Function Description    : CallBack function for NvM Module
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
/****************************************************************************/
Std_ReturnType NvMSingleBlockCallback(uint8 ServiceId,NvM_RequestResultType JobResult )
{
    if(JobResult == NVM_REQ_OK)
    {
        if(ServiceId ==NVM_WRITEBLOCK_API_ID)
        {
            StoringRequestResult = E_OK ;
        }
        else if(ServiceId == NVM_READBLOCK_API_ID)
        {
            ReadingRequestResult =E_OK ;
        }
        else
        {

        }
    }
    else if(JobResult == NVM_REQ_RESTORED_FROM_ROM)
    {
        if(ServiceId ==NVM_WRITEBLOCK_API_ID)
        {
            StoringRequestResult = E_NOT_OK ;
        }
        else if(ServiceId == NVM_READBLOCK_API_ID)
        {
            ReadingRequestResult =E_NOT_OK ;
        }
        else
        {

        }
    }
    else if(JobResult == NVM_REQ_NOT_OK )
    {
        if(ServiceId ==NVM_WRITEBLOCK_API_ID)
        {
            StoringRequestResult = E_NOT_OK ;
        }
        else if(ServiceId == NVM_READBLOCK_API_ID)
        {
            ReadingRequestResult =E_NOT_OK ;
        }
        else
        {

        }
    }
    else if(JobResult == NVM_REQ_NV_INVALIDATED)
    {
        if(ServiceId ==NVM_WRITEBLOCK_API_ID)
        {
            StoringRequestResult = E_NOT_OK ;
        }
        else if(ServiceId == NVM_READBLOCK_API_ID)
        {
            ReadingRequestResult =E_NOT_OK ;
        }
        else
        {

        }
    }
    else
    {}

    return E_OK ;
}
