/*
 * Fee_Sector.c
 *
 *  Created on: Apr 29, 2020
 *      Author: Sahar Elnagar
 */

#include "Fee_Sector.h"
#include "Fls.h"

static Std_ReturnType GetNextActiveBlcokIndex(uint16 CurrentIndex,uint16* NextIndex);

extern str_ActiveSectorInfoType ActiveSectorInfo ;
extern uint8 BlockData[MAX_CONFIGURED_BLOCK_SIZE + DATA_BLOCK_HEADER_SIZE] ;
extern Fee_BlockConfigType Fee_BlockConfig[BLOCKS_NUM];

extern uint8 JobDone ;
extern uint8 JobError ;

/****************************************************************************************/
/*    Function Name           : FindValidBlocks                                         */
/*    Function Description    : Find Active blocks in Active sector  , Save info        */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Fee_ErrorsType                                          */
/*    Notes                   : We have 3 cases :                                       */
/*                              1- No active blocks found in the sector                 */
/*                              2- Found Active blocks , but didn't find the desired    */
/*                              block .                                                 */
/*                              3- Found the desired block                              */
/********S********************************************************************************/
Std_ReturnType FindValidBlocks(uint16 BlockNumber)
{
    /*Array to read the block header*/
    static uint8 cmd[DATA_BLOCK_HEADER_SIZE] ;
    /*Variable to read block status*/
    BlockStatusType BlockStatus = 0 ;
    uint16 BlockNum =0,index=0 , BlockSize = 0;
    static uint32 NextBlockAddress = 0 ;
    static uint8 rtn_val1 = E_PENDING ;
    uint8  rtn_val2 = E_PENDING;

    /*Check if it's the first call of the function to find active blocks
     * always start searching blocks from the beginning of the current Active sector
     */
    if(ActiveSectorInfo.LastValidBlock == 0 && NextBlockAddress ==0)
    {
        /*first block address = sector address + Sector header size*/
        NextBlockAddress =ActiveSectorInfo.StartAddress +SECTOR_HEADER_SIZE ;
    }
    else if(NextBlockAddress == 0 /*||( NextBlockAddress > VIRTUAL_SECTOR_SIZE-1)*/)
    {
        NextBlockAddress = ActiveSectorInfo.LastValidBlock ;
    }

    /*keep calling function if we didn't receive result or found no errors*/
    if( rtn_val1 == E_PENDING)
    {
        /*Read block header*/
        rtn_val1= ReadVariable(NextBlockAddress,cmd, DATA_BLOCK_HEADER_SIZE);
    }

    if(rtn_val1 == E_OK)
    {
         /*Read block status*/
         BlockStatus =  *((BlockStatusType*)cmd) ;

         /*Read block logical number*/
         BlockNum    = *((uint16*)(cmd+16))    ;

         /*Read Block size */
         BlockSize = *((uint16*)(cmd+18))    ;

         /*check if the current block */
         if( BlockStatus == VALID_BLOCK)
         {
             /*Get current block index*/
              GetIndexFromBlockNum(BlockNum,&index);

             /*1-Save block number */
             ActiveSectorInfo.SectorValidBlocksInfo[index].LogicalAddress = BlockNum ;

             /*2-Save Physical Address*/
             ActiveSectorInfo.SectorValidBlocksInfo[index].PhsicalAddressStart = NextBlockAddress ;

             /*3-Increment Active Blocks counter*/
             if(ActiveSectorInfo.SectorValidBlocksInfo[index].Valid != TRUE)
              ActiveSectorInfo.ValidBlocksNumber++;

             /*4-Save block size*/
             ActiveSectorInfo.SectorValidBlocksInfo[index].BlockSize = BlockSize ;

             /*5-Set block valid flag*/
             ActiveSectorInfo.SectorValidBlocksInfo[index].Valid = TRUE;

             /*6-Save block address as it's the last valid block found in this sector*/
             ActiveSectorInfo.LastValidBlock = NextBlockAddress ;

             /*check if it's the required block */
             if(BlockNum == BlockNumber )
             {
                 /*return found block*/
                 rtn_val2 = E_OK ;
             }
         }

        /*Check if we reached the end of the LinkedList and block not found
         * or any error occurred
         */
        if(((*((uint32*)(cmd+12))) == ERASED_WORD && rtn_val2 != E_OK)||\
                 ( NextBlockAddress>= ActiveSectorInfo.StartAddress +VIRTUAL_SECTOR_SIZE))
        {
             /*return didn't find the block*/
             rtn_val2 = E_NOT_OK ;

             /*Set next start address to read valid blocks from to SECTOR_HEADER_SIZE , to follow the right sequence for next sector*/
             NextBlockAddress = 0;

             /*Reset flags*/
             rtn_val1 = E_PENDING ;
        }
        /*Update next address*/
        if(rtn_val2 == E_PENDING)
        {
            /*Get next block start address*/
            NextBlockAddress += BlockSize + DATA_BLOCK_HEADER_SIZE ;
             rtn_val1 =E_PENDING;
        }
    }

    else if (rtn_val1 == E_NOT_OK)
    {
        rtn_val2 = E_NOT_OK ;
    }

            return rtn_val2;
}

