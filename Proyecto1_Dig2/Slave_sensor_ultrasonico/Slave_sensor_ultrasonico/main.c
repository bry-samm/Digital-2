	/*
 * Slave_sensor_ultrasonico.c
 * 
 * Created:
 * Author:
 */
//************************************************************************************
//================================== ESCLAVO 1 =====================================
//Este eslavo tiene la función de realizar la lectura para el sensor ultrasonico
//*************************************************************************************
//Recordar que siempre hay que poner una resistencia pull-up 
// Para ver conexión (ver grabación clase 1:15:16)
// Para calcular valor (ver modulos de clase)

/*
// Encabezado (librerías)
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "I2C_conf/I2C_conf.h"
#include "CONF_PWM/CONF_PWM.h"

//Se define la dirección del esclavo, en este caso como es mi programa yo decido que dirección tiene
// caso contrario cuando se trabaja con un sensor, se debe de colocar la dirección descrita por el datasheet del sensor
#define SlaveAddress 0x30

uint8_t buffer = 0;
uint16_t last_distance = 0;
uint16_t ticks;
//************************************************************************************
// Function prototypes
void trigger_ultrasonic(void);
uint8_t measure_echo(uint16_t *ticks_out);
void inicializar_pulso(void);
//************************************************************************************
// Main Function
int main(void)
{
	//Configuración para poder iniciar los pines trigger y echo del sensor
	inicializar_pulso();
	PORTD &= ~(1 << PORTD4);
	PORTD &= ~(1 << PORTD5);
	//********************************
	
	DDRB |= (1 << DDB5);
	PORTB &= ~(1 << PORTB5); //Led para encender y apgar indicando una comunicación exitosa

	//inicializar ADC
	I2C_Slave_Init(SlaveAddress); //Se define la dirección del esclavo
	
	sei(); //Habilitar interrupciones
	
	while (1)
	{
		trigger_ultrasonic();
		if (measure_echo(&ticks)) {
			// Solo si hay nueva medición
			float distancia_cm = (ticks * 0.5) / 58.0;
			last_distance = (uint16_t)distancia_cm;
		}
		
		if(buffer == 'R'){ //Reviso si el caractér de lectura esta recibiendose
			PINB |= (1 << PINB5); //Se hace un toggle para indicar que si hay datos 
			buffer = 0;
		}
		//Iniciar la secuencia de ADC
		
	}
}

//************************************************************************************
// NON-INterrupt subroutines
void trigger_ultrasonic(void) {
	PORTD |= (1 << PORTD4);
	_delay_us(10);
	PORTD &= ~(1 << PORTD4);
}

uint8_t measure_echo(uint16_t *ticks_out) {
	uint32_t timeout;

	// 1?? Asegurar ECHO en bajo
	timeout = 300000UL;
	while (PIND & (1 << PORTD5)) {
		if (--timeout == 0) return 0;
	}

	// 2?? Esperar flanco de subida
	timeout = 300000UL;
	while (!(PIND & (1 << PORTD5))) {
		if (--timeout == 0) return 0;
	}

	TCNT1 = 0;
	TCCR1B = (1 << CS11);

	// 3?? Esperar flanco de bajada
	timeout = 300000UL;
	while (PIND & (1 << PORTD5)) {
		if (--timeout == 0) {
			TCCR1B = 0;
			return 0;
		}
	}

	TCCR1B = 0;
	*ticks_out = TCNT1;
	return 1;
}

void inicializar_pulso(void)
{
	DDRD |= (1 << PORTD4);    // TRIG como salida
	DDRD &= ~(1 << PORTD5);   // ECHO como entrada
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;
}

//************************************************************************************
// Interrupt subroutines

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
			TWDR = last_distance; //Dato a enviar
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

*/
		
			/*
 * Slave_sensor_ultrasonico.c
 * 
 * Created:
 * Author:
 */
