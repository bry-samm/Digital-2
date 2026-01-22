/*
 * hex_display.h
 *
 * Created: 22/01/2026 01:38:07
 *  Author: bsmor
 */ 


#ifndef HEX_DISPLAY_H_
#define HEX_DISPLAY_H_

#include <avr/io.h>
#include <stdint.h>

/* Inicializa el display */
void display_init(void);

/* Muestra un valor hexadecimal (0–15) */
void display_show(uint8_t numero);

#endif /* HEX_DISPLAY_H_ */
