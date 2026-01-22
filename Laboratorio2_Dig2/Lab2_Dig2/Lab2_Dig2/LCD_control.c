/*
 * LCD_control.c
 *
 * Created: 22/01/2026 02:11:55
 *  Author: bsmor
 */ 

#include "LCD_control.h"

//Función para inicializar LCD en modo 8 bits
void initLCD8bits(void){
//Habilito los pines que voy a utilizar en el LCD como salida
	DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
	DDRB |= (1 << DDB0) | (1 << DDB1);
//Inicialmente apagados
	PORTD = 0
	PORTB &=~((1 << PORTB0) | (1 << PORTB1));
	
	LCD_Port(0x00);
	_delay_ms(20);
/*
	LCD_CMD(0x03);
	_delay_ms(5);
	LCD_CMD(0x03);
	_delay_ms(11);
	LCD_CMD(0x03);
	
	LCD_CMD(0x02);
	*/
//=================================================================
	//Function Set
	LCD_CMD(0x38);
	//Display ON/OFF
	LCD_CMD(0x0C);
	//Entry Mode
	LCD_CMD(0x06);
	//Clear Display
	LCD_CMD(0x01);
//=================================================================
void LCD_CMD(char a){
	// RS = 0; Dato en el puerto lo va a interpretar como COMANDO
	PORTB &= ~(1 << PORTB3);
	LCD_Port(a);
	//EN = 1;	esto es para activar el funcionamiento y recibir datos
	PORTB |= (1 << PORTB2);
	_delay_ms(4);
	//EN = 0;	Datos/comandos transmitiendose al LCD
	PORTB &= ~(1 << PORTB2);
}
//Función para colocar en el puerto un valor
void LCD_Port(char a)
{
	if (a & 1)
	//D0 = 1;
	PORTD |= (1 << PORTD2);
	else
	//D0 = 0;
	PORTD &= ~(1 << PORTD2);

	if (a & 2)
	//D1 = 1;
	PORTD |= (1 << PORTD3);
	else
	//D1 = 0;
	PORTD &= ~(1 << PORTD3);

	if (a & 4)
	//D2 = 1;
	PORTD |= (1 << PORTD4);
	else
	//D2 = 0;
	PORTD &= ~(1 << PORTD4);

	if (a & 8)
	//D3 = 1;
	PORTD |= (1 << PORTD5);
	else
	//D3 = 0;
	PORTD &= ~(1 << PORTD5);

	if (a & 16)
	//D4 = 1;
	PORTD |= (1 << PORTD6);
	else
	//D4 = 0;
	PORTD &= ~(1 << PORTD6);

	if (a & 32)
	//D5 = 1;
	PORTD |= (1 << PORTD7);
	else
	//D5 = 0;
	PORTD &= ~(1 << PORTD7);

	if (a & 64)
	//D6 = 1;
	PORTB |= (1 << PORTB0);
	else
	//D6 = 0;
	PORTB &= ~(1 << PORTB0);

	if (a & 128)
	//D7 = 1;
	PORTB |= (1 << PORTB1);
	else
	//D7 = 0;
	PORTB &= ~(1 << PORTB1);
}

//Función para enviar un caracter
void LCD_Write_Char(char c){
	// RS = 1; Dato en el puerto lo va a interpretar como DATO
	PORTB |= (1 << PORTB3);
	LCD_Port(c);
	//EN = 1;	esto es para activar el funcionamiento y recibir datos
	PORTB |= (1 << PORTB2);
	_delay_ms(4);
	//EN = 0;	Datos/comandos transmitiendose al LCD
	PORTB &= ~(1 << PORTB2);
}

//Función para enviar una cadena
void LCD_Write_String(char *a){
	int i;
	for (i = 0; a[i] != '\0'; i++)
	LCD_Write_Char(a[i]);
}

//??????????????????????????????????????????????????
//Desplazamiento hacia la derecha
void LCD_Shift_Right(void){
	LCD_CMD(0x01);
}

//Desplazamiento hacia la izquierda
void LCD_Shift_Left(void){
	LCD_CMD(0x01);
}

//Establecer el cursor
void LCD_Set_Cursor(char c, char f){
	if (f == 1){
		LCD_CMD(c);
	} else if (f == 2){
		LCD_CMD(c)
	}
}