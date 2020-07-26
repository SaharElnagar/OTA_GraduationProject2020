/**************************************************************************/
/*                                                                        */
/* File : Rte.c                                                           */
/*                                                                        */
/* Date : 23 June 2020                                                    */
/*                                                                        */
/* Author : Yomna Mokhtar                                                 */
/*                                                                        */
/**************************************************************************/

#include "Rte.h"

/**************************************************************************/
/*                        Type Definitions                                */
/**************************************************************************/
typedef uint8 VariableState ;
#define IDLE       ((VariableState)0U)
#define BUSY       ((VariableState)1U)

/**************************************************************************/
/*                            Local Macros                                */
/**************************************************************************/

/**************************************************************************/
/*                         Local Variables                                */
/**************************************************************************/
static boolean Rte_NewUpdateRequest = FALSE ;
static Std_ReturnType Rte_UpdateRequestAccepted = E_PENDING ;
static Std_ReturnType Rte_StoreDataState = E_PENDING ;
static uint8* Rte_EncryptedBuffer ;
static boolean Rte_DoneDownloading = FALSE ;
static boolean Rte_NewEncryptedDataFlg = FALSE ;
static uint16 Rte_PacketSize = 0 ;
static uint8* Rte_DecryptedBuffer ;
static boolean Rte_NewDecryptedData = FALSE ;
static uint8* Rte_Decryption_Key ;
static boolean Rte_KeyChanged_Flag ;

/* Protection flags */
static VariableState NewUpdate_ProtFlg = IDLE ;
static VariableState UpdateReqAccepted_ProtFlg = IDLE ;
static VariableState StoreDataState_ProtFlg = IDLE ;
static VariableState EncryptedBuffer_ProtFlg = IDLE ;
static VariableState DoneDownloading_ProtFlg = IDLE ;
static VariableState NewEncrypted_ProtFlg = IDLE ;
static VariableState PacketSize_ProtFlg = IDLE ;
static VariableState DecryptedBuffer_ProtFlg = IDLE ;
static VariableState NewDecrypted_ProtFlg = IDLE ;
static VariableState DecryptionKey_ProtFlg = IDLE ;
static VariableState KeyChanged_ProtFlg = IDLE ;

