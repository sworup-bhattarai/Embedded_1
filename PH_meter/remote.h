/*
 * remote.h
 *
 *  Created on: Apr 26, 2022
 *      Author: soup
 */

#ifndef REMOTE_H_
#define REMOTE_H_


extern uint32_t  frequency;
extern uint32_t  time[50];
extern uint32_t  time2[33];
extern uint8_t   count;
extern bool valid;
extern uint32_t num ;
extern uint32_t   T ;
extern char buffer[44][15];

extern uint32_t fieldData[44][32];

//extern uint8_t field_hex[44][5];

void initRemote();
void enableTimerMode();
float convert(uint32_t num);
void gpiolsr();



#endif /* REMOTE_H_ */
