/**************************************************************************/
/*                                                                        */
/* File : DownloadUpdate_swc.c                                            */
/*                                                                        */
/* Date : 23 June 2020                                                    */
/*                                                                        */
/* Author : Yomna Mokhtar                                                 */
/*                                                                        */
/**************************************************************************/


/***************************************************************************/
/*                             File includes                               */
/***************************************************************************/
#include "Rte_DownloadUpdate.h"
#include "DownloadUpdate_swc.h"
#include "uart.h"
#include "GPIO_.h"

/***************************************************************************/
/*                        Local type definitions                           */
/***************************************************************************/
typedef uint8 Internal_State ;
#define   IDLE_STATE               ((Internal_State)0U)
#define   READ_REQUEST_RESULT      ((Internal_State)1U)
#define   RECEIVE_SIZE             ((Internal_State)2U)
#define   RECEIVE_PACKET           ((Internal_State)3U)
#define   WRITE_DATA_TO_RTE        ((Internal_State)4U)
#define   STORE_PACKET             ((Internal_State)5U)
#define   RETRANSMISSION_STATE     ((Internal_State)6U)
#define   END_STATE                ((Internal_State)7U)

/***************************************************************************/
/*                            Local Macros                                 */
/***************************************************************************/
/* Transmitted Messages sent to ESP */
#define DISCARD_UPDATE                        (0x01U)
#define READY_FOR_UPDATE                      (0x02U)
#define READY_TO_RECEIVE_NEXT_PACKET          (0x03U)
#define RETRANSMIT_LAST_PACKET                (0x05U)

/* Received Messages */
#define DOWNLOAD_REQ                          (0x04U)

/*Packet size = 1024 bytes*/
#define PACKET_SIZE                           1024
#define FIRST_FRAME_SIZE                      36
#define KEY_SIZE                              16

/***************************************************************************/
/*                          Local variables                                */
/***************************************************************************/
static Internal_State ModuleState ;
static uint8 ReceivedPacketBuffer[1024] ;
static uint8 ByteBuffer ;
static uint32 FileSize ;
static uint16 NumOfPackets = 0, Extra_Bytes = 0 ;
static uint8 C = 0 ;
static uint16 BufferSize = 0 ;
static boolean First_Frame = TRUE, NewRequest = FALSE ;
static uint8 Key[KEY_SIZE] ;
static Std_ReturnType StoreState = E_PENDING ;
static boolean RequestResult = E_PENDING ;

static enumAltFunc Alternate_Function = UART ;
static strGPIOPinInit GPIO_Pin_UART_TX ;
static strGPIOPinInit GPIO_Pin_UART_RX ;

/***************************************************************************/
/*                          Local Functions                                */
/***************************************************************************/
static Std_ReturnType GPIO_PinConfigure(strGPIOPinInit* PtrToStr, uint8 port, uint8 pin, uint8 Dir, uint8 digital, enumAltFunc alt_func) ;
static void UART_ReceivePacket(void) ;
void UART0_Handler(void) ;

/***************************************************************************/
/*                    Local Functions implementation                       */
/***************************************************************************/

/****************************************************************************/
/*    Function Name           : GPIO_PinConfigure                           */
/*    Function Description    : Function to make GPIO pin configuration     */
/*    Parameter in            : PtrToStr, port, pin, Dir, digital, alt_func */
/*    Parameter inout         : none                                        */
/*    Parameter out           : none                                        */
/*    Return value            : Std_ReturnType                              */
/****************************************************************************/
static Std_ReturnType GPIO_PinConfigure(strGPIOPinInit* PtrToStr, uint8 port, uint8 pin, uint8 Dir, uint8 digital, enumAltFunc alt_func)
{
    Std_ReturnType Return_Val = E_OK ;

    if(PtrToStr == NULL){
        return Unvalid_Parameter;
    }
    PtrToStr->port = port;
    PtrToStr->pin = pin;
    PtrToStr->Dir = Dir;
    PtrToStr->Digital = digital;
    PtrToStr->alt_func = alt_func;

    if(GPIO_Pin_Init(PtrToStr) != NoErrors)
    {
        Return_Val = E_NOT_OK ;
    }
    return Return_Val ;
}


