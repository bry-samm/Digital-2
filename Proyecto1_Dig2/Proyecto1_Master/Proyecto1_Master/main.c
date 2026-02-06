/*
/*
 * Programa_Maestro.c
 * 
 * Created:
 * Author:
 * Description:
 */
//************************************************************************************
// Encabezado (librerías)
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include "I2C_conf.h"

#define slave1 0x30
#define slave2 0x40

#define slave1R (0x30 << 1) | 0x01 //Pongo el último bit en 1 para lectira
#define slave1W (0x30 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir


uint8_t direccion;
uint8_t temp;
uint8_t bufferI2C = 0;


//************************************************************************************
// Function prototypes
void refreshPORT(uint8_t valor);

//************************************************************************************
// Main Function
int main(void)
{
	//Puerto de salida para leds (en este caso sería el LCD)
	
	I2C_Slave_Init(100000, 1); //Inicializar como MAster F_SCL 100kHz, prescaler 1
	
	while (1)
	{
		PORTB |= (1 << PORTB5); //Solo sirve para ver si está enviando datos
		
		if(!I2C_Master_Start()) return; //Si no recibe dato solo se regresa
		
		if(!I2C_Master_Write(slave1W)){ //Mandar la dirección de escritura, si no recibo el ACK manda el stop
			I2C_Master_Stop();
			return;
		}
		I2C_Master_Write('R'); //Comando para leer "te voy a leer"
		
		//Ahora configuro para leer
		if (!I2C_Master_RepeatedStart()){
			I2C_Master_Stop();// SI no recibo nada termino la comunicación con return
			return;
		}
		if(!I2C_Master_Write(slave1R)){ //Si si se da la comunicación entonces leer 
			I2C_Master_Stop();
			return;
		}
		//Esto se ejecuta en el caso en el caso que si se da la comunicación
		I2C_Master_Read(&bufferI2C, 0); //Esto es solo para recibir un solo byte de info ya que no se manda ACK
		
		//Esto es para recopilar varios bytes ya que si se manda ACK
		//I2C_Master_Read(&bufferI2C1, 1);
		//I2C_Master_Read(&bufferI2C2, 1);
		//I2C_Master_Read(&bufferI2C3, 1);
		
		I2C_Master_Stop(); //Finalizamos
		
		PORTB &= ~(1 << PORTB); //Apago el led para indicar que ya no hay comunicación
		
		refreshPORT(bufferI2C); //Muestro los valores
	}
}
//************************************************************************************
// NON-INterrupt subroutines
void refreshPORT(uint8_t valor)
{
	// Bit 7 ? PORTB1 (MSB)
	if (valor & (1 << 7))
	PORTB |= (1 << PORTB1);
	else
	PORTB &= ~(1 << PORTB1);

	// Bit 6 ? PORTB0
	if (valor & (1 << 6))
	PORTB |= (1 << PORTB0);
	else
	PORTB &= ~(1 << PORTB0);

	// Bit 5 ? PORTD7
	if (valor & (1 << 5))
	PORTD |= (1 << PORTD7);
	else
	PORTD &= ~(1 << PORTD7);

	// Bit 4 ? PORTD6
	if (valor & (1 << 4))
	PORTD |= (1 << PORTD6);
	else
	PORTD &= ~(1 << PORTD6);

	// Bit 3 ? PORTD5
	if (valor & (1 << 3))
	PORTD |= (1 << PORTD5);
	else
	PORTD &= ~(1 << PORTD5);

	// Bit 2 ? PORTD4
	if (valor & (1 << 2))
	PORTD |= (1 << PORTD4);
	else
	PORTD &= ~(1 << PORTD4);

	// Bit 1 ? PORTD3
	if (valor & (1 << 1))
	PORTD |= (1 << PORTD3);
	else
	PORTD &= ~(1 << PORTD3);

	// Bit 0 ? PORTD2 (LSB)
	if (valor & (1 << 0))
	PORTD |= (1 << PORTD2);
	else
	PORTD &= ~(1 << PORTD2);
}
//************************************************************************************
// Interrupt subroutines
