#include "stm32f4xx.h"
#include "common.h"
#include "USART.h"

static uint8_t rx_data[MAX_BUFFER_LENGTH];
uint32_t rx_data_length = 0;
static uint8_t * app_buffer;

/* Private functions */
void USART2_init(void) {
	//PA2(TX)-PA3(RX)    UART2
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	while (!(RCC->AHB1ENR & RCC_AHB1ENR_GPIOAEN))
		;

	GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
	GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;

	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR2 | GPIO_PUPDR_PUPDR3);
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR2_0 | GPIO_PUPDR_PUPDR3_0);  //pullup

	GPIOA->AFR[0] = (GPIOA->AFR[0] & ~0x0000FF00) | 0x00007700;

	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	while (!(RCC->APB1ENR & RCC_APB1ENR_USART2EN))
		;

	USART2->CR1 &= ~USART_CR1_UE;

	//oversampling by 16,1 Start bit, 8 Data bits
	USART2->CR1 &= ~(USART_CR1_OVER8 | USART_CR1_M);

	USART2->CR2 &= ~ USART_CR2_STOP;   //one stop bit

	//DMA enable receiver
//	USART2->CR3 |=USART_CR3_DMAR;
	//DMA enable transmitter
//	USART2->CR3 |=USART_CR3_DMAT;

	USART2->BRR = ((16000000 / (115200 * 16)) << 4) + 10;

	USART2->CR1 |= USART_CR1_UE;

	USART2->CR1 |= USART_CR1_RE | USART_CR1_TE;
	//IDLE line interrupt enable
//	USART2->CR1 |=USART_CR1_IDLEIE;

	//enable interrupt at NVIC
//	NVIC->ISER[1] |=0x40;
}

void USART2_reset(void) {
	RCC->AHB1RSTR = RCC_AHB1RSTR_GPIOARST;
	RCC->AHB1RSTR = 0;
	RCC->APB1RSTR = RCC_APB1RSTR_USART2RST;
	RCC->APB1RSTR = 0;

}
enum_error_type USART2_transmit_blocking(uint8_t * pdata, uint32_t size) {
	enum_error_type ret_val = E_OK;
	if (pdata == NULL) {
		ret_val = E_NOT_OK;
	} else {
		for (uint32_t i = 0; i < size; i++) {
			while (!(USART2->SR & USART_SR_TXE)) {
			}
			USART2->DR = *pdata++;
		}
		while (!(USART2->SR & USART_SR_TC))
			;
	}
	return ret_val;
}

enum_error_type USART2_receive_blocking(uint8_t * pdata, uint32_t size) {
	enum_error_type ret_val = E_OK;
	if (pdata == NULL) {
		ret_val = E_NOT_OK;
	} else {
		for (uint32_t i = 0; i < size; i++) {
			while (!(USART2->SR & USART_SR_RXNE)) {
			}
			*pdata = USART2->DR;
			pdata++;
		}
	}
	return ret_val;
}

void USART1_init(void) {
	//PB6(TX)-PB7(RX)    UART1
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	while (!(RCC->AHB1ENR & RCC_AHB1ENR_GPIOBEN))
		;

	GPIOB->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
	GPIOB->MODER |= GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;

	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR7);
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPDR6_0 | GPIO_PUPDR_PUPDR7_0);  //pullup

	GPIOB->AFR[0] = (GPIOB->AFR[0] & ~0xFF000000) | 0x77000000;

	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	while (!(RCC->APB2ENR & RCC_APB2ENR_USART1EN))
		;

	USART1->CR1 &= ~USART_CR1_UE;

	//oversampling by 16,1 Start bit, 8 Data bits
	USART1->CR1 &= ~(USART_CR1_OVER8 | USART_CR1_M);

	USART1->CR2 &= ~ USART_CR2_STOP;   //one stop bit

	//DMA enable receiver
//	USART1->CR3 |=USART_CR3_DMAR;
	//DMA enable transmitter
//	USART1->CR3 |=USART_CR3_DMAT;

	USART1->BRR = ((16000000 / (115200 * 16)) << 4) + 10;

	USART1->CR1 |= USART_CR1_UE;

	USART1->CR1 |= USART_CR1_RE | USART_CR1_TE;
	uint8_t delay = 5; //When TE is set, there is a 1 bit-time delay before the transmission starts
	while (delay--)
		;
}

enum_error_type USART1_transmit_blocking(uint8_t * pdata, uint32_t size) {
	enum_error_type ret_val = E_OK;
	if (pdata == NULL) {
		ret_val = E_NOT_OK;
	} else {
		for (uint32_t i = 0; i < size; i++) {
			while (!(USART1->SR & USART_SR_TXE)) {
			}
			USART1->DR = *pdata++;
		}
		while (!(USART1->SR & USART_SR_TC))
			;
	}
	return ret_val;
}

enum_error_type USART1_receive_blocking(uint8_t * pdata, uint32_t size) {
	enum_error_type ret_val = E_OK;
	if (pdata == NULL) {
		ret_val = E_NOT_OK;
	} else {
		for (uint32_t i = 0; i < size; i++) {
			while (!(USART1->SR & USART_SR_RXNE)) {
			}
			*pdata = USART1->DR;
			pdata++;
		}
	}
	return ret_val;
}

