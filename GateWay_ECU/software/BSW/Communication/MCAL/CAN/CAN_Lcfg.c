/*
 * CAN_Lcfg.c
 *
 *  Created on: Jul 2, 2020
 *      Author: Sahar
 */
#include "can_driver.h"
#include "Can_Cfg.h"

tCANConfigTXMsgObj CANConfigTXMsgObj[CAN_TX_MESSAGES_NUM] =
{
     /*TIVA_NODE_REQUEST_MESSAGE*/
     {
          .Flags     = 0 ,
          .Msg_ID    =TIVA_NODE_REQUEST_TX_ID
     },
    /*TIVA_NODE_UPDATE_MESSAGE*/
    {
         .Flags     = 0 ,
         .Msg_ID    =TIVA_NODE_UPDATE_MESSAGE_TX_ID
    },
    /*STM_NODE_UPDATE_MESSAGE*/
    {
         .Flags     = 0 ,
         .Msg_ID    =STM_NODE_UPDATE_MESSAGE_TX_ID
    },
    /*LCD_MESSAGE*/
    {
         .Flags     = 0 ,
         .Msg_ID    =LCD_MESSAGE_TX_ID
    },
};


tCANConfigRXMsgObj CANConfigRXMsgObj[CAN_RX_MESSAGES_NUM]=
{
     /*TIVA_NODE_UPDATE_MESSAGE*/
     {
          .Flags        = 0x00001000 ,
          .Msg_ID_MSK   = CAN_MESSAGE_MASK      ,
          .Msg_ID       = TIVA_NODE_UPDATE_MESSAGE_RX_ID
     },

     /*STM_NODE_UPDATE_MESSAGE*/
     {
          .Flags        = 0x00001000 ,
          .Msg_ID_MSK   = CAN_MESSAGE_MASK      ,
          .Msg_ID       = STM_NODE_UPDATE_MESSAGE_RX_ID
     },

     /*LCD_MESSAGE*/
     {
          .Flags        = 0x00001000 ,
          .Msg_ID_MSK   = CAN_MESSAGE_MASK      ,
          .Msg_ID       = LCD_MESSAGE_RX_ID
     }

};

