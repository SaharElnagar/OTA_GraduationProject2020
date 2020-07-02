/*
 * GPIO.h
 *
 * Created : 
 * Author : Yomna_Mokhtar
 */ 

#ifndef MCAL_GPIO_H_
#define MCAL_GPIO_H_

#include "Platform_Types.h"
#include "GPIO_HW_Types.h"

#ifndef TRUE
#define TRUE   (1U)
#endif

#ifndef FALSE
#define FALSE  (0U)
#endif

typedef enum{
	
	NON = 0,
	UART = 1,
	I2C = 3,
	PWM_M0 = 4,
	PWM_M1 = 5,
	TIMER = 7,
	
}enumAltFunc;

typedef struct{

	uint8 port;
	uint8 pin;
	uint8 Dir;
	uint8 Digital;
	enumAltFunc alt_func;

}strGPIOPinInit; 

// GPIO_Errors 0 -> 9
typedef enum{
	
    NoErrors = 0,
	Uninitialized_Port = 1,
	Unvalid_Port = 2,
	Unvalid_Pin = 3,
	Unvalid_Parameter = 4,
	Unvalid_Pin_Direction = 5,
	Unvalid_Alternate_Function = 6,
	No_Read_Permission = 7,
	No_Write_Permission = 8,

}enumGPIOErrors;


enumGPIOErrors GPIO_Port_Init(uint8 port);
enumGPIOErrors GPIO_Pin_Init(strGPIOPinInit* str);
enumGPIOErrors Read_Pin(strGPIOPinInit* str, uint8* val);
enumGPIOErrors Write_Pin(strGPIOPinInit* str, uint8 val);
enumGPIOErrors Write_Port(uint8 port, uint8 val);

#endif

