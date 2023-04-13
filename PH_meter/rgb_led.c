// RGB LED Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL with LCD Interface
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red Backlight LED:
//   M1PWM5 (PF1) drives an NPN transistor that powers the red LED
// Green Backlight LED:
//   M1PWM7 (PF3) drives an NPN transistor that powers the green LED
// Blue Backlight LED:
//   M1PWM6 (PF2) drives an NPN transistor that powers the blue LED

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <rgb_led.h>
#include <stdint.h>
#include "tm4c123gh6pm.h"

// PortF masks
#define RED_LED_MASK 2
#define BLUE_LED_MASK 32
#define GREEN_LED_MASK 1

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize RGB
void initRgb()
{
    // Enable clocks
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1 | SYSCTL_RCGCPWM_R0; //|PB5|, M0, PWM3, GEN1, R0 ---- BLUE
                                                               //|PD0|, M1, PWM0, GEN0, R0 ---- GREEN
                                                               //|PD1|, M1, PWM1, GEN0, R1 ---- RED

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1| SYSCTL_RCGCGPIO_R3;
    _delay_cycles(3);

    // Configure three LEDs
    GPIO_PORTD_DEN_R |= RED_LED_MASK | GREEN_LED_MASK ;
    GPIO_PORTD_AFSEL_R |= RED_LED_MASK | GREEN_LED_MASK ;
    GPIO_PORTD_PCTL_R &= ~(GPIO_PCTL_PD0_M | GPIO_PCTL_PD1_M );
    GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD0_M1PWM0 | GPIO_PCTL_PD1_M1PWM1;

    GPIO_PORTB_DEN_R |= BLUE_LED_MASK ;
    GPIO_PORTB_AFSEL_R |= BLUE_LED_MASK ;
    GPIO_PORTB_PCTL_R &= ~(GPIO_PCTL_PB5_M );
    GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB5_M0PWM3 ;


    // Configure PWM module 1 to drive RGB LED
    // RED   on M1PWM5 (PF1), M1PWM2b
    // BLUE  on M1PWM6 (PF2), M1PWM3a
    // GREEN on M1PWM7 (PF3), M1PWM3b
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R1;                // reset PWM1 module
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R0;                // reset PWM0 module
    SYSCTL_SRPWM_R = 0;                              // leave reset state
    PWM0_1_CTL_R = 0;                                // turn-off PWM0 generator 1 (drives outs 1 and 2)
    PWM1_0_CTL_R = 0;                                // turn-off PWM1 generator 0 (drives outs 3)
    PWM0_1_GENB_R = PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTLOAD_ZERO;
                                                     // output 3 on PWM0, gen 1B, cmpb
    PWM1_0_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;
                                                     // output 0 on PWM1, gen 0a, cmpa
    PWM1_0_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;
                                                     // output 1 on PWM1, gen 0b, cmpb

    PWM0_1_LOAD_R = 4032;                            // set frequency to 40 MHz sys clock / 2 / 4096 = 19.53125 kHz
    PWM1_0_LOAD_R = 4032;

    PWM0_1_CMPB_R = 0;                               // red off (0=always low, 1023=always high)
    PWM1_0_CMPB_R = 0;                               // green off
    PWM1_0_CMPA_R = 0;                               // blue off

    PWM0_1_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM1 generator 2
    PWM1_0_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM0 generator 3
    PWM1_ENABLE_R = PWM_ENABLE_PWM0EN | PWM_ENABLE_PWM1EN;
    PWM0_ENABLE_R = PWM_ENABLE_PWM3EN;
                                                     // enable outputs
}

void setRgbColor(uint16_t red, uint16_t green, uint16_t blue)
{
    PWM0_1_CMPB_R = blue;
    PWM1_0_CMPA_R = green;
    PWM1_0_CMPB_R = red;
}


