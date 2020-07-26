#ifndef _BL_CAN_H
#define _BL_CAN_H
/*****************************************************************************************/
/*                                   Include Common headres                              */
/*****************************************************************************************/
#include <stdint.h>


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

void CanIf_Init(void) ;
void CanReceiveBlocking(uint8_t* DataPtr ,uint16_t size);
void CanTransmitBlocking(const uint8_t*DataPtr , uint16_t size);

#endif

