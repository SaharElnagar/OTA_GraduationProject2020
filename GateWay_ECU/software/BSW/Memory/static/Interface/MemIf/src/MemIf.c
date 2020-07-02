/*
 * MemIf.c
 *
 *  Created on: Jun 22, 2020
 *      Author: Sahar
 */

#include "Std_Types.h"
#include "MemIf_Types.h"
#include "MemIf_Cfg.h"
#include "Fee.h"
#include "Ea.h"

/*******************************************************************************/
//  Write functions pointers
/*******************************************************************************/
Std_ReturnType (*MemIf_WriteFctPtr[MEMIF_NUMBER_OF_DEVICES])(uint16 , const uint8* )=
{
     Fee_Write  ,
     Ea_Write
};

/*******************************************************************************/
//  Read functions pointers
/*******************************************************************************/
Std_ReturnType (*MemIf_ReadFctPtr [MEMIF_NUMBER_OF_DEVICES])(uint16 , uint16 , uint8* , uint16 )=
{
     Fee_Read   ,
     Ea_Read
};
/*******************************************************************************/
//  Get_status functions pointers
/*******************************************************************************/
MemIf_StatusType (*MemIf_GetStatusFctPtr[MEMIF_NUMBER_OF_DEVICES])(void ) =
{
     Fee_GetStatus  ,
     Ea_GetStatus
};

/*******************************************************************************/
//  Get_JobResult functions pointers
/*******************************************************************************/
MemIf_JobResultType (*MemIf_GetResultFctPtr[MEMIF_NUMBER_OF_DEVICES])(void )=
{
     Fee_GetJobResult   ,
     Ea_GetJobResult
};

/*******************************************************************************/
//  InvalidateBlock functions pointers
/*******************************************************************************/
Std_ReturnType (*MemIf_InvalidateFctPtr[MEMIF_NUMBER_OF_DEVICES])(uint16 )=
{
     Fee_InvalidateBlock    ,
     Ea_InvalidateBlock
};


Std_ReturnType MemIf_Read(uint8 DeviceIndex, uint16 BlockNumber, uint8* DataBufferPtr, uint16 Length)
{
    return MemIf_ReadFctPtr[DeviceIndex](BlockNumber ,0,DataBufferPtr ,Length );
}


Std_ReturnType MemIf_Write(uint8 DeviceIndex,uint16 BlockNumber,const uint8* DataBufferPtr)
{
    return MemIf_WriteFctPtr[DeviceIndex](BlockNumber ,DataBufferPtr);
}

MemIf_StatusType MemIf_GetStatus(uint8 DeviceIndex)
{
   return MemIf_GetStatusFctPtr[DeviceIndex]();
}

MemIf_JobResultType MemIf_GetJobResult(uint8 DeviceIndex)
{
    return MemIf_GetResultFctPtr[DeviceIndex]();
}

Std_ReturnType MemIf_InvalidateBlock(uint8 DeviceIndex,uint16 BlockNumber)
{
    return MemIf_InvalidateFctPtr[DeviceIndex](BlockNumber);
}

