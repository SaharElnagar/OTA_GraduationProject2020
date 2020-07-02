/**************************************************************************/
/*                                                                        */
/* File : Rte_DownloadUpdate.h                                            */
/*                                                                        */
/* Date : 23 June 2020                                                    */
/*                                                                        */
/* Author : Yomna Mokhtar                                                 */
/*                                                                        */
/**************************************************************************/


#ifndef RTE_DOWNLOAD_UPDATE_H_
#define RTE_DOWNLOAD_UPDATE_H_

#include "Rte.h"

/**************************************************************************/
/*                         Provided Ports                                 */
/**************************************************************************/
#define    RTE_WRITE_NEW_UPDATE_REQ             Rte_Write_NewUpdateRequest
#define    RTE_WRITE_DATA_PACKET                Rte_Write_EncryptedBuffer
#define    RTE_WRITE_DONE_DOWNLOADING           Rte_Write_DoneDownloading
#define    RTE_WRITE_UPDATE_REQ_ACCEPTED        Rte_Write_UpdateRequestAccepted
#define    RTE_WRITE_STORE_DATA_STATE           Rte_Write_StoreDataState
#define    RTE_WRITE_NEW_ENCRYPTED_FLAG         Rte_Write_NewEncryptedDataFlg
#define    RTE_WRITE_PACKET_SIZE                Rte_Write_PacketSize
#define    RTE_WRITE_DECRYPTION_KEY             Rte_Write_Decryption_key
#define    RTE_WRITE_KEY_CHANGED                Rte_Write_KeyChanged

/**************************************************************************/
/*                         Requested Ports                                */
/**************************************************************************/
#define    RTE_READ_UPDATE_REQ_ACCEPTED         Rte_Read_UpdateRequestAccepted
#define    RTE_READ_STORE_DATA_STATE            Rte_Read_StoreDataState

#endif
