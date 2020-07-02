/*
 * GPIO_HW_Types.h
 *
 * Created : 
 * Author : Yomna_Mokhtar
 */ 

#ifndef GPIO_HW_TYPES_H_
#define GPIO_HW_TYPES_H_

                          /***************** BASE ADDRESSES *****************/

#define   BASE_A      0x40004000
#define   BASE_B      0x40005000
#define   BASE_C      0x40006000
#define   BASE_D      0x40007000
#define   BASE_E      0x40024000
#define   BASE_F      0x40025000

                          /***************** OFFSETS *****************/
													
#define   DATA        0x3FC
#define   DIR         0x400
#define   IS          0x404
#define   IBE         0x408
#define   IEV         0x40C
#define   IM          0x410
#define   RIS         0x414
#define   MIS         0x418
#define   ICR         0x41c
#define   AFSEL       0x420
#define   PUR         0x510
#define   PDR         0x514
#define   DEN         0x51C
#define   LOCK        0x520
#define   CR          0x524
#define   AMSEL       0x528
#define   PCTL        0x52C



                           /*****************Ports**************************/
														 
#define PortA      0
#define PortB      1
#define PortC      2
#define PortD      3
#define PortE      4
#define PortF      5

                           /*****************defines**************************/
													 
#define RCGC_GPIO_R       0x400FE608
#define PR_GPIO_R         0x400FEA08
#define LOCK_KEY          0x4C4F434B


#define Port_Input              0x0
#define Port_Output             0xFF
#define Port_Digital_Enable     0xFF
#define Port_Digital_Disable    0x0

#define Pin_Input                              0
#define Pin_Output                             1
#define Pin_Digital_Enable_Analog_Disable      1
#define Pin_Digital_Disable_Analog_Enable      0

#define MAX_PORT_NUMBERS        5
#define MAX_PIN_NUMBERS         7


#endif