/****************************************************************************************/
/*    Function Name           : SearchSectorValidBlocks                                 */
/*    Function Description    : Find all valid blocks in the sector                     */
/*                            : If not found make first INVALID_VIRTUAL_SECTOR Active   */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Fee_ErrorsType                                          */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType SearchSectorValidBlocks(uint32 SectorAddress)
{
        /*Array to read the block header*/
        static uint8 cmd[DATA_BLOCK_HEADER_SIZE] ;
        /*Variable to read block status*/
        BlockStatusType BlockStatus = 0 ;
        uint16 BlockNum =0,index=0 , BlockSize = 0;
        static uint32 NextBlockAddress = 0 ;
        static uint8 rtn_val1 = E_PENDING;;
        uint8   rtn_val2= E_PENDING;

        /*Check if it's the first call of the function to find active blocks
         * always start searching blocks from the beginning of the current Active sector
         */
        if(ActiveSectorInfo.LastValidBlock == 0 && NextBlockAddress == 0)
        {
            /*first block address = sector address + Sector header size*/
            NextBlockAddress =ActiveSectorInfo.StartAddress + SECTOR_HEADER_SIZE ;
        }
        else if(NextBlockAddress == 0 )
        {
            NextBlockAddress = ActiveSectorInfo.LastValidBlock ;
        }

        /*keep calling function if we didn't receive result or found no errors*/
        if( rtn_val1 == E_PENDING)
        {
            /*Read block header*/
            rtn_val1 = ReadVariable(NextBlockAddress, cmd, DATA_BLOCK_HEADER_SIZE) ;
        }

        if(rtn_val1 == E_OK)
        {
             /*Read block status*/
             BlockStatus =  *((BlockStatusType*)cmd) ;

             /*Read block logical number*/
             BlockNum    = *((uint16*)(cmd+16)) ;

             /*Read block size*/
             BlockSize = *((uint16*)(cmd+18)) ;

             /*check if the current block */
             if( BlockStatus == VALID_BLOCK)
             {
                 /*Get current block index*/
                  GetIndexFromBlockNum(BlockNum,&index);

                 /*1-Save block number */
                 ActiveSectorInfo.SectorValidBlocksInfo[index].LogicalAddress = BlockNum ;

                 /*2-Save Physical Address*/
                 ActiveSectorInfo.SectorValidBlocksInfo[index].PhsicalAddressStart = NextBlockAddress ;

                 /*3-Increment Active Blocks counter*/
                 if(ActiveSectorInfo.SectorValidBlocksInfo[index].Valid != TRUE)
                  ActiveSectorInfo.ValidBlocksNumber++ ;

                 /*4-Save block size*/
                 ActiveSectorInfo.SectorValidBlocksInfo[index].BlockSize = BlockSize ;

                 /*5-Set block valid flag*/
                 ActiveSectorInfo.SectorValidBlocksInfo[index].Valid = TRUE;

                 /*6-Save block address as it's the last valid block found in this sector*/
                  ActiveSectorInfo.LastValidBlock = NextBlockAddress ;
             }

             /*check if it's last block in the sector */
             /*if NextBlockAddress of this block is an erased word*/
             if((*((uint32*)(cmd+12))) == ERASED_WORD )
             {
                 /*Set rtn_val2 = E_OK indication of end of sector*/
                 rtn_val2 = E_OK ;
                 /* Save current address as the previous address for next block
                  * to build a linked list between blocks
                  */
                 ActiveSectorInfo.PreviousBlockAddress = NextBlockAddress ;

                 /*Get next available address in the sector*/
                 if(ActiveSectorInfo.LastValidBlock !=0)
                 {
                     /*Found active blocks , so it's not the first block in sector*/
                     ActiveSectorInfo.InternalNextAvailableAddress = NextBlockAddress+\
                     Fee_BlockConfig[index].FeeBlockSize + DATA_BLOCK_HEADER_SIZE ;
                 }
                 /*first block in sector*/
                 else
                 {
                     ActiveSectorInfo.InternalNextAvailableAddress = SectorAddress + SECTOR_HEADER_SIZE ;
                 }
                 /*Next time start searching from last active block*/
                NextBlockAddress = 0 ;
                rtn_val2 = E_OK ;
                rtn_val1 =E_PENDING ;
             }

             /*Update next address*/
             if(rtn_val2 == E_PENDING)
             {
                 /*Get next block start address*/
                 NextBlockAddress += BlockSize + DATA_BLOCK_HEADER_SIZE ;
                 rtn_val1 =E_PENDING;
             }
        }
        else if (rtn_val1 == E_NOT_OK)
        {
            rtn_val2 = E_NOT_OK ;
        }
        else
        {}
        return rtn_val2;
}
/****************************************************************************************/
/*    Function Name           : FindActiveSector                                        */
/*    Function Description    : Find Active Sector , Save its info                      */
/*                            : If not found make first INVALID_VIRTUAL_SECTOR Active   */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Fee_ErrorsType                                          */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType FindActiveSector(void)
{
    /*Counter to loop Virtual Sectors */
     static SectorStatusType SectorStartAddress = FLASH_BASE_ADDRESS ;
    /*Variable to save sector Status */
     static SectorStatusType SectorStatus = 0  ;
     static uint8 rtn_val1 = E_PENDING;
     uint8 rtn_val2 =E_PENDING;

        /*Read sector status*/
        if(rtn_val1 == E_PENDING)
        {
            rtn_val1 = GetSectorStatus(SectorStartAddress,&SectorStatus) ;
        }
        if(rtn_val1 == E_OK)
        {
            /*Check if status active*/
            if(SectorStatus ==  ACTIVE_VIRTUAL_SECTOR )
            {
                /*Save sector start address*/
                ActiveSectorInfo.StartAddress = SectorStartAddress ;
                rtn_val2 = E_OK ;
            }
            else
            {
                /*Get next sector start address*/
                SectorStartAddress= SectorStartAddress + VIRTUAL_SECTOR_SIZE ;
                rtn_val1 = E_PENDING ;
            }
        }
        if(SectorStartAddress == (FLASH_BASE_ADDRESS+(VIRTUAL_SECTOR_NUMBER*VIRTUAL_SECTOR_SIZE)))
        {
            rtn_val2 = E_NOT_OK ;
        }
    return rtn_val2 ;
}

