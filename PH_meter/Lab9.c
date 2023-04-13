//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "rgb_led.h"
#include "adc0.h"
#include "remote.h"
#include "Lab9.h"

#define BLACK_WIRE         (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 7*4))) //PA7
#define WHITE_WIRE         (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 6*4))) //PA6 --- 2
#define GREEN_WIRE         (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 5*4))) //PA5
#define YELLOW_WIRE        (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 4*4))) //PB4
#define AIN5_MASK 1


// PortA masks
#define BLACK_MASK  128
#define WHITE_MASK  64
#define GREEN_MASK  32
#define YELLOW_MASK 16
//#define DEBUG
//-----------------------------------------------------------------------------
// the maximum number of characters that can be accepted from the user and the structure for holding UI information
//-----------------------------------------------------------------------------

#define MAX_CHARS 80
#define MAX_FIELDS 5

typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------



uint8_t phase = 1;
uint16_t position = 199;
uint16_t pwm_r, pwm_g, pwm_b, raw, CAL_Red, CAL_Green, CAL_Blue;
char str[100];
uint16_t pHraw_r[6] = {3144, 1895, 1399, 2787, 1459, 1239};
uint16_t pHraw_g[6] = {2889, 2133, 1271, 1630, 230, 402};
uint16_t pHraw_b[6] = {2950, 413, 619, 230, 487, 714};
float pHvalues[7] = {0.0, 6.83, 7.21, 7.5, 7.801, 8.27, 0.0};
bool v, val, kb;
uint16_t posIn = 192;
uint8_t posnum, i, j , p;
uint16_t red, green, blue;


// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks  PA4-7

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0 | SYSCTL_RCGCGPIO_R1 | SYSCTL_RCGCGPIO_R3 | SYSCTL_RCGCGPIO_R5;  //PA
    _delay_cycles(3);

    // Configure Motor Function
    GPIO_PORTA_DIR_R |= BLACK_MASK | WHITE_MASK| GREEN_MASK;   // outputs

    GPIO_PORTB_DIR_R |= YELLOW_MASK;
    GPIO_PORTB_DR2R_R |= YELLOW_MASK;


    GPIO_PORTA_DR2R_R |= BLACK_MASK | WHITE_MASK| GREEN_MASK;  // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTA_DEN_R |=  BLACK_MASK| WHITE_MASK| GREEN_MASK;  // enable LEDs and pushbuttons
    GPIO_PORTB_DEN_R |= YELLOW_MASK;


    GPIO_PORTD_AFSEL_R |= AIN5_MASK;                 // select alternative functions for AN3 (PE0)
    GPIO_PORTD_DEN_R &= ~AIN5_MASK;                  // turn off digital operation on pin PD2
    GPIO_PORTD_AMSEL_R |= AIN5_MASK;

}







// Function to swap two numbers
void swap(char *x, char *y)
{
    char t = *x; *x = *y; *y = t;
}

// Function to reverse `buffer[i…j]`
char* reverse(char *buffer, int i, int j)
{
    while (i < j) {
        swap(&buffer[i++], &buffer[j--]);
    }

    return buffer;
}

// Iterative function to implement `itoa()` function in C
char* itoa(int value, char* buffer, int base)
{
    // invalid input
    if (base < 2 || base > 32) {
        return buffer;
    }

    // consider the absolute value of the number
    int n = abs(value);

    int i = 0;
    while (n)
    {
        int r = n % base;

        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        }
        else {
            buffer[i++] = 48 + r;
        }

        n = n / base;
    }

    // if the number is 0
    if (i == 0) {
        buffer[i++] = '0';
    }

    // If the base is 10 and the value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0'; // null terminate string

    // reverse the string and return it
    return reverse(buffer, 0, i - 1);
}



//-----------------------------------------------------------------------------
// getsUart0
//-----------------------------------------------------------------------------

void getsUart0(USER_DATA *d)
{
    int count = 0;

    while(true)
    {
        char c = getcUart0();
        if(c == 8 || c == 127 && count > 0)
        {
            count--;
        }
        else if (c == 13)
        {
            d->buffer[count] = '\0';
            return;
        }
        else if (c >= 32)
        {
            d->buffer[count] = c;
            count++;
            if (count == MAX_CHARS)
            {
                d->buffer[count] = '\0';
                return;
            }
        }
    }

}

