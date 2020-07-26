/*
 * LCD.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: hager mohamed
 */

#ifndef LCD_H_
#define LCD_H_

#include "common.h"

void LCD_init(void);
/*
 * Name :LCD_set_sys_config
 * Description : Baud Rate and system parameter configuration
 * Input : Baudrate:
 * 					0x00 = 1200bps
 *                  0x01 = 2400bps
 *                  0x02 = 4800bps
 *                  0x03 = 9600bps
 *                  0x04 = 19200bps
 *                  0x05 = 38400bps
 *                  0x06 = 57600bps
 *                  0x07 = 115200bps
 *		   sys_par1:
 *		   			Bit7 = 0: Touch panel function disable
 *                  Bit7 = 1: Touch panel functions enable
 *                  Bit[1..0]: Touch actions configuration (see datasheet)
 */
void LCD_set_sys_config(uint8_t Baudrate, uint8_t sys_par1);
/*
 * Name : LCD_hand_shake
 * Description : Read a Hand Shake
 * Output :
 *          ptr_response: pointer to the response code
 */
void LCD_hand_shake(uint8_t * ptr_response);

/*
 * Name : LCD_read_version
 * Description : Read firmware version
 * Output :
 *          ptr_response: pointer to the response code
 */
void LCD_read_version(uint8_t * ptr_response);

/*
 * Name : LCD_backlight_ctrl
 * Description : backlight brightness control
 * Input :
 * 			PWM_LE=0x00 ~ 0x3F   to control the backlight brightness
 */
void LCD_backlight_ctrl(uint8_t PWM_LE);

/*
 * Name : LCD_read_pg_id
 * Description : Read Current page ID
 * Output :
 *          ptr_response: pointer to the response code
 */
void LCD_read_pg_id(uint8_t * ptr_response);

/*
 * Name :LCD_buzzer_touch_sound
 * Description : buzzer touch sound control
 * Input :
 * 			Time : Sounding time length (in 10ms), range 0x00~0x3F
 *                 0x00= disable
 */
void LCD_buzzer_touch_sound(uint8_t Time);

/*
 * Name : LCD_ buzzer_ctrl
 * Description : Buzzer control
 * Input :
 * 			Loop count : Loop count, Range: 0x01 ~ 0xFF.
 *					   	 0xFF = buzzer infinite loop.
 *			T1 : Buzzer play time 1, Range: 0x00 ~ 0xFF (0~25.5s)(unit 100ms)
 *          T2 : Buzzer play time 2, Range: 0x00 ~ 0xFF (0~25.5s)(unit 100ms)
 *          Freq1 : T1 time Buzzer frequency, Unit 100 Hz, Ranges: 0x05 ~ 0x32 (500Hz ~ 5KHz)
 *                  0x00 = T1 time period buzzer turn off
 *          Freq2 : T2 time Buzzer frequency, Unit 100 Hz, Ranges: 0x05 ~ 0x32 (500Hz ~ 5KHz),
 *                  0x00 = T1 time period buzzer turn off
 */
void LCD_buzzer_ctrl(uint8_t Loop_count, uint8_t T1, uint8_t T2, uint8_t Freq1,
		uint8_t Freq2);

/*
 * Name : LCD_disp_page
 * Description : Display a pre-stored page
 * Input :
 * 			Page_ID = 0~999
 */
void LCD_disp_page(uint16_t Page_ID);

/*
 * Name : LCD_suspend_vp_refresh
 * Description : Set the screen to pause the refresh and deactivate the touchkey or
 *               release the pause to refresh and enable the touchkey
 * Input :
 * 			Mode :
 * 			       0x00: release the pause to refresh and enable the touchkey
 *                 0x01: pause the refresh and deactivate the touchkey
 */
void LCD_suspend_vp_refresh(uint8_t Mode);

/*
 * Name : LCD_STR_write
 * Description : Write string to VP_STR
 * Input :
 * 			VP_STR_Address = 0x00000 ~ 0x01FFFF ,(each VP_STR = 128 bytes)
 *                           (address value must be divisible by 128)
 *          ptr_data : pointer to String to write, Total no. of byte in string <=128
 *                 ‘\0’(0x00): string end mark (put in the end of the string)
 */
void LCD_STR_write(uint32_t VP_STR_Address, uint8_t* ptr_data);

/*
 * Name : LCD_ STR_read
 * Description : Read string from VP_STR
 * Input :
 * 			VP_STR_Address = 0x00000 ~ 0x01FFFF, (each VP_STR = 128 bytes)
 *                           (address value must be divisible by 128)
 * Output :
 *          ptr_data : pointer to string to read
 */
void LCD_STR_read(uint32_t VP_STR_Address, uint8_t * ptr_data);

/*
 * Name : LCD_N16_write
 * Description : Write 16bit number to VP_N16
 * Input :
 * 			VP_N16_Address = 0x080000 ~ 0x08FFFF,(each VP_N16 = 2 byte)
 *                           (address value must be divisible by 2)
 *          data : The 16 bit value to write
 */
void LCD_N16_write(uint32_t VP_N16_Address, int16_t data);

/*
 * Name : LCD_N16_read
 * Description : Read 16bit number from VP_N16
 * Input :
 * 			VP_N16_Address = 0x080000 ~ 0x08FFFF,(each VP_N16 = 2 byte)
 *                           (address value must be divisible by 2)
 * Output :
 *          16 bit number of data
 */
uint16_t LCD_N16_read(uint32_t VP_N16_Address);

/*
 * Name : LCD_N32_write
 * Description : Write 32bit number to VP_N32
 * Input :
 * 			VP_N32_Address =  0x020000 ~ 0x02FFFF,(each VP_N32 = 4 byte)
 *                           (address value must be divisible by 4)
 *          data : The 32 bit value to write
 */
void LCD_N32_write(uint32_t VP_N32_Address, int32_t data);

/*
 * Name : LCD_N32_read
 * Description : Read 32bit number from VP_N32
 * Input :
 * 			VP_N16_Address = 0x020000 ~ 0x02FFFF,(each VP_N32 = 4 byte)
 *                           (address value must be divisible by 4)
 * Output :
 *          32 bit number of data
 */
uint32_t LCD_N32_read(uint32_t VP_N32_Address);

/*
 * Warning : To use this function "sys_par1" input in "LCD_set_sys_config" function must be configured to 0x83
 * Name :LCD_wait_touch_response
 * Description : wait for a specific touch key to be pressed (in a specific page)
 * Input :   Page_ID: ID for the page containing the touch key
 *           Key_ID : Key_ID (8bit binary value)
 */
void LCD_wait_TouchKey_press(uint16_t Page_ID, uint8_t Key_ID);
#endif /* LCD_H_ */
