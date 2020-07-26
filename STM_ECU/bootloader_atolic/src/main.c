/*
 ******************************************************************************
 *File:     main.c
 *Info:
 *     bootlaoder design : divide the flash into 3 sections.
 *						  - first section for bootloader code  -> in sector 0 (16 KBytes)
 *						  - second section for the application -> start from sector 1 (0x0800 4000)to the beginning of
 *                          third section(backup) can be to the end of the flash (sector 11) if there is
 *                          no backup.
 *                          the first 4 words of this section save the data used is bootloader process(backup_start_address,backup_size,App_size,App_end_address).
 *                          the 5th word is used as a flage to enter the boot mode or start the application (0 -> start the application, else -> enter the boot mode   )
 *                          the Aplication starts from address 0x0800 4200 as Vector Table base offset field must be a multiple of 0x200.
 *                        - third section for backup the previous application -> start from sector 11 to
 *                          the end of second section but can't take more than the half of the flash (sector 8)
 *     flash sectors:
 *                   - sector 1-3  :  16 KBytes
 *                   - sector 4    :  64 KBytes
 *                   - sector 5-11 : 128 KBytes
 ******************************************************************************
 */

/* Includes */
#include "stm32f4xx.h"
#include "main.h"
#include "common.h"

/* Private macro */
#define SECTOR_0_ADD    0x08000000
#define SECTOR_1_ADD    0x08004000
#define SECTOR_2_ADD 	0x08008000
#define SECTOR_3_ADD	0x0800C000
#define SECTOR_4_ADD	0x08010000
#define SECTOR_5_ADD	0x08020000
#define SECTOR_6_ADD	0x08040000
#define SECTOR_7_ADD	0x08060000
#define SECTOR_8_ADD	0x08080000
#define SECTOR_9_ADD	0x080A0000
#define SECTOR_10_ADD	0x080C0000
#define SECTOR_11_ADD	0x080E0000
#define FLASH_END_ADD   0x08100000
#define APP_START_ADD 	0x08004200

/* Private variables */
static enum_bootloader_error_t BL_error = BL_OK;
static uint8_t BL_buffer[PACKET_SIZE];
static uint16_t Cmd_Status = IDEL;
static str_FrameInfo FrameInfo;
static uint32_t code_size;
static uint8_t sectors_num = 0;     //number of sectors to erase
static uint32_t BL_pointer = SECTOR_1_ADD; //sector 1 start address (the first sector after bootloade)
static uint32_t backup_start_address; //read from add 0x08004000  (sector 1 add)
static uint32_t backup_size;              //read from add 0x08004004
static uint32_t App_size;                 //read from add 0x08004008
static uint32_t App_end_address;          //read from add 0x0800400C
static uint32_t boot_flage;               //read from add 0x08004010

#ifdef DISPLAY
float MAX_NUM_PACKETS;                 //for display
float current_num;
float remain_pres;
float current_pres;
#endif

/* Private function prototypes */
void StartApplication(void);
uint32_t Erase_Backup(uint32_t app_size);
enum_bootloader_error_t CopyAppToBackup(void);
enum_bootloader_error_t Flash_copy(uint32_t source_add, uint32_t dest_add,
		uint32_t size);
enum_bootloader_error_t Erase_App(void);

/**
 **===========================================================================
 **
 **  Abstract: main program
 **
 **===========================================================================
 */
