/*
 * UART_HW_Types.h
 *
 * Created : 
 * Author : Yomna_Mokhtar
 */ 

#ifndef UART_HW_TYPES_H_
#define UART_HW_TYPES_H_


                          /***************** BASE ADDRESSES *****************/

#define		BASE_UART0		0x4000C000
#define		BASE_UART1		0x4000D000
#define		BASE_UART2		0x4000E000
#define		BASE_UART3		0x4000F000
#define		BASE_UART4		0x40010000
#define		BASE_UART5		0x40011000
#define		BASE_UART6		0x40012000
#define		BASE_UART7		0x40013000

                          /***************** OFFSETS *****************/

#define		CTL			0x030
#define		IBRD		0x024
#define		FBRD		0x028
#define		LCRH		0x02C
#define		CC			0xFC8
#define		FR			0x018
#define		DR			0x000
#define     IM          0x038

	
	                        /***************** BIT FIELDS *****************/

#define   bit0        0
#define   bit1        1
#define   bit2        2
#define   bit3        3
#define   bit4        4
#define   bit5        5
#define   bit6        6
#define   bit7        7
#define   bit8        8
#define   bit9        9
#define   bit10       10
#define   bit11       11
#define   bit12       12
#define   bit13       13
#define   bit14       14
#define   bit15       15


	                   /***************** System Control Registers' Adresses *****************/


#define		RCGC_UART	  0x400FE618
#define		PR_UART		  0x400FEA18


#endif
