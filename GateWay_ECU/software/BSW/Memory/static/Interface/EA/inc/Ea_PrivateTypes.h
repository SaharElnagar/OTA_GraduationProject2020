/*
 * Ea_Types.h
 *
 *  Created on: 2020-2-17
 *      Author: Sahar
 */

#ifndef BSW_STATIC_INTERFACE_EA_INC_EA_PRIVATETYPES_H_
#define BSW_STATIC_INTERFACE_EA_INC_EA_PRIVATETYPES_H_

#include "Std_Types.h"

typedef struct {
    uint16              BlockNumber;
    Eep_AddressType     PhysicalStartAddress;
    uint16              BlockSize;
} Ea_BlockConfigType;




#endif /* BSW_STATIC_INTERFACE_EA_INC_EA_PRIVATETYPES_H_ */
