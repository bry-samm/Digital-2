/*
 * initUART.c
 *
 * Created: 27/01/2026 00:02:54
 *  Author: bsmor
 */ 

#include <avr/io.h>
#include "initUART.h"

void initUART(){
	//Step1 : configurate pin PD0 (rx) and PD1 (tx)
	DDRD |= (1 << DDD1);
	DDRD &= ~(1 << DDD0);
	//Step 2: UCSR0A
	UCSR0A = 0;
	//Step 3: UCSR0B: enable interrupts, enable recibir, enable transmition
	UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
	//Step 4 : UCSR0C
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	//Step 5: UBRR0 0 103 => 9600 16kHz
	UBRR0 = 103;

}

