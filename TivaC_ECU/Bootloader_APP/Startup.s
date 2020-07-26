




NVIC_VTABLE                     equ     0xE000ED08
_VTABLE_START_ADDRESS           equ     0x00001000
_APP_START_ADDRESS              equ     0x00001000	
	

 thumb
 require8	 
 preserve8
	 
	 
_STACK_SIZE     EQU   64
	
;******************************************************************************
;
; The stack gets placed into the zero-init section.
;
;******************************************************************************	 
    area    ||.bss||, noinit, align=2

;******************************************************************************
;
; Allocate storage for the stack.
;
;******************************************************************************
g_pulStack                      ; pointer to start address of the next space 
Stack_Mem    space   _STACK_SIZE*4 

		
;******************************************************************************
;
; This portion of the file goes into the reset section.
;
;******************************************************************************
    area    RESET, code, readonly, align=3		
		
	; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size
					

__Vectors       DCD     g_pulStack + (_STACK_SIZE *4 )              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
				DCD     IntDefaultHandler               ; Offset 10: MPU fault handler
				DCD     IntDefaultHandler               ; Offset 14: Bus fault handler
				DCD     IntDefaultHandler               ; Offset 18: Usage fault handler
				DCD     0                               ; Offset 1C: Reserved
				DCD     0                               ; Offset 20: Reserved
				DCD     0                               ; Offset 24: Reserved
				DCD     0                               ; Offset 28: Reserved
				DCD     IntDefaultHandler;UpdateHandler                   ; Offset 2C: SVCall handler
				DCD     IntDefaultHandler               ; Offset 30: Debug monitor handler
				DCD     0                               ; Offset 34: Reserved
				DCD     IntDefaultHandler               ; Offset 38: PendSV handler
				if      :def:_ENET_ENABLE_UPDATE
				import  SysTickIntHandler
				DCD     SysTickIntHandler               ; Offset 3C: SysTick handler
				else
				DCD     IntDefaultHandler               ; Offset 3C: SysTick handler
				endif
				if      :def:_UART_ENABLE_UPDATE :land: :def:_UART_AUTOBAUD
				import  GPIOIntHandler
				DCD     GPIOIntHandler                  ; Offset 40: GPIO port A handler
				else
				DCD     IntDefaultHandler               ; Offset 40: GPIO port A handler
				endif
                DCD     IntDefaultHandler             ;   1: GPIO Port B
                DCD     IntDefaultHandler             ;   2: GPIO Port C
                DCD     IntDefaultHandler             ;   3: GPIO Port D
                DCD     IntDefaultHandler             ;   4: GPIO Port E
                DCD     UART0_Handler             ;   5: UART0 Rx and Tx
                DCD     IntDefaultHandler             ;   6: UART1 Rx and Tx
                DCD     IntDefaultHandler              ;   7: SSI0 Rx and Tx
                DCD     IntDefaultHandler              ;   8: I2C0 Master and Slave
                DCD     IntDefaultHandler        ;   9: PWM Fault
                DCD     IntDefaultHandler            ;  10: PWM Generator 0
                DCD     IntDefaultHandler            ;  11: PWM Generator 1
                DCD     IntDefaultHandler            ;  12: PWM Generator 2
                DCD     IntDefaultHandler              ;  13: Quadrature Encoder 0
                DCD     IntDefaultHandler           ;  14: ADC Sequence 0
                DCD     IntDefaultHandler           ;  15: ADC Sequence 1
                DCD     IntDefaultHandler           ;  16: ADC Sequence 2
                DCD     IntDefaultHandler           ;  17: ADC Sequence 3
                DCD     IntDefaultHandler              ;  18: Watchdog timer
                DCD     IntDefaultHandler           ;  19: Timer 0 subtimer A
                DCD     IntDefaultHandler           ;  20: Timer 0 subtimer B
                DCD     IntDefaultHandler           ;  21: Timer 1 subtimer A
                DCD     IntDefaultHandler           ;  22: Timer 1 subtimer B
                DCD     IntDefaultHandler           ;  23: Timer 2 subtimer A
                DCD     IntDefaultHandler           ;  24: Timer 2 subtimer B
                DCD     IntDefaultHandler             ;  25: Analog Comparator 0
                DCD     IntDefaultHandler             ;  26: Analog Comparator 1
                DCD     IntDefaultHandler             ;  27: Analog Comparator 2
                DCD     IntDefaultHandler            ;  28: System Control (PLL, OSC, BO)
                DCD     IntDefaultHandler             ;  29: FLASH Control
                DCD     IntDefaultHandler             ;  30: GPIO Port F
                DCD     IntDefaultHandler             ;  31: GPIO Port G
                DCD     IntDefaultHandler             ;  32: GPIO Port H
                DCD     IntDefaultHandler             ;  33: UART2 Rx and Tx
                DCD     IntDefaultHandler              ;  34: SSI1 Rx and Tx
                DCD     IntDefaultHandler           ;  35: Timer 3 subtimer A
                DCD     IntDefaultHandler           ;  36: Timer 3 subtimer B
                DCD     IntDefaultHandler              ;  37: I2C1 Master and Slave
                DCD     IntDefaultHandler              ;  38: Quadrature Encoder 1
                DCD     IntDefaultHandler              ;  39: CAN0
                DCD     IntDefaultHandler              ;  40: CAN1
                DCD     IntDefaultHandler              ;  41: CAN2
                DCD     0                         ;  42: Reserved
                DCD     IntDefaultHandler               ;  43: Hibernate
                DCD     IntDefaultHandler              ;  44: USB0
                DCD     IntDefaultHandler            ;  45: PWM Generator 3
                DCD     IntDefaultHandler              ;  46: uDMA Software Transfer
                DCD     IntDefaultHandler           ;  47: uDMA Error
                DCD     IntDefaultHandler           ;  48: ADC1 Sequence 0
                DCD     IntDefaultHandler           ;  49: ADC1 Sequence 1
                DCD     IntDefaultHandler           ;  50: ADC1 Sequence 2
                DCD     IntDefaultHandler           ;  51: ADC1 Sequence 3
                DCD     0                         ;  52: Reserved
                DCD     0                         ;  53: Reserved
                DCD     IntDefaultHandler             ;  54: GPIO Port J
                DCD     IntDefaultHandler             ;  55: GPIO Port K
                DCD     IntDefaultHandler             ;  56: GPIO Port L
                DCD     IntDefaultHandler              ;  57: SSI2 Rx and Tx
                DCD     IntDefaultHandler              ;  58: SSI3 Rx and Tx
                DCD     IntDefaultHandler             ;  59: UART3 Rx and Tx
                DCD     IntDefaultHandler             ;  60: UART4 Rx and Tx
                DCD     IntDefaultHandler             ;  61: UART5 Rx and Tx
                DCD     IntDefaultHandler             ;  62: UART6 Rx and Tx
                DCD     IntDefaultHandler             ;  63: UART7 Rx and Tx
                DCD     0                         ;  64: Reserved
                DCD     0                         ;  65: Reserved
                DCD     0                         ;  66: Reserved
                DCD     0                         ;  67: Reserved
                DCD     IntDefaultHandler              ;  68: I2C2 Master and Slave
                DCD     IntDefaultHandler              ;  69: I2C3 Master and Slave
                DCD     IntDefaultHandler           ;  70: Timer 4 subtimer A
                DCD     IntDefaultHandler           ;  71: Timer 4 subtimer B
                DCD     0                         ;  72: Reserved
                DCD     0                         ;  73: Reserved
                DCD     0                         ;  74: Reserved
                DCD     0                         ;  75: Reserved
                DCD     0                         ;  76: Reserved
                DCD     0                         ;  77: Reserved
                DCD     0                         ;  78: Reserved
                DCD     0                         ;  79: Reserved
                DCD     0                         ;  80: Reserved
                DCD     0                         ;  81: Reserved
                DCD     0                         ;  82: Reserved
                DCD     0                         ;  83: Reserved
                DCD     0                         ;  84: Reserved
                DCD     0                         ;  85: Reserved
                DCD     0                         ;  86: Reserved
                DCD     0                         ;  87: Reserved
                DCD     0                         ;  88: Reserved
                DCD     0                         ;  89: Reserved
                DCD     0                         ;  90: Reserved
                DCD     0                         ;  91: Reserved
                DCD     IntDefaultHandler           ;  92: Timer 5 subtimer A
                DCD     IntDefaultHandler           ;  93: Timer 5 subtimer B
                DCD     IntDefaultHandler          ;  94: Wide Timer 0 subtimer A
                DCD     IntDefaultHandler          ;  95: Wide Timer 0 subtimer B
                DCD     IntDefaultHandler          ;  96: Wide Timer 1 subtimer A
                DCD     IntDefaultHandler          ;  97: Wide Timer 1 subtimer B
                DCD     IntDefaultHandler          ;  98: Wide Timer 2 subtimer A
                DCD     IntDefaultHandler          ;  99: Wide Timer 2 subtimer B
                DCD     IntDefaultHandler          ; 100: Wide Timer 3 subtimer A
                DCD     IntDefaultHandler          ; 101: Wide Timer 3 subtimer B
                DCD     IntDefaultHandler          ; 102: Wide Timer 4 subtimer A
                DCD     IntDefaultHandler          ; 103: Wide Timer 4 subtimer B
                DCD     IntDefaultHandler          ; 104: Wide Timer 5 subtimer A
                DCD     IntDefaultHandler          ; 105: Wide Timer 5 subtimer B
                DCD     IntDefaultHandler               ; 106: FPU
                DCD     0                         ; 107: Reserved
                DCD     0                         ; 108: Reserved
                DCD     IntDefaultHandler              ; 109: I2C4 Master and Slave
                DCD     IntDefaultHandler              ; 110: I2C5 Master and Slave
                DCD     IntDefaultHandler             ; 111: GPIO Port M
                DCD     IntDefaultHandler             ; 112: GPIO Port N
                DCD     IntDefaultHandler              ; 113: Quadrature Encoder 2
                DCD     0                         ; 114: Reserved
                DCD     0                         ; 115: Reserved
                DCD     IntDefaultHandler            ; 116: GPIO Port P (Summary or P0)
                DCD     IntDefaultHandler            ; 117: GPIO Port P1
                DCD     IntDefaultHandler            ; 118: GPIO Port P2
                DCD     IntDefaultHandler            ; 119: GPIO Port P3
                DCD     IntDefaultHandler            ; 120: GPIO Port P4
                DCD     IntDefaultHandler            ; 121: GPIO Port P5
                DCD     IntDefaultHandler            ; 122: GPIO Port P6
                DCD     IntDefaultHandler            ; 123: GPIO Port P7
                DCD     IntDefaultHandler            ; 124: GPIO Port Q (Summary or Q0)
                DCD     IntDefaultHandler            ; 125: GPIO Port Q1
                DCD     IntDefaultHandler            ; 126: GPIO Port Q2
                DCD     IntDefaultHandler            ; 127: GPIO Port Q3
                DCD     IntDefaultHandler            ; 128: GPIO Port Q4
                DCD     IntDefaultHandler            ; 129: GPIO Port Q5
                DCD     IntDefaultHandler            ; 130: GPIO Port Q6
                DCD     IntDefaultHandler            ; 131: GPIO Port Q7
                DCD     IntDefaultHandler             ; 132: GPIO Port R
                DCD     IntDefaultHandler             ; 133: GPIO Port S
                DCD     IntDefaultHandler            ; 134: PWM 1 Generator 0
                DCD     IntDefaultHandler            ; 135: PWM 1 Generator 1
                DCD     IntDefaultHandler            ; 136: PWM 1 Generator 2
                DCD     IntDefaultHandler            ; 137: PWM 1 Generator 3
                DCD     IntDefaultHandler        ; 138: PWM 1 Fault

