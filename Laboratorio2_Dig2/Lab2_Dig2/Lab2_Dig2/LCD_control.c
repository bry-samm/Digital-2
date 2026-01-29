/*
 * LCD_control.c
 *
 * Created: 22/01/2026 02:11:55
 *  Author: bsmor
 */ 

#include "LCD_control.h"

//Función para inicializar LCD en modo 8 bits
//Aqui defino que pines voy a emplear para conectar el LCD
void initLCD8bits(void){
//Habilito los pines que voy a utilizar en el LCD como salida
	DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
	DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3);
//Inicialmente apagados
	//PORTD = 0x00
	PORTB &=~((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3));
	
	_delay_ms(5);
	LCD_Port(0x00);
	_delay_ms(20);

//=================================================================
	//Function Set
	//Aqui se inicializa el LCD, los valores se encuentran en la tabla de la presentación de digital 2 en canvas
	_delay_ms(20);
	LCD_CMD(0x38);
	//Display ON/OFF
	_delay_ms(20);
	LCD_CMD(0x0C);
	//Entry Mode
	_delay_ms(20);
	LCD_CMD(0x06);
	//Clear Display
	_delay_ms(20);
	LCD_CMD(0x01);
}
//=================================================================
void LCD_CMD(char a)
{
	PORTB &= ~(1 << PORTB3);   // RS = 0
	PORTB &= ~(1 << PORTB2);   // EN = 0 (asegurar)

	LCD_Port(a);               // datos estables
	_delay_ms(4);

	PORTB |= (1 << PORTB2);    // EN ?
	_delay_ms(4);
	PORTB &= ~(1 << PORTB2);   // EN ?

	_delay_ms(4);             // tiempo ejecución
}

//Función para colocar en el puerto un valor
/*
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

	if (a & 3)
	//D2 = 1;
	PORTD |= (1 << PORTD4);
	else
	//D2 = 0;
	PORTD &= ~(1 << PORTD4);

	if (a & 4)
	//D3 = 1;
	PORTD |= (1 << PORTD5);
	else
	//D3 = 0;
	PORTD &= ~(1 << PORTD5);

	if (a & 5)
	//D4 = 1;
	PORTD |= (1 << PORTD6);
	else
	//D4 = 0;
	PORTD &= ~(1 << PORTD6);

	if (a & 6)
	//D5 = 1;
	PORTD |= (1 << PORTD7);
	else
	//D5 = 0;
	PORTD &= ~(1 << PORTD7);

	if (a & 7)
	//D6 = 1;
	PORTB |= (1 << PORTB0);
	else
	//D6 = 0;
	PORTB &= ~(1 << PORTB0);

	if (a & 8)
	//D7 = 1;
	PORTB |= (1 << PORTB1);
	else
	//D7 = 0;
	PORTB &= ~(1 << PORTB1);
}
*/
//Función para colocar en el puerto un valor (mascaras)
//Para esto se emplean máscaras y se toma cada bit del byte a para colocarlo en el respectivo puerto de salida
//la sintaxis (1<<n) es para crear la máscara y analizar únicamente ese bit

void LCD_Port(char a)
{
	if (a & (1<<0)) PORTD |=  (1<<PORTD2); else PORTD &= ~(1<<PORTD2);
	if (a & (1<<1)) PORTD |=  (1<<PORTD3); else PORTD &= ~(1<<PORTD3);
	if (a & (1<<2)) PORTD |=  (1<<PORTD4); else PORTD &= ~(1<<PORTD4);
	if (a & (1<<3)) PORTD |=  (1<<PORTD5); else PORTD &= ~(1<<PORTD5);
	if (a & (1<<4)) PORTD |=  (1<<PORTD6); else PORTD &= ~(1<<PORTD6);
	if (a & (1<<5)) PORTD |=  (1<<PORTD7); else PORTD &= ~(1<<PORTD7);
	if (a & (1<<6)) PORTB |=  (1<<PORTB0); else PORTB &= ~(1<<PORTB0);
	if (a & (1<<7)) PORTB |=  (1<<PORTB1); else PORTB &= ~(1<<PORTB1);
}

//Función para enviar un caracter
void LCD_Write_Char(char c){
	// RS = 1; Dato en el puerto lo va a interpretar como DATO
	PORTB |= (1 << PORTB3);
	LCD_Port(c);
	//EN = 1;	esto es para activar el funcionamiento y recibir datos
	PORTB |= (1 << PORTB2);
	_delay_us(1);
	//EN = 0;	Datos/comandos transmitiendose al LCD
	PORTB &= ~(1 << PORTB2);
	_delay_us(100);
}

//Función para enviar una cadena
void LCD_Write_String(char *a){
	int i;
	for (i = 0; a[i] != '\0'; i++)
	LCD_Write_Char(a[i]);
}

//Desplazamiento hacia la derecha
void LCD_Shift_Right(void){
	LCD_CMD(0x1C);
}

//Desplazamiento hacia la izquierda
void LCD_Shift_Left(void){
	LCD_CMD(0x18);
}

//Establecer el cursor
void LCD_Set_Cursor(char c, char f)
{
	char temp;
	if (f==1)
	{
		temp = 0x80 + c - 1;
	}
	else if (f == 2)
	{
		temp = 0xC0 + c - 1;
	}
	LCD_CMD(temp);
}