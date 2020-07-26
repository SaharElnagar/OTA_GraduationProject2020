/*
 * StoreUpdate.c
 *
 *  Created on: June 23, 2020
 *      Author: Sahar Elnagar
 */

/***************************************************************************/
/*                             File includes                               */
/***************************************************************************/
#include "NvM.h"
#include "Rte.h"
#include "StoreUpdate_Cfg.h"
#include "StoreUpdate.h"

/***************************************************************************/
/*                            Local data types                             */
/***************************************************************************/
typedef enum
{
    CheckNewPacket      ,
    StorePacket         ,
    WaitStoreComplete   ,
}StoreUpdateStates_type;

/***************************************************************************/
/*                          Local variables                                */
/***************************************************************************/
/*SWC Internal states variable*/
StoreUpdateStates_type StoreUpdateStates ;

/*Data Buffer Pointer*/
static uint8* DataPtr;

/*Storing in memory result variable*/
Std_ReturnType StoringRequestResult  ;

/*Packets counter*/
static uint16 PacketBlockcounter ;

/*Variale to save the number of packets required to be saved*/
static uint16 PacketsNum ;

/***************************************************************************/
/*                          functions implementation                       */
/***************************************************************************/

/****************************************************************************/
//    Function Name           : StoreUpdate_Init
//    Function Description    : To Initialize StoreUpdate SW-C
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
/****************************************************************************/
void StoreUpdate_Init(void)
{
    StoreUpdateStates    = CheckNewPacket ;
    StoringRequestResult = E_PENDING ;
    PacketBlockcounter   = NvM_FRAME_INFO_BLOCK_ID;
    PacketsNum  = 0 ;
}



/****************************************************************************/
//    Function Name           : StoreUpdate_MainFunction
//    Function Description    : To Handle all the states of StoreUpdate
//                                  SW-C
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
/****************************************************************************/
void StoreUpdate_MainFunction(void)
{
    boolean StoreFlag = FALSE;
    NvM_RequestResultType RequestResultPtr = 0;

    switch(StoreUpdateStates)
    {
/********************************Case: CheckNewPacket ********************************/
        case CheckNewPacket:
            /*Check New Store Request*/
            Rte_Read_NewEncryptedDataFlg(&StoreFlag);
            if(StoreFlag == TRUE)
            {
                /*Read data to be stored */
                if(Rte_Read_EncryptedBuffer(&DataPtr) == E_OK)
                {
                    /*Get Number of packets expected to be stored*/
                    if(!PacketsNum)
                    {
                        /*read first ecu num of packets*/
                        PacketsNum+=*(uint16*)(DataPtr+FIRST_ECU_SIZE_INDEX) ;
                        /*Check if 1st ECU Has extra bytes*/
                        if(*(uint16*)(DataPtr+FIRST_ECU_SIZE_INDEX+2))
                        {
                            PacketsNum++;
                        }

                        /*read second ecu num of packets*/
                        PacketsNum+=*(uint16*)(DataPtr+SECOND_ECU_SIZE_INDEX) ;
                        /*Check if 2st ECU Has extra bytes*/
                        if(*(uint16*)(DataPtr+SECOND_ECU_SIZE_INDEX+2))
                        {
                           PacketsNum++;
                        }

                        /*Set required block ID*/
                        PacketBlockcounter  = NvM_FRAME_INFO_BLOCK_ID;
                    }
                    /*Normal data packet*/
                    else
                    {
                        /*Increment block counter */
                        PacketBlockcounter++;
                        /*decrement number of packets*/
                        PacketsNum--;
                    }
                    /*Go to store packet state*/
                    StoreUpdateStates   = StorePacket;
                }
            }
        break;
/********************************Case: StorePacket ********************************/
        case StorePacket:

            /*reset store flag*/
            StoreFlag = FALSE;
            /*Check if NvM Block not busy*/
            if(NvM_GetErrorStatus(PacketBlockcounter, &RequestResultPtr)==E_OK)
            {
                if(RequestResultPtr != NVM_REQ_PENDING)
                {
                    /*Store data in the Non volatile device*/
                    NvM_WriteBlock(PacketBlockcounter, DataPtr) ;
                    /*Wait for Storing completion*/
                    StoreUpdateStates = WaitStoreComplete ;
                    StoringRequestResult = E_PENDING ;
                }
            }
        break;
/********************************Case: WaitStoreComplete ********************************/
        case WaitStoreComplete:
            /*Check if storing request is done*/
            if(StoringRequestResult != E_PENDING)
            {
                Rte_Write_StoreDataState(StoringRequestResult);
                /*Reset Request flag*/
                StoringRequestResult = E_PENDING ;
                /*Go to check for new packet*/
                StoreUpdateStates = CheckNewPacket ;
            }
            else
            {
                /*Do nothing*/
            }
        break;
    }
}