//-----------------------------------------------------------------------------
// parseFields
//-----------------------------------------------------------------------------
void parseFields(USER_DATA *d)
{
    char prevtype = 'd';
    d->fieldCount = 0;
    uint8_t i;

    //set 1 2
    for(i = 0; i < MAX_CHARS && d->buffer[i] != '\0'; i++)
    {
        if (prevtype == 'd')
        {
            if((d->buffer[i] >= 65 && d->buffer[i] <= 90) || (d->buffer[i] >= 97 && d->buffer[i] <= 122)) // char
            {
               d->fieldPosition[d->fieldCount] = i;
               d->fieldType[d->fieldCount] = 'a';
               d->fieldCount += 1;
               prevtype = 'a';
            }
            else if ( d->buffer[i] >= 48 && d->buffer[i] <= 57|| d->buffer[i] == 46)  // number
            {
                d->fieldPosition[d->fieldCount] = i;
                d->fieldType[d->fieldCount] = 'n';
                d->fieldCount += 1;
                prevtype = 'n';
            }
            else
            {
                d->buffer[i] = '\0';
                prevtype = 'd';
            }
        }

        else if(d->buffer[i] ==32 || d->buffer[i] == 44 || d->buffer[i] == 124)
        {
            prevtype = 'd';
            d->buffer[i] = '\0';
        }
    }

}
//-----------------------------------------------------------------------------
// getFieldString
//-----------------------------------------------------------------------------
char* getFieldString(USER_DATA* d, uint8_t fieldNumber)
{
    //return the value of a field requested if the field number is in range
    if (fieldNumber <= d->fieldCount)
        {
            return &d->buffer[d->fieldPosition[fieldNumber]];
        }
    else
    {
        return 0;
    }

}

//-----------------------------------------------------------------------------
// String Compare
//-----------------------------------------------------------------------------

int strCmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}


//-----------------------------------------------------------------------------
// getFieldInteger
//-----------------------------------------------------------------------------
int32_t getFieldInteger(USER_DATA* d, uint8_t fieldNumber)
{

     char *word = getFieldString(d,fieldNumber);

    int num = atoi(word);

    return num;

}


//-----------------------------------------------------------------------------
// isCommand
//-----------------------------------------------------------------------------

bool isCommand(USER_DATA* d, const char* strCommand, uint8_t minArguments)
{

    if ( (d->fieldCount - 1) >= minArguments && ( strCmp(d->buffer, strCommand) == 0 ) )
    {
        return true;
    }
    else
        return false;

}






void applyPhase()
{
    /*  BWYG
     *  +-
     *    +-
     *  -+
     *    -+
     */
      switch(phase)
     {
     case 0:
         GREEN_WIRE = 0;
         YELLOW_WIRE = 0;
         BLACK_WIRE = 1;
         WHITE_WIRE = 0;


         break;
     case 1:
         BLACK_WIRE = 0;
         WHITE_WIRE = 0;
         GREEN_WIRE = 1;
         YELLOW_WIRE = 0;

         break;
     case 2:
         GREEN_WIRE = 0;
         YELLOW_WIRE = 0;
         BLACK_WIRE = 0;
         WHITE_WIRE = 1;


         break;
     case 3:
         BLACK_WIRE = 0;
         WHITE_WIRE = 0;
         GREEN_WIRE = 0;
         YELLOW_WIRE = 1;

         break;
     }
     //phase = phasein;
}

void stepCw()
{

    position ++;
    phase = (phase + 1) % 4;
    applyPhase();
    waitMicrosecond(16000);

}
void stepCCw()
{
    position --;
    if(phase == 0)
    {
        phase = 3;
    }
    else
    {
        phase = (phase - 1) % 4;
    }
    applyPhase();
    waitMicrosecond(16000);

}

void setPosition(uint16_t positionIn)
{
    while(position != positionIn)
    {
        if(position < positionIn)
        {
            stepCw();
        }
        else if (position > positionIn)
        {
            stepCCw();
        }
    }

}

void pickTube(uint8_t num)
{

    switch(num)
    {
    case 0:
        posIn = 195;
        setPosition(posIn);
        break;
    case 1:
        posIn = 29;
        setPosition(posIn);
        break;
    case 2:
        posIn = 63;
        setPosition(posIn);
        break;
    case 3:
        posIn = 96;
        setPosition(posIn);
        break;
    case 4:
        posIn = 130;
        setPosition(posIn);
        break;
    case 5:
        posIn = 163;
        setPosition(posIn);
        break;
    }

}

void homing()
{
    uint8_t i;
        for(i= 0; i < 199; i++)
        {
            stepCw();

        }
        position = 199;
        waitMicrosecond(100000);
        pickTube(0);
}

