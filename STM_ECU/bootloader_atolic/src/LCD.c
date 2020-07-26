/*
 * LCD.c
 *
 *  Created on: ??þ/??þ/????
 *      Author: hager mohamed
 */
#include "stm32f4xx.h"
#include "common.h"
#include "USART.h"
#include "LCD.h"

/* Private functions */

/* functions */
void LCD_init(void) {
	USART1_init();

	//Baud Rate and system parameter configuration
	//Baudrate Set:0x07 = 115200bps,Touch panel functions enable,Touch Key_ID will be response to host
	LCD_set_sys_config(0x07, 0x83);
}

void LCD_set_sys_config(uint8_t Baudrate, uint8_t sys_par1) {
	uint8_t set_sys_config_cmd[] = { 0xAA, 0xe0, 0x55, 0xaa, 0x5a, 0xa5, 0x07,
			0x00, 0x00, 0xCC, 0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(set_sys_config_cmd, 13);
}

void LCD_hand_shake(uint8_t * ptr_response) {           //tested
	uint8_t hand_shake_cmd[] = { 0xAA, 0x30, 0xCC, 0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(hand_shake_cmd, 6);
	USART1_receive_blocking(ptr_response, 23);
}

void LCD_read_version(uint8_t * ptr_response) {        //tested
	uint8_t read_version_cmd[] = { 0xAA, 0x31, 0xCC, 0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(read_version_cmd, 6);
	USART1_receive_blocking(ptr_response, 11);
}

void LCD_read_pg_id(uint8_t * ptr_response) {        //tested
	uint8_t read_pg_id_cmd[] = { 0xAA, 0x32, 0xCC, 0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(read_pg_id_cmd, 6);
	USART1_receive_blocking(ptr_response, 8);
}

void LCD_backlight_ctrl(uint8_t PWM_LE) {       //tested
	uint8_t backlight_ctrl_cmd[] =
			{ 0xAA, 0x5F, PWM_LE, 0xCC, 0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(backlight_ctrl_cmd, 7);
}

void LCD_buzzer_touch_sound(uint8_t Time) {       //tested
	uint8_t buzzer_touch_sound_cmd[] = { 0xAA, 0x79, Time, 0xCC, 0x33, 0xC3,
			0x3C };
	USART1_transmit_blocking(buzzer_touch_sound_cmd, 7);
}

void LCD_buzzer_ctrl(uint8_t Loop_count, uint8_t T1, uint8_t T2, uint8_t Freq1,
		uint8_t Freq2) {                      //tested
	uint8_t buzzer_ctrl_cmd[] = { 0xAA, 0x7A, Loop_count, T1, T2, Freq1, Freq2,
			0xCC, 0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(buzzer_ctrl_cmd, 11);
}

void LCD_disp_page(uint16_t Page_ID) {           //tested
	uint8_t Page_IDh = (Page_ID >> 8);
	uint8_t Page_IDl = (uint8_t) Page_ID;
	uint8_t disp_page_cmd[] = { 0xAA, 0x70, Page_IDh, Page_IDl, 0xCC, 0x33,
			0xC3, 0x3C };
	USART1_transmit_blocking(disp_page_cmd, 8);
}

void LCD_suspend_vp_refresh(uint8_t Mode) {            //tested
	uint8_t suspend_vp_refresh_cmd[] = { 0xAA, 0xE8, 0x55, 0xAA, 0x5A, 0xA5,
			Mode, 0xCC, 0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(suspend_vp_refresh_cmd, 11);
}

void LCD_STR_write(uint32_t VP_STR_Address, uint8_t* ptr_data) {      //tested
	uint8_t Addr3 = (VP_STR_Address >> 24);
	uint8_t Addr2 = (VP_STR_Address >> 16);
	uint8_t Addr1 = (VP_STR_Address >> 8);
	uint8_t Addr0 = (uint8_t) VP_STR_Address;
	uint8_t STR_write_cmd[] = { 0xAA, 0x42, Addr3, Addr2, Addr1, Addr0 };
	uint8_t tail_cmd[] = { 0xCC, 0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(STR_write_cmd, 6);
	for (uint8_t i = 0; i <= 128; i++) {
		USART1_transmit_blocking(&ptr_data[i], 1);
		if (ptr_data[i] == 0x00)
			break;
	}
	USART1_transmit_blocking(tail_cmd, 4);
}

void LCD_STR_read(uint32_t VP_STR_Address, uint8_t * ptr_data) {       //tested
	uint8_t Addr3 = (VP_STR_Address >> 24);
	uint8_t Addr2 = (VP_STR_Address >> 16);
	uint8_t Addr1 = (VP_STR_Address >> 8);
	uint8_t Addr0 = (uint8_t) VP_STR_Address;
	uint8_t STR_read_cmd[] = { 0xAA, 0x43, Addr3, Addr2, Addr1, Addr0, 0xCC,
			0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(STR_read_cmd, 11);
	USART1_receive_blocking(ptr_data, 2); //read Communication packet header and Command executed
	uint8_t i;
	for (i = 2; i <= 128; i++) {
		USART1_receive_blocking(&ptr_data[i], 1);
		if (ptr_data[i] == 0x00)
			break;
	}
	i = i + 1;
	USART1_receive_blocking(&ptr_data[i], 4);   //read Communication packet tail
}

void LCD_N16_write(uint32_t VP_N16_Address, int16_t data) {        //tested
	uint8_t Addr3 = (VP_N16_Address >> 24);
	uint8_t Addr2 = (VP_N16_Address >> 16);
	uint8_t Addr1 = (VP_N16_Address >> 8);
	uint8_t Addr0 = (uint8_t) VP_N16_Address;
	uint8_t High_Byte = (data >> 8);
	uint8_t Low_Byte = (uint8_t) data;
	uint8_t N16_write_cmd[] = { 0xAA, 0x3D, Addr3, Addr2, Addr1, Addr0,
			High_Byte, Low_Byte, 0xCC, 0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(N16_write_cmd, 12);
}

uint16_t LCD_N16_read(uint32_t VP_N16_Address) {               //tested
	uint8_t Response[8];
	uint16_t data;
	uint8_t Addr3 = (VP_N16_Address >> 24);
	uint8_t Addr2 = (VP_N16_Address >> 16);
	uint8_t Addr1 = (VP_N16_Address >> 8);
	uint8_t Addr0 = (uint8_t) VP_N16_Address;
	uint8_t N16_read_cmd[] = { 0xAA, 0x3E, Addr3, Addr2, Addr1, Addr0, 0xCC,
			0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(N16_read_cmd, 10);
	USART1_receive_blocking(Response, 8);
	data = (Response[2] << 8) + Response[3];
	return data;
}

void LCD_N32_write(uint32_t VP_N32_Address, int32_t data) {       //tested
	uint8_t Addr3 = (VP_N32_Address >> 24);
	uint8_t Addr2 = (VP_N32_Address >> 16);
	uint8_t Addr1 = (VP_N32_Address >> 8);
	uint8_t Addr0 = (uint8_t) VP_N32_Address;
	uint8_t Data3 = (data >> 24);
	uint8_t Data2 = (data >> 16);
	uint8_t Data1 = (data >> 8);
	uint8_t Data0 = (uint8_t) data;
	uint8_t N16_write_cmd[] = { 0xAA, 0x44, Addr3, Addr2, Addr1, Addr0, Data3,
			Data2, Data1, Data0, 0xCC, 0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(N16_write_cmd, 14);
}

uint32_t LCD_N32_read(uint32_t VP_N32_Address) {        //tested
	uint32_t data;
	uint8_t Response[10];
	uint8_t Addr3 = (VP_N32_Address >> 24);
	uint8_t Addr2 = (VP_N32_Address >> 16);
	uint8_t Addr1 = (VP_N32_Address >> 8);
	uint8_t Addr0 = (uint8_t) VP_N32_Address;
	uint8_t N32_read_cmd[] = { 0xAA, 0x45, Addr3, Addr2, Addr1, Addr0, 0xCC,
			0x33, 0xC3, 0x3C };
	USART1_transmit_blocking(N32_read_cmd, 10);
	USART1_receive_blocking(Response, 10);
	data = (Response[2] << 24) + (Response[3] << 16) + (Response[4] << 8)
			+ Response[5];
	return data;
}

void LCD_wait_TouchKey_press(uint16_t Page_ID, uint8_t Key_ID) {
	uint8_t Response[9];
	uint8_t cmd;
	uint16_t rx_Page_ID;
	uint8_t rx_Key_ID;
	while (1) {
		USART1_receive_blocking(Response, 10);
		cmd = Response[2];
		rx_Page_ID = (Response[3] << 8) + Response[4];
		rx_Key_ID = Response[5];
		if ((cmd == 0x78 || cmd == 0x79) && (rx_Page_ID == Page_ID)
				&& (rx_Key_ID == Key_ID))
			break;

	}
}