/****************************************************************************/
/*    Function Name           : UART_ReceivePacket                          */
/*    Function Description    : Function to receive a full packet           */
/*                                   or Extra_Bytes from UART               */
/*    Parameter in            : none                                        */
/*    Parameter inout         : none                                        */
/*    Parameter out           : none                                        */
/*    Return value            : none                                        */
/****************************************************************************/
static void UART_ReceivePacket(void)
{
    uint16 counter = 0 ;

    if(First_Frame == TRUE)
    {
        /* Read First 16 byte into Key buffer */
        for(counter = 0 ; counter < KEY_SIZE ; counter++)
        {
            UART_Rx(UART0, (Key + counter)) ;
            counter++ ;
        }
        /* Read the rest of frame into ReceivedPacketBuffer */
        for(counter = 0 ; counter < FIRST_FRAME_SIZE - KEY_SIZE  ; counter++)
        {
            UART_Rx(UART0, (ReceivedPacketBuffer + counter)) ;
            counter++ ;
        }
        /* calculate size of frame sent to the next module */
        BufferSize = FIRST_FRAME_SIZE - KEY_SIZE ;
    }

    /* Read a full packet */
    else if(NumOfPackets > 0)
    {
        while(counter < PACKET_SIZE)
        {
            /* There is must be a delay */
            UART_Rx(UART0, (ReceivedPacketBuffer + counter)) ;
            counter++ ;
        }
        NumOfPackets-- ;
        FileSize -= PACKET_SIZE ;
        BufferSize = PACKET_SIZE ;
    }
    /* Read a number of bytes less than packet size = Extra_Bytes */
    else
    {
        while(counter < Extra_Bytes)
        {
            /* There is must be a delay */
            UART_Rx(UART0, (ReceivedPacketBuffer + counter)) ;
            counter++ ;
        }
        FileSize -= Extra_Bytes ;
        BufferSize = Extra_Bytes ;
    }
}


/***************************************************************************/
/*                    Global Functions implementation                      */
/***************************************************************************/

/***************************************************************************/
/*    Function Name           : Download_Update_Init                       */
/*    Function Description    : Initialize DownloadUpdate Module           */
/*    Parameter in            : none                                       */
/*    Parameter inout         : none                                       */
/*    Parameter out           : none                                       */
/*    Return value            : none                                       */
/***************************************************************************/
void Download_Update_Init(void)
{
    /*Initialize portA*/
    GPIO_Port_Init(PortA) ;

    /*PortA Pin1 configuration as UART TX Pin*/
    GPIO_PinConfigure(&GPIO_Pin_UART_TX , PortA, 1, Pin_Output, Pin_Digital_Enable_Analog_Disable, Alternate_Function) ;

    /*PortA Pin0 configuration as UART RX Pin*/
    GPIO_PinConfigure(&GPIO_Pin_UART_RX, PortA, 0, Pin_Input, Pin_Digital_Enable_Analog_Disable, Alternate_Function) ;

    /* UART0 Initialization */
    UART_Init(UART0) ;

    /* UART Interrupt enable */
    UARTIntEnable(UART0, UART_INT_RX) ;

    /* Set module internal state to Idle */
    ModuleState = IDLE_STATE ;

}

