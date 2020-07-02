/*
 * Fls_PBcfg.c
 *
 *  Created on: Jun 29, 2020
 *      Author: Sahar
 */

#include "Fls.h"
#include "Fee_Cbk.h"

Fls_ConfigType Fls_Config =
{
     /* Address offset in RAM to which the erase flash access code shall be loaded.
      * */
     .FlsJobEndNotification   = Fee_JobEndNotification ,
     .FlsJobErrorNotification = Fee_JobErrorNotification
};



