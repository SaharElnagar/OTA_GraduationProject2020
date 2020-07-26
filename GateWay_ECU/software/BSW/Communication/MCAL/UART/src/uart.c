/*
 * uart.c
 *
 * Created : 
 * Author : Yomna_Mokhtar
 */ 

#include "Platform_Types.h"
#include "uart.h"
#include "UART_HW_Types.h"
#include "hw_uart.h"
#include "hw_sysctl.h"
#include "hw_memmap.h"

/**************************************defines***********************************************/

#define  WRITE_REGISTER(address,value) (*(volatile uint32*)address = value)
#define  READ_REGISTER(address)        (*(volatile uint32*)address)
#define  SET_BIT(reg_address, bit)     (*(volatile uint32*)reg_address = *(volatile uint32*)reg_address | (1<<bit))
#define  CLEAR_BIT(reg_address, bit)   (*(volatile uint32*)reg_address = *(volatile uint32*)reg_address & ~(1<<bit))
#define  READ_BIT(reg_address, bit)	   (*(volatile uint32*)reg_address & (1<<bit))

#define	 IBRD_VALUE				0x208
#define	 FBRD_VALUE				0x35
#define	 MAX_UART_NUMBER		7

/********************************* Declarations********************************************/

uint32 Arr_Bases[8] = {BASE_UART0, BASE_UART1, BASE_UART2, BASE_UART3, BASE_UART4, BASE_UART5, BASE_UART6, BASE_UART7};
	

/******************************** functions ************************************************/
#define HWREG(x)                                                              \
        (*((volatile uint32 *)(x)))
#define UART_RX                 (1 << UART_RXPIN_POS)
#define UART_RX_PCTL            (UART_RXPIN_PCTL << (4 * UART_RXPIN_POS))

//*****************************************************************************
//
// This defines the UART transmit pin that is being used by the boot loader.
//
//*****************************************************************************
#define UART_TX                 (1 << UART_TXPIN_POS)
#define UART_TX_PCTL            (UART_TXPIN_PCTL << (4 * UART_TXPIN_POS))


enumUARTErrors UART_Init(uint8 UART_Num){
	
	if(UART_Num > MAX_UART_NUMBER){
		return UNVALID_UART_NUMBER;
	}
	
	SET_BIT(RCGC_UART, UART_Num);							     // enable clock to UART
	while((READ_BIT(PR_UART, UART_Num)) == 0){}	 			     // wait for peripheral to be ready
	CLEAR_BIT((Arr_Bases[UART_Num] + CTL), bit1);                // Disable UART until finishing initialization
	WRITE_REGISTER((Arr_Bases[UART_Num] + IBRD), IBRD_VALUE);	 // Integer Baud Rate division
	WRITE_REGISTER((Arr_Bases[UART_Num] + FBRD), FBRD_VALUE);	 // Fractional Baud Rate division
	WRITE_REGISTER((Arr_Bases[UART_Num] + LCRH), 0x70);			 // Serial parameters select -> select the word length = 8 bits
	WRITE_REGISTER((Arr_Bases[UART_Num] + CC), 0x0);			 // Clock source control  
	//SET_BIT((Arr_Bases[UART_Num] + CTL), bit7);				 // Loop back enable
	SET_BIT((Arr_Bases[UART_Num] + CTL), bit9);					 // Enable RX
	SET_BIT((Arr_Bases[UART_Num] + CTL), bit8);					 // Enable TX
	SET_BIT((Arr_Bases[UART_Num] + CTL), bit0);					 // Enable UART
    HWREG(Arr_Bases[UART_Num] + UART_O_LCRH) = UART_LCRH_WLEN_8 ;//| UART_LCRH_FEN;

    NVIC_PRI1_REG &=~UART0_PRI_BITS;             /* Clear priorty bit field */
    NVIC_PRI1_REG |= UART0_PRI_1;                /* Set Priority to 1 */
    NVIC_EN0_REG  |= UART0_ENABLE_INT;           /* Enable UART0 Interrupt in NVIC */
    *((uint32*)BASE_UART0+UART_O_IM)|=UART_INT_RX;    /*Enable interrupt mask*/
    *((uint32*)BASE_UART0+ UART_O_CTL) = (UART_CTL_UARTEN | UART_CTL_TXE |
                                          UART_CTL_RXE);
    return No_Errors;
}

/*******************************************************************************************/

void UART_Tx(uint8 UART_Num, uint8 c)
{
	while(READ_BIT((Arr_Bases[UART_Num] + FR), bit5) != 0){}           
	WRITE_REGISTER((Arr_Bases[UART_Num] + DR), c);
}

/*******************************************************************************************/

void UART_Rx(uint8 UART_Num, uint8* Ptr_Buffer)
{
	while(READ_BIT((Arr_Bases[UART_Num] + FR), bit4) != 0){}
	*Ptr_Buffer = (uint8)READ_REGISTER(Arr_Bases[UART_Num] + DR) ;
}

/*******************************************************************************************/

void UARTIntEnable(uint8 UART_Num, uint32 ui32IntFlags)
{
    // Enable UART interrupt in NVIC Interrupt Enable 0 Register
    switch(UART_Num)
    {
      case UART0 :
          NVIC_EN0_R |= (1<<5) ;
          break ;

      case UART1 :
          NVIC_EN0_R |= (1<<6) ;
          break ;

      case UART2 :
          NVIC_EN1_R |= (1<<1) ;
          break ;

      case UART3 :
          NVIC_EN1_R |= (1<<27) ;
          break ;

      case UART4 :
          NVIC_EN1_R |= (1<<28) ;
          break ;

      case UART5 :
          NVIC_EN1_R |= (1<<29) ;
          break ;

      case UART6 :
          NVIC_EN1_R |= (1<<30) ;
          break ;

      case UART7 :
          NVIC_EN1_R |= (uint32)(1<<31) ;
          break ;
    }
    WRITE_REGISTER((Arr_Bases[UART_Num] + IM) , ui32IntFlags) ;

    NVIC_PRI1_REG &=~UART0_PRI_BITS;             /* Clear priorty bit field */
    NVIC_PRI1_REG |= UART0_PRI_1;                /* Set Priority to 1 */
    NVIC_EN0_REG  |= UART0_ENABLE_INT;           /* Enable UART0 Interrupt in NVIC */
    *((uint32*)BASE_UART0+UART_O_IM)|=UART_INT_RX;    /*Enable interrupt mask*/
    *((uint32*)BASE_UART0+ UART_O_CTL) = (UART_CTL_UARTEN | UART_CTL_TXE |
                                          UART_CTL_RXE);

}

/*******************************************************************************************/

void UARTIntDisable(uint8 UART_Num, uint32 ui32IntFlags)
{
   uint32 RegVal = READ_REGISTER(Arr_Bases[UART_Num] + IM) & ~(ui32IntFlags) ;
   WRITE_REGISTER((Arr_Bases[UART_Num] + IM) , RegVal) ;
}


