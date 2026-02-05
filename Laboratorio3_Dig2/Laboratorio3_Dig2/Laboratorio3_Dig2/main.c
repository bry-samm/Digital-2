/*
 * NombreProgra.c
 * 
 * Created:
 * Author:
 * Description:
 */
//************************************************************************************
// Encabezado (librerías)
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "SPI_conf.h"
#include "initUART.h"

//************************************************************************************
// Function prototypes
void setup();
void refreshPORT(uint8_t valor);
uint8_t convertir_cadena_a_byte(char *cadena);
void mostrar_menu(void);
void convertir_3_digitos(uint16_t numero, char *c, char *d, char *u);

#define RX_BUF_SIZE 4

volatile char rx_buffer[RX_BUF_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t comando_listo = 0;
volatile char comando_rx = 0;
volatile uint8_t estado = 0;

uint8_t iniciar_comunicacion = 1;

uint8_t valorSPI = 0;
uint8_t valorSPI_2 = 0;


uint8_t recibido;

uint8_t numero_convertido;

//************************************************************************************
// Main Function
int main(void)
{
	setup();
	initUART();
	mostrar_menu();
	//Configuro los bits a utilizar como salida
	DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
	DDRB |= (1 << DDB0) | (1 << DDB1);
	
	// MSS para seleccionar el esclavo
	PORTC &= ~(1 << PORTC5);
	
	//Limpiando el puerto de leds = 0;
	PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7));
	PORTB &= ~((1 << PORTB0) | (1 << PORTB1));
	
	
	spiInt(SPI_MASTER_OSC_DIV2, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
	
		// Variables para S1 (ADC3 mapeado)
		char s1_c, s1_d, s1_u;

		// Variables para S2 (ADC4 completo)
		char s2_c, s2_d, s2_u;
		
		DDRC |= (1 << DDC5);   // PC5 como salida
		PORTC |= (1 << PORTC5); // SS en HIGH por defecto


	while (1)
	{
		
		if (comando_listo)
		{
			comando_listo = 0;

			// ===== MENÚ =====
			if (estado == 0) {
				if (comando_rx == '1') {
					PORTC &= ~(1 << PORTC5); // Selector de escalvo = 0 para iniciar comunicación
					_delay_ms(300);
					spiWrite('c'); //Envio el caracter de comando de inicialización
					_delay_ms(500);
					valorSPI = spiRead(); //Guardo el valor de entrada
					PORTC |= (1 << PORTC5);
					
					PORTC &= ~(1 << PORTC5);
					_delay_ms(300);
					spiWrite('a'); //Envío otro caractér para que la lógica del esclavo sea más simple
					_delay_ms(500);
					valorSPI_2 = spiRead(); //Guardo el valor de entrada
					PORTC |= (1 << PORTC5);
					_delay_ms(400);
					
					convertir_3_digitos(valorSPI, &s1_c, &s1_d, &s1_u);
					convertir_3_digitos(valorSPI_2, &s2_c, &s2_d, &s2_u);


					cadena_texto("\nValor potenciometros:\n"); // Se imprime en la hiperterminal
					cadena_texto("S1: ");
					writeChar(s1_c);
					writeChar(s1_d);
					writeChar(s1_u);
					cadena_texto("\n");
					
					cadena_texto("S2: ");
					writeChar(s2_c);
					writeChar(s2_d);
					writeChar(s2_u);
					cadena_texto("\n\n");
					_delay_ms(250);

					mostrar_menu();
					
				}
				else if (comando_rx == '2') {
					cadena_texto("\n Ingrese número\n");
					estado = 1;
					iniciar_comunicacion = 0;
				}
				else {
					cadena_texto("\n Entrada no válida \n");
					mostrar_menu();
				}
			}

			// ===== INGRESO DE NÚMERO =====
			else if (estado == 1) {

				uint8_t numero_convertido =
				convertir_cadena_a_byte((char*)rx_buffer);

				refreshPORT(numero_convertido);

				cadena_texto("\nValor recibido: ");
				writeChar((numero_convertido / 100) + '0');
				writeChar(((numero_convertido / 10) % 10) + '0');
				writeChar((numero_convertido % 10) + '0');
				cadena_texto("\n");
				
				PORTC &= ~(1 << PORTC5); // Selector de escalvo = 0 para iniciar comunicación

				spiWrite(numero_convertido); //Envio el caracter de comando de inicialización
				spiRead();
				PORTC |= (1 << PORTC5);
				
				estado = 0;
				mostrar_menu();				
			}
		}
	}
}
	
