#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "remote.h"
#include "Lab9.h"


#define FREQ_IN_MASK 16

uint32_t  frequency = 0;
uint32_t  time[50];
uint32_t  time2[33];
uint8_t   count = 0;
bool valid = 0;
uint32_t num = 0;
uint32_t   T = 22500;
char buffer[44][15] = {
                             "Power",
                             "Triangle line",
                             "Light Down",
                             "Light Up",
                             "2 1",
                             "2 2",
                             "2 3",
                             "2 4",
                             "3 1",
                             "3 2",
                             "3 3",
                             "3 4",
                             "4 1",
                             "4 2",
                             "4 3",
                             "4 4",
                             "5 1",
                             "5 2",
                             "5 3",
                             "5 4",
                             "6 1",
                             "6 2",
                             "6 3",
                             "6 4",
                             "Red Up",
                             "Green Up",
                             "Blue Up",
                             "Quick",
                             "Red Down",
                             "Green Down",
                             "Blue Down",
                             "Slow",
                             "DIY1",
                             "DIY2",
                             "DIY3",
                             "AUTO",
                             "DIY4",
                             "DIY5",
                             "DIY6",
                             "FLASH",
                             "JUMP3",
                             "JUMP7",
                             "FADE3",
                             "FADE7"} ;

uint32_t fieldData[44][32] = {
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,1,1,1,1,1,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,1,0,0,0,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,0,1,0,1,1,0,0,0,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,0,1,0,1,1,1,0,0,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,1,0,1,1,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,0,0,0,1,0,1,1,0,1,1,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,1,0,0,1,0,0,1,1,0,1,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,0,0,1,0,1,1,1,0,1,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,0,1,0,1,1,1,1,0,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,1,0,1,0,0,1,1,1,0,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1,0,0,1,0,0,1,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,0,0,0,1,1,0,0,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,0,0,0,1,0,0,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,1,1,1,0,0,0,1,0,0,0,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,1,1,0,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,0,1,1,0,0,0,1,0,1,0,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,1,0,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,0,1,0,0,0,1,1,0,1,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,0,0,0,1,0,1,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,0,0,0,1,1,1,1,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,1,0,0,0,0,1,1,1,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,1,1,0,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,1,1,0,0,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,1,0,0,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,1,1,0,0,0,0,1,0,0,0,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,0,0,0,0,1,1,1,0,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,1,1,0,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,0,1,0,0,0,0,1,0,1,0,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,0,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,1,1,0,1,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,0,1,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,1,0,0,1,1,1,1,1},
                                    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1}
    };

/*char field_hex[44][5] = {
                             "40BF",
                             "41BE",
                             "5DA2",
                             "5CA3",
                             "58A7",
                             "59A6",
                             "45BA",
                             "44BB",
                             "54AB",
                             "55AA",
                             "49B6",
                             "48B7",
                             "50AF",
                             "51AE",
                             "4DB2",
                             "4CB3",
                             "1CE3",
                             "1DE2",
                             "1EE1",
                             "1FE0",
                             "18E7",
                             "19E6",
                             "1AE5",
                             "1BE4",
                             "14EB",
                             "15EA",
                             "16E9",
                             "17E8",
                             "10EF",
                             "11EE",
                             "12ED",
                             "13EC",
                             "CF3",
                             "DF2",
                             "EF1",
                             "FF0",
                             "8F7",
                             "9F6",
                             "AF5",
                             "BF4",
                             "4FB",
                             "5FA",
                             "6F9",
                             "7F8"} ;*/



void initRemote()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0| SYSCTL_RCGCWTIMER_R1; //WTIMER2 TODO change to correct timer

    _delay_cycles(3);

    // Configure SIGNAL_IN for frequency and time measurements
    GPIO_PORTC_AFSEL_R |= FREQ_IN_MASK;              // select alternative functions for SIGNAL_IN pin
    GPIO_PORTC_PCTL_R &= ~GPIO_PCTL_PC4_M;           // map alt fns to SIGNAL_IN
    GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC4_WT0CCP0;
    GPIO_PORTC_DEN_R |= FREQ_IN_MASK;                // enable bit 6 for digital input
}