/****************************************************************************************/
/*    Function Name           : ReadVariable                                            */
/*    Function Description    : read data from flash                                    */
/*    Parameter in            : SourceAddress , Length                                  */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : TargetAddressPtr                                        */
/*    Return value            : none                                                    */
/*    Notes                   : Std_ReturnType                                          */
/****************************************************************************************/
Std_ReturnType ReadVariable(Fls_AddressType SourceAddress, uint8* TargetAddressPtr, Fls_LengthType Length)
{
    uint8 ret_val1 = E_PENDING ;
    Std_ReturnType Read_Return ;
    /*check internal module if not busy*/
    if(!(Fls_GetStatus() == MEMIF_BUSY) && JobDone!=1)
    {
        /*Reset Job done flag and Error flag*/
        JobDone  = 0 ;
        JobError = 0 ;

       /*Request Read job*/
        Read_Return = Fls_Read(SourceAddress , TargetAddressPtr, Length ) ;
    }
    else
    {
        /*Do nothing if underlying module busy*/
    }

    if(Read_Return == E_NOT_OK)
    {
        ret_val1 = E_NOT_OK ;
    }

    /*Check if requested job done OK*/
    else if(JobDone)
    {
        /*Reset Job done flag */
        JobDone  = 0;
        ret_val1 = E_OK ;
    }

    /*Check if error occurred */
    else if(JobError)
    {
        /*Reset Error flag*/
        JobError = 0;
        ret_val1 = E_NOT_OK ;
    }
    return ret_val1 ;
}

