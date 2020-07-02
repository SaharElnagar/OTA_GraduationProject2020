/**************************************************************************/
/*                                                                        */
/* File : Decryption_swc.h                                                */
/*                                                                        */
/* Date : 29 June 2020                                                    */
/*                                                                        */
/* Author : Yomna Mokhtar                                                 */
/*                                                                        */
/**************************************************************************/

/***************************************************************************/
/*                             File includes                               */
/***************************************************************************/
#include "Rte_Decryption.h"
#include "Decryption_SWC.h"

/***************************************************************************/
/*                        Local type definitions                           */
/***************************************************************************/
typedef uint8 Internal_State ;
#define   IDLE_STATE               ((Internal_State)0U)
#define   GET_PACKET               ((Internal_State)1U)
#define   GET_KEY                  ((Internal_State)2U)
#define   DECRYPT_PACKET           ((Internal_State)3U)
#define   WRITE_DECRYPTED_INFO     ((Internal_State)4U)


/***************************************************************************/
/*                            Local Macros                                 */
/***************************************************************************/
#define  FULL_PACKET_SIZE             1024
#define  DECRYPTION_BLOCK_SIZE        16
#define  KEY_SIZE                     16

/***************************************************************************/
/*                          Local variables                                */
/***************************************************************************/
static Internal_State ModuleState ;
static boolean NewDataFlag ;
static uint16 PacketSize , counter ;
static uint8* KeyPtr ;
static uint8 roundkeys[AES_ROUND_KEY_SIZE];
static uint8 DecryptedBuffer[FULL_PACKET_SIZE] ;
static uint8* PtrToEncryptedBuffer ;
static uint8* PtrToDecryptedBuffer ;
static boolean KeyChangedFlag ;
static Std_ReturnType RTE_Return_Value ;

/***************************************************************************/
/*                          Local Functions                                */
/***************************************************************************/


/***************************************************************************/
/*                    Global Functions implementation                      */
/***************************************************************************/

/***************************************************************************/
/*    Function Name           : Decryption_Init                            */
/*    Function Description    : Initialize Decryption Module               */
/*    Parameter in            : none                                       */
/*    Parameter inout         : none                                       */
/*    Parameter out           : none                                       */
/*    Return value            : none                                       */
/***************************************************************************/
void Decryption_Init(void)
{
    ModuleState = IDLE_STATE ;
    NewDataFlag = FALSE ;
    PtrToDecryptedBuffer = DecryptedBuffer ;
}

