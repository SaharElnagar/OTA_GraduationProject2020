/**************************************************************************/
/*                                                                        */
/* File : TransmitECUsUpdate.c                                            */
/*                                                                        */
/* Date : 23 June 2020                                                    */
/*                                                                        */
/* Author : Sahar Elnagar                                                 */
/*                                                                        */
/**************************************************************************/


/*****************************************************************************************/
/*                               Local Macros Definition                                 */
/*****************************************************************************************/

#define CMD_SIZE                        (2U)        /*2 bytes*/
#define CAN_FRAME_SIZE                  (8U)
#define EXTRA_BYTES_FROM_DECRYPTION     (16U)


/*****************************************************************************************/
/*                               Local Type definitions                                  */
/*****************************************************************************************/
typedef enum
{
    Read_FirstBlock     ,
    Read_SecondBlock    ,
}ReadPacketStates_Type;

/***************************************************************************/
/*                             File includes                               */
/***************************************************************************/

#include "CANIf.h"
#include "TransmitECUsUpdate_Types.h"
#include "Rte.h"
#include "Std_Types.h"
#include "StoreUpdate_Cfg.h"
#include "NvM.h"

/***************************************************************************/
/*                          Local variables                                */
/***************************************************************************/
uint8_t CanMessageNum ;
ECUsUpdateStatesType        ECUsUpdateStates;
LCDCommStatesType           LCDCommStates;
SingleECU_UpdateStates_Type Cmd_Status ;

typedef struct
{
  uint16 PacketsNum ;
  uint16 ExtraBytes ;
}str_FrameInfo ;

static str_FrameInfo FrameInfo ;
Std_ReturnType ReadingRequestResult ;
static uint8 TotalFrameInfo[FRAME_INFO_SIZE];
static uint8 Frame[CAN_FRAME_SIZE] ;
static uint32 FirstECUId = 0 ;
static uint8 Packet_Buffer[PACKET_SIZE];
static uint8 Temp_Buffer[PACKET_SIZE];
/*Packets counter*/
static uint16 PacketBlockcounter ;
static ReadPacketStates_Type ReadPacketStates;
static uint16 PacketPointer = 0;
static uint16 ReadSize = 0;

/***************************************************************************/
/*                         local functions Prototyoes                      */
/***************************************************************************/
static void InformDriver_MainFunction(void);
static void Update_SingleECU(ECUsId_Types ECUsId) ;
static void CopyData(uint16 Size,uint16 TStartAddress,uint16 PStartAddress);
static Std_ReturnType ReadSecondECUPackets(void);

/***************************************************************************/
/*                          functions implementation                       */
/***************************************************************************/

/****************************************************************************/
//    Function Name           : Init_TransmitECUsUpdate
//    Function Description    : To Initialize TransmitECUsUpdate SW-C
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
/****************************************************************************/
void Init_TransmitECUsUpdate(void)
{
    Can_Init();
    CanMessageNum       = LCD_MESSAGE   ;
    ECUsUpdateStates    = WaitForUpdateRequest ;
    PacketBlockcounter  = NvM_PACKET_1_BLOCK_ID;
}