__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors
	
	
;******************************************************************************
;
; Initialize the processor by copying the boot loader from flash to SRAM, zero
; filling the .bss section, and moving the vector table to the beginning of
; SRAM.  The return address is modified to point to the SRAM copy of the boot
; loader instead of the flash copy, resulting in a branch to the copy now in
; SRAM.
;
;******************************************************************************
    export  ProcessorInit
ProcessorInit
    ;
    ; Copy the code image from flash to SRAM.
    ;
    movs    r0, #0x0000
    movs    r1, #0x0000           ; raise Z F V flags if result has them
    movt    r1, #0x2000           ;move upper 16-bits
    import  ||Image$$SRAM$$ZI$$Base||
    ldr     r2, =||Image$$SRAM$$ZI$$Base||
copy_loop
        ldr     r3, [r0], #4    
        str     r3, [r1], #4    
        cmp     r1, r2          
        blt     copy_loop       

    ;
    ; Zero fill the .bss section.
    ;
	import  ||Image$$SRAM$$ZI$$Base||
    ldr     r1, =||Image$$SRAM$$ZI$$Base||
    movs    r0, #0x0000
    import  ||Image$$SRAM$$ZI$$Limit||
    ldr     r2, =||Image$$SRAM$$ZI$$Limit||
zero_loop
        str     r0, [r1], #4
        cmp     r1, r2
        blt     zero_loop

    ;
    ; Set the vector table pointer to the beginning of SRAM.
    ;
   movw    r0, #(NVIC_VTABLE & 0xffff)
   movt    r0, #(NVIC_VTABLE >> 16)
   movs    r1, #0x0000
   movt    r1, #0x2000
   str     r1, [r0]

    ;
    ; Return to the caller.
    ;
    bx      lr	
	
	    export  Reset_Handler