/****************************************************************************/
/*    Function Name           : Download_Update_MainFunction                */
/*    Function Description    : main processing of DownloadUpdate Module    */
/*    Parameter in            : none                                        */
/*    Parameter inout         : none                                        */
/*    Parameter out           : none                                        */
/*    Return value            : none                                        */
/****************************************************************************/
void Download_Update_MainFunction(void)
{
    Std_ReturnType RTE_Return_Value = E_PENDING ;

    switch(ModuleState)
    {
/****************************IDLE STATE***********************************/
      case IDLE_STATE :

          /* wait for the first UART receive interrupt*/
          if(NewRequest == TRUE)
          {
              /* raise new update request flag to RTE */
              RTE_Return_Value = RTE_WRITE_NEW_UPDATE_REQ(TRUE) ;

              if(RTE_Return_Value == E_OK)
              {
                  /* set new request flag to false */
                  NewRequest = FALSE ;

                  /* set first frame flag to true */
                  First_Frame = TRUE ;

                  /* Set the module internal state to receive update request*/
                  ModuleState = READ_REQUEST_RESULT ;
              }
              else {}
          }
          break ;
/*************************READ_REQUEST_RESULT*******************************/
      case READ_REQUEST_RESULT :

            /* read update request flag value from RTE */
             RTE_READ_UPDATE_REQ_ACCEPTED(&RequestResult) ;

            /* Update request is Accepted */
            if( RequestResult == TRUE )
            {
                /* return the request accepted flag in RTE to pending state for future requests */
                RTE_Return_Value = RTE_WRITE_UPDATE_REQ_ACCEPTED(E_PENDING) ;

                if(RTE_Return_Value == E_OK)
                {
                    /* send UART message to esp to send file size */
                    UART_Tx(UART0, READY_FOR_UPDATE) ;

                    /* go to receive size state */
                    ModuleState = RECEIVE_SIZE ;

                    C = 0 ;
                }
                else
                {}
            }
            /* Update request is Refused */
            else if( RequestResult == E_NOT_OK )
            {
                /* return the request accepted flag in RTE to pending state for future requests */
                RTE_Return_Value = RTE_WRITE_UPDATE_REQ_ACCEPTED(E_PENDING) ;

                if(RTE_Return_Value == E_OK)
                {
                    /* Enable UART Interrupts for future requests */
                    UARTIntEnable(UART0, UART_INT_RX) ;

                    /* if the user refused the update, send UART message to discard it */
                    UART_Tx(UART0, DISCARD_UPDATE) ;

                    /* return to IDLE state */
                    ModuleState = IDLE_STATE ;
                }
                else
                {}
            }

            /* if the update request is still pending */
            else
            {
                /* Wait for user response */
            }

            break ;
/***************************RECEIVE_SIZE***********************************/
      case RECEIVE_SIZE :

          /* Receive first part of the FileSize number */
          if(C == 0)
          {
              UART_Rx(UART0, &ByteBuffer) ;
              FileSize = ByteBuffer * 1000 ;
              C = 1 ;
          }
          /* Receive second part of the FileSize number */
          else if(C == 1)
          {
             UART_Rx(UART0, &ByteBuffer) ;
             FileSize += ByteBuffer * 100 ;
             C = 2 ;
          }
          /* Receive third part of the FileSize number */
          else if(C == 2)
          {
             UART_Rx(UART0, &ByteBuffer) ;
             FileSize += ByteBuffer ;
             C = 0 ;

             /* calculate number of packets and Extra bytes to be received*/
             NumOfPackets = FileSize / PACKET_SIZE ;
             Extra_Bytes = FileSize % PACKET_SIZE ;

             /* tell the ESP module that I am ready for receiving packets */
             UART_Tx(UART0, READY_TO_RECEIVE_NEXT_PACKET) ;

             /* Go to receive packet state */
             ModuleState = RECEIVE_PACKET ;
          }
          break ;
/***************************RECEIVE_PACKET***********************************/
      case RECEIVE_PACKET :

            /* receive 1024 full packet or extra bytes or first frame */
            UART_ReceivePacket() ;

            /* if this frame is the first one */
            if(First_Frame == TRUE)
            {
                /* set first frame flag to false */
                First_Frame = FALSE ;

                /* write key pointer into RTE */
                RTE_Return_Value = RTE_WRITE_DECRYPTION_KEY(Key) ;

                if(RTE_Return_Value == E_OK)
                {
                    /* Set key changed flag in RTE to true */
                    RTE_Return_Value = RTE_WRITE_KEY_CHANGED(TRUE) ;

                    if(RTE_Return_Value == E_OK)
                    {
                        /* go to write data pointer into RTE */
                        ModuleState = WRITE_DATA_TO_RTE ;
                    }
                    else {}
                }
                else {}
            }
            /* if it is not the first frame */
            else
            {
                /* go to write data pointer into RTE directly without writing key */
                ModuleState = WRITE_DATA_TO_RTE ;
            }
            break ;
/*****************************WRITE_TO_RTE***********************************/
      case WRITE_DATA_TO_RTE :

            /* write data packet pointer into RTE */
            RTE_Return_Value = RTE_WRITE_DATA_PACKET(ReceivedPacketBuffer) ;

            if(RTE_Return_Value == E_OK)
            {
                /* write the size of packet into RTE */
                RTE_Return_Value = RTE_WRITE_PACKET_SIZE(BufferSize) ;
                if(RTE_Return_Value == E_OK)
                {
                    /* set new encrypted data flag */
                    RTE_Return_Value = RTE_WRITE_NEW_ENCRYPTED_FLAG(TRUE) ;
                    if(RTE_Return_Value == E_OK)
                    {
                        /* go to store packet state */
                        ModuleState = STORE_PACKET ;
                    }
                    else
                    {}
                }
                else
                {}
            }
            else
            {}
            break ;
/***************************STORE_PACKET***********************************/
      case STORE_PACKET :

          /* read store state flag to find its state */
          RTE_Return_Value = RTE_READ_STORE_DATA_STATE(&StoreState) ;

          if(RTE_Return_Value == E_OK)
          {
              /* if storing the packet is done */
              if(StoreState == E_OK)
              {
                  /* return store data state flag in RTE to pending state for next store process */
                  RTE_Return_Value = RTE_WRITE_STORE_DATA_STATE(E_PENDING) ;

                  if(RTE_Return_Value == E_OK)
                  {
                      if(FileSize == 0)
                      {
                          ModuleState = END_STATE ;
                      }
                      else
                      {
                          /* send UART message to esp to send the next packet */
                          UART_Tx(UART0, READY_TO_RECEIVE_NEXT_PACKET) ;

                          /* go to receive packet state again */
                          ModuleState = RECEIVE_PACKET ;
                      }
                  }
                  else{}
              }
              /* if an error happened while storing packet */
              else if(StoreState == E_NOT_OK)
              {
                  /* return store data state flag in RTE to pending state for next store process */
                  RTE_Return_Value = RTE_WRITE_STORE_DATA_STATE(E_PENDING) ;

                  if(RTE_Return_Value == E_OK)
                  {
                      /* go to receive packet state again */
                      ModuleState = RETRANSMISSION_STATE ;
                  }

              }
              else
              { /* wait until storing data is done */ }
          }
          else {}
          break ;
/********************RETRANSMISSION_STATE*******************************/
      case RETRANSMISSION_STATE :

          /* send UART message to ESP to retransmit the last packet again */
          UART_Tx(UART0, RETRANSMIT_LAST_PACKET) ;

          /* if the problem happened while storing extra data */
          if(FileSize == 0 && Extra_Bytes != 0)
          {
              /* re-increment the file size by Extra_Bytes number */
              FileSize += Extra_Bytes ;
          }
          /* if the problem happened while storing a full packet */
          else
          {
              /* re-increment the file size by the packet size and re-increment NumOfPackets by 1 */
              FileSize += PACKET_SIZE ;
              NumOfPackets++ ;
          }
          /* Go to receive packet state */
          ModuleState = RECEIVE_PACKET ;
          break ;
/***************************END_STATE***********************************/
      case END_STATE :

          /* raise update done flag to RTE */
          RTE_Return_Value = RTE_WRITE_DONE_DOWNLOADING(TRUE) ;

          if(RTE_Return_Value == E_OK)
          {
              /* Enable UART Interrupts for future requests */
              UARTIntEnable(UART0, UART_INT_RX) ;

              /* return to IDLE state */
              ModuleState = IDLE_STATE ;
          }
          else{}
          break ;
    }
}

__attribute__((naked)) void irq_Disable(void)
{
    __asm(" CPSID I");
    __asm(" BX LR") ;
}

/****************************************************************************/
/*    Function Name           : UART0_Handler                               */
/*    Function Description    : UART0 interrupt service routine             */
/*    Parameter in            : none                                        */
/*    Parameter inout         : none                                        */
/*    Parameter out           : none                                        */
/*    Return value            : none                                        */
/****************************************************************************/
void UART0_Handler(void)
{
    uint8 Msg = (uint8)UART0_DR_R ;

    if(Msg == DOWNLOAD_REQ)
    {
        /* clear Receive interrupt flag */
        UART0_ICR_R &= ~(1<<4) ;

        /* Disable UART interrupts */
        UARTIntDisable(UART0, UART_INT_RX) ;

        /* set internal new request flag to true */
        NewRequest = TRUE ;
    }
    *((uint32*)0x4000C000+0x00000044)|= UART_INT_RX ;
    irq_Disable();

}




