/*
 * MemIf.h
 *
 *  Created on: Jun 9, 2020
 *      Author: Sahar
 */

#ifndef BSP_MEMIF_MEMIF_H_
#define BSP_MEMIF_MEMIF_H_

#include "MemIf_Types.h"
#include "Std_Types.h"

Std_ReturnType MemIf_Read(uint8 DeviceIndex, uint16 BlockNumber, uint8* DataBufferPtr, uint16 Length);

Std_ReturnType MemIf_Write(uint8 DeviceIndex,uint16 BlockNumber,const uint8* DataBufferPtr);

MemIf_StatusType MemIf_GetStatus(uint8 DeviceIndex);

MemIf_JobResultType MemIf_GetJobResult(uint8 DeviceIndex);

Std_ReturnType MemIf_InvalidateBlock(uint8 DeviceIndex,uint16 BlockNumber) ;
#endif /* BSP_MEMIF_MEMIF_H_ */
