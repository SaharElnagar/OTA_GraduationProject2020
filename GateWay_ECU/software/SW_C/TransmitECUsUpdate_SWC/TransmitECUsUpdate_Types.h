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
    Update_Idle                    =       0x0000      ,
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
    WaitForUpdateRequest        ,
    InformDriver                ,
    GET_ECU_Update_Info         ,
    UpdateTivaECU               ,
    UpdateStmECU                ,
}ECUsUpdateStatesType;

typedef enum
{
    GUI_Idle                        =   0x0000 ,
    Transmit_new_update_request     =   0x1111 ,
    Wiat_For_User_response          =   0x6666 ,
    Wait_For_Update_Download_OK     =   0x7777 ,
    Wait_For_Start_Update_request   =   0x8888 ,
    Download_new_update_request     =   0x2222 ,
    New_update_downloaded           =   0x3333 ,
    Start_update_software           =   0x4444 ,
    Start_bootloader                =   0x5555 ,
    Send_Update_Percent             =   0x9999
}LCDCommStatesType;



typedef enum
{
    TivaC_ECU       =  0x55 ,
    STM_ECU         =  0x66 ,
}ECUsId_Types;

#endif /* BSW_SW_C_TRANSMITECUSUPDATE_SWC_TRANSMITECUSUPDATE_TYPES_H_ */
