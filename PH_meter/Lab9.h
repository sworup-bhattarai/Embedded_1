/*
 * Lab8.h
 *
 *  Created on: May 3, 2022
 *      Author: soup
 */

#ifndef LAB9_H_
#define LAB9_H_

extern uint8_t phase;
extern uint16_t position;
extern uint16_t pwm_r, pwm_g, pwm_b, raw;
extern char str[100];
extern uint16_t pHraw_r[6];
extern uint16_t pHraw_g[6];
extern uint16_t pHraw_b[6];
extern float pHvalues[7];
uint8_t posnum, i, j , p;
uint16_t red, green, blue;

void initHw();
void applyPhase();
void stepCw();
void stepCCw();
void setPosition(uint16_t positionIn);
void pickTube(uint8_t num);
void homing();
void calibrate();
void measure(uint8_t tube, uint16_t *r, uint16_t *g, uint16_t *b);
void measure_pH(uint8_t tube, uint16_t r, uint16_t g, uint16_t b);
void calibrate_raw_all();
int main(void);




#endif /* LAB8_H_ */