void enableTimerMode()
{
    WTIMER0_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER0_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER0_TAMR_R = TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge time mode, count up
    WTIMER0_CTL_R = TIMER_CTL_TAEVENT_NEG;           // measure time from negative edge to negative edge
    WTIMER0_IMR_R = TIMER_IMR_CAEIM;                 // turn-on interrupts
    WTIMER0_TAV_R = 0;                               // zero counter for first period
    WTIMER0_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
    NVIC_EN2_R |= 1 << (INT_WTIMER0A - 16 - 64);         // turn-on interrupt 112 (WTIMER1A)
}


float convert(uint32_t num)
{
    float num2 = num * (25/1000000);
    return num2;
}

void gpiolsr()
{
    //T = 1.125ms/2;
    uint8_t i , j, val;
    char value[30];
    if (WTIMER0_TAV_R > 80 * 40000) count = 0;

    if(count == 0)
    {
        WTIMER0_TAV_R = 0;
    }
    time[count] = WTIMER0_TAV_R;

    if(count == 0)
    {
        count++;
    }
    else if(count == 1)
    {
        if(((time[1] - time[0]) > 520000) && ((time[1] - time[0]) < 560000))
        {
            count++;
        }
        else
        {
            count = 0;
        }
    }
    else if(count > 1)
    {
        uint32_t lilT = (time[count]-time[count-1]);
        if ((((lilT) > (1.5*T)) && ((lilT) < (2.5*T))) || (((lilT) > (3.5*T)) && ((lilT) < (4.5*T))))
        {
            count++;
        }
        else
        {
            count = 0;
        }
    }

    WTIMER0_ICR_R = TIMER_ICR_CAECINT;

    if (count == 34)
    {
        count = 0;
        for (count = 1; count < 34; count++)
        {
            uint32_t lilT = (time[count]-time[count-1]);
            if ((((lilT) > (1.5*T)) && ((lilT) < (2.5*T))))
            {

                time2[count-2] = 0;
                //putcUart0('0');


            }
            else if ((((lilT) > (3.5*T)) && ((lilT) < (4.5*T))))
            {

                time2[count-2] = 1;
                //putcUart0('1');


            }
        }
        valid = 1;
        for (count = 0; count <= 7 && valid; count++)
        {
            if (time2[count]^time2[count+8] != 1)
            {
                valid = 0;
            }
            else
            {
                valid = 1;
            }

        }
        for (count = 15; count <= 23 && valid; count++)
        {
            if (time2[count] ^ time2[count+8] != 1)
            {
                valid = 0;
            }
            else
            {
                valid = 1;
            }

        }
        putcUart0('\n');

    waitMicrosecond(500000);
    if(valid)
    {
        for(j = 0; j < 44; j++)
                 {
                     for(i = 0; i < 32; i++)
                     {
                         if (time2[i] != fieldData[j][i])
                         {
                             break;
                         }
                         else
                         {
                             val = 1;
                             if(i == 31)
                             {
                                 p = j;
                             }
                         }
                     }
                 }
                 i = 0;
                 valid = false;
                 if(val == 1)
                 {
                     switch(p)
                     {
                     case(0):
                             //homing
                             putsUart0("Homing...\n");
                             homing();
                             break;
                     case(1):
                             //calibrate
                             putsUart0("Calibrating...\n");
                             calibrate();
                             break;
                     case(2):
                             //calibrate all
                             putsUart0("Calibrating All");
                             calibrate_raw_all();
                             break;
                     case(3):
                              //measure all
                              putsUart0("Measuring All...\n");
                              for(i = 1; i < 6 ; i++)
                              {
                                 measure(i,&red, &green,&blue);

                                 sprintf(value, "Tube%3u\n(%5u,%5u,%5u)\n",i, red, green, blue);
                                 putsUart0(value);
                                 measure_pH(i,red, green,blue);
                              }
                              break;
                     case(4):
                             putsUart0("Measuring 5 Raw...\n");
                             measure(5,&red, &green,&blue);
                             sprintf(value, "(%5u,%5u,%5u)\n", red, green, blue);
                             putsUart0(value);
                             break;
                     case(5):
                             putsUart0("Measuring 5 pH...\n");

                             measure(5,&red, &green,&blue);
                             measure_pH(5,red, green,blue);
        #ifdef DEBUG
                             sprintf(value, "R: %5u, G: %5u, B: %5u\n", red, green, blue);
                             putsUart0(value);
        #endif
                             break;
                     case(8):
                              putsUart0("Measuring 4 Raw...\n");
                              measure(4,&red, &green,&blue);
                              sprintf(value, "(%5u,%5u,%5u)\n", red, green, blue);
                              putsUart0(value);
                              break;
                     case(9):
                              putsUart0("Measuring 4 pH...\n");
                              measure(4,&red, &green,&blue);
                              measure_pH(4,red, green,blue);
         #ifdef DEBUG
                              sprintf(value, "R: %5u, G: %5u, B: %5u\n", red, green, blue);
                              putsUart0(value);
         #endif
                              break;
                     case(12):
                             putsUart0("Measuring 3 Raw...\n");
                              measure(3,&red, &green,&blue);
                              sprintf(value, "(%5u,%5u,%5u)\n", red, green, blue);
                              putsUart0(value);
                              break;
                     case(13):
                              putsUart0("Measuring 3 pH...\n");
                              measure(3,&red, &green,&blue);
                              measure_pH(3,red, green,blue);
         #ifdef DEBUG
                              sprintf(value, "R: %5u, G: %5u, B: %u\n", red, green, blue);
                              putsUart0(value);
         #endif
                              break;
                     case(16):
                              putsUart0("Measuring 2 Raw...\n");
                              measure(2,&red, &green,&blue);
                              sprintf(value, "(%5u,%5u,%5u)\n", red, green, blue);
                              putsUart0(value);
                              break;
                     case(17):
                              putsUart0("Measuring 2 pH...\n");
                              measure(2,&red, &green,&blue);
                              measure_pH(2,red, green,blue);
         #ifdef DEBUG
                              sprintf(value, "R: %5u, G: %5u, B: %5u\n", red, green, blue);
                              putsUart0(value);
         #endif
                              break;
                     case(20):
                              putsUart0("Measuring 1 Raw...\n");
                              measure(1,&red, &green,&blue);
                              sprintf(value, "(%5u,%5u,%5u)\n", red, green, blue);
                              putsUart0(value);
                              break;
                     case(21):
                              putsUart0("Measuring 1 pH...\n");
                              measure(1,&red, &green,&blue);
                              measure_pH(1,red, green,blue);
         #ifdef DEBUG
                              sprintf(value, "R: %5u, G: %5u, B: %5u\n", red, green, blue);
                              putsUart0(value);
         #endif
                              break;
                     case(24):
                             pickTube(0);
                             break;
                     case(25):
                             pickTube(1);
                             break;
                     case(26):
                             pickTube(2);
                             break;
                     case(28):
                             pickTube(3);
                             break;
                     case(29):
                             pickTube(4);
                             break;
                     case(30):
                             pickTube(5);
                             break;
                     case(43):
                            for(i = 1; i < 6 ; i++)
                            {
                                measure(i,&red, &green,&blue);

                                sprintf(value, "Tube%u\n%6u\t%6u\t%6u\n",i, red, green, blue);
                                putsUart0(value);
                                //measure_pH(i,red, green,blue);
                            }
                            break;

                     }
                 }
        }
    }
}