/****************************************************************************/
//    Function Name           : TransmitECUsUpdate_MainFunction
//    Function Description    : To Handle all the states of TransmitECUsUpdate
//                                  SW-C
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
/****************************************************************************/
void TransmitECUsUpdate_MainFunction(void)
{
    boolean UpdateRequest = FALSE ;
    NvM_RequestResultType RequestResultPtr = 0;

    switch(ECUsUpdateStates)
    {
        case WaitForUpdateRequest:
            /*Read update request flag from DownloadUpdate SWC*/
            Rte_Read_NewUpdateRequest(&UpdateRequest);
            if(UpdateRequest == TRUE)
            {
                /*Reset flag*/
                UpdateRequest = FALSE ;
                /*if available go to InformDriver states */
                CanMessageNum       = LCD_MESSAGE   ;
                LCDCommStates       = Transmit_new_update_request ;
                ECUsUpdateStates    = InformDriver ;
            }
        break;

        case InformDriver:
            /*Inform driver and get confirmation to start downloading update*/
            InformDriver_MainFunction();
        break;
        case GET_ECU_Update_Info:
            /*Check if NvM Block not busy*/
            if(ReadingRequestResult ==E_PENDING )
            {
                if(NvM_GetErrorStatus(NvM_FRAME_INFO_BLOCK_ID, &RequestResultPtr)==E_OK)
                {
                    if(RequestResultPtr != NVM_REQ_PENDING)
                    {
                        /*Store data in the Non volatile device*/
                        NvM_ReadBlock(NvM_FRAME_INFO_BLOCK_ID, TotalFrameInfo) ;
                    }
                }
            }
            /*Read Frame Info OK*/
            else if (ReadingRequestResult ==E_OK)
            {
                /*Reset flag*/
                ReadingRequestResult = E_PENDING ;
                /*Get First ECU Info*/
                FirstECUId = *((uint32*)(TotalFrameInfo+FIRST_ECU_ID_INDEX));

                /*Check If First Node is TivaC */
                if(FirstECUId == TivaC_ECU)
                {
                    /*Go to update TivaC ECU*/
                    ECUsUpdateStates =UpdateTivaECU ;
                }
                /*Check If First Node is STM*/
                else if(FirstECUId == STM_ECU)
                {
                    /*Go to update STM ECU*/
                    ECUsUpdateStates =UpdateStmECU ;
                }
                /*Get ECU Frame Info */
                FrameInfo.PacketsNum = *(((uint16*)TotalFrameInfo)+FIRST_ECU_SIZE_INDEX);
                FrameInfo.ExtraBytes = *(((uint16*)TotalFrameInfo)+FIRST_ECU_SIZE_INDEX+2);

                /*Calculate start address of first packet  for the second ECU */
                if(FrameInfo.ExtraBytes > EXTRA_BYTES_FROM_DECRYPTION)
                {
                    PacketPointer = FrameInfo.ExtraBytes +EXTRA_BYTES_FROM_DECRYPTION -\
                            (FrameInfo.ExtraBytes%EXTRA_BYTES_FROM_DECRYPTION);
                }
                else
                {
                    PacketPointer = EXTRA_BYTES_FROM_DECRYPTION;
                }
                /*Calculate the size of second ECU's first packet remaining in
                 *the last  block of first ECU
                 */
                ReadSize = PACKET_SIZE - PacketPointer;
            }
            else
            {
                ReadingRequestResult =E_PENDING ;
            }
        break;
        case UpdateTivaECU :
            Update_SingleECU(TivaC_ECU);
        break;

        case UpdateStmECU :
            Update_SingleECU(STM_ECU);
        break;
    }
}

/****************************************************************************/
//    Function Name           : InformDriver_MainFunction
//    Function Description    : To Handle The states or communicating with GUI
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
/****************************************************************************/
static void InformDriver_MainFunction(void)
{
    boolean DownloadOK = FALSE ;
    switch(LCDCommStates)
    {
        case GUI_Idle :
            /*do nothing*/
        break;
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
                if(Rte_Write_UpdateRequestAccepted(TRUE)== E_OK)
                {
                    LCDCommStates = Wait_For_Update_Download_OK ;
                }
            }

        break;

        case Wait_For_Update_Download_OK:
            /*Check if  update is fully downloaded */
            Rte_Read_DoneDownloading(&DownloadOK);
            if(DownloadOK == TRUE)
            {
                LCDCommStates = New_update_downloaded;
                /*1- Send to the GUI that update is fully downloaded*/
                CanTransmitBlocking_Function(2,(uint8_t*) &LCDCommStates, LCD_MESSAGE)  ;
                LCDCommStates = Wait_For_Start_Update_request ;
            }
        break;

        case Wait_For_Start_Update_request :
            /* Wait for the GUI response to start the update*/
            CanReceiveBlocking_Function(2,(uint8_t*) &LCDCommStates, LCD_MESSAGE)  ;
            if(LCDCommStates == Start_update_software)
            {
                ECUsUpdateStates = GET_ECU_Update_Info ;
                LCDCommStates    = GUI_Idle ;
                ReadingRequestResult = E_PENDING;
            }
        break;
    }
}