int main(void) {
	FLASH_init();
	boot_flage = *(uint32_t*) (0x08004010);
	if (boot_flage == 0)
		StartApplication();
	Communication_init();
	__enable_irq();
#ifdef DISPLAY
	LCD_init();
#endif
	/* Infinite loop */
	while (1) {
		switch (Cmd_Status) {
		case IDEL:
			/* read backup address,backup size and app size from the flash(first two words in the app)
			 * Listen to the Communication */
#ifdef DISPLAY
			current_pres = LCD_N16_read(0x080000);
			remain_pres = 100 - current_pres;
#endif
			backup_start_address = *(uint32_t*) BL_pointer;
			backup_size = *(uint32_t*) (BL_pointer + 4);
			App_size = *(uint32_t*) (BL_pointer + 8);
			App_end_address = *(uint32_t*) (BL_pointer + 12);

			receive_blocking((uint8_t*) &Cmd_Status, 2);
			break;
		case REQUEST_TO_UPDATE:
			/* Update is initiated , the switch to state  SEND_FREAME_INFO */
			Cmd_Status = SEND_FREAME_INFO;
			break;
		case SEND_FREAME_INFO:
			/* Request the Frame Info from the gateway,
			 * Then go to RECEIVE_FRAME_INFO */
			transmit_blocking((uint8_t*) &Cmd_Status, 2);
			Cmd_Status = RECEIVE_FRAME_INFO;
			break;
		case RECEIVE_FRAME_INFO:
			/*Save the frame info,
			 * Then go to READY_TO_RECEIVE_UPDATE */
			receive_blocking(BL_buffer, FrameInfo_SIZE);
			FrameInfo.PacketsNum = *(uint16_t*) &BL_buffer[0];
			FrameInfo.ExtraBytes = *(uint16_t*) &BL_buffer[2];
			FrameInfo.CheckSum = *(uint32_t*) &BL_buffer[4];
#ifdef DISPLAY
			MAX_NUM_PACKETS = FrameInfo.PacketsNum;          //for display
			if (FrameInfo.ExtraBytes != 0)
			MAX_NUM_PACKETS++;
			current_num = 0;
#endif
			code_size = FrameInfo.PacketsNum * PACKET_SIZE
					+ FrameInfo.ExtraBytes;
			Cmd_Status = READY_TO_RECEIVE_UPDATE;
			break;
		case READY_TO_RECEIVE_UPDATE:
			/* copy current app (if exist) to backup
			 * Erase flash space for the new update
			 * Send to the Gateway that node is ready to receive update */
			if (App_size != 0xFFFFFFFF) {     //the is an application
				backup_start_address = Erase_Backup(App_size);
				if (backup_start_address != 0xFFFFFFFF) { //erase backup successfully
					Flash_copy(APP_START_ADD, backup_start_address, App_size);
				} else {
					/* NO space for backup*/
				}
			}
			BL_error = Erase_App();
			backup_size = App_size;
			App_size = code_size;
			//save backup start address,backup size and App size at the begining of the app area
			BL_error |= Flash_write(BL_pointer, &backup_start_address, 1);
			BL_pointer += 4;
			BL_error |= Flash_write(BL_pointer, &backup_size, 1);
			BL_pointer += 4;
			BL_error |= Flash_write(BL_pointer, &App_size, 1);
			BL_pointer += 4;
			BL_error |= Flash_write(BL_pointer, &App_end_address, 1);
			//write '0' to boot_flage
			boot_flage = 0;
			BL_pointer += 4;
			BL_error |= Flash_write(BL_pointer, &boot_flage, 1);
			BL_pointer = APP_START_ADD;
			if (BL_error != BL_OK) {
				Cmd_Status = ERROR;
				break;
			}

			transmit_blocking((uint8_t*) &Cmd_Status, 2);
			Cmd_Status = RECEIVE_PACKET;
			break;
		case RECEIVE_PACKET:
			/* Receive a packet from the gate way
			 */
			if (FrameInfo.PacketsNum > 0) {
				receive_blocking(BL_buffer, PACKET_SIZE);
				FrameInfo.PacketsNum -= 1;
				BL_error = Flash_write(BL_pointer, (uint32_t*) BL_buffer,
				PACKET_SIZE / 4);
				if (BL_error != BL_OK) {
					Cmd_Status = ERROR;
					break;
				}
				Cmd_Status = SEND_NEW_PACKET;
				BL_pointer += PACKET_SIZE;
#ifdef DISPLAY
				current_num++;
				current_pres +=(current_num / MAX_NUM_PACKETS * remain_pres);
				LCD_N16_write(0x080000,current_pres );
#endif
			} else {
				receive_blocking(BL_buffer, FrameInfo.ExtraBytes);
				BL_error = Flash_write(BL_pointer, (uint32_t*) BL_buffer,
						FrameInfo.ExtraBytes / 4);
				if (BL_error != BL_OK) {
					Cmd_Status = ERROR;
					break;
				}
				FrameInfo.ExtraBytes = 0;
				Cmd_Status = END_OF_FRAME;
#ifdef DISPLAY
				LCD_N16_write(0x080000, 100);
#endif
			}
			break;
		case SEND_NEW_PACKET:
			/* Request to send new packet */
			transmit_blocking((uint8_t*) &Cmd_Status, 2);
			Cmd_Status = RECEIVE_PACKET;
			break;
		case END_OF_FRAME:
			/* later request to calculate checksum and report update ok */
			Cmd_Status = UPDATE_SUCCESS;
			break;
		case UPDATE_SUCCESS:
			transmit_blocking((uint8_t*) &Cmd_Status, 2);
			Cmd_Status = JUMP_TO_NEW_APP;
			break;
		case JUMP_TO_NEW_APP:
			/* Call the new app */
			StartApplication();
			BL_error = BL_INVALID_APP_ADD;
			Cmd_Status = ERROR;
			break;
		case RESTORE_BACKUP:
			/* Erase space for the App
			 * Copy the App from backup to App section
			 * Backup section now is empty*/
			if (backup_size == 0xFFFFFFFF) {
				BL_error = BL_NO_BACKUP;
				Cmd_Status = ERROR;
				break;
			}
			code_size = backup_size;
			BL_error = Erase_App();
			BL_error |= Flash_copy(backup_start_address, APP_START_ADD,
					code_size);
			backup_start_address = 0xFFFFFFFF;
			backup_size = 0xFFFFFFFF;
			App_size = code_size;
			//save backup start address,backup size and APP size at the begining of the app area
			BL_error |= Flash_write(BL_pointer, &backup_start_address, 1);
			BL_pointer += 4;
			BL_error |= Flash_write(BL_pointer, &backup_size, 1);
			BL_pointer += 4;
			BL_error |= Flash_write(BL_pointer, &App_size, 1);
			BL_pointer += 4;
			BL_error |= Flash_write(BL_pointer, &App_end_address, 1);
			//write '0' to boot_flage
			boot_flage = 0;
			BL_pointer += 4;
			BL_error |= Flash_write(BL_pointer, &boot_flage, 1);
			if (BL_error != BL_OK) {
				Cmd_Status = ERROR;
				break;
			}
			Cmd_Status = RESTORE_BACKUP_SUCCESS;
			break;
		case RESTORE_BACKUP_SUCCESS:
			transmit_blocking((uint8_t*) &Cmd_Status, 2);
			Cmd_Status = JUMP_TO_NEW_APP;
			break;
		case ERROR:
			/* for handling the error*/
			transmit_blocking((uint8_t*) &Cmd_Status, 2);
			transmit_blocking((uint8_t*) &BL_error, 2);
			while (1)
				;
			break;
		}
	}
}

