/*
 * LCD_control.h
 *
 * Created: 22/01/2026 02:11:36
 *  Author: bsmor
 */ 


#ifndef LCD_CONTROL_H_
#define LCD_CONTROL_H_

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

#define E (1 << PORTC1) //Activo la transición bit6

//Función para inicializar LCD en modo 8 bits
void initLCD8bits(void);

//Función para colocar en el puerto un valor
void LCD_Port(char a);

//Función para enviar un comando
void LCD_CMD(char a);

//Función para enviar un caracter
void LCD_Write_Char(char c);

//Función para enviar una cadena
void LCD_Write_String(char *a);

//Desplazamiento hacia la derecha
void LCD_Shift_Right(void);

//Desplazamiento hacia la izquierda
void LCD_Shift_Left(void);

//Establecer el cursor
void LCD_Set_Cursor(char c, char f);

#endif /* LCD_CONTROL_H_ */