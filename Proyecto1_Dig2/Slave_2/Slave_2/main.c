/*
 * Slave_1.c
 * 
 * Created:
 * Author:
 */
//************************************************************************************
//================================== ESCLAVO 1 =====================================
//Este eslavo tiene la función de realizar la lectura para el MPU6050
//*************************************************************************************
//Recordar que siempre hay que poner una resistencia pull-up 
// Para ver conexión (ver grabación clase 1:15:16)
// Para calcular valor (ver modulos de clase)

// Encabezado (librerías)
#include <avr/io.h>
#include <stdint.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "I2C_conf/I2C_conf.h"
#include "PWMTimer1/PWMTimer1.h"
#include "initUART/initUART.h"

//Se define la dirección del esclavo, en este caso como es mi programa yo decido que dirección tiene
// caso contrario cuando se trabaja con un sensor, se debe de colocar la dirección descrita por el datasheet del sensor
#define SlaveAddress 0x30

//#define SlaveAddress 0x50

uint8_t indice = 0;
uint8_t bandera = 0;
uint8_t colorDetectado = 0; 
volatile uint16_t dato_r [4];
volatile int16_t anguloX = 0;
volatile int16_t anguloY = 0;
volatile int8_t modo_stepper = 0; 
volatile uint8_t paso_actual = 0;

//variables de prueba. 
volatile uint32_t pulsos_conteo = 0;
volatile uint16_t r_count, g_count, b_count;
volatile uint8_t color_fase = 0; // 0:R, 1:G, 2:B
volatile uint8_t color_timer = 0;
volatile uint8_t last_state = 0;

//************************************************************************************
// Function prototypes
void initPCINT(void);
void initTimer0(void);
void control_servo(int16_t angulo2);
void procesarColor(void); 
void writeChar(char caracter);
void cadena_texto(char* texto);

//************************************************************************************
// Main Function
int main(void)
{
	//Conexiones para el control se sensor de color
	DDRD |= (1<<DDD2) | (1<<DDD4) | (1<<DDD5) | (1<<DDD7); // S0,S1,S2,S3 salida
	DDRD &= ~(1<<DDD6); // OUT del sensor
	PORTD |= (1<<PORTD2);   // S0 = 1
	PORTD &= ~(1<<PORTD4);  // S1 = 0
	
	//Pines para el control del motor stepper
	DDRC |= (1 << DDC0) | (1 << DDC1) | (1 << DDC2) | (1 << DDC3); 
	//Puerto para mostrar comunicación activa
	DDRB |= (1 << DDB5);
	
	Servo2(0, 8); //Configuración para el PWMT1 del motor servo
	initTimer0(); //Configuración para el timer0
	initPCINT(); //Configuración para pinchange
	I2C_Slave_Init(0x30); //Se define la dirección del esclavo
	initUART(); // Se inicia el uart
	cadena_texto("Esclavo Iniciado...\r\n");
	sei(); //Habilitar interrupciones
	
	while (1)
	{
	
		if(bandera){ //Reviso si el caractér de lectura esta recibiendose
			PINB |= (1 << PINB5); //Se hace un toggle para indicar que si hay datos 
			bandera = 0; 
		
			anguloX = (dato_r[0] << 8) | dato_r[1]; // Se define que los primero bits son de X
			anguloY = (dato_r[2] << 8) | dato_r[3]; // Se define que los segundos bits son de Y
			control_servo((int16_t) anguloX); //control de servo
			
			//Logica para el movimiento del Stepper
			if (anguloY > 45) modo_stepper = 1;
			else if (anguloY < -45) modo_stepper = 2;
			else modo_stepper = 0;
			
		}		
		
	// El color se procesa aquí cuando el Timer termina una vuelta completa (RGB)
	if (color_fase == 3) {
		procesarColor();
		color_fase = 0; // Reiniciar ciclo de lectura
	}
		
	}
}

//************************************************************************************
// NON-INterrupt subroutines


// logica para mover el servo, si es menor a -90 va a la izquierda y si es mayor a 90 va a la derecha.
void control_servo(int16_t angulo2)
{
	if (angulo2 > -45 && angulo2 < 45)
	{
		angulo2 = 0; // zona muerta
	}

	if (angulo2 < -90)
	{
		angulo2 = -90;
	} 

	if (angulo2 > 90) 
	{
		angulo2 = 90;

	} 
	uint32_t duty = 1000 + ((uint32_t)(angulo2 + 90) * 4000) / 180;
	updateDutyCycle_servo2((uint16_t)duty);
}

