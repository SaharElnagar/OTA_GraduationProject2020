
/*****************************************************************************************/
/*                                   Include Common headres                              */
/*****************************************************************************************/
#include <stdint.h>
#include "Can_Cfg.h"

/*****************************************************************************************/
/*                                   Local Macro Definition                              */
/*****************************************************************************************/

//define used CAN base address for bootloader
#ifndef CANx_BASE
#define CANx_BASE                    CAN0_BASE
#endif

//define used Transmit hw object number
#ifndef HWOBJ_TRANSMIT_NUM
#define HWOBJ_TRANSMIT_NUM           (1U)
#endif

//define used Receive hw object number
#ifndef HWOBJ_RECEIVE_NUM
#define HWOBJ_RECEIVE_NUM            (2U)
#endif

/*****************************************************************************************/
/*                                   Function ProtoTypes                                 */
/*****************************************************************************************/

void Can_Init(void) ;
void CanReceiveBlocking_Function(uint16_t size, uint8_t*DataPtr,uint8_t MessageNum);
void CanTransmitBlocking_Function(uint16_t size,const uint8_t*DataPtr,uint8_t MessageNum);

