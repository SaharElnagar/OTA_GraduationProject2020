
#ifndef __DET_H__
#define __DET_H__





#include "Std_Types.h"

/*****************************************************************************************/
/*                                    Function Declaration                               */
/*****************************************************************************************/


Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);
Std_ReturnType Det_ReportTransientFault(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);
Std_ReturnType Det_ReportRuntimeError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);



#endif