/****************************************************************************/
//    Function Name           : Update_SingleECU
//    Function Description    : To Handle Single ECU Update states
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
/****************************************************************************/
static void Update_SingleECU(ECUsId_Types ECUsId)
{
    Std_ReturnType rtn_val = E_PENDING ;
    NvM_RequestResultType RequestResultPtr = 0;
    uint8* Ptr;

    if(ECUsId == TivaC_ECU)
    {
        CanMessageNum = TIVA_NODE_UPDATE_MESSAGE ;
    }
    else
    {
        CanMessageNum = STM_NODE_UPDATE_MESSAGE ;
    }

  /*Gateway state machine to update one ECU*/
     switch(Cmd_Status)
     {
/****************************case :  Update_Idle ****************************/
         case Update_Idle:
             /*Do nothing*/
         break;
/****************************case :  REQUEST_TO_UPDATE ****************************/
         case REQUEST_TO_UPDATE:
            /*
             * Send to the ECU request to update
             */
             if(ECUsId == TivaC_ECU)
             {
                 CanTransmitBlocking_Function(CMD_SIZE,(uint8_t*)&Cmd_Status,TIVA_NODE_REQUEST_MESSAGE);
             }
             else
             {
                 CanTransmitBlocking_Function(CMD_SIZE,(uint8_t*)&Cmd_Status,LCD_MESSAGE);
             }
            Cmd_Status = RECEIVE_FREAME_INFO_REQUEST ;
         break;

/****************************case :  RECEIVE_FREAME_INFO_REQUEST ****************************/
         case RECEIVE_FREAME_INFO_REQUEST:
            /*
            * Wait till the ECU request the frame info
            * Then go to SEND_FRAME_INFO
            */
            CanReceiveBlocking_Function(CMD_SIZE,(uint8_t*)&Cmd_Status,CanMessageNum);
         break;
 /****************************case :  SEND_FRAME_INFO ****************************/
         case SEND_FRAME_INFO:
           /*
            * Send the frame info
            * Then go to READY_TO_RECEIVE_UPDATE
            */
             (*(uint16_t*)Frame)      = FrameInfo.PacketsNum ;
             (*(uint16_t*)(Frame+2))  = FrameInfo.ExtraBytes ;
             CanTransmitBlocking_Function(CAN_FRAME_SIZE,Frame,CanMessageNum)  ;
             Cmd_Status = READY_TO_RECEIVE_UPDATE;
         break;
 /****************************case :  READY_TO_RECEIVE_UPDATE ****************************/
         case READY_TO_RECEIVE_UPDATE :
           /*
            * Wait for the ECU to be ready to receive the update
            */
             CanReceiveBlocking_Function(CMD_SIZE,(uint8_t*)&Cmd_Status,CanMessageNum) ;
             ReadPacketStates = Read_FirstBlock;
             ReadingRequestResult = E_PENDING ;
         break;
 /****************************case :  SEND_PACKET ****************************/
         case SEND_PACKET:
            if(ECUsId == STM_ECU)
            {
               if(rtn_val== E_PENDING)
              rtn_val =ReadSecondECUPackets();
               Ptr = Temp_Buffer ;
            }
            else
            {
                /*Check if NvM Block not busy*/
                if(ReadingRequestResult ==E_PENDING )
                {
                   if(NvM_GetErrorStatus(PacketBlockcounter, &RequestResultPtr)==E_OK)
                   {
                       if(RequestResultPtr != NVM_REQ_PENDING)
                       {
                           /*Store data in the Non volatile device*/
                           NvM_ReadBlock(PacketBlockcounter, Packet_Buffer) ;
                       }
                   }
                }
                else
                {
                    rtn_val = E_OK ;
                    ReadingRequestResult = E_PENDING ;
                    PacketBlockcounter++;
                    Ptr = Packet_Buffer;
                }
            }
          /*
           * Send a packet to the ECU
           */
            if( rtn_val == E_OK )
            {
                if(FrameInfo.PacketsNum>0)
                {
                    CanTransmitBlocking_Function(PACKET_SIZE,Ptr,CanMessageNum);
                    FrameInfo.PacketsNum-=1 ;
                    Cmd_Status = SEND_NEW_PACKET_REQ ;
                }
                else
                {
                    CanTransmitBlocking_Function(FrameInfo.ExtraBytes,Ptr,CanMessageNum);
                    FrameInfo.ExtraBytes = 0 ;
                    Cmd_Status = END_OF_FRAME ;
                }
            }
         break;
 /****************************case :  SEND_NEW_PACKET_REQ ****************************/
         case SEND_NEW_PACKET_REQ:
           /*
            * Request to send new packet
            */
            CanReceiveBlocking_Function(CMD_SIZE,(uint8_t*)&Cmd_Status,CanMessageNum);
            //CanTransmitBlocking_Function(2,(uint8_t*)&Cmd_Status);
            if(Cmd_Status == SEND_NEW_PACKET )
            Cmd_Status = SEND_PACKET ;
         break;
 /****************************case :  END_OF_FRAME ****************************/
         case END_OF_FRAME:
             /*Wait For ECU to confirm update succeeded*/
            CanReceiveBlocking_Function(CMD_SIZE,(uint8_t*)&Cmd_Status,CanMessageNum);
         break;
 /****************************case :  UPDATE_SUCCESS ****************************/
         case UPDATE_SUCCESS:
          Cmd_Status = REQUEST_TO_UPDATE  ;
          ECUsUpdateStates = WaitForUpdateRequest ;
          if(*((uint32*)TotalFrameInfo)>1 && ECUsId == TivaC_ECU)
          {
              /*Get ECU Frame Info */
              FrameInfo.PacketsNum = *(((uint16*)TotalFrameInfo)+SECOND_ECU_SIZE_INDEX);
              FrameInfo.ExtraBytes = *(((uint16*)TotalFrameInfo)+SECOND_ECU_SIZE_INDEX+2);
              ECUsUpdateStates = UpdateStmECU ;
          }
         break;
     }
}

