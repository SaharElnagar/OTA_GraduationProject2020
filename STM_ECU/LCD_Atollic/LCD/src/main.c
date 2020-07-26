/*
 ******************************************************************************
 File:     main.c
 Info:
 ******************************************************************************
 */

/* Includes */
#include "stm32f4xx.h"
#include "common.h"
#include "main.h"
#include "LCD.h"
#include "CAN.h"
#include "Flash_if.h"

/* Private macro */
/* Private variables */
uint8_t response_code[23];
uint8_t Software_version[] = "00\0";
uint8_t can_rx_data[8];
uint32_t zeros = 0;

uint16_t Status = 0x5555;

/* Private function prototypes */
void button_init(void);
/* Private functions */
void button_init(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	while (!(RCC->AHB1ENR & RCC_AHB1ENR_GPIOAEN))
		;
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
}

/**
 **===========================================================================
 **
 **  Abstract: main program
 **
 **===========================================================================
 */
int main(void) {

	LCD_init();
	CAN_init();
	FLASH_init();
	LCD_disp_page(0);
	LCD_STR_write(LCD_version_add, Software_version);
	/* Infinite loop */
	while (1) {
		switch (Status) {
		case IDEL:
			LCD_disp_page(0);
			//wait to receive "Receive_new_update" from gateway
			while (1) {
				CAN_receive_blocking((uint8_t*) &Status, 2);
				if (Status == Receive_new_update)
					break;
			}
			break;
		case Receive_new_update:
			LCD_disp_page(1);
			LCD_wait_TouchKey_press(1, 0); //wait for "receive new update" key to be pressed
			Status = Download_new_update;
			break;
		case Download_new_update:
			LCD_disp_page(0);
			// send to gateway to download the new update
			CAN_transmit_blocking((uint8_t*) &Status, 2);
			//wait to receive "New_update_downloaded" from gateway
			while (1) {
				CAN_receive_blocking((uint8_t*) &Status, 2);
				if (Status == New_update_downloaded)
					break;
			}
			break;
		case New_update_downloaded:
			LCD_disp_page(2);
			LCD_wait_TouchKey_press(2, 0); //wait for "update software" key to be pressed
			Status = Start_update_software;
			break;
		case Start_update_software:
			LCD_disp_page(3);
			// send to gateway to start update the software
			CAN_transmit_blocking((uint8_t*) &Status, 2);
			while (1) {
				//wait to receive "Update percentage" from gateway
				CAN_receive_blocking((uint8_t*) &Status, 2);
				if (Status == Start_bootloader)
					break;
				LCD_N16_write(percentage_add, Status);
			}
			break;
		case Start_bootloader:
			//	Flash_write(0x08004010,&zeros,1);
			NVIC_SystemReset();            //software reset
			break;
		default:
			/* ERROR */
			while (1)
				;
			break;
		}

	}
}
