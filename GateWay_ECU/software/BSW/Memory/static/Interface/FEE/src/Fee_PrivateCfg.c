/*******************************************************************************
**                                                                            **
**  FILENAME     : Fee_Cfg.c                                                  **
**                                                                            **
**  Description  :   Module configurations                                    **
*******************************************************************************/

/*****************************************************************************************/
/*                                   Include headres                                     */
/*****************************************************************************************/
#include "Fee_Cfg.h"
#include "Fee_PrivateTypes.h"

Fee_BlockConfigType Fee_BlockConfig[BLOCKS_NUM] =
{
    /*first block*/
    {
        BLOCK_1_NUMBER,
        BLOCK_1_SIZE  ,
        BLOCK_1_WRITE_CYCLES
        },

        {
        BLOCK_2_NUMBER,
        BLOCK_2_SIZE  ,
        BLOCK_1_WRITE_CYCLES
        }
        ,

        {
        BLOCK_3_NUMBER,
        BLOCK_3_SIZE  ,
        BLOCK_1_WRITE_CYCLES
        }
        ,

        {
        BLOCK_4_NUMBER,
        BLOCK_4_SIZE  ,
        BLOCK_1_WRITE_CYCLES
        }
        ,

        {
        BLOCK_5_NUMBER,
        BLOCK_5_SIZE  ,
        BLOCK_1_WRITE_CYCLES
        }
        ,

        {
        BLOCK_6_NUMBER,
        BLOCK_6_SIZE  ,
        BLOCK_1_WRITE_CYCLES
        }
        ,

        {
        BLOCK_7_NUMBER,
        BLOCK_7_SIZE  ,
        BLOCK_1_WRITE_CYCLES
        }
        ,

        {
        BLOCK_8_NUMBER,
        BLOCK_8_SIZE  ,
        BLOCK_1_WRITE_CYCLES
        }
        ,

        {
        BLOCK_9_NUMBER,
        BLOCK_9_SIZE  ,
        BLOCK_1_WRITE_CYCLES
        }
        ,

        {
        BLOCK_10_NUMBER,
        BLOCK_10_SIZE  ,
        BLOCK_1_WRITE_CYCLES
        }
} ;
