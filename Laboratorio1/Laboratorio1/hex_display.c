/*
 * hex_display.c
 *
 * Created: 22/01/2026 01:38:27
 *  Author: bsmor
 */ 
#include "hex_display.h"

/* Tabla de conversión hexadecimal a 7 segmentos */
static const uint8_t dig_display[16] =
{
	0x7E, // 0
	0x18, // 1
	0x6D, // 2
	0x3D, // 3
	0x1B, // 4
	0x37, // 5
	0x77, // 6
	0x1C, // 7
	0x7F, // 8
	0x3F, // 9
	0x5F, // A
	0x73, // b
	0x66, // C
	0x79, // d
	0x67, // E
	0x47  // F
};

void display_init(void)
{
	DDRD = 0xFF;   // PORTD como salida
	PORTD = 0x00;  // display apagado
}

void display_show(uint8_t numero)
{
	if (numero <= 16)
	{
		PORTD = dig_display[numero];
	}
}
