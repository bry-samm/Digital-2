/*
 * CONF_ADC.c
 *
 * Created: 8/05/2025 15:37:56
 *  Author: bsmor
 */ 
#include <avr/io.h>


//Con esta función se configura para que el ADC lea de 0-1023
void initADC(void)
{
	ADMUX = 0;
	ADMUX |= (1 << REFS0);              // AVcc = 5V el cual sirve para referencia

	ADCSRA = 0;
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler 128
	ADCSRA |= (1 << ADIE);   // Habilitar interrupción
	ADCSRA |= (1 << ADEN);   // ADC enable
	ADCSRA |= (1 << ADSC);   // Iniciar conversión
}

//Con esta función se configura para que el ADC lea de 0-255
/*
void initADC(){
	ADMUX = 0;
	ADMUX	|= (1<<REFS0);  // 5V as reference

	ADMUX	|= (1 << ADLAR); // Left justification
	
	ADMUX	|= (1 << MUX1) | (1<< MUX0); //Select ADC3 to have a start value
	
	ADCSRA	= 0;
	ADCSRA	|= (1 << ADPS1) | (1 << ADPS0); // Sampling frequency = 125kHz "sampling = muestreo"
	ADCSRA	|= (1 << ADIE); // Enable interruption
	ADCSRA	|= (1 << ADEN); // Enable ADC
	
	ADCSRA	|= (1<< ADSC); // Start conversion
}
*/
