

/**
 * main.c
 */

#include "CANIf.h"
#include "NvM.h"
#include "MemIf.h"
#include "Fee.h"
#include "Ea.h"
#include "Eep.h"
#include "Decryption_SWC.h"
#include "sysctl.h"
#include "PLL.h"
void Set_SystemClock(void);
void irq_Enable(void);

/*extern*/
extern Eep_ConfigType Eep_Config ;
extern Fls_ConfigType Fls_Config;


int main(void)
{
    /*Initialize All Modules*/
    Set_SystemClock();
    irq_Enable();
    Fls_Init(&Fls_Config);
    Eep_Init(&Eep_Config);
    Can_Init();
    Download_Update_Init();
    StoreUpdate_Init();
    Init_TransmitECUsUpdate();
    Ea_Init(NULL) ;
    Fee_Init(NULL);
    NvM_Init(NULL);
    while(Fee_GetStatus()== MEMIF_BUSY)
    {
        Fls_MainFunction();
        Fee_MainFunction();
    }
    while(Ea_GetStatus()==MEMIF_BUSY)
    {
        Eep_MainFunction();
        Ea_MainFunction();
    }
    Decryption_Init();
    while(1)
    {
        Fls_MainFunction();
        Fee_MainFunction();
        NvM_MainFunction();
        Download_Update_MainFunction();
        Decryption_MainFunction();
        StoreUpdate_MainFunction();
        TransmitECUsUpdate_MainFunction();
    }
	return 0;
}



void Set_SystemClock(void)
{
    /*Use 80 MHz clock*/
    PLL_Init();
    SysCtlDelay(20000000);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
}


/* Enable all interrupts */
__attribute__((naked)) void irq_Enable(void)
{
    __asm(" CPSIE I");
    __asm(" BX LR") ;
}

/* Disable all interrupts */

__attribute__((naked)) void irq_Disable(void)
{
    __asm(" CPSID I");
    __asm(" BX LR") ;
}
