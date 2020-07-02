/**************************************************************************/
/*                                                                        */
/* File : Rte_Decryption.h                                                */
/*                                                                        */
/* Date : 29 June 2020                                                    */
/*                                                                        */
/* Author : Yomna Mokhtar                                                 */
/*                                                                        */
/**************************************************************************/


#ifndef RTE_DECRYPTION_H_
#define RTE_DECRYPTION_H_

#include "Rte.h"

/**************************************************************************/
/*                         Provided Ports                                 */
/**************************************************************************/
#define  RTE_WRITE_NEW_ENCRYPTED_FLAG      Rte_Write_NewEncryptedDataFlg
#define  RTE_WRITE_DECRYPTED_BUFFER        Rte_Write_DecryptedBuffer
#define  RTE_WRITE_NEW_DECRYPTED_FLAG      Rte_Write_NewDecryptedDataFlag
#define  RTE_WRITE_KEY_CHANGED             Rte_Write_KeyChanged

/**************************************************************************/
/*                         Requested Ports                                */
/**************************************************************************/
#define  RTE_READ_ENCRYPTED_BUFFER         Rte_Read_EncryptedBuffer
#define  RTE_READ_PACKET_SIZE              Rte_Read_PacketSize
#define  RTE_READ_NEW_ENCRYPTED_FLAG       Rte_Read_NewEncryptedDataFlg
#define  RTE_READ_DECRYPTION_KEY           Rte_Read_Decryption_key
#define  RTE_READ_KEY_CHANGED_FLAG         Rte_Read_KeyChanged

#endif