void calibrate()
{
    int16_t  r, dc;
    char rstr[30], gstr[30];

    pickTube(0);
    dc = 0;
    r = 0;
    while(dc < 4032 && r < 3072)
    {
        setRgbColor(dc, 0, 0);
        waitMicrosecond(10000);
        r = readAdc0Ss3();
        //waitMicrosecond(10000);
#ifdef DEBUG
        sprintf(str, "RED:\nDC: %4u\nR: %4u\n", dc, r);
        putsUart0(str);
#endif
        dc ++;
    }

    sprintf(rstr, "RED:\nDC: %4u\tR: %4u\n", dc/2, r);

    pwm_r = dc;  //~8??
    CAL_Red = r;
    dc = 0;
    r = 0;
    while(dc < 4032 && r < 3072)
    {
        setRgbColor(0, dc, 0);
        waitMicrosecond(10000);
        r = readAdc0Ss3();
        //waitMicrosecond(10000);
#ifdef DEBUG
        sprintf(str, "GREEN:\nDC: %4u\nR: %4u\n", dc, r);
        putsUart0(str);
#endif
        dc ++;
    }

    sprintf(gstr, "GREEN:\nDC: %4u\tR: %4u\n", dc, r);

    pwm_g = dc;  //~201??
    CAL_Green = r;
    dc = 0;
    r = 0;
    while(dc < 4032 && r < 3072)
    {
        setRgbColor(0, 0, dc);
        waitMicrosecond(10000);
        r = readAdc0Ss3();
        //waitMicrosecond(10000);
#ifdef DEBUG
        sprintf(str, "BLUE:\nDC: %4u\nR: %4u\n", dc, r);
        putsUart0(str);
#endif
        dc ++;
    }

    sprintf(str, "BLUE:\nDC: %4u\tR: %4u\n", dc, r);
    putsUart0(rstr);
    putsUart0(gstr);
    putsUart0(str);

    CAL_Blue = r;
    pwm_b = dc;  //~16??
    setRgbColor(0, 0, 0);

}

void measure(uint8_t tube, uint16_t *r, uint16_t *g, uint16_t *b)
{

    pickTube(tube);
    waitMicrosecond(1000000);
    setRgbColor(pwm_r, 0, 0);
    waitMicrosecond(100000);
    *r = readAdc0Ss3();
    waitMicrosecond(100000);
    setRgbColor(0, pwm_g, 0);
    waitMicrosecond(100000);
    *g = readAdc0Ss3();
    waitMicrosecond(100000);
    setRgbColor(0, 0, pwm_b);
    waitMicrosecond(100000);
    *b = readAdc0Ss3();
    waitMicrosecond(100000);
    setRgbColor(0, 0, 0);
}

