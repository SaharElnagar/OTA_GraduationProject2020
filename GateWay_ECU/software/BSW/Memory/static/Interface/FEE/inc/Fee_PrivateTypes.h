/*
 * Fee_Types.h
 *
 *  Created on: Apr 26, 2020
 *      Author: Sahar
 */

#ifndef FEE_TYPES_H_
#define FEE_TYPES_H_

#include "Std_Types.h"

/*Struct to hold Configuration of each block*/
typedef struct{
   uint16 FeeBlockNumber ;
   uint16 FeeBlockSize ;
   boolean FeeImmediateData ;
   uint32 FeeNumberOfWriteCycles ;
}Fee_BlockConfigType;


#endif /* FEE_TYPES_H_ */