//************************************************************************************
//================================== ESCLAVO 1 =====================================
//Este eslavo tiene la función de realizar la lectura para el sensor ultrasonico
//*************************************************************************************
//Recordar que siempre hay que poner una resistencia pull-up 
// Para ver conexión (ver grabación clase 1:15:16)
// Para calcular valor (ver modulos de clase)

// Encabezado (librerías)
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "I2C_conf/I2C_conf.h"
#include "CONF_PWM/CONF_PWM.h"

//Se define la dirección del esclavo, en este caso como es mi programa yo decido que dirección tiene
// caso contrario cuando se trabaja con un sensor, se debe de colocar la dirección descrita por el datasheet del sensor
#define SlaveAddress 0x30

uint8_t buffer = 0;
uint16_t last_distance = 0;
uint16_t ticks;
//************************************************************************************
// Function prototypes
void trigger_ultrasonic(void);
uint8_t measure_echo(uint16_t *ticks_out);
void inicializar_pulso(void);
//************************************************************************************
// Main Function
int main(void)
{
	//Configuración para poder iniciar los pines trigger y echo del sensor
	inicializar_pulso();
	PORTD &= ~(1 << PORTD4);
	PORTD &= ~(1 << PORTD5);
	//********************************
	
	DDRB |= (1 << DDB5);
	PORTB &= ~(1 << PORTB5); //Led para encender y apgar indicando una comunicación exitosa

	//inicializar ADC
	I2C_Slave_Init(SlaveAddress); //Se define la dirección del esclavo
	
	sei(); //Habilitar interrupciones
	
	while (1)
	{
		trigger_ultrasonic();
		if (measure_echo(&ticks)) {
			// Solo si hay nueva medición
			float distancia_cm = (ticks * 0.5) / 58.0;
			last_distance = (uint16_t)distancia_cm;
		}
		
		if(buffer == 'R'){ //Reviso si el caractér de lectura esta recibiendose
			PINB |= (1 << PINB5); //Se hace un toggle para indicar que si hay datos 
			buffer = 0;
		}
		//Iniciar la secuencia de ADC
		
	}
}

//************************************************************************************
// NON-INterrupt subroutines
void trigger_ultrasonic(void) {
	PORTD |= (1 << PORTD4);
	_delay_us(10);
	PORTD &= ~(1 << PORTD4);
}

uint8_t measure_echo(uint16_t *ticks_out) {
	uint32_t timeout;

	// 1?? Asegurar ECHO en bajo
	timeout = 300000UL;
	while (PIND & (1 << PORTD5)) {
		if (--timeout == 0) return 0;
	}

	// 2?? Esperar flanco de subida
	timeout = 300000UL;
	while (!(PIND & (1 << PORTD5))) {
		if (--timeout == 0) return 0;
	}

	TCNT1 = 0;
	TCCR1B = (1 << CS11);

	// 3?? Esperar flanco de bajada
	timeout = 300000UL;
	while (PIND & (1 << PORTD5)) {
		if (--timeout == 0) {
			TCCR1B = 0;
			return 0;
		}
	}

	TCCR1B = 0;
	*ticks_out = TCNT1;
	return 1;
}

void inicializar_pulso(void)
{
	DDRD |= (1 << PORTD4);    // TRIG como salida
	DDRD &= ~(1 << PORTD5);   // ECHO como entrada
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;
}

//************************************************************************************
// Interrupt subroutines

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
		
		volatile uint8_t comando_recibido = 0;

		case 0x80:
		comando_recibido = TWDR;
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
		break;

		case 0xA8:
		case 0xB8:

		if(comando_recibido == 'R'){
			TWDR = last_distance;
			}else{
			TWDR = 0x00;
		}

		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
		break;

		//IMPORTANTE: que pasa si quiero enviar más de un dato?
		//Se puede hacer un arreglo por cada vez que envío un dato (ver grabación clase 1:07:30)
		
		case 0xC0: // NACK recibido
		case 0xC8:
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
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