void measure_pH(uint8_t tube, uint16_t r, uint16_t g, uint16_t b)
{
    float REL_Red, REL_Green, REL_Blue, calculated, norm, log;
    float pH = 0.0;
    REL_Red = (float)r / (float)CAL_Red;
    REL_Green = (float)g / (float)CAL_Green;
    REL_Blue = (float)b / (float)CAL_Blue;
    calculated = pow(REL_Green, 2) + pow(REL_Green * REL_Blue, 2) +(REL_Green * REL_Blue);
    //=Green^2+(Green*Blue)^2+(Green*Blue)
     norm = calculated / (REL_Red + REL_Green + REL_Blue);
    //cal/SUM(red, green, blue)
     log =  10*log10(norm);
    pH = (-0.1556*log) + 5.881;
    //-0.1556*X + 5.881
    sprintf(str, "Tube %u\npH: %.4f\n", tube, pH );
    putsUart0(str);
//    uint32_t euclid[7] = {0,0,0,0,0,0,0};
//    float er, eg, eb;
//    float pH = 0.0;
//    uint8_t i, q;
//    char str[50];
//    uint32_t lowest;
//    lowest = 999999999;
//    for(i = 1; i < 6; i++)
//    {
//        er = pow((r - pHraw_r[i] ), 2);
//        eg = pow((g - pHraw_g[i]), 2);
//        eb = pow((b - pHraw_b[i]), 2);
//        euclid[i] = er + eg + eb;
//#ifdef DEBUG
//        sprintf(str,"Euclid %u\n",euclid[i]);
//        putsUart0(str);
//#endif
//    }
//    for(i = 1; i < 6; i++)
//    {
//        if( euclid[i] < lowest)
//        {
//         lowest = euclid[i];
//         q = i;
//        }
//    }
//    if((euclid[q-1] <= euclid[q+1]) && euclid[q-1] != 0)
//    {
//
//        if(lowest < 50000)
//        {
//            pH = ((5.3*pHvalues[q]) + pHvalues[q - 1])/6;
//        }
//        else if(lowest < 100000)
//        {
//            pH = ((4.1*pHvalues[q]) + pHvalues[q - 1])/5;
//        }
//        else if(lowest < 300000)
//        {
//            pH = ((3.5*pHvalues[q]) + pHvalues[q - 1])/4;
//        }
//        else if(lowest < 800000)
//        {
//            pH = ((2.9*pHvalues[q]) + pHvalues[q - 1])/3;
//        }
//        else
//        {
//            pH = ((1*pHvalues[q]) + pHvalues[q - 1])/2;
//        }
//
//    }
//    else if ((euclid[q-1] >= euclid[q+1]) && euclid[q+1] != 0)
//    {
//
//
//        if(lowest < 50000)
//       {
//           pH = ((5.3*pHvalues[q]) + pHvalues[q + 1])/6;
//       }
//       else if(lowest < 100000)
//       {
//           pH = ((4.1*pHvalues[q]) + pHvalues[q + 1])/5;
//       }
//       else if(lowest < 300000)
//       {
//           pH = ((3.5*pHvalues[q]) + pHvalues[q + 1])/4;
//       }
//       else if(lowest < 800000)
//       {
//           pH = ((2.9*pHvalues[q]) + pHvalues[q + 1])/3;
//       }
//       else
//       {
//           pH = ((1*pHvalues[q]) + pHvalues[q + 1])/2;
//       }
//
//    }
//    else if (euclid[q-1] == 0)
//    {
//        pH = (((6.803) + (2*pHvalues[q])) / 3);
//    }
//    else if (euclid[q+1] == 0)
//    {
//        pH = ((8.2 + (2*pHvalues[q])) / 3);
//    }
//
//    sprintf(str, "Tube %u\npH: %.2f\n", tube, pH );
//    putsUart0(str);
//

}