/* Private functions */

void StartApplication(void) {
	uint32_t* address = (uint32_t*) APP_START_ADD;
	uint32_t stack_Pionter = *address;
	if ((stack_Pionter > 0x20000000) && (stack_Pionter <= 0x20020000)) { //valid stack pointer(in RAM)
		__disable_irq();                            //disable interrupt
		//reset the modules used by the bootloader
		Communication_reset();

#ifdef DISPLAY
		RCC->AHB1RSTR = RCC_AHB1RSTR_GPIOBRST;
		RCC->AHB1RSTR = 0;
		RCC->APB2RSTR = RCC_APB2RSTR_USART1RST;
		RCC->APB2RSTR = 0;
#endif

		//vector table relocation
		__DMB();
		SCB->VTOR = (uint32_t) address;
		__DSB();
		//initiate the stack pointer
		__set_MSP(stack_Pionter);
		//jump to the reset handler address
		void (*jump_address)(void)=(void(*)(void))(*(address+1)); //address of reset handler of the app
		jump_address();
	} else {
		/* Invalid address to jump */
	}
}

/*  Erase_Backup
 * description : function used to erase space for the current app (if its size < 4*128 Kbytes) to backup
 * input :
 * 			app_size : the size of the app you want to backup
 * return value : the start address of the backup
 *                0xFFFFFFFF if no space for backup
 * */
