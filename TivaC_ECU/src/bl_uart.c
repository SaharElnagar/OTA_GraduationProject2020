//*****************************************************************************
//
// bl_uart.c - Functions to transfer data via the UART port.
//
// Copyright (c) 2006-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the Tiva Firmware Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "bl_config.h"
#include "bl_uart.h"
#include "uart.h"

//*****************************************************************************
//
//! \addtogroup bl_uart_api
//! @{
//
//*****************************************************************************
#ifdef UART_UPDATE 



static uint8_t Cmd_Status;

void UART_Init(void)
{
	  //
    // Enable the the clocks to the UART and GPIO modules.
    //
    HWREG(SYSCTL_RCGCGPIO) |= (UART_RXPIN_CLOCK_ENABLE |
                               UART_TXPIN_CLOCK_ENABLE);
    HWREG(SYSCTL_RCGCUART) |= UART_CLOCK_ENABLE;

    //
    // Make the pin be peripheral controlled.
    //
    HWREG(UART_RXPIN_BASE + GPIO_O_AFSEL) |= UART_RX;
    HWREG(UART_RXPIN_BASE + GPIO_O_PCTL) |= UART_RX_PCTL;
    HWREG(UART_RXPIN_BASE + GPIO_O_ODR) &= ~(UART_RX);
    HWREG(UART_RXPIN_BASE + GPIO_O_DEN) |= UART_RX;

    HWREG(UART_TXPIN_BASE + GPIO_O_AFSEL) |= UART_TX;
    HWREG(UART_TXPIN_BASE + GPIO_O_PCTL) |= UART_TX_PCTL;
    HWREG(UART_TXPIN_BASE + GPIO_O_ODR) &= ~(UART_TX);
    HWREG(UART_TXPIN_BASE + GPIO_O_DEN) |= UART_TX;

    //
    // Set the baud rate.
    //
    HWREG(UARTx_BASE + UART_O_IBRD) = 520;
    HWREG(UARTx_BASE + UART_O_FBRD) = 53;

    //
    // Set data length, parity, and number of stop bits to 8-N-1.
    //
    HWREG(UARTx_BASE + UART_O_LCRH) = UART_LCRH_WLEN_8 ;//| UART_LCRH_FEN;
		
		//
		//Set FIFO Level to 2 bytes to suit the cmd length
    //
		 /*UARTFIFOEnable(UARTx_BASE);
     UARTFIFOLevelSet(UARTx_BASE, UART_IFLS_TX1_8,
                             UART_IFLS_RX1_8);	*/	
		//
    // Enable Receive Interrupt  and NIVC
    //		
		 NVIC_PRI1_REG &=~UART0_PRI_BITS;             /* Clear priorty bit field */
		 NVIC_PRI1_REG |= UART0_PRI_1;                /* Set Priority to 1 */
		 NVIC_EN0_REG  |= UART0_ENABLE_INT;           /* Enable UART0 Interrupt in NVIC */
		 HWREG(UARTx_BASE+UART_O_IM)|=UART_INT_RX;    /*Enable interrupt mask*/
     
		 
    //
    // Enable RX, TX, and the UART.
    //
    HWREG(UARTx_BASE + UART_O_CTL) = (UART_CTL_UARTEN | UART_CTL_TXE |
                                      UART_CTL_RXE);
}
//*****************************************************************************
//
//! Sends data over the UART port.
//!
//! \param pui8Data is the buffer containing the data to write out to the UART
//! port.
//! \param ui32Size is the number of bytes provided in \e pui8Data buffer that
//! will be written out to the UART port.
//!
//! This function sends \e ui32Size bytes of data from the buffer pointed to by
//! \e pui8Data via the UART port.
//!
//! \return None.
//
//*****************************************************************************
void
UARTSend(const uint8_t *pui8Data, uint32_t ui32Size)
{
    //
    // Transmit the number of bytes requested on the UART port.
    //
    while(ui32Size--)
    {
        //
        // Make sure that the transmit FIFO is not full.
        //
        while((HWREG(UARTx_BASE + UART_O_FR) & UART_FR_TXFF))
        {
        }

        //
        // Send out the next byte.
        //
        HWREG(UARTx_BASE + UART_O_DR) = *pui8Data++;
    }

    //
    // Wait until the UART is done transmitting.
    //
    UARTFlush();
}

//*****************************************************************************
//
//! Waits until all data has been transmitted by the UART port.
//!
//! This function waits until all data written to the UART port has been
//! transmitted.
//!
//! \return None.
//
//*****************************************************************************
void
UARTFlush(void)
{
    //
    // Wait for the UART FIFO to empty and then wait for the shifter to get the
    // bytes out the port.
    //
    while(!(HWREG(UARTx_BASE + UART_O_FR) & UART_FR_TXFE))
    {
    }

    //
    // Wait for the FIFO to not be busy so that the shifter completes.
    //
    while((HWREG(UARTx_BASE + UART_O_FR) & UART_FR_BUSY))
    {
    }
}

//*****************************************************************************
//
//! Receives data over the UART port.
//!
//! \param pui8Data is the buffer to read data into from the UART port.
//! \param ui32Size is the number of bytes provided in the \e pui8Data buffer
//! that should be written with data from the UART port.
//!
//! This function reads back \e ui32Size bytes of data from the UART port, into
//! the buffer that is pointed to by \e pui8Data.  This function will not
//! return until \e ui32Size number of bytes have been received.
//!
//! \return None.
//
//*****************************************************************************
void
UARTReceive(uint8_t *pui8Data, uint32_t ui32Size)
{
    //
    // Send out the number of bytes requested.
    //
    while(ui32Size--)
    {
        //
        // Wait for the FIFO to not be empty.
        //
        while((HWREG(UARTx_BASE + UART_O_FR) & UART_FR_RXFE))
        {
        }

        //
        // Receive a byte from the UART.
        //
        *pui8Data++ = HWREG(UARTx_BASE + UART_O_DR);
    }
}
/*ISR_Handler*/
void UART0_Handler()
{

  Cmd_Status= 0xFF&HWREG(UARTx_BASE+UART_O_DR);
	
	HWREG(UARTx_BASE+UART_O_ICR)|= UART_INT_RX ;
}

void write_update_notification_Request(uint16_t*cmd)
{
    *cmd=Cmd_Status;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
#endif
