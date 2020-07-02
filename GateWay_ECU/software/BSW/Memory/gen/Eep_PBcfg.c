/*
 * Eep_PBcfg.c
 *
 *  Created on: Nov 24, 2019
 *      Author: Sahar
 */

#include "Eep_Types.h"
#include "MemIf_Types.h"
#include "Eep_Cbk.h"

EepInitConfigurationType EepInitConfiguration =
{
    .EepBaseAddress            = 0                              ,
    .EepDefaultMode            = MEMIF_MODE_SLOW                ,
    .EepJobEndNotification     = Ea_JobEndNotification          ,
    .EepJobErrorNotification   = Ea_JobErrorNotification        ,
    .EepNormalReadBlockSize    = 4                              ,
    .EepNormalWriteBlockSize   = 4                              ,
    .EepSize                   = 2048
};

Eep_ConfigType Eep_Config =
{
     .EepInitConfigurationRef = &EepInitConfiguration ,
};