void initTimer0() {
	TCCR0A = (1 << WGM01); // Modo CTC
	TCCR0B = (1 << CS02) | (1 << CS00); // Prescaler 1024
	OCR0A = 156; // Aproximadamente 10ms a 16MHz
	TIMSK0 |= (1 << OCIE0A); // Habilitar interrupción por comparación
}

//función para detectar cambio en un pin 
void initPCINT() {
	PCICR |= (1 << PCIE2);    // Habilitar puerto D para interrupción por cambio de pin
	PCMSK2 |= (1 << PCINT22); // Habilitar PD6
}

void procesarColor(void)
 {
	if(r_count > g_count && r_count > b_count) colorDetectado = 1; // ROJO
	else if(g_count > r_count && g_count > b_count) colorDetectado = 2; // VERDE
	else if(b_count > r_count && b_count > g_count) colorDetectado = 3; // AZUL
	else colorDetectado = 0;

	// Imprimir resultado sin detener el programa
	if(colorDetectado == 1) cadena_texto("R ");
	else if(colorDetectado == 2) cadena_texto("V ");
	else if(colorDetectado == 3) cadena_texto("A ");
}

//Función para poder enviar caracteres
void writeChar(char caracter){
	while ((UCSR0A & (1 << UDRE0)) == 0); //Indica si aun sigue ocupado la transmisión de datos
	UDR0 = caracter;	//Escribe la información en este registro
}

//Empleamos un puntero para ir mandando cada caracter del string
void cadena_texto(char* texto) {
	while (*texto != '\0') {
		writeChar(*texto);
		texto++;
	}
}


//************************************************************************************
// Interrupt subroutines
ISR(PCINT2_vect) {
//Interrupcipon para un cambio en el pin D6, donde se encuentra el Out
	 uint8_t current = (PIND & (1 << PIND6));

    if (current && !last_state) {
        pulsos_conteo++;  // Solo flanco de subida
    }

    last_state = current;
}

ISR(TIMER0_COMPA_vect) 
{
	// 1. Lógica del Stepper (Cada 10ms un paso)
	if (modo_stepper != 0) {
		if (modo_stepper == 1) paso_actual = (paso_actual + 1) % 4;
		else paso_actual = (paso_actual == 0) ? 3 : paso_actual - 1;

		PORTC &= ~0x0F; // Limpiar PC0-PC3
		PORTC |= (1 << paso_actual);
		} else {
		PORTC &= ~0x0F;
	}

	// 2. Lógica del Sensor de Color (Estado de tiempo no bloqueante)
	color_timer++;
	if (color_timer >= 10) { // Han pasado 100ms (10 * 10ms)
		color_timer = 0;
		
		switch(color_fase) {
			case 0: // Terminó lectura ROJO
			r_count = pulsos_conteo;
			PORTD |= (1<<PORTD5) | (1<<PORTD7); // S2=1, S3=1 (Filtro Verde)
			color_fase = 1;
			break;
			case 1: // Terminó lectura VERDE
			g_count = pulsos_conteo;
			PORTD &= ~(1<<PORTD5); PORTD |= (1<<PORTD7); // S2=0, S3=1 (Filtro Azul)
			color_fase = 2;
			break;
			case 2: // Terminó lectura AZUL
			b_count = pulsos_conteo;
			PORTD &= ~(1<<PORTD5); PORTD &= ~(1<<PORTD7); // S2=0, S3=0 (Filtro Rojo)
			color_fase = 3; // Indicar al main que procese
			break;
		}
		pulsos_conteo = 0; // Reiniciar contador para el siguiente color
	}
}

ISR(TWI_vect)
{
	uint8_t estado = TWSR & 0xF8;

	switch (estado)
	{
		//================ MASTER WRITE =================
		case 0x60: // SLA+W
		indice = 0;
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		case 0x80: // Byte recibido
		if(indice < 4)
		{
			dato_r[indice] = TWDR;
			indice++;
		}
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		case 0xA0: // STOP recibido
		bandera = 1;
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		//================ MASTER READ =================
		case 0xA8: // SLA+R recibido
		case 0xB8: // Dato transmitido, ACK recibido
		TWDR = colorDetectado;   // ? AQUÍ ENVÍAS EL COLOR
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		case 0xC0: // NACK recibido
		case 0xC8:
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		default:
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;
	}
}
