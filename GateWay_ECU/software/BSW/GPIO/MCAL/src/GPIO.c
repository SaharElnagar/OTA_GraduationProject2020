/*
 * GPIO.c
 *
 * Created : 
 * Author : Yomna_Mokhtar
 */ 

#include "Platform_Types.h"
#include "Std_Types.h"
#include "GPIO_.h"
#include "GPIO_HW_Types.h"

#define  WRITE_REGISTER(address,value) (*(volatile uint32*)address = value)
#define  READ_REGISTER(address)        (*(volatile uint32*)address)
#define  SET_BIT(reg_address, pin)     (*(volatile uint32*)reg_address = *(volatile uint32*)reg_address | (1<<pin))
#define  CLEAR_BIT(reg_address, pin)   (*(volatile uint32*)reg_address = *(volatile uint32*)reg_address & ~(1<<pin))
	
	
uint32 arr_bases[] ={BASE_A, BASE_B, BASE_C, BASE_D, BASE_E, BASE_F};
uint8 Initialized_Ports[] = {0, 0, 0, 0, 0, 0};

enumGPIOErrors error = NoErrors;

/******************************************************************************************************/

enumGPIOErrors GPIO_Port_Init(uint8 port){
		if(port > MAX_PORT_NUMBERS){
			return Unvalid_Port;
		} else{
			if(Initialized_Ports[port] == 0){
				WRITE_REGISTER(RCGC_GPIO_R, ((*(volatile uint32*)RCGC_GPIO_R)|(1<<port)));
				while((READ_REGISTER(PR_GPIO_R) & (1<<port)) == 0){}
				WRITE_REGISTER((arr_bases[port]+LOCK), LOCK_KEY);
				Initialized_Ports[port] = 1;
			}
			return NoErrors;
		}
}

/******************************************************************************************************/

enumGPIOErrors GPIO_Pin_Init(strGPIOPinInit* str){
	
		        /*******************Check for NULL Pointer*******************/
	if(str == NULL){
		return Unvalid_Parameter;
	} else if(str->port > MAX_PORT_NUMBERS){
		return Unvalid_Port;
	} else if(str->pin > MAX_PIN_NUMBERS){
		return Unvalid_Pin;
	}
	else{                        /*Not NULL Pointer*/
		error = GPIO_Port_Init(str->port);
		if(error != NoErrors){
			return error;
		}
		WRITE_REGISTER((arr_bases[str->port] + CR), READ_REGISTER((arr_bases[str->port] + CR))|(1<<str->port));
		
		        /*******************Direction*******************/
		if(str->Dir == Pin_Input){
			CLEAR_BIT((arr_bases[str->port] + DIR), str->pin);
		} else if(str->Dir == Pin_Output){
			SET_BIT((arr_bases[str->port] + DIR), str->pin);
		} else{
			return Unvalid_Pin_Direction;
		}
		
		        /*******************Digital vs Analog mode*******************/
		if(str->Digital == Pin_Digital_Enable_Analog_Disable){
			SET_BIT((arr_bases[str->port] + DEN), str->pin);
			CLEAR_BIT((arr_bases[str->port] + AMSEL), str->pin);
		} else if(str->Digital == Pin_Digital_Disable_Analog_Enable){
			CLEAR_BIT((arr_bases[str->port] + DEN), str->pin);
			SET_BIT((arr_bases[str->port] + AMSEL), str->pin);
		} else{
			return Unvalid_Parameter;
		}

		        /*******************Alternative functions*******************/
		if(str->alt_func == NON){
			CLEAR_BIT((arr_bases[str->port] + AFSEL), str->pin);
		} else if(str->alt_func == UART | str->alt_func == I2C | str->alt_func == PWM_M0 | str->alt_func == PWM_M1 | str->alt_func == TIMER | str->alt_func == I2C){
			
			SET_BIT((arr_bases[str->port] + AFSEL), str->pin);
			WRITE_REGISTER((arr_bases[str->port] + PCTL), READ_REGISTER((arr_bases[str->port] + PCTL)) & ~ (0xF<<(str->pin*4)));
			WRITE_REGISTER((arr_bases[str->port] + PCTL), READ_REGISTER((arr_bases[str->port] + PCTL)) | (str->alt_func << (str->pin *4)));
			
			} 
			else{
				return Unvalid_Alternate_Function;
			}
	}
	
	return NoErrors;
	
}

/******************************************************************************************************/

enumGPIOErrors Read_Pin(strGPIOPinInit* str, uint8* val){
	
	if(Initialized_Ports[str->port] != 1){
		return Uninitialized_Port;
	} else if(str->pin > MAX_PIN_NUMBERS){
		return Unvalid_Pin;
	} else if(str->port > MAX_PORT_NUMBERS){
		return Unvalid_Pin;
	} else if(str->Dir != Pin_Input){
		return No_Read_Permission;
	} 
	else{
		*val = *((volatile uint32*)(arr_bases[str->port]+DATA))&(1<<str->pin);
		return NoErrors;
	}
}

/******************************************************************************************************/

enumGPIOErrors Write_Pin(strGPIOPinInit* str, uint8 val){
	
	
	if(Initialized_Ports[str->port] != 1){
		return Uninitialized_Port;
	} else if(str->pin > MAX_PIN_NUMBERS){
		return Unvalid_Pin;
	} else if(str->port > MAX_PORT_NUMBERS){
		return Unvalid_Pin;
	} else if(str->Dir != Pin_Output){
		return No_Write_Permission;
	} else{
		if(val == TRUE){
			SET_BIT((arr_bases[str->port]+DATA), str->pin);
		} else if(val == FALSE){
			CLEAR_BIT((arr_bases[str->port]+DATA), str->pin);
		} else{
			return Unvalid_Parameter;
		}
	}
	return NoErrors;
}

/******************************************************************************************************/

enumGPIOErrors Read_Port(uint8 port, uint8* val){
	if(Initialized_Ports[port] != 1){
		return Uninitialized_Port;
	} else if(port > MAX_PORT_NUMBERS){
		return Unvalid_Port;
		
	} else{
		*val = READ_REGISTER(arr_bases[port]+DATA);
		return NoErrors;
	}
}

enumGPIOErrors Write_Port(uint8 port, uint8 val){
	if(Initialized_Ports[port] != 1){
		return Uninitialized_Port;
	} else if(port > MAX_PORT_NUMBERS){
		return Unvalid_Port;
	}
	else{
		WRITE_REGISTER((arr_bases[port]+DATA), val);
		return NoErrors;
	}
}
