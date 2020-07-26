/*
 * main.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: hager mohamed
 */

#ifndef MAIN_H_
#define MAIN_H_
#include "stm32f4xx.h"
#include "common.h"

#define LCD_version_add           0x000180
#define percentage_add            0x080000

#define IDEL                  0x0000
#define Receive_new_update    0x1111
#define Download_new_update   0x2222
#define New_update_downloaded 0x3333
#define Start_update_software 0x4444
#define Start_bootloader      0x5555

#endif /* MAIN_H_ */
