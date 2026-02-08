/*
 * Slave_1.c
 * 
 * Created:
 * Author:
 */
//************************************************************************************
//================================== ESCLAVO 1 =====================================
//Este eslavo tiene la función de realizar la lectura para el MPU680
//*************************************************************************************
//Recordar que siempre hay que poner una resistencia pull-up 
// Para ver conexión (ver grabación clase 1:15:16)
// Para calcular valor (ver modulos de clase)

// Encabezado (librerías)
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "I2C_conf.h"
#include "CONF_ADC.h"

//Se define la dirección del esclavo, en este caso como es mi programa yo decido que dirección tiene
// caso contrario cuando se trabaja con un sensor, se debe de colocar la dirección descrita por el datasheet del sensor
#define SlaveAddress 0x30

uint8_t buffer = 0;
uint8_t valor_ADC = 0;

//************************************************************************************
// Function prototypes

//************************************************************************************
// Main Function
int main(void)
{
	DDRB |= (1 << DDB5);
	PORTB &= ~(1 << PORTB5); //Led para encender y apgar indicando una comunicación exitosa
	
	initADC();
	
	//inicializar ADC
	I2C_Slave_Init(SlaveAddress); //Se define la dirección del esclavo
	
	sei(); //Habilitar interrupciones
	
	while (1)
	{
		if(buffer == 'R'){ //Reviso si el caractér de lectura esta recibiendose
			PINB |= (1 << PINB5); //Se hace un toggle para indicar que si hay datos 
			buffer = 0;
		}
		//Iniciar la secuencia de ADC
		
	}
}

//************************************************************************************
// NON-INterrupt subroutines

//************************************************************************************
// Interrupt subroutines

ISR(ADC_vect){
	valor_ADC = ADCH;
	ADCSRA |= (1 << ADSC); // nueva conversión
}

ISR(TWI_vect){ //(garbación clase 01:06:00)
	uint8_t estado = TWSR & 0xFC; //Nos quedamos unicamente con los bits de estado TWI Status
	switch(estado){
		//**************************
		// Slave debe recibir dato
		//**************************
		case 0x60: //SLA+W recibido
		case 0x70: //General call
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA); //Indica "si te escuche"
			break;
		
		case 0x80: //Dato recibido, ACK enviado
		case 0x90: //Dato recibido General call, ACK enviado
			buffer = TWDR; //Ya puedo utilizar los datos
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
			break;
		//*****************************
		// Slave debe transmitir dato
		//*****************************
		//en cada case hay un comando que ya está predeterminado (ver presentacion)
		case 0xA8: //SLA+R recibido
		case 0xB8: //Dato transmitido, ACK recibido
			TWDR = valor_ADC; //Dato a enviar
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
			break;
		//IMPORTANTE: que pasa si quiero enviar más de un dato?
		//Se puede hacer un arreglo por cada vez que envío un dato (ver grabación clase 1:07:30)
		
		case 0xC0: //Dato transmitido, NACK recibido
		case 0xC8: //Último dato transmitido
			TWCR = 0; //Limpio la interfaz para rebibir nuevo dato
			TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE); //Reiniciarse
			break;
			
		case 0xA0: // STOP o repeated START recibido como slave
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
			break;
		//**********************
		// Cualquier error
		//**********************
		default:
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
			break;
		
	}
	
}