Reset_Handler

    ;
    ; Enable the floating-point unit.  This must be done here in case any
    ; later C functions use floating point.  Note that some toolchains will
    ; use the FPU registers for general workspace even if no explicit floating
    ; point data types are in use.
    ;
    movw    r0, #0xED88
    movt    r0, #0xE000
    ldr     r1, [r0]
    orr     r1, #0x00F00000
    str     r1, [r0]
   
    ;
    ; Initialize the processor.
    ;
    bl      ProcessorInit

    ;
    ; Branch to the SRAM copy of the reset handler.
    ;
    ldr     pc, =Reset_Handler_In_SRAM
	
;******************************************************************************
;
; The NMI handler.
;
;******************************************************************************
NMI_Handler
    if      :def:_ENABLE_MOSCFAIL_HANDLER
    ;
    ; Grab the fault frame from the stack (the stack will be cleared by the
    ; processor initialization that follows).
    ;
    ldm     sp, {r4-r11}
    mov     r12, lr

    ;
    ; Initialize the processor.
    ;
    bl      ProcessorInit

    ;
    ; Branch to the SRAM copy of the NMI handler.
    ;
    ldr     pc, =NmiSR_In_SRAM
    else
    ;
    ; Loop forever since there is nothing that we can do about a NMI.
    ;
    B       .
    endif
		
