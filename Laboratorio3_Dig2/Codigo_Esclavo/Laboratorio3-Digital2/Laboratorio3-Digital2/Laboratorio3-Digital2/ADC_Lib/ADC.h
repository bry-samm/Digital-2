/*
 * ADC.h
 *
 * Created: 1/24/2026 4:39:17 PM
 *  Author: razer
 */ 


#ifndef ADC_H_
#define ADC_H_
#define F_CPU 16000000
#include <avr/io.h>

void initADC(void); 
uint16_t lecADC(uint8_t canal); 


#endif /* ADC_H_ */