/****************************************************************************/
//    Function Name           : ReadSecondECUPackets
//    Function Description    : To save second ECU's packet in one buffer
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
/****************************************************************************/
static Std_ReturnType ReadSecondECUPackets(void)
{
    NvM_RequestResultType RequestResultPtr = 0;
    Std_ReturnType rtn_val = E_PENDING ;

    switch(ReadPacketStates)
    {

        case Read_FirstBlock :
            CopyData(ReadSize,0,PacketPointer);
            ReadPacketStates = Read_SecondBlock;
        break;
        case Read_SecondBlock :
            /*Check if NvM Block not busy*/
            if(ReadingRequestResult ==E_PENDING )
            {
               if(NvM_GetErrorStatus(PacketBlockcounter, &RequestResultPtr)==E_OK)
               {
                   if(RequestResultPtr != NVM_REQ_PENDING)
                   {
                       /*Store data in the Non volatile device*/
                       NvM_ReadBlock(PacketBlockcounter, Packet_Buffer) ;
                   }
               }
            }
            else
            {
                ReadingRequestResult = E_PENDING ;
                PacketBlockcounter++;
                CopyData(PACKET_SIZE -ReadSize ,ReadSize,0);
                PacketPointer = PACKET_SIZE -ReadSize ;
                ReadSize = PACKET_SIZE - PacketPointer ;
                rtn_val = E_OK ;
                ReadPacketStates = Read_FirstBlock;
            }
        break;
    }
    return rtn_val ;
}


/****************************************************************************/
//    Function Name           : CopyData
//    Function Description    : To data from Packet_Buffer to Temp_Bufer
//    Parameter in            : none
//    Parameter inout         : none
//    Parameter out           : none
//    Return value            : none
/****************************************************************************/
static void CopyData(uint16 Size,uint16 TStartAddress,uint16 PStartAddress)
{
    while(Size)
    {
        Temp_Buffer[TStartAddress] = Packet_Buffer[PStartAddress];
        Size--;
        TStartAddress++;
        PStartAddress++;
    }
}