;******************************************************************************
;
; The hard fault handler.
;
;******************************************************************************	
UART0_Handler\
                PROC
                EXPORT  UART0_Handler [WEAK]
                B       .
                ENDP
					
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP

	;******************************************************************************
;
; This portion of the file goes into the text section.
;
;******************************************************************************
    align   4
    area    ||.text||, code, readonly, align=2

Reset_Handler_In_SRAM
    ;
    ; Call the user-supplied low level hardware initialization function
    ; if provided.
    ;
   
    import  INIT_EEPROM
    bl      INIT_EEPROM
  

    ;
    ; See if an update should be performed.
    ;
   import  CheckForceUpdate
    bl     CheckForceUpdate
   ; cbz     r0, CallApplication

    ;
    ; Configure the microcontroller.
    ;
EnterBootLoader
    if      :def:_ENET_ENABLE_UPDATE
    import  ConfigureEnet
    bl      ConfigureEnet
    elif    :def:_CAN_ENABLE_UPDATE
    import  ConfigureCAN
    bl      ConfigureCAN
    elif    :def:_USB_ENABLE_UPDATE
    import  ConfigureUSB
    bl      ConfigureUSB
    else
  ;  import  ConfigureDevice     ;init clock and UART
  ;  bl      ConfigureDevice
	import Bl_main
	bl    Bl_main	
    endif


    ;
    ; This is a second symbol to allow starting the application from the boot
    ; loader the linker may not like the perceived jump.
    ;
    export StartApplication
