/*
 * Fee_Sector.h
 *
 *  Created on: Apr 29, 2020
 *      Author: Sahar Elnagar
 */

#ifndef FEE_SECTOR_H_
#define FEE_SECTOR_H_

#include "Std_Types.h"
#include "Fee_Cfg.h"
#include "Fls.h"
#include "Fee_PrivateTypes.h"


//*****************************************************************************
//  Virtual Sector Header States
//*****************************************************************************



/*Empty Virtual Sector:  This indicates the Virtual Sector has been erased and can be used to store data*/
#define EMPTY_VIRTUAL_SECTOR            0xFFFFFFFFFFFFFFFF

/*This indicates that the Data Block Structure is being moved from a full Virtual Sector to
 *this one to allow for moving of the Active Virtual Sector*/
#define COPY_VIRTUAL_SECTOR             0x00000000FFFFFFFF

/*Active Virtual Sector:  This Virtual Sector is the active one*/
#define ACTIVE_VIRTUAL_SECTOR           0x000000000000FFFF

/*Ready for Erase:   This Virtual Sector’s Data Block Structure has been correctly replicated
 *to a new Virtual Sector and is now ready to be erased and initialized for re-use. */
#define READY_FOR_ERASE                 0x0000000000000000

//*****************************************************************************
//  Data Block States
//*****************************************************************************

/*New Data can be written to this Block. */
#define EMPTY_BLOCK                     0xFFFFFFFFFFFFFFFF

/*This indicates that the Data Block is in the progress of being programmed with data. */
#define START_PROGRAM_BLOCK             0xFFFFFFFFFFFF0000

/*This indicates that the Data Block is fully programmed and contains Valid Data.*/
#define VALID_BLOCK                     0xFFFFFFFF00000000

/*This indicates that the Data can not be written or read from this block*/
#define INVALID_BLOCK                   0xFFFF000000000000
//*****************************************************************************
//  Flash base address
//*****************************************************************************
#define FLASH_BASE_ADDRESS              0x030000
//*****************************************************************************
//  Flash Size
//*****************************************************************************
#define FLASH_SIZE                      0x2000          /*used space for blocks allocation in flash = 64KB*/
#define ERASED_HALF_WORD                0xFFFF
#define ERASED_WORD                     0xFFFFFFFF
//*****************************************************************************
//  Data block header size in bytes
//*****************************************************************************
#define DATA_BLOCK_HEADER_SIZE          (20U)
#define ALL_BLOCKS_HEADER_SIZE          (DATA_BLOCK_HEADER_SIZE * BLOCKS_NUM)

//*****************************************************************************
//  Sector header size in bytes
//*****************************************************************************
#define SECTOR_HEADER_SIZE              (12U)

//*****************************************************************************
//  Sector and Block header Status length in bytes
//*****************************************************************************
#define STATUS_LENGTH                   (8U)

//*****************************************************************************
//  Physical sector size
//*****************************************************************************
#define PHYSICAL_SECTOR_SIZE            FlsSectorSize

//*****************************************************************************
//  Calculate number of physical sectors in one Virtual sector
//*****************************************************************************
#define NUM_OF_PHYSICAL_SECTORS          (((SECTOR_HEADER_SIZE + ALL_BLOCKS_HEADER_SIZE + BLOCKS_SIZE)\
                                                                /PHYSICAL_SECTOR_SIZE) +(1U))


//*****************************************************************************
//  Virtual sector size
//*****************************************************************************
#define VIRTUAL_SECTOR_SIZE              (NUM_OF_PHYSICAL_SECTORS * PHYSICAL_SECTOR_SIZE)
//*****************************************************************************

//*****************************************************************************
//  Calculate Available space in sector in bytes
//*****************************************************************************
#define GET_AVAILABLE_SPACE(SectorAddress,SpaceStartAddress)        (SectorAddress + VIRTUAL_SECTOR_SIZE - SpaceStartAddress )

//*****************************************************************************
//  Number of Virtual sector
//*****************************************************************************
#define VIRTUAL_SECTOR_NUMBER                       (FLASH_SIZE / VIRTUAL_SECTOR_SIZE)

#define ERASE_PROGRAM_CYCLES                        FLS_SPECIFIED_ERASE_CYCLES

#define SECTOR_MAX_ERASE_CYCLES                     (ERASE_PROGRAM_CYCLES*NUM_OF_PHYSICAL_SECTORS*VIRTUAL_SECTOR_NUMBER)
//*****************************************************************************
//  Set the new next valid block address
//  Next block address in header is byte number 12
//*****************************************************************************
#define SET_NEW_NEXT_BLOCK                         *((uint32*)(BlockData+12))


/*****************************************************************************************/
/*                                   Local types Definition                              */
/*****************************************************************************************/
/*Sector status type*/
typedef uint64 SectorStatusType ;

/*Block status type*/
typedef uint64 BlockStatusType ;

typedef struct
{
    SectorStatusType VirtualSectorStatus ;
    uint32 EraseCount ;
}str_SectorHeaderType ;

typedef struct
{
    BlockStatusType BlockStatus ;
    Fls_AddressType WriteCycleCount ;
    Fls_AddressType NextBlockAddress ;
    uint16 BlockNumber ;
    uint16 BlockSize ;
}str_DataBlockHeaderType ;

typedef struct
{
    Fls_AddressType PhsicalAddressStart ;
    uint32 WriteCycles ;
    uint16 LogicalAddress ;
    uint16 BlockSize ;
    uint8  Valid ;
}str_ValidBlocksInfoType;

typedef struct
{
    Fls_AddressType StartAddress ;
    Fls_AddressType InternalNextAvailableAddress ;
    Fls_AddressType PreviousBlockAddress;
    Fls_AddressType LastValidBlock ;
    str_ValidBlocksInfoType* SectorValidBlocksInfo;
    uint16 ValidBlocksNumber ;
}str_ActiveSectorInfoType;


//*****************************************************************************
//  Strut to save the required data for the reading process
//  BlockID : Block logical number
//  Length : the number of bytes to read must be divisible by 4
//  DataPtr: pointer  that points to the place where data will be saved
//*****************************************************************************
typedef struct
{
    uint16 BlockNum ;
    uint16 Len;
    uint8* DataBufPtr;
}str_JobParametersCopy;


Std_ReturnType GetSectorStatus(uint32 Sector_Address, SectorStatusType* SectorStatus);
Std_ReturnType SetSectorStatus(uint32 Sector_Address, SectorStatusType* SectorStatus) ;
Std_ReturnType Erase_Virtualsector(uint32 Sector_Address) ;
Std_ReturnType ReadVariable(Fls_AddressType SourceAddress, uint8* TargetAddressPtr, Fls_LengthType Length) ;
Std_ReturnType FindActiveSector(void) ;
Std_ReturnType TransferSector(uint32 Source , uint32 Traget ) ;
Std_ReturnType WriteVariable(Fls_AddressType TargetAddressPtr, uint8* SourceAddress, Fls_LengthType Length) ;
Std_ReturnType GetIndexFromBlockNum(uint16 BlockNumber, uint16* Index) ;
Std_ReturnType FindValidBlocks(uint16 BlockNumber) ;
Std_ReturnType SearchSectorValidBlocks(uint32 SectorAddress) ;
void SetBlocksInvalid(uint16 ValidBlockNumber);

#endif /* FEE_SECTOR_H_ */