void calibrate_raw_all()
{
    uint8_t i;
    uint16_t rr, rg, rb;
#ifdef DEBUG
    //char str[20];
#endif
    rr = 0;
    rg = 0;
    rb = 0;
    for(i = 1; i < 6; i++)
    {
        pickTube(1);
        setRgbColor(pwm_r, 0, 0);
        waitMicrosecond(500000);
        rr += readAdc0Ss3();
        putsUart0(".");

        setRgbColor(0, pwm_g, 0);
        waitMicrosecond(500000);
        rg += readAdc0Ss3();


        setRgbColor(0, 0, pwm_b);
        waitMicrosecond(500000);
        rb += readAdc0Ss3();
    }
    setRgbColor(0, 0, 0);
    rr = rr /5;
    pHraw_r[1] = (rr);
    rg = rg /5;
    pHraw_g[1] = (rg);
    rb = rb /5;
    pHraw_b[1] = (rb);
#ifdef DEBUG
    sprintf(str, "pHraw_r[1]: %u \n", pHraw_r[1] );
    putsUart0(str);
    sprintf(str, "pHraw_g[1]: %u \n", pHraw_g[1] );
    putsUart0(str);
    sprintf(str, "pHraw_b[1]: %u \n", pHraw_b[1] );
    putsUart0(str);
#endif
    for(i = 1; i < 6; i++)
    {
        pickTube(2);

        setRgbColor(pwm_r, 0, 0);
        waitMicrosecond(500000);
        rr += readAdc0Ss3();
        putsUart0(".");

        setRgbColor(0, pwm_g, 0);
        waitMicrosecond(500000);
        rg += readAdc0Ss3();


        setRgbColor(0, 0, pwm_b);
        waitMicrosecond(500000);
        rb += readAdc0Ss3();
    }
    setRgbColor(0, 0, 0);
    rr = rr /5;
    pHraw_r[2] = (rr);
    rg = rg /5;
    pHraw_g[2] = (rg);
    rb = rb /5;
    pHraw_b[2] = (rb);
#ifdef DEBUG
    sprintf(str, "pHraw_r[2]: %u \n", pHraw_r[2] );
    putsUart0(str);
    sprintf(str, "pHraw_g[2]: %u \n", pHraw_g[2] );
    putsUart0(str);
    sprintf(str, "pHraw_b[2]: %u \n", pHraw_b[2] );
    putsUart0(str);
#endif
    for(i = 1; i < 6; i++)
    {
        pickTube(3);

        setRgbColor(pwm_r, 0, 0);
        waitMicrosecond(500000);
        rr += readAdc0Ss3();
        putsUart0(".");

        setRgbColor(0, pwm_g, 0);
        waitMicrosecond(500000);
        rg += readAdc0Ss3();


        setRgbColor(0, 0, pwm_b);
        waitMicrosecond(500000);
        rb += readAdc0Ss3();
    }
    setRgbColor(0, 0, 0);
    rr = rr /5;
    pHraw_r[3] = (rr);
    rg = rg /5;
    pHraw_g[3] = (rg);
    rb = rb /5;
    pHraw_b[3] = (rb);
#ifdef DEBUG
    sprintf(str, "pHraw_r[3]: %u \n", pHraw_r[3] );
    putsUart0(str);
    sprintf(str, "pHraw_g[3]: %u \n", pHraw_g[3] );
    putsUart0(str);
    sprintf(str, "pHraw_b[3]: %u \n", pHraw_b[3] );
    putsUart0(str);
#endif
    for(i = 0; i < 6; i++)
    {
        pickTube(4);

        setRgbColor(pwm_r, 0, 0);
        waitMicrosecond(500000);
        rr += readAdc0Ss3();

        putsUart0(".");
        setRgbColor(0, pwm_g, 0);
        waitMicrosecond(500000);
        rg += readAdc0Ss3();


        setRgbColor(0, 0, pwm_b);
        waitMicrosecond(500000);
        rb += readAdc0Ss3();

    }
    setRgbColor(0, 0, 0);
    rr = rr /5;
    pHraw_r[4] = (rr);
    rg = rg /5;
    pHraw_g[4] = (rg);
    rb = rb /5;
    pHraw_b[4] = (rb);
#ifdef DEBUG
    sprintf(str, "pHraw_r[4]: %u \n", pHraw_r[4] );
    putsUart0(str);
    sprintf(str, "pHraw_g[4]: %u \n", pHraw_g[4] );
    putsUart0(str);
    sprintf(str, "pHraw_b[4]: %u \n", pHraw_b[4] );
    putsUart0(str);
#endif
    for(i = 0; i < 6; i++)
    {
        pickTube(5);

        setRgbColor(pwm_r, 0, 0);
        waitMicrosecond(500000);
        rr += readAdc0Ss3();
        putsUart0(".");

        setRgbColor(0, pwm_g, 0);
        waitMicrosecond(500000);
        rg += readAdc0Ss3();


        setRgbColor(0, 0, pwm_b);
        waitMicrosecond(500000);
        rb += readAdc0Ss3();
    }
    setRgbColor(0, 0, 0);
    rr = rr /5;
    pHraw_r[4] = (rr);
    rg = rg /5;
    pHraw_g[4] = (rg);
    rb = rb /5;
    pHraw_b[4] = (rb);
#ifdef DEBUG
    sprintf(str, "pHraw_r[5]: %u \n", pHraw_r[5] );
    putsUart0(str);
    sprintf(str, "pHraw_g[5]: %u \n", pHraw_g[5] );
    putsUart0(str);
    sprintf(str, "pHraw_b[5]: %u \n", pHraw_b[5] );
    putsUart0(str);
#endif
    putsUart0("\n");

}

