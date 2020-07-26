/*
 * Flash_if.h
 *
 *  Created on: ??þ/??þ/????
 *      Author: hager mohamed
 */

#ifndef FLASH_IF_H_
#define FLASH_IF_H_
#include "common.h"
#include "stm32f4xx.h"


void FLASH_init(void);
void Flash_lock(void);
void Flash_unlock(void);
enum_error_type EraseSectors(uint8_t first_sector, uint8_t sectors_num);
enum_error_type Flash_write(uint32_t Address, uint32_t* pdata,
		uint16_t Size);


#endif /* FLASH_IF_H_ */
