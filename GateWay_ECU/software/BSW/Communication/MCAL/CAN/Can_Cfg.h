/*
 * Can_Cfg.h
 *
 *  Created on: Jul 2, 2020
 *      Author: Sahar
 */

#ifndef BSW_COMMUNICATION_INTERFACE_CANIF_CAN_CFG_H_
#define BSW_COMMUNICATION_INTERFACE_CANIF_CAN_CFG_H_

#define CAN_TX_MESSAGES_NUM                    (4U)
#define CAN_RX_MESSAGES_NUM                    (3U)

#define TIVA_NODE_REQUEST_MESSAGE           (1U)
#define TIVA_NODE_UPDATE_MESSAGE            (2U)
#define STM_NODE_UPDATE_MESSAGE             (3U)
#define LCD_MESSAGE                         (4U)

#define TIVA_NODE_REQUEST_TX_ID             (2U)
#define TIVA_NODE_UPDATE_MESSAGE_TX_ID      (3U)
#define TIVA_NODE_UPDATE_MESSAGE_RX_ID      (4U)
#define STM_NODE_UPDATE_MESSAGE_TX_ID       (0x77U)
#define STM_NODE_UPDATE_MESSAGE_RX_ID       (0x55U)
#define LCD_MESSAGE_TX_ID                   (0x88U)
#define LCD_MESSAGE_RX_ID                   (0x66U)

#define CAN_MESSAGE_MASK                    (0x7FF)

#endif /* BSW_COMMUNICATION_INTERFACE_CANIF_CAN_CFG_H_ */
