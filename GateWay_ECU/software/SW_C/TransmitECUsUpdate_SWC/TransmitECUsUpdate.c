/*
 * TransmitECUsUpdate.c
 *
 *  Created on: Jul 2, 2020
 *      Author: Sahar
 */

#include "CANIf.h"
#include "TransmitECUsUpdate_Types.h"

uint8_t CanMessageNum ;
ECUsUpdateStatesType    ECUsUpdateStates;
LCDCommStatesType       LCDCommStates;
void Init_TransmitECUsUpdate(void)
{
    Can_Init();
    CanMessageNum       = LCD_MESSAGE   ;
    ECUsUpdateStates    = WaitForUpdate ;
}

void TransmitECUsUpdate_MainFunction(void)
{
    switch(ECUsUpdateStates)
    {
        case WaitForUpdate:
            /*Read request update , if available go to InformDriver states */
            CanMessageNum       = LCD_MESSAGE   ;
            LCDCommStates       = Transmit_new_update_request ;
        break;

        case InformDriver:

        break;

        case UpdateTivaECU :
        break;

        case UpdateStmECU :
        break;
    }
}


void InformDriver_MainFunction(void)
{
    switch(LCDCommStates)
    {
        case Transmit_new_update_request:
            /*1- Send to the GUI that the car has new update*/
            CanTransmitBlocking_Function(2,(uint8_t*) &LCDCommStates, LCD_MESSAGE)  ;
            LCDCommStates   =   Wiat_For_User_response;
        break;
        case Wiat_For_User_response :
            /*2- Wait for the GUI response to start downloading the update*/
            CanReceiveBlocking_Function(2,(uint8_t*) &LCDCommStates, LCD_MESSAGE)  ;

            /*Check user acceptance */
            if(LCDCommStates == Download_new_update_request)
            {
                /*Inform Download SWC to start downloading the update*/
            }

        break;
    }

}
