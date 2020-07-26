
#include "GPIO.h"
#include "tm4c123gh6pm.h"



void Led_On(uint8_t color)
{
	GPIO_PORTF_DATA_R &= ~0xF;
  GPIO_PORTF_DATA_R |=  color ;
}

void PortF_Init(void){
  SYSCTL_RCGC2_R |= SYSCTL_RCGCGPIO_R5;                    // 1) F clock
  while(!(SYSCTL_PRGPIO_R &SYSCTL_PRGPIO_R5)){};           // delay
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0
}