StartApplication
    ;
    ; Call the application via the reset handler in its vector table.  Load the
    ; address of the application vector table.
    ;
CallApplication
    ;
    ; Copy the application's vector table to the target address if necessary.
    ; Note that incorrect boot loader configuration could cause this to
    ; corrupt the code!  Setting VTABLE_START_ADDRESS to 0x20000000 (the start
    ; of SRAM) is safe since this will use the same memory that the boot loader
    ; already uses for its vector table.  Great care will have to be taken if
    ; other addresses are to be used.
    ;
  ; if (_APP_START_ADDRESS != _VTABLE_START_ADDRESS)
  ;  movw    r0, #(_VTABLE_START_ADDRESS & 0xffff)
  ;  if (_VTABLE_START_ADDRESS > 0xffff)
  ;  movt    r0, #(_VTABLE_START_ADDRESS >> 16)
  ;  endif
  ;  movw    r1, #(_APP_START_ADDRESS & 0xffff)
  ;  if (_APP_START_ADDRESS > 0xffff)
  ;  movt    r1, #(_APP_START_ADDRESS >> 16)
  ;  endif*/

    ;
    ; Calculate the end address of the vector table assuming that it has the
    ; maximum possible number of vectors.  We don't know how many the app has
    ; populated so this is the safest approach though it may copy some non
    ; vector data if the app table is smaller than the maximum.
    ;
 ;   movw    r2, #(70 * 4)
 ;   adds    r2, r2, r0
;VectorCopyLoop
 ;       ldr     r3, [r1], #4
 ;       str     r3, [r0], #4
 ;       cmp     r0, r2
 ;       blt     VectorCopyLoop
 ;   endif*/

    ;
    ; Set the vector table address to the beginning of the application.
	; the beginning of the application is an input parameter saved in r0 register
    ;
 ;   /*movw    r0, #(_VTABLE_START_ADDRESS & 0xffff)
 ;   if (_VTABLE_START_ADDRESS > 0xffff)
 ;   movt    r0, #(_VTABLE_START_ADDRESS >> 16)
 ;   endif*/
    movw    r1, #(NVIC_VTABLE & 0xffff)
    movt    r1, #(NVIC_VTABLE >> 16)
    str     r0, [r1]

    ;
    ; Load the stack pointer from the application's vector table.
    ;
; /*   if (_APP_START_ADDRESS != _VTABLE_START_ADDRESS)
;    movw    r0, #(_APP_START_ADDRESS & 0xffff)
;    if (_APP_START_ADDRESS > 0xffff)
;    movt    r0, #(_APP_START_ADDRESS >> 16)
;    endif
;    endif*/
    ldr     sp, [r0]

    ;
    ; Load the initial PC from the application's vector table and branch to
    ; the application's entry point.
    ;
	ADD     r0,#4
    ldr     pc,[r0]
	
;******************************************************************************
;
; The default interrupt handler.
;
;******************************************************************************
IntDefaultHandler
    ;
    ; Loop forever since there is nothing that we can do about an unexpected
    ; interrupt.
    ;
    b       .	

;******************************************************************************
;
; Provides a small delay.  The loop below takes 3 cycles/loop.
;
;******************************************************************************
ALIGN				;**************************
        EXPORT  DisableInterrupts
        EXPORT  EnableInterrupts
	    EXPORT SysCtlDelay
    export  Delay
Delay
    subs    r0, #1
    bne     Delay
    bx      lr
	

SysCtlDelay
 subs    r0, #1;
    bne     SysCtlDelay;
    bx      lr;
	
			;*********** DisableInterrupts ***************
; disable interrupts
; inputs:  none
; outputs: none
DisableInterrupts
        CPSID  I
        BX     LR

;*********** EnableInterrupts ***************
; disable interrupts
; inputs:  none
; outputs: none
EnableInterrupts
        CPSIE  I
        BX     LR
;******************************************************************************
;
; This is the end of the file.
;
;******************************************************************************
    align 4  
		
    END