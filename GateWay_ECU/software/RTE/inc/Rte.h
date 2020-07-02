/**************************************************************************/
/*                                                                        */
/* File : Rte.h                                                           */
/*                                                                        */
/* Date : 23 June 2020                                                    */
/*                                                                        */
/* Author : Yomna Mokhtar                                                 */
/*                                                                        */
/**************************************************************************/


#ifndef RTE_INC_RTE_H_
#define RTE_INC_RTE_H_

#include "Platform_Types.h"
#include "Std_Types.h"


Std_ReturnType Rte_Write_NewUpdateRequest( boolean val ) ;               // download update SWC
Std_ReturnType Rte_Read_NewUpdateRequest( boolean* var ) ;               // GUI Interface SWC

Std_ReturnType Rte_Write_EncryptedBuffer( uint8* PtrBuffer ) ;           // download update SWC
Std_ReturnType Rte_Read_EncryptedBuffer( uint8** BufferAddress ) ;       // Decryption SWC

Std_ReturnType Rte_Write_PacketSize( uint16 Size ) ;                     // download update SWC
Std_ReturnType Rte_Read_PacketSize( uint16* Size ) ;                     // Decryption SWC , Store SWC

Std_ReturnType Rte_Write_NewEncryptedDataFlg( boolean val ) ;            // download update SWC , Decryption SWC
Std_ReturnType Rte_Read_NewEncryptedDataFlg( boolean* var ) ;            // Decryption SWC

Std_ReturnType Rte_Write_Decryption_key( uint8* key ) ;                  // download update SWC
Std_ReturnType Rte_Read_Decryption_key( uint8** KeyAddress ) ;           // Decryption SWC

Std_ReturnType Rte_Write_KeyChanged( boolean val ) ;                     // download update SWC
Std_ReturnType Rte_Read_KeyChanged( boolean *var ) ;                     // Decryption SWC

Std_ReturnType Rte_Write_UpdateRequestAccepted( Std_ReturnType val ) ;   // GUI Interface SWC
Std_ReturnType Rte_Read_UpdateRequestAccepted( Std_ReturnType* var ) ;   // download update SWC

Std_ReturnType Rte_Write_StoreDataState( Std_ReturnType val ) ;          // Store SWC
Std_ReturnType Rte_Read_StoreDataState( Std_ReturnType* var ) ;          // download update SWC

Std_ReturnType Rte_Write_DoneDownloading( boolean val ) ;                // download update SWC
Std_ReturnType Rte_Read_DoneDownloading( boolean* var ) ;                // Distribution SWC

Std_ReturnType Rte_Write_DecryptedBuffer( uint8* PtrBuffer ) ;           // Decryption SWC
Std_ReturnType Rte_Read_DecryptedBuffer( uint8** BufferAddress ) ;       // Store SWC

Std_ReturnType Rte_Write_NewDecryptedDataFlag( boolean val ) ;           // Decryption SWC
Std_ReturnType Rte_Read_NewDecryptedDataFlag( boolean* var ) ;           // Store SWC

#endif