/***************************************************************************/
/*    Function Name           : Decryption_MainFunction                    */
/*    Function Description    : Decryption Module main function            */
/*    Parameter in            : none                                       */
/*    Parameter inout         : none                                       */
/*    Parameter out           : none                                       */
/*    Return value            : none                                       */
/***************************************************************************/
void Decryption_MainFunction(void)
{
    uint8 NumOfBlocks;
    uint8 ExtraBytes ;

    switch(ModuleState)
    {
/************************IDLE_STATE***************************/
       case IDLE_STATE :

           /* wait for new encrypted data flag to be equal to TRUE */
           RTE_Return_Value = RTE_READ_NEW_ENCRYPTED_FLAG(&NewDataFlag) ;

           if(RTE_Return_Value == E_OK)
           {
               if(NewDataFlag == TRUE)
               {
                  /* set new encrypted data flag to FALSE */
                   RTE_Return_Value = RTE_WRITE_NEW_ENCRYPTED_FLAG(FALSE) ;

                   if(RTE_Return_Value == E_OK)
                   {
                       /* Go to receive packet state */
                       ModuleState = GET_KEY ;
                   }
                   else {}
               }
               else {}
           }
           else {}
           break ;
/****************************GET_KEY***************************/
       case GET_KEY :

           /* read key changed flag */
           RTE_Return_Value = RTE_READ_KEY_CHANGED_FLAG(&KeyChangedFlag) ;

           if(RTE_Return_Value == E_OK)
           {
               /* if key changed flag is true , read the key and calculate round keys */
               if(KeyChangedFlag == TRUE)
               {
                   RTE_Return_Value = RTE_READ_DECRYPTION_KEY(&KeyPtr) ;

                   if(RTE_Return_Value == E_OK)
                   {
                       /* Key schedule */
                       aes_key_schedule_128(KeyPtr, roundkeys) ;

                       /* Go to GET_PACKET state */
                       ModuleState = GET_PACKET ;
                   }
                   else {}
               }
               /* if key changed flag is false */
               else
               {
                   /* Go to GET_PACKET state directly without recalculation of roundkeys because it is not changed */
                   ModuleState = GET_PACKET ;
               }
           }
           else {}

           break ;
/****************************GET_PACKET***************************/
      case GET_PACKET :

           /* Get packet size written by the Download update SWC */
           RTE_Return_Value = RTE_READ_PACKET_SIZE(&PacketSize) ;

           if(RTE_Return_Value == E_OK)
           {
               /* Read the pointer to the Encrypted buffer written by the Download update SWC */
               RTE_Return_Value = RTE_READ_ENCRYPTED_BUFFER(&PtrToEncryptedBuffer) ;

               if(RTE_Return_Value == E_OK)
               {
                   /* Go to Decrypt packet state */
                   ModuleState = DECRYPT_PACKET ;
               }
               else {}
           }
           else {}
           break ;
/****************************DECRYPT_PACKET*******************/
       case DECRYPT_PACKET :

           PtrToDecryptedBuffer = DecryptedBuffer ;

           /* calculate number of decryption blocks inside packet */
           /* Note : the decryption function decrypts a block of 16 bytes at a time */
           NumOfBlocks = PacketSize / DECRYPTION_BLOCK_SIZE ;

           /* calculate number of ExtraBytes */
           ExtraBytes = PacketSize % DECRYPTION_BLOCK_SIZE ;

           /* loop over blocks of data inside packet */
           for(counter = 0 ; counter < NumOfBlocks ; counter++)
           {
               /* Decrypt a block of 16 bytes of data */
               aes_decrypt_128(roundkeys, PtrToEncryptedBuffer, PtrToDecryptedBuffer) ;

               /* increment pointers to point to the next block inside packet */
               PtrToEncryptedBuffer += DECRYPTION_BLOCK_SIZE ;
               PtrToDecryptedBuffer += DECRYPTION_BLOCK_SIZE ;
           }

           /* if there is extra bytes --> data size is not aligned to 16 bytes(decryption block size) */
           if(ExtraBytes != 0)
           {
               /* decrypt the last block */
               aes_decrypt_128(roundkeys, PtrToEncryptedBuffer, PtrToDecryptedBuffer) ;

               /* make the decrypted buffer pointer points to beginning of area after the actual data  */
               PtrToDecryptedBuffer += ExtraBytes ;

               /* Set the remaining bytes to be equal to 0 */
               for(counter = ExtraBytes ; counter < DECRYPTION_BLOCK_SIZE ; counter++)
               {
                   *PtrToDecryptedBuffer++ = 0 ;
               }

               /* if number of blocks is less than 63 (number of blocks inside a full packet - 1) */
               if(NumOfBlocks < 63)
               {
                   /* Set the remaining bytes inside the packet to be equal to 0 */
                   while(PtrToDecryptedBuffer <= &DecryptedBuffer[FULL_PACKET_SIZE - 1] )
                   {
                       *PtrToDecryptedBuffer++ = 0 ;
                   }
               }
           }
           /* else , if there is no extra bytes (data is aligned to block size "16 bytes")
            * and number of blocks inside the packet is less than number of blocks inside a full packet
            */
           else if(NumOfBlocks < FULL_PACKET_SIZE / DECRYPTION_BLOCK_SIZE)
           {
               /* Set the remaining bytes inside the packet to be equal to 0 */
               while(PtrToDecryptedBuffer <= &DecryptedBuffer[FULL_PACKET_SIZE - 1] )
               {
                   *PtrToDecryptedBuffer++ = 0 ;
               }
           }
           ModuleState = WRITE_DECRYPTED_INFO ;
           break ;
/***********************WRITE_DECRYPTED_INFO*******************/
       case WRITE_DECRYPTED_INFO:

           /* write Decrypted buffer pointer into RTE */
           RTE_Return_Value = RTE_WRITE_DECRYPTED_BUFFER(DecryptedBuffer) ;

           if(RTE_Return_Value == E_OK)
           {
               /* Raise New decrypted data flag to the RTE */
               RTE_Return_Value = RTE_WRITE_NEW_DECRYPTED_FLAG(TRUE) ;

               if(RTE_Return_Value == E_OK)
               {
                   /* go to IDLE STATE and wait for future decryption requests */
                   ModuleState = IDLE_STATE ;
               }
               else {}
           }
           else {}
           break ;
    }
}