int main(void)
{
    char* stri;

    initHw();
    initUart0();
    initRgb();
    initAdc0Ss3();
    initRemote();
    enableTimerMode();

    USER_DATA data;


    char value[50];
    setUart0BaudRate(19200 , 40e6);
    setAdc0Ss3Mux(5);
    setAdc0Ss3Log2AverageCount(6);

    //setPosition(100);
    homing();
    calibrate();

    while(true)
    {



        if(kbhitUart0());
        {
            v = false;
            kb = false;
            // Get the string from the user
            getsUart0(&data);

            // Parse fields
            parseFields(&data);
            #ifdef DEBUGUART
            uint8_t i;
            for (i = 0; i < data.fieldCount; i++)
            {
                putcUart0(data.fieldType[i]);
                putcUart0('\t');
                putsUart0(&data.buffer[data.fieldPosition[i]]);
                putcUart0('\n');
            }
            #endif

            // Command evaluation
            // set add, data → add and data are integers
            if (isCommand(&data, "tube", 1))
            {
                stri = getFieldString(&data, 1);

                if(strCmp(stri, "r") == 0)
                {
                    pickTube(0);
                    v = true;

                }
                else
                {
                    posnum = getFieldInteger(&data, 1);
                    pickTube(posnum);
                    v = true;
                }
            }

            else if (isCommand(&data, "measure", 2))
            {
                stri = getFieldString(&data, 1);
                if(strCmp(stri, "r") == 0)
                {
                    stri = getFieldString(&data, 2);
                    if(strCmp(stri, "raw") == 0)
                    {
                        v = true;
                        pickTube(0);
                        measure(0,&red, &green,&blue);
                        sprintf(value, "(%5u,%5u,%5u)\n", red, green, blue);
                        putsUart0(value);
                    }
                    else if (strCmp(stri, "pH") == 0)
                    {
                        pickTube(0);
                        measure(0,&red, &green,&blue);
                        measure_pH(0,red, green,blue);
    #ifdef DEBUG
                        sprintf(value, "R: %5u, G: %5u, B: %5u\n", red, green, blue);
                        putsUart0(value);
    #endif
                        v = true;
                    }
                }
                else
                {
                    stri = getFieldString(&data, 2);
                    if(strCmp(stri, "raw") == 0)
                    {
                        v = true;
                        posnum = getFieldInteger(&data, 1);
                        pickTube(posnum);
                        measure(posnum,&red, &green,&blue);
                        sprintf(value,"(%4u,%5u,%5u)\n", red, green, blue);
                        putsUart0(value);

                    }
                    else if (strCmp(stri, "pH"))
                    {
                        v = true;
                        posnum = getFieldInteger(&data, 1);
                        pickTube(posnum);
                        measure(posnum,&red, &green,&blue);
                        measure_pH(posnum, red,green, blue);
    #ifdef DEBUG
                        sprintf(value, "R: %5u, G: %5u, B: %5u\n", red, green, blue);
                        putsUart0(value);
    #endif
                    }
                }
            }
            else if (isCommand(&data, "measure", 1))
            {
                stri = getFieldString(&data, 1);

                if(strCmp(stri, "all") == 0)
                {
                    v = true;
                    for(i = 1; i < 6 ; i++)
                    {
                        measure(i,&red, &green,&blue);

                        sprintf(value, "Tube%u\n%5u\t%5u\t%5u\n",i, red, green, blue);
                        putsUart0(value);
                        //measure_pH(i,red, green,blue);
                    }
                }

            }

            else if (isCommand(&data, "calibrate", 1))
                        {
                            stri = getFieldString(&data, 1);

                            if(strCmp(stri, "all") == 0)
                            {
                                v = true;
                                calibrate_raw_all();

                            }

                        }

            else if (isCommand(&data, "run", 0))
            {
                while(true)
                {
                    v = true;
                    posnum = rand()% 6;
                    measure(posnum,&red, &green,&blue);
                    sprintf(value, "Tube%3u\n(%5u,%5u,%5u)\n",i, red, green, blue);
                    putsUart0(value);
                }
            }
            else if (isCommand(&data, "home", 0))
            {
                v = true;
                homing();
            }
            else if (isCommand(&data, "calibrate", 0))
            {
                v = true;
                calibrate();
            }
            else if (isCommand(&data, "pause", 0))
            {
               v = true;
               waitMicrosecond(10000000);
            }
            else if (isCommand(&data, "add", 1))
            {
                v = true;
                if ( 199 < position + getFieldInteger(&data, 1))
               {
                   posnum =  ((position + getFieldInteger(&data, 1)) - 200) ;
               }
               else
               {
                   posnum = position + getFieldInteger(&data, 1);
               }

                setPosition(posnum);
            }
            else if (isCommand(&data, "sub", 1))
            {
                v = true;
                if (0 > position - getFieldInteger(&data, 1))
                {
                    posnum = 200 + (position - getFieldInteger(&data, 1));
                }
                else
                {
                    posnum = position - getFieldInteger(&data, 1);
                }
                setPosition(posnum);
            }
            else if (isCommand(&data, "pos", 1))
            {
                posIn = getFieldInteger(&data, 1);
                if (posIn > 0 && posIn < 200)
                {
                    setPosition(posIn);
                    v = true;
                }
            }
            // Process other commands here
            // Look for error
            if (!v)
            {
               putsUart0("Invalid command\0");
               putcUart0('\n');
            }

        }
    }


}
/*
 * 192 = R
 * 159 = 5
 * 126 = 4
 * 92 = 3
 * 59 = 2
 * 26 = 1
 */
