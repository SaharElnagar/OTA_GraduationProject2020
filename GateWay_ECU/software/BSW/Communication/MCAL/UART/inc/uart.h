/*
 * uart.h
 *
 * Created : 
 * Author : Yomna_Mokhtar
 */ 

#ifndef UART_H_
#define UART_H_

#include "Platform_Types.h"
#include "tm4c123gh6pm.h"

/*this definition is only to make me able to use tm4c123gh6pm.h file*/
#define uint32_t    uint32

/************************************** defines ********************************/

#define		UART0			0
#define		UART1			1
#define		UART2			2
#define		UART3			3
#define		UART4			4
#define		UART5			5
#define		UART6			6
#define		UART7			7

//*****************************************************************************
//
// Values that can be passed to UARTIntEnable, UARTIntDisable, and UARTIntClear
// as the ui32IntFlags parameter, and returned from UARTIntStatus.
//
//*****************************************************************************
#define UART_INT_DMATX          0x20000     // DMA TX interrupt
#define UART_INT_DMARX          0x10000     // DMA RX interrupt
#define UART_INT_9BIT           0x1000      // 9-bit address match interrupt
#define UART_INT_OE             0x400       // Overrun Error Interrupt Mask
#define UART_INT_BE             0x200       // Break Error Interrupt Mask
#define UART_INT_PE             0x100       // Parity Error Interrupt Mask
#define UART_INT_FE             0x080       // Framing Error Interrupt Mask
#define UART_INT_RT             0x040       // Receive Timeout Interrupt Mask
#define UART_INT_TX             0x020       // Transmit Interrupt Mask
#define UART_INT_RX             0x010       // Receive Interrupt Mask
#define UART_INT_DSR            0x008       // DSR Modem Interrupt Mask
#define UART_INT_DCD            0x004       // DCD Modem Interrupt Mask
#define UART_INT_CTS            0x002       // CTS Modem Interrupt Mask
#define UART_INT_RI             0x001       // RI Modem Interrupt Mask


// UART Errors 20 -> 29
typedef enum{
	No_Errors = 20,
	UNVALID_UART_NUMBER = 21,
	
}enumUARTErrors;


enumUARTErrors UART_Init(uint8 UART_Num) ;
void UART_Tx(uint8 UART_Num, uint8 c) ;
void UART_Rx(uint8 UART_Num, uint8* Ptr_Buffer) ;
void UARTIntEnable(uint8 UART_Num, uint32 ui32IntFlags) ;
void UARTIntDisable(uint8 UART_Num, uint32 ui32IntFlags) ;


#endif

