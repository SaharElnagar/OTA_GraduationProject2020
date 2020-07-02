/*******************************************************************************
**                                                                            **
**  FILENAME     : stub.c                                                      **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2019-12-1                                                  **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Yomna Mokhtar                                              **
**                                                                            **
**                                                                            **
*******************************************************************************/
#include "Platform_Types.h"
#include "Det.h"

// Development errors report function
Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId){
		
	return E_NOT_OK;
}

// Transient Faults report function
Std_ReturnType Det_ReportTransientFault(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId){
	return E_NOT_OK;
}

// Runtime errors report function
Std_ReturnType Det_ReportRuntimeError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId){
	return E_NOT_OK;
}


