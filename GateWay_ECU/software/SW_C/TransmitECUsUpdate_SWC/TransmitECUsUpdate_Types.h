/*
 * TransmitECUsUpdate_Types.h
 *
 *  Created on: Jul 2, 2020
 *      Author: Sahar
 */

#ifndef BSW_SW_C_TRANSMITECUSUPDATE_SWC_TRANSMITECUSUPDATE_TYPES_H_
#define BSW_SW_C_TRANSMITECUSUPDATE_SWC_TRANSMITECUSUPDATE_TYPES_H_



typedef enum
{
    REQUEST_TO_UPDATE              =       0x1111      ,
    RECEIVE_FREAME_INFO_REQUEST    =       0x3333      ,
    SEND_FRAME_INFO                =       0x2222      ,
    SEND_PACKET                    =       0x4444      ,
    READY_TO_RECEIVE_UPDATE        =       0x5555      ,
    SEND_NEW_PACKET                =       0x6666      ,
    SEND_NEW_PACKET_REQ            =       0xBBBB      ,
    END_OF_FRAME                   =       0x7777      ,
    CANCEL_UPDATE                  =       0x9999      ,
    UPDATE_SUCCESS                 =       0xAAAA      ,
}SingleECU_UpdateStates_Type;

typedef enum
{
    WaitForUpdate       ,
    InformDriver        ,
    UpdateTivaECU       ,
    UpdateStmECU        ,
}ECUsUpdateStatesType;

typedef enum
{
    Transmit_new_update_request     =   0x1111 ,
    Wiat_For_User_response          =   0x6666 ,
    Download_new_update_request     =   0x2222 ,
    New_update_downloaded           =   0x3333 ,
    Start_update_software           =   0x4444 ,
    Start_bootloader                =   0x5555
}LCDCommStatesType;

#endif /* BSW_SW_C_TRANSMITECUSUPDATE_SWC_TRANSMITECUSUPDATE_TYPES_H_ */