/**************************************************************************/
/*                     Rte_NewUpdateRequest                               */
/**************************************************************************/
Std_ReturnType Rte_Write_NewUpdateRequest( boolean val )
{
    Std_ReturnType Return_Value ;
    if(NewUpdate_ProtFlg != BUSY)
    {
        NewUpdate_ProtFlg = BUSY ;

        Rte_NewUpdateRequest = val ;
       // NewUpdate_ProtFlg = IDLE ;  /*it should be back to idle  after reading */
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
void Rte_Read_NewUpdateRequest( boolean* var )
{
    /*transmit the flag value to the caller*/
    *var = Rte_NewUpdateRequest ;

    /*Reset flag if it's true*/
    if(Rte_NewUpdateRequest == TRUE)
    {
        Rte_NewUpdateRequest = FALSE ;

        /*Set port flag to idle*/
        NewUpdate_ProtFlg = IDLE ;
    }
}

/**************************************************************************/
/*                         Rte_EncryptedBuffer                            */
/**************************************************************************/
Std_ReturnType Rte_Write_EncryptedBuffer( uint8* PtrBuffer )
{
    Std_ReturnType Return_Value ;
    if(EncryptedBuffer_ProtFlg != BUSY)
    {
        EncryptedBuffer_ProtFlg = BUSY ;
        Rte_EncryptedBuffer = PtrBuffer ;
        EncryptedBuffer_ProtFlg = IDLE ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
Std_ReturnType Rte_Read_EncryptedBuffer( uint8** BufferAddress )
{
    Std_ReturnType Return_Value ;
    if(EncryptedBuffer_ProtFlg != BUSY)
    {
        EncryptedBuffer_ProtFlg = BUSY ;
        *BufferAddress = Rte_EncryptedBuffer ;
        EncryptedBuffer_ProtFlg = IDLE ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}

/**************************************************************************/
/*                       Rte_UpdateRequestAccepted                        */
/**************************************************************************/
Std_ReturnType Rte_Write_UpdateRequestAccepted( boolean val )
{
    Std_ReturnType Return_Value ;
    if(UpdateReqAccepted_ProtFlg != BUSY)
    {
        UpdateReqAccepted_ProtFlg = BUSY ;
        Rte_UpdateRequestAccepted = val ;
       // UpdateReqAccepted_ProtFlg = IDLE ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
void Rte_Read_UpdateRequestAccepted( boolean* var )
{
    *var = Rte_UpdateRequestAccepted ;
    if(Rte_UpdateRequestAccepted == TRUE)
    {
        Rte_UpdateRequestAccepted = FALSE;
        UpdateReqAccepted_ProtFlg = IDLE ;
    }
}

/**************************************************************************/
/*                       Rte_StoreDataState                               */
/**************************************************************************/
Std_ReturnType Rte_Write_StoreDataState( Std_ReturnType val )
{
    Std_ReturnType Return_Value ;
    if(StoreDataState_ProtFlg != BUSY)
    {
        StoreDataState_ProtFlg = BUSY ;
        Rte_StoreDataState = val ;
        StoreDataState_ProtFlg = IDLE ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
Std_ReturnType Rte_Read_StoreDataState( Std_ReturnType* var )
{
    Std_ReturnType Return_Value ;
    if(StoreDataState_ProtFlg != BUSY)
    {
        *var = Rte_StoreDataState ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}

/**************************************************************************/
/*                       Rte_DoneDownloading                               */
/**************************************************************************/
Std_ReturnType Rte_Write_DoneDownloading( boolean val )
{
    Std_ReturnType Return_Value ;
    if(DoneDownloading_ProtFlg != BUSY)
    {
        DoneDownloading_ProtFlg = BUSY ;
        Rte_DoneDownloading = val ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
void Rte_Read_DoneDownloading( boolean* var )
{
    *var = Rte_DoneDownloading ;
    if(Rte_DoneDownloading== TRUE)
    {
        Rte_DoneDownloading = FALSE ;
        DoneDownloading_ProtFlg = IDLE ;
    }
}

/**************************************************************************/
/*                       Rte_NewEncryptedDataFlg                          */
/**************************************************************************/
Std_ReturnType Rte_Write_NewEncryptedDataFlg( boolean val )
{
    Std_ReturnType Return_Value ;
    if(NewEncrypted_ProtFlg != BUSY)
    {
        NewEncrypted_ProtFlg = BUSY ;
        Rte_NewEncryptedDataFlg = val ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
void Rte_Read_NewEncryptedDataFlg( boolean* var )
{

    *var = Rte_NewEncryptedDataFlg ;
    if(Rte_NewEncryptedDataFlg == TRUE)
    {
        Rte_NewEncryptedDataFlg = FALSE;
        NewEncrypted_ProtFlg = IDLE ;
    }
}

/**************************************************************************/
/*                           Rte_PacketSize                               */
/**************************************************************************/
Std_ReturnType Rte_Write_PacketSize( uint16 Size )
{
    Std_ReturnType Return_Value ;
    if(PacketSize_ProtFlg != BUSY)
    {
        PacketSize_ProtFlg = BUSY ;
        Rte_PacketSize = Size ;
        PacketSize_ProtFlg = IDLE ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
Std_ReturnType Rte_Read_PacketSize( uint16* Size )
{
    Std_ReturnType Return_Value ;
    if(PacketSize_ProtFlg != BUSY)
    {
        *Size = Rte_PacketSize ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}

/**************************************************************************/
/*                         Rte_DecryptedBuffer                            */
/**************************************************************************/
Std_ReturnType Rte_Write_DecryptedBuffer( uint8* PtrBuffer )
{
    Std_ReturnType Return_Value ;
    if(DecryptedBuffer_ProtFlg != BUSY)
    {
        DecryptedBuffer_ProtFlg = BUSY ;
        Rte_DecryptedBuffer = PtrBuffer ;
        DecryptedBuffer_ProtFlg = IDLE ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
Std_ReturnType Rte_Read_DecryptedBuffer( uint8** BufferAddress )
{
    Std_ReturnType Return_Value ;
    if(DecryptedBuffer_ProtFlg != BUSY)
    {
        DecryptedBuffer_ProtFlg = BUSY ;
        *BufferAddress = Rte_DecryptedBuffer ;
        DecryptedBuffer_ProtFlg = IDLE ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}

/**************************************************************************/
/*                         Rte_NewDecryptedData                           */
/**************************************************************************/
Std_ReturnType Rte_Write_NewDecryptedDataFlag( boolean val )
{
    Std_ReturnType Return_Value ;
    if(NewDecrypted_ProtFlg != BUSY)
    {
        NewDecrypted_ProtFlg = BUSY ;
        Rte_NewDecryptedData = val ;
        NewDecrypted_ProtFlg = IDLE ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
Std_ReturnType Rte_Read_NewDecryptedDataFlag( boolean* var )
{
    Std_ReturnType Return_Value ;
    if(NewDecrypted_ProtFlg != BUSY)
    {
        *var = Rte_NewDecryptedData ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}

/**************************************************************************/
/*                          Rte_Decryption_Key                            */
/**************************************************************************/
Std_ReturnType Rte_Write_Decryption_key( uint8* key )
{
    Std_ReturnType Return_Value ;
    if(DecryptionKey_ProtFlg != BUSY)
    {
        DecryptionKey_ProtFlg = BUSY ;
        Rte_Decryption_Key = key ;
        DecryptionKey_ProtFlg = IDLE ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
Std_ReturnType Rte_Read_Decryption_key( uint8** KeyAddress )
{
    Std_ReturnType Return_Value ;
    if(DecryptionKey_ProtFlg != BUSY)
    {
        *KeyAddress = Rte_Decryption_Key ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}

/**************************************************************************/
/*                          Rte_KeyChanged_Flag                           */
/**************************************************************************/
Std_ReturnType Rte_Write_KeyChanged( boolean val )
{
    Std_ReturnType Return_Value ;
    if(KeyChanged_ProtFlg != BUSY)
    {
        KeyChanged_ProtFlg = BUSY ;
        Rte_KeyChanged_Flag = val ;
        KeyChanged_ProtFlg = IDLE ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}
/**************************************************************************/
Std_ReturnType Rte_Read_KeyChanged( boolean *var )
{
    Std_ReturnType Return_Value ;
    if(KeyChanged_ProtFlg != BUSY)
    {
        *var = Rte_KeyChanged_Flag ;
        Return_Value = E_OK ;
    }
    else
    {
        Return_Value = E_NOT_OK ;
    }
    return Return_Value ;
}



