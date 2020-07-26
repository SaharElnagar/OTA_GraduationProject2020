
#ifndef USART_H
#define USART_H

#include "common.h"
#include "stm32f4xx.h"

#define MAX_BUFFER_LENGTH   128	


void USART1_init(void);
enum_error_type USART1_transmit_blocking(uint8_t * pdata,uint32_t size);
enum_error_type USART1_receive_blocking(uint8_t * pdata,uint32_t size);

void USART2_init(void);
void USART2_reset(void);
enum_error_type USART2_transmit_blocking(uint8_t * pdata,uint32_t size);
enum_error_type USART2_receive_blocking(uint8_t * pdata,uint32_t size);

#endif  /* USART_H */
