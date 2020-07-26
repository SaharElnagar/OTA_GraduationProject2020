/*
 * Flash_if.c
 *
 *  Created on: ??þ/??þ/????
 *      Author: hager mohamed
 */

#include "Flash_if.h"
#include "common.h"
#include "stm32f4xx.h"




void FLASH_init(void) {
//unlock flash
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
//config parallelism
	FLASH->CR &= ~FLASH_CR_PSIZE;
	FLASH->CR |= FLASH_CR_PSIZE_1;   //program x32
//lock flash
	FLASH->CR |= FLASH_CR_LOCK;
}

enum_error_type EraseSectors(uint8_t first_sector, uint8_t sectors_num) {
	enum_error_type ret_val = E_OK;
	Flash_unlock();
	for (uint8_t i = 0; i < sectors_num; i++) {
		while (FLASH->SR & FLASH_SR_BSY)
			;
		FLASH->CR |= FLASH_CR_SER;
		FLASH->CR &= ~FLASH_CR_SNB;
		FLASH->CR |= first_sector << 3;
		FLASH->CR |= FLASH_CR_STRT;
		while (FLASH->SR & FLASH_SR_BSY)
			;
		if (!(FLASH->SR & FLASH_SR_WRPERR)) {
			/*Sector erased successfully*/
		} else {
			/*Sector erase faild*/
			ret_val = E_NOT_OK;
			FLASH->SR |= FLASH_SR_WRPERR;    //clear the flage
			break;
		}
		first_sector++;
	}
	Flash_lock();
	return ret_val;
}

enum_error_type Flash_write(uint32_t Address, uint32_t* pdata,
		uint16_t Size) {
	enum_error_type ret_val = E_OK;
	Flash_unlock();
	while (FLASH->SR & FLASH_SR_BSY)
		;
	FLASH->CR |= FLASH_CR_PG;
	for (int i = 0; i < Size; i++) {
		*(unsigned int *) Address = *(&pdata[i]);
		Address += 4;
	}
	while (FLASH->SR & FLASH_SR_BSY)
		;
	if (!(FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR))) {
		/* write data success*/
	} else {
		/* write data faild*/
		FLASH->SR |= FLASH_SR_WRPERR | FLASH_SR_PGAERR; //clear the flages
		ret_val = E_NOT_OK;
	}
	Flash_lock();
	return ret_val;
}

void Flash_lock(void) {
	FLASH->CR |= FLASH_CR_LOCK;
}

void Flash_unlock(void) {
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
}

