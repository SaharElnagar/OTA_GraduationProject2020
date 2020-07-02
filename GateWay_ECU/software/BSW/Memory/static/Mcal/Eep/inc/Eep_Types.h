
/*******************************************************************************
**                                                                            **
**  FILENAME     : Eep_Types.h                                                **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : Jun 27, 2020                                               **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Sahar Elnagar                                              **
**                                                                            **
**                                                                            **
*******************************************************************************/
#ifndef BSW_STATIC_MCAL_EEP_INC_EEP_TYPES_H_
#define BSW_STATIC_MCAL_EEP_INC_EEP_TYPES_H_

#include "Std_Types.h"

/*Create Module Types*/
typedef uint8   EepDefaultModeType;

typedef uint16  Eep_AddressType;

typedef uint16  Eep_LengthType;

/*******************************************************************************/
//  Container for  configuration parameters of the EEPROM driver.
//   Implementation Type: Eep_ConfigType.
/*******************************************************************************/
typedef struct
{
    /*This parameter is the EEPROM device base address. Implementation Type: Eep_AddressType*/
    uint32                       EepBaseAddress ;
    EepDefaultModeType           EepDefaultMode;
    void(*EepJobEndNotification)(void);
    void(*EepJobErrorNotification)(void);
    uint32                       EepNormalReadBlockSize;
    uint32                       EepNormalWriteBlockSize;
    uint32                       EepSize;
}EepInitConfigurationType;

typedef struct
{
    EepInitConfigurationType* EepInitConfigurationRef;
}Eep_ConfigType;



#endif /* BSW_STATIC_MCAL_EEP_INC_EEP_TYPES_H_ */
