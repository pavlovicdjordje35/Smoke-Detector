/* 
 * File:   pocetak.h
 * Author: student
 *
 * Created on 16. decembar 2023., 15.27
 */

#include <stdio.h>
#include <stdlib.h>
#include <p30fxxxx.h>
#include "adc.h"
#include "driverGLCD.h"
#include "tajmeri.h"


//void Delay_ms (int vreme);
void ConfigureTSPins(void);
void initUART1(void);
void WriteUART1(unsigned int data);
void WriteUART1dec2string(unsigned int data);
void Write_GLCD(unsigned int data);
void Delay(unsigned int N);
void writeStringUART1(char* data);
void delay_X(unsigned int vrednost);
void Touch_Panel (void);
void init_interrupt (void);

#ifndef POCETAK_H
#define	POCETAK_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* POCETAK_H */