uint32_t Erase_Backup(uint32_t app_size) {
	uint32_t backup_start_add = 0xFFFFFFFF;
	if (app_size > (FLASH_END_ADD - App_end_address)) {
		//no space for backup
	} else if (app_size <= 128 * 1024) {
		EraseSectors(11, 1);             //erase sector 11
		backup_start_add = SECTOR_11_ADD;  //start address of sector 11
	} else if (app_size <= 2 * 128 * 1024) {
		EraseSectors(10, 2);             //erase sector 10,11
		backup_start_add = SECTOR_10_ADD;  //start address of sector 10
	} else if (app_size <= 3 * 128 * 1024) {
		EraseSectors(9, 3);             //erase sector 9,10,11
		backup_start_add = SECTOR_9_ADD;  //start address of sector 9
	} else if (app_size <= 4 * 128 * 1024) {
		EraseSectors(8, 3);             //erase sector 8,9,10,11
		backup_start_add = SECTOR_8_ADD;  //start address of sector 8
	} else {
		//no space for backup
	}
	return backup_start_add;
}

/*  Flash_copy
 * description : function used to copy section from the flash to another section in the flash
 * input :
 * 		  source_add : the address of the section to copy.
 * 		  dest_add   : the address of the section to copy to.
 * 		  size       : size of the section to copy.
 * return value : the code of the error
 *   */
enum_bootloader_error_t Flash_copy(uint32_t source_add, uint32_t dest_add,
		uint32_t size) {
	enum_bootloader_error_t ret_val = BL_OK;
	uint16_t packet_num = size / PACKET_SIZE;
	uint16_t extra_bytes = size - packet_num * PACKET_SIZE;
	while (packet_num > 0) {
		for (int i = 0; i < PACKET_SIZE; i = i + 4) { //copy packet from flash to RAM(BL_buffer)
			*(uint32_t*) &BL_buffer[i] = *(uint32_t*) source_add;
			source_add += 4;
		}
		ret_val += Flash_write(dest_add, (uint32_t*) BL_buffer,
				(PACKET_SIZE / 4));
		dest_add += PACKET_SIZE;
		packet_num -= 1;
	}
	if (extra_bytes != 0) {
		for (int i = 0; i < extra_bytes; i = i + 4) { //copy packet from flash to RAM(BL_buffer)
			*(uint32_t*) &BL_buffer[i] = *(uint32_t*) source_add;
			source_add += 4;
		}
		ret_val += Flash_write(dest_add, (uint32_t*) BL_buffer,
				(extra_bytes / 4));
		dest_add += extra_bytes / 4;
	}
	return ret_val;
}

enum_bootloader_error_t Erase_App(void) {
	enum_bootloader_error_t ret_val = BL_OK;
	/*8 bytes to store backup start add and app size &
	 * but  Vector Table base offset field must be a multiple of 0x200. */
	if (code_size > (backup_start_address - APP_START_ADD)) {
		/* no enough space*/
		ret_val = BL_NO_SPACE;
	} else if (code_size <= (48 * 1024 - 0x200)) {
		/* Erase one or more of the first 3 sectors (1,2,3) 16K for each */
		sectors_num = code_size / (16 * 1024) + 1;
		ret_val = EraseSectors(1, sectors_num);
		App_end_address = SECTOR_1_ADD + sectors_num * 16 * 1024;
	} else if (code_size <= (112 * 1024 - 0x200)) {
		/* Erase the first 3 sectors (1,2,3) 16K for each
		 * + sector 4 (64K) */
		ret_val = EraseSectors(1, 4);
		App_end_address = SECTOR_5_ADD;
	} else if (code_size <= (1008 * 1024 - 0x200)) {
		/* Erase the first 3 sectors (1,2,3) 16K for each
		 * + sector 4 (64K)
		 * + one or more of the last 7 sectors (5,6,7,8,9,10,11) 128K for each*/
		sectors_num = (code_size - 112 * 1024) / (128 * 1024) + 1;
		ret_val = EraseSectors(1, (4 + sectors_num));
		App_end_address = SECTOR_5_ADD + sectors_num * 128 * 1024;
	} else {
		/* no enough space*/
		ret_val = BL_NO_SPACE;
	}
	return ret_val;
}