/****************************************************************************************/
/*    Function Name           : WriteVariable                                           */
/*    Function Description    : Write data to flash                                     */
/*    Parameter in            : SourceAddress ,TargetAddressPtr ,Length                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Notes                   : Std_ReturnType                                          */
/****************************************************************************************/
Std_ReturnType WriteVariable(Fls_AddressType TargetAddressPtr, uint8* SourceAddress, Fls_LengthType Length)
{
    uint8 ret_val1 = E_PENDING;
    Std_ReturnType Write_Return ;
    /*check internal module if not busy*/
    if(!(Fls_GetStatus() == MEMIF_BUSY) && JobDone!=1)
    {
        /*Reset Job done flag and Error flag*/
        JobDone  = 0;
        JobError = 0;
        /*Request Write job*/
        Write_Return = Fls_Write(TargetAddressPtr, SourceAddress, Length ) ;
    }
    else
    {
        /*Do nothing if underlying module busy*/
    }

    if(Write_Return == E_NOT_OK)
    {
        ret_val1 = E_NOT_OK ;
    }

    /*Check if requested job done OK*/
    else if(JobDone)
    {
        /*Reset Job done flag */
        JobDone  = 0;
        ret_val1 = E_OK ;
    }

    /*Check if error occurred */
    else if(JobError)
    {
        /*Reset Error flag*/
        JobError = 0;
        ret_val1 = E_NOT_OK ;
    }
    return ret_val1 ;
}

/****************************************************************************************/
/*    Function Name           : Erase_Virtualsector                                     */
/*    Function Description    : perform the processing of erasing a virtual sector      */
/*    Parameter in            : Sector_Address                                          */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/
Std_ReturnType Erase_Virtualsector(uint32 Sector_Address)
{
    uint8 ret_val1 = E_PENDING;
    Std_ReturnType Erase_Return ;
    /*check internal module if not busy*/
    if(!(Fls_GetStatus() == MEMIF_BUSY) && JobDone!=1)
    {
        /*Reset Job done flag and Error flag*/
        JobDone  = 0;
        JobError = 0;

        /*Call erase job*/
        Erase_Return = Fls_Erase(Sector_Address , VIRTUAL_SECTOR_SIZE);
    }
    else
    {
        /*Do nothing if underlying module busy*/
    }

    if(Erase_Return == E_NOT_OK)
    {
        ret_val1 = E_NOT_OK ;
    }
    /*Check if requested job done OK*/
    else if(JobDone)
    {
        /*Reset Job done flag */
        JobDone  = 0;
        ret_val1 = E_OK ;
    }

    /*Check if error occurred */
    else if(JobError)
    {
        /*Reset Error flag*/
        JobError = 0;
        ret_val1 = E_NOT_OK ;
    }
    return ret_val1 ;
}

/****************************************************************************************/
/*    Function Name           : GetSectorStatus                                         */
/*    Function Description    : reads sector status in sector header                    */
/*    Parameter in            : Sector_Address                                          */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : SectorStatus                                            */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/
Std_ReturnType GetSectorStatus(uint32 Sector_Address, SectorStatusType* SectorStatus)
{
    uint8 ret_val1 = E_PENDING ;

        /*Read sector Status*/
    ret_val1 = ReadVariable(Sector_Address,(uint8*)SectorStatus, STATUS_LENGTH) ;

    return ret_val1 ;
}

/****************************************************************************************/
/*    Function Name           : SetSectorStatus                                         */
/*    Function Description    : Writes sector status in sector header                   */
/*    Parameter in            : Sector_Address ,SectorStatus                            */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/
Std_ReturnType SetSectorStatus(uint32 Sector_Address, SectorStatusType* SectorStatus)
{
    uint8 ret_val1 = E_PENDING ;

    /*Write sector Status*/
    ret_val1= WriteVariable(Sector_Address,(uint8*)SectorStatus,STATUS_LENGTH) ;

    return ret_val1 ;
}