void USART2_RX_DMA1_config(uint8_t * prxdata) {
	app_buffer = prxdata;
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	DMA1_Stream5->CR &= ~DMA_SxCR_EN;
	while (DMA1_Stream5->CR & DMA_SxCR_EN)
		;
//>>peripheral-to-Memory
//>>Memory data size:byte (8-bit)  >>Peripheral data size:byte (8-bit)
//Peripheral address pointer is fixed
//Circular mode disabled
	DMA1_Stream5->CR &= ~(DMA_SxCR_CHSEL + DMA_SxCR_PL + DMA_SxCR_DIR
			+ DMA_SxCR_MSIZE + DMA_SxCR_PSIZE + DMA_SxCR_PINC + DMA_SxCR_CIRC);
//channel 4 selected(USART2_RX) >>Priority level:Medium
//Memory address pointer is incremented after each data transfer
	DMA1_Stream5->CR |= DMA_SxCR_CHSEL_2 + DMA_SxCR_PL_0 + DMA_SxCR_MINC;
//number of data to be transfered
	DMA1_Stream5->NDTR = MAX_BUFFER_LENGTH;
	DMA1_Stream5->PAR = (uint32_t) &USART2->DR;
	DMA1_Stream5->M0AR = (uint32_t) rx_data;
	DMA1_Stream5->FCR &= ~DMA_SxFCR_FTH;
	DMA1_Stream5->FCR |= DMA_SxFCR_DMDIS + DMA_SxFCR_FTH_0; //FIFO mode enabled   ,1/2 full FIFO threshold
	DMA1_Stream5->CR |= DMA_SxCR_EN;
}

void USART2_TX_DMA1_config(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	DMA1_Stream6->CR &= ~DMA_SxCR_EN;
	while (DMA1_Stream6->CR & DMA_SxCR_EN)
		;

//>>Memory data size:byte (8-bit)  >>Peripheral data size:byte (8-bit)	
//Peripheral address pointer is fixed	
//Circular mode disabled	
	DMA1_Stream6->CR &= ~(DMA_SxCR_CHSEL + DMA_SxCR_PL + DMA_SxCR_DIR
			+ DMA_SxCR_MSIZE + DMA_SxCR_PSIZE + DMA_SxCR_PINC + DMA_SxCR_CIRC);
//channel 4 selected(USART2_TX) >>Priority level:Medium  
//>>Memory-to-peripheral
//Memory address pointer is incremented after each data transfer
//Transfer complete interrupt enable	
	DMA1_Stream6->CR |= DMA_SxCR_CHSEL_2 + DMA_SxCR_PL_0 + DMA_SxCR_DIR_0
			+ DMA_SxCR_MINC + DMA_SxCR_TCIE;
	DMA1_Stream6->PAR = (uint32_t) &USART2->DR;
	DMA1_Stream6->FCR &= ~DMA_SxFCR_FTH;
	DMA1_Stream6->FCR |= DMA_SxFCR_DMDIS + DMA_SxFCR_FTH_0; //FIFO mode enabled   ,1/2 full FIFO threshold
//NVIC_enable
	NVIC->ISER[0] |= 1 << 17;

}
enum_error_type USART2_transmit_DMA1(uint8_t * pdata, uint32_t size) {
	enum_error_type ret_val = E_OK;
	if (pdata == NULL) {
		ret_val = E_NOT_OK;
	} else {
		while (DMA1_Stream6->CR & DMA_SxCR_EN)
			;         //wait for the previoud transmit
		//number of data to be transfered
		DMA1_Stream6->NDTR = size;
		DMA1_Stream6->M0AR = (uint32_t) pdata;
		DMA1->HIFCR = DMA_HIFCR_CTCIF6 + DMA_HIFCR_CHTIF6 + DMA_HIFCR_CTEIF6
				+ DMA_HIFCR_CDMEIF6 + DMA_HIFCR_CFEIF6;
		DMA1_Stream6->CR |= DMA_SxCR_EN;
	}
	return ret_val;
}

void USART2_IRQHandler(void) {
	if (USART2->SR & USART_SR_RXNE) {
	}
	if (USART2->SR & USART_SR_TC) {
	}
	if (USART2->SR & USART_SR_IDLE) {
		(void) USART2->SR;      //to clear the flag
		(void) USART2->DR;          //to clear the flag
		DMA1_Stream5->CR &= ~DMA_SxCR_EN;
		while (DMA1_Stream5->CR & DMA_SxCR_EN)
			;
		rx_data_length = MAX_BUFFER_LENGTH - DMA1_Stream5->NDTR;
		for (uint32_t i = 0; i < rx_data_length; i++) {
			app_buffer[i] = rx_data[i];
		}
		DMA1->HIFCR = DMA_HIFCR_CTCIF5 + DMA_HIFCR_CHTIF5 + DMA_HIFCR_CTEIF5
				+ DMA_HIFCR_CDMEIF5 + DMA_HIFCR_CFEIF5;
		DMA1_Stream5->CR |= DMA_SxCR_EN;

	}
}
void DMA1_Stream6_IRQHandler(void) {
	if (DMA1->HISR & DMA_HISR_TCIF6) {
		DMA1->HIFCR |= DMA_HIFCR_CTCIF6;

	}
}

/* functions */