//********************************************************************************************
// NON-INterrupt subroutines

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

void setup()
{
	initUART();
	sei();
}

void convertir_3_digitos(uint16_t numero, char *c, char *d, char *u) //Para que la función devuelva más de un dato se necesita emplear punteros
{
	//Con el puntero me permite guardar el valor en la dirección que le indico, dicha dirección es la variable para cada contador
	*c = '0';
	*d = '0';
	*u = '0';

	if (numero >= 100) { // Lo separa en centenas
		*c = (numero / 100) + '0';
		numero %= 100;
	}

	if (numero >= 10) { // Lo separa en decenas
		*d = (numero / 10) + '0';
		numero %= 10;
	}

	*u = numero + '0';
	//A cada valor se le suma el CARACTER '0' debido a que 0 en ASCII es 48 y de esta forma iniciamos los dígitos desde 48
}

// Menú el cual se desplegará en la pantalla de la hiperterminal
void mostrar_menu(){
	writeChar(' ');
	writeChar('B');
	writeChar(':');
	writeChar(' ');
	cadena_texto("\n Ingrese el número para ejecutar la opción");
	cadena_texto("\n 1. Leer potenciómetro");
	cadena_texto("\n 2. Enviar Ascii");
}


uint8_t convertir_cadena_a_byte(char *cadena) {
	uint16_t valor_string = 0; // Usamos 16 bits para detectar si se pasa de 255
	uint8_t i = 0;

	// Recorremos la cadena hasta encontrar el final '\0'
	while (cadena[i] != '\0') {
		// Verificamos que sea un número válido (0-9)
		if (cadena[i] >= '0' && cadena[i] <= '9') {
			
			// Lógica matemática: (Valor_Anterior * 10) + Nuevo_Digito
			valor_string = (valor_string * 10) + (cadena[i] - '0');
			
			// Si supera 255 se devuelve 255 para que no haya problemas
			if (valor_string > 255) {
				return 255;
			}
		}
		i++;
	}

	return (uint8_t)valor_string; // Retornamos el valor convertido a 8 bits
}

//************************************************************************************
// Interrupt subroutines

/*
//En el caso de ser escalvo ver grabación clase 01:22:55
ISR(SPI_STC_vect){
	uint8_t spiValor = SPDR;
	if(spiValor == 'c');
		SPDR = valorADC; //en este caso se carga el valor de la lectura del ADC
}

//En el main tengo que habilitar la interrupción:
	//SPCR |= (1 << SPIE);
	
*/

ISR(USART_RX_vect)
{
	char c = UDR0;	// Leer el carácter recibido por UART

	// Cuando se presiona ENTER se marca el fin del número recibido
	if (c == '\r' || c == '\n') {  // Verificar si es ENTER
		if (estado == 1) {         // Solo si estamos en modo número
			rx_buffer[rx_index] = '\0'; // Cerrar la cadena de caracteres
			rx_index = 0;          // Reiniciar índice del buffer
			comando_listo = 1;     // Avisar que el dato está listo
		}
		return;		// Salir de la interrupción
	}

	if (estado == 0) {		// Si el programa está en modo menú
		comando_rx = c;		// Guardar la opción del menú
		comando_listo = 1;	// Avisar que hay una opción nueva
		return;				// Salir de la interrupción
	}

	if (estado == 1) {             // Si el programa está en modo número
		if (c >= '0' && c <= '9' && rx_index < RX_BUF_SIZE - 1) { // Validar dígito y espacio
			rx_buffer[rx_index++] = c; // Guardar dígito y avanzar posición
		}
	}
}




