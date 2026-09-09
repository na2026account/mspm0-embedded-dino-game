/*
 * Switch.c
 *
 *  Created on: January 12, 2026
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"


// LaunchPad.h defines all the indices into the PINCM table
// PA17 and PA18 Indices are defined in LaunchPad.h (usually PA17INDEX and PA18INDEX)

// Initialize PA17 and PA18 as inputs with internal pull-up resistors
void Switch_Init(void){
    // 1. Standard Pin configuration (Input with Pull-up)
    IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00050081; 
    IOMUX->SECCFG.PINCM[PA18INDEX] = 0x00050081;

    // 2. Configure Interrupts for Falling Edge
    GPIOA->POLARITY31_16 = 0x00000028; // Set PA17 and PA18 to Falling Edge
    GPIOA->CPU_INT.IMASK = (1<<17) | (1<<18); // Enable these pins in GPIO
    
    // 3. Enable Group 1 Interrupts in the NVIC (Nested Vectored Interrupt Controller)
    NVIC_EnableIRQ(GPIOA_INT_IRQn); 
    NVIC_SetPriority(GPIOA_INT_IRQn, 2); // Set a medium priority
}
// return current state of switches
uint32_t Switch_In(void){
    // Mask for bit 17 (0x20000) and bit 18 (0x40000)
    // 0x00060000 = (1<<17) | (1<<18)
    return (GPIOA->DIN31_0 & 0x00060000); 
}