/****************************************************************************************/
/*    Function Name           : ReadBlock                                               */
/*    Function Description    : read block data and header and save                     */
/*                              it in global BlockData                                  */
/*    Parameter in            : Block_Address ,BlockSize                                */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/
static Std_ReturnType ReadBlock(uint32 Block_Address, uint32 BlockSize)
{
    uint8 ret_val1 = E_PENDING ;

    ret_val1=ReadVariable(Block_Address,BlockData,BlockSize) ;

    return ret_val1 ;
}

/****************************************************************************************/
/*    Function Name           : ReadBlock                                               */
/*    Function Description    : read block data and header and save                     */
/*                              it in global BlockData                                  */
/*    Parameter in            : Block_Address ,BlockSize                                */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/
 Std_ReturnType WriteBlock(Fls_AddressType Block_Address, uint32 BlockSize)
{
    uint8 ret_val1 = E_PENDING ;

        /*Read block data and header*/
    ret_val1 = WriteVariable(Block_Address, BlockData, BlockSize) ;

    return ret_val1 ;
}

/****************************************************************************************/
/*    Function Name           : TransferSector                                          */
/*    Function Description    : transfers valid blocks of current sector to next sector */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
Std_ReturnType TransferSector(uint32 Source , uint32 Traget )
{
    static uint8 Read_ret_val  = E_PENDING ;
    static uint8 Write_ret_val = E_PENDING ;
    uint8 ret_val = E_PENDING ;
    uint32 BlockAddress ;
    uint16 BlockSize ;
    static uint32 TargetBlockAddress = SECTOR_HEADER_SIZE  ;
    static uint16 counter = 0 ,Valid_Blocks = 0;
    uint16 next_index =0;

    /*loop only if there are valid blocks*/
    if( ActiveSectorInfo.ValidBlocksNumber)
    {
    /*loop all configured blocks */
    if(counter < BLOCKS_NUM )
    {
        /*check if the block active*/
        if(ActiveSectorInfo.SectorValidBlocksInfo[counter].Valid)
        {
            /*Get block size*/
            BlockSize    = ActiveSectorInfo.SectorValidBlocksInfo[counter].BlockSize ;
            /*Get block physical start address */
            BlockAddress = ActiveSectorInfo.SectorValidBlocksInfo[counter].PhsicalAddressStart ;

            /*read block only if if it's configurations not changed*/
            if(BlockSize == Fee_BlockConfig[counter].FeeBlockSize)
            {
                /*Read Block */
                if(Read_ret_val == E_PENDING)
                {
                    Read_ret_val =ReadBlock(BlockAddress ,BlockSize +DATA_BLOCK_HEADER_SIZE );
                }
            }
            else
            {
                /*Mark block not valid as configurations changed*/
                ActiveSectorInfo.SectorValidBlocksInfo[counter].Valid = FALSE ;
            }
            /*Change next block address saved in the current block header
             *only if it's not the last valid block
             *the next block is the next active block in the lookup table
             */
            if(counter <ActiveSectorInfo.ValidBlocksNumber-1     && \
               (GetNextActiveBlcokIndex(counter, &next_index)== E_OK) && \
                    Read_ret_val==E_OK )
            {
                   /*set the next block physical address as the next available address
                    * in the sector we are moving blocks to
                    */
                    SET_NEW_NEXT_BLOCK = BlockSize + TargetBlockAddress + Traget + DATA_BLOCK_HEADER_SIZE;
            }
            else
            {
                /*If it's the last active blocks ,
                 *save next block address in the header as ERASED WORD
                 */
                SET_NEW_NEXT_BLOCK = ERASED_WORD ;
            }

            /*Write block only if reading done OK*/
            if(Read_ret_val == E_OK)
            {
                /*write block in it's new place in the new sector*/
                Write_ret_val =WriteBlock(TargetBlockAddress +Traget,BlockSize + DATA_BLOCK_HEADER_SIZE );
            }

            if(Write_ret_val == E_OK )
            {

                Valid_Blocks++;
                /*Save block physical address in the lookup table*/
                ActiveSectorInfo.SectorValidBlocksInfo[counter].PhsicalAddressStart = TargetBlockAddress +Traget ;

                /*Updated next target block address*/
                TargetBlockAddress += ActiveSectorInfo.SectorValidBlocksInfo[counter].BlockSize + DATA_BLOCK_HEADER_SIZE ;

                /*Reset flags*/
                Read_ret_val  = E_PENDING ;
                Write_ret_val = E_PENDING;

                /*If we reach end of valid blocks return E_OK*/
                if(Valid_Blocks == ActiveSectorInfo.ValidBlocksNumber )
                {
                    /*save current as previous */
                    ActiveSectorInfo.PreviousBlockAddress =\
                            ActiveSectorInfo.SectorValidBlocksInfo[counter].PhsicalAddressStart ;
                    ActiveSectorInfo.LastValidBlock = ActiveSectorInfo.SectorValidBlocksInfo[counter].PhsicalAddressStart ;
                    /*Save first available physical address in the new sector*/
                    ActiveSectorInfo.InternalNextAvailableAddress = TargetBlockAddress +Traget ;

                    /*return OK indication blocks transfered successfully*/
                    ret_val = E_OK ;

                    /*Reset flags for next sector switch*/
                    counter = 0;
                    Valid_Blocks = 0;
                    TargetBlockAddress = SECTOR_HEADER_SIZE ;
                    Read_ret_val = E_PENDING ;
                    Write_ret_val = E_PENDING ;
                }
                else
                {
                    /*Increment block counter */
                    counter++;
                }
            }
        }
        else
        {
            /*Increment block counter if the current block is not valid*/
            counter++;
        }
    }
    }
    else
    {
        /*No valid blocks to transfer*/
        ret_val = E_OK ;
        ActiveSectorInfo.InternalNextAvailableAddress = Traget + SECTOR_HEADER_SIZE ;
        ActiveSectorInfo.StartAddress = Traget ;
        ActiveSectorInfo.LastValidBlock =0;
        Read_ret_val = E_PENDING ;
        Write_ret_val = E_PENDING ;
        /*Reset flags for next sector switch*/
        counter = 0;
        Valid_Blocks = 0;
        TargetBlockAddress = SECTOR_HEADER_SIZE ;
    }

    /*If any error occurred return E_NOT_OK */
    if((Read_ret_val == E_NOT_OK ) || (Write_ret_val == E_NOT_OK ))
    {
        ret_val = E_NOT_OK ;

        /*Reset flags for next sector switch*/
        counter = 0;
        Valid_Blocks = 0;
        TargetBlockAddress = SECTOR_HEADER_SIZE ;
    }

   return ret_val  ;
}

