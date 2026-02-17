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
#define SlaveAddress 0x40

volatile uint8_t buffer = 0;
uint16_t ticks;
uint8_t led_state;
volatile uint16_t contador = 0;

volatile uint16_t tiempo_inicio = 0;
volatile uint16_t tiempo_fin = 0;
volatile uint8_t captura_estado = 0;
volatile uint16_t last_distance = 0;
static uint16_t last_trigger = 0;

//************************************************************************************
// Function prototypes
void trigger_ultrasonic(void);
void init_ultrasonic_icp(void);
void secuencia_leds(uint8_t activar);
void setup(void);
//************************************************************************************
// Main Function
int main(void)
{
	setup();
	while (1)
	{
		PORTB ^= (1 << PORTB5);
		if (contador - last_trigger >= 110) {
			trigger_ultrasonic();
			last_trigger = contador;
		}
		//trigger_ultrasonic();
		//_delay_ms(60);   // tiempo entre disparos

		if (buffer == 'N'){
			PORTC |= (1 << PORTC1);
			OCR2A = 130;
			secuencia_leds(1);
			//secuencia_leds(1);
			buffer = 0;
		}else if (buffer == 'F'){
			PORTC &= ~(1 << PORTC1);
			OCR2A = 0;
			secuencia_leds(0);
			//secuencia_leds(0);
			buffer = 0;
		}
		_delay_ms(10);

	}
}


//************************************************************************************
// NON-INterrupt subroutines
void setup(){
	//***************************************************************************************
	//COnfiguración de leds para estrella de la muerte
	//Configurar como salida
	DDRD |= (1<<DDD2) | (1<<DDD3) | (1<<DDD4) | (1<<DDD5) | (1<<DDD6) | (1<<DDD7);
	DDRC |= (1<<DDC1) | (1<<DDC2) | (1 << DDC0);

	//Apagar todos inicialmente
	PORTD &= ~((1<<PORTD2) | (1<<PORTD3) | (1<<PORTD4) | (1<<PORTD5) | (1<<PORTD6) | (1<<PORTD7));
	PORTC &= ~((1<<PORTC1) | (1<<PORTC2) | (1<<PORTC0));
	//****************************************************************************************	
	//Configuración para poder iniciar los pines trigger y echo del sensor
	init_ultrasonic_icp();
	initPWM2();
	//****************************************************************************************
	DDRB |= (1 << DDB5);
	PORTB &= ~(1 << PORTB5); //Led para encender y apgar indicando una comunicación exitosa
	//****************************************************************************************

	//inicializar esclavo
	I2C_Slave_Init(SlaveAddress); //Se define la dirección del esclavo
	
	TIMSK2 |= (1 << TOIE2); //Activo interrupción por overflow del TIMER 2
	sei(); //Habilitar interrupciones
}


void secuencia_leds(uint8_t activar)
{
	static uint16_t inicio = 0;
	static uint8_t estado = 0;

	if(activar)
	{
		if(estado == 0)
		{
			inicio = contador;
			estado = 1;
		}

		else if(estado == 1 && contador - inicio >= 500)
		{
			PORTD |= (1 << PORTD2);
			estado = 2;
		}

		else if(estado == 2 && contador - inicio >= 1000)
		{
			PORTD |= (1 << PORTD3);
			estado = 3;
		}

		else if(estado == 3 && contador - inicio >= 1500)
		{
			PORTD |= (1 << PORTD4);
			estado = 4;
		}
		else if(estado == 4 && contador - inicio >= 2000)
		{
			PORTD |= (1 << PORTD5);
			estado = 5;
		}
		else if(estado == 5 && contador - inicio >= 2200)
		{
			PORTD |= (1 << PORTD6);
			estado = 6;
		}
		else if(estado == 6 && contador - inicio >= 2300)
		{
			PORTD |= (1 << PORTD7);
			estado = 7;
		}
		else if(estado == 7 && contador - inicio >= 2400)
		{
			PORTC |= (1 << PORTC1);
			estado = 8;
		}
		else if(estado == 8 && contador - inicio >= 2500)
		{
			PORTC |= (1 << PORTB2);
			estado = 9;
		}
		else if(estado == 9 && contador - inicio >= 2550)
		{
			PORTC |= (1 << PORTC0);
		}
	}

	else if (activar == 0)
	{
		PORTD &= ~((1<<PORTD2) | (1<<PORTD3) | (1<<PORTD4) | (1<<PORTD5) | (1<<PORTD6) | (1<<PORTD7));
		PORTC &= ~((1 << PORTC1) | (1 << PORTC2) | (1 << PORTC0));
		
		estado = 0;
	}
}
/*
void init_ultrasonic_icp(void)
{
	DDRC |= (1 << PORTC2);     // TRIG como salida
	DDRB &= ~(1 << DDB0);      // ICP1 (PB0 / D8) como entrada

	TCCR1A = 0;

	// Prescaler = 8
	TCCR1B = (1 << CS11);

	// Captura en flanco de subida inicialmente
	TCCR1B |= (1 << ICES1);

	// Habilitar interrupciones
	TIMSK1 |= (1 << ICIE1);

	TCNT1 = 0;
}
*/

void init_ultrasonic_icp(void)
{
	DDRC |= (1 << PORTC3);
	DDRB &= ~(1 << DDB0);

	TCCR1A = 0;
	TCCR1B = (1 << CS11) | (1 << ICES1);

	TIFR1 |= (1 << ICF1);      // <-- LIMPIAR FLAG

	TIMSK1 |= (1 << ICIE1);

	TCNT1 = 0;
}

void trigger_ultrasonic(void)
{
	PORTC |= (1 << PORTC3);
	_delay_us(10);
	PORTC &= ~(1 << PORTC3);
}


//************************************************************************************
// Interrupt subroutines
ISR(TIMER1_CAPT_vect)
{
	if(captura_estado == 0)
	{
		tiempo_inicio = ICR1;
		TCCR1B &= ~(1 << ICES1);
		captura_estado = 1;
	}
	else
	{
		tiempo_fin = ICR1;
		uint16_t ticks = tiempo_fin - tiempo_inicio;
		last_distance = ticks / 116;   // SIN FLOAT
		TCCR1B |= (1 << ICES1);
		captura_estado = 0;
	}
}

ISR(TIMER2_OVF_vect)
{
	contador++;
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