// Sound.c
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// your name
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC5.h"
#include "../inc/Timer.h"

const uint8_t *SoundPtr;
uint32_t SoundCount = 0;

void SysTick_IntArm(uint32_t period, uint32_t priority){
SysTick->CTRL = 0;           // disable SysTick during setup
    SysTick->LOAD = period - 1;  // reload value
    SysTick->VAL = 0;            // any write to current clears it
    SCB->SHP[1] = (SCB->SHP[1] & 0x00FFFFFF) | (priority << 24); // set priority
    SysTick->CTRL = 0x07;        // enable SysTick with core clock and interrupts
}
// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5-bit DAC
void Sound_Init(void){
DAC5_Init();      // Initialize your 5-bit DAC pins
    SoundCount = 0;   // No sound playing initially
    // 80MHz / 11025Hz = 7256 cycles for the period
    SysTick_IntArm(7256, 2); 
}
void SysTick_Handler(void){ // called at 11 kHz
if(SoundCount > 0){
        // Output current sample to DAC
        // If your sound data is 8-bit, shift right by 3 for 5-bit DAC
        DAC5_Out((*SoundPtr) >> 3); 
        SoundPtr++;     // Point to next sample
        SoundCount--;   // Decrement remaining samples
    } else {
        DAC5_Out(0);    // Output 0 (silence) when done
    }
}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count){
    SoundPtr = pt;      // Set the start of the audio array
    SoundCount = count; // Set how many samples to play
}

// Trigger functions for your game events
void Sound_Shoot(void){
    Sound_Start(shoot, 4080);
}

void Sound_Killed(void){
    Sound_Start(invaderkilled, 3377); // Assuming names in sounds.h
}

void Sound_Explosion(void){
    Sound_Start(explosion, 2000); 
}

// Example for other sounds
void Sound_Fastinvader1(void){
    Sound_Start(fastinvader1, 982);
}