/****************************************************************************************/
/*    Function Name           : GetIndexFromBlockNum                                    */
/*    Function Description    : map block number to specific index in the global array  */
/*    Parameter in            : BlockNumber                                             */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType, OK if found                             */
/*    Notes                   :                                                         */
/****************************************************************************************/

Std_ReturnType GetIndexFromBlockNum(uint16 BlockNumber,uint16* Index)
{
    /*counter to loop blocks*/
    uint16 counter = 0 ;
    uint8 rtn_val = E_NOT_OK ;

    for(counter = 0 ; counter< BLOCKS_NUM ; counter++)
    {
        if(Fee_BlockConfig[counter].FeeBlockNumber == BlockNumber)
        {
            *Index = counter;
            rtn_val = E_OK ;
            break ;
        }
    }
    return rtn_val ;
}


/****************************************************************************************/
/*    Function Name           : GetNextActiveBlcokIndex                                 */
/*    Function Description    : get the next valid block's index from current index     */
/*    Parameter in            : BlockNumber                                             */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType, OK if found                             */
/*    Notes                   :                                                         */
/****************************************************************************************/
static Std_ReturnType GetNextActiveBlcokIndex(uint16 CurrentIndex,uint16* NextIndex)
{
    /*counter to loop blocks*/
    uint16 counter = 0 ;
    uint8 rtn_val = E_NOT_OK ;

    for(counter = CurrentIndex+1 ; counter< BLOCKS_NUM ; counter++)
    {
        if(ActiveSectorInfo.SectorValidBlocksInfo[counter].Valid == TRUE)
        {
            *NextIndex = counter;
            rtn_val = E_OK ;
            break ;
        }
    }
    return rtn_val ;
}

