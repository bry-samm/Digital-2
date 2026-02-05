/*
 * Slave.c
 *
 * Created: 1/29/2026 1:34:46 PM
 *  Author: Ervin Gomez 231226
 * En este programa se hace la logica del esclavo. 
 */ 
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "ADC_Lib/ADC.h"
#include "SPI_Lib/SPI.h"

/****************************************/
// Variables
volatile uint8_t temp1 = 0; 
volatile uint8_t valor_ADC7 = 0; 
volatile uint8_t valor_ADC6 = 0; 
volatile uint8_t band = 0; 
volatile uint8_t spivalor; 
uint8_t spi_alter; 

/****************************************/
// Function prototypes
void mostrarleds(uint16_t val); 

/****************************************/
// Main Function
int main(void)
{
	initADC(); //inicar adc
	spiInt(SPI_SLAVE_SS, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);  
	SPCR |= (1<<SPIE); 
	sei(); 
	
	//Seteo de las puertos de salida
	DDRD |= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7);
	PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7));
	
	DDRB |= (1 << PORTB0) | (1 << PORTB1);
	PORTB &= ~((1 << PORTB0) | (1 << PORTB1));
	
	
	while (1)
	{
		//logia para llamar a la función de muestro de leds
		if (band == 1)
		{
			mostrarleds(spi_alter);
			band = 0;   
		}
		_delay_ms(100); 
	}
}


void mostrarleds(uint16_t val)
{
	// Limpiar solo los bits usados
	PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7));
	PORTB &= ~((1 << PORTB0) | (1 << PORTB1));

	// Bits 0–5 ? PD2–PD7
	PORTD |= (val & 0x3F) << 2;

	// Bits 6–7 ? PB0–PB1
	PORTB |= (val >> 6) & 0x03;
}

ISR(ADC_vect)
{
	//Muxeo de los ADC 6 y 7 
	uint8_t currentADC = ADMUX & 0x07; 
	temp1 = ADCH; 
	if (currentADC == 7)
	{
		valor_ADC7 = temp1; 
		ADMUX = 0;
		ADMUX |= (1<<REFS0) | (1<<ADLAR);
		ADMUX |= (1<<MUX1) | (1<<MUX2);
	}
	if (currentADC == 6)
	{
		valor_ADC6 = temp1;
		ADMUX = 0;
		ADMUX |= (1<<REFS0) | (1<<ADLAR);
		ADMUX |= (1 << MUX0) |  (1 << MUX1) | (1<<MUX2);
	} 
	ADCSRA |= (1<<ADSC);
}

ISR(SPI_STC_vect)
{
	//Lectura de lo que me envia el Maestro 
	spivalor = SPDR; 
	if (spivalor == 'c')
	{
		SPDR = valor_ADC7;
	}
	else if (spivalor == 'a')
	{
		SPDR = valor_ADC6;
	}
	else 
	{
		spi_alter = spivalor; 
		band = 1;  
	}
}