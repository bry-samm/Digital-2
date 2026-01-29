/*
 * Lab2_Dig2.c
 * 
 * Created: Laboratorio 2 digital 2
 * Author: Bryan MOrales
 * Description: Despliegue de lectura ADC en LCD
 */
//************************************************************************************
// Encabezado (librerías)
#define F_CPU 16000000#include <avr/io.h>#include <avr/interrupt.h>#include "CONF_ADC.h"#include "LCD_control.h"
//************************************************************************************
// Function prototypes
void setup();
void enviar_numero(valor_mapeado);
void enviar_lectura_completa(lectura_ADC);

void writeChar(char caracter);
void cadena_texto(char* texto);
void mostrar_menu();

uint16_t lectura_ADC;
uint16_t valor_mapeado;
uint8_t unidades;
uint8_t decenas;
uint8_t centenas;

uint8_t canal_actual;
uint16_t ADC3_valor;
uint16_t ADC4_valor;

uint8_t miles_completo;
uint8_t centenas_completo;
uint8_t decenas_completo;
uint8_t unidad_completo;

uint8_t estado;
uint8_t S3_contador;

uint8_t comando_listo;
uint8_t comando_rx;


//************************************************************************************
// Main Function
int main(void)
{
	setup();
	initADC();
	initLCD8bits();
	mostrar_menu();

//En estas variables se guardan los números modificados por la función convertir_4_digitos
	// Variables para S1 (ADC3 mapeado)
	char s1_m, s1_c, s1_d, s1_u;

	// Variables para S2 (ADC4 completo)
	char s2_m, s2_c, s2_d, s2_u;
	
	//Variables para S3 (S3_contador)
	char s3_m, s3_c, s3_d, s3_u;

	while (1)
	{
		// ---- S1 ----
		uint16_t valor_mapeado = (ADC3_valor * 500UL) / 1023;
		convertir_4_digitos(valor_mapeado, &s1_m, &s1_c, &s1_d, &s1_u);

		// ---- S2 ----
		convertir_4_digitos(ADC4_valor, &s2_m, &s2_c, &s2_d, &s2_u);
		
		// ---- S3 -----
		convertir_4_digitos(S3_contador, &s3_m, &s3_c, &s3_d, &s3_u); // usar & sirve para poder dar direcciones y no valores de variables

		// ---- LCD ----
		LCD_Set_Cursor(1,1);
		LCD_Write_String("S1:");
		LCD_Set_Cursor(7,1);
		LCD_Write_String("S2:");
		LCD_Set_Cursor(12,1);
		LCD_Write_String("S3:");

		LCD_Set_Cursor(1,2);
		LCD_Write_Char(s1_c);
		LCD_Write_Char('.');
		LCD_Write_Char(s1_d);
		LCD_Write_Char(s1_u);
		LCD_Write_Char('V');

		LCD_Set_Cursor(7,2);
		LCD_Write_Char(s2_m);
		LCD_Write_Char(s2_c);
		LCD_Write_Char(s2_d);
		LCD_Write_Char(s2_u);
		
		LCD_Set_Cursor(12,2);
		LCD_Write_Char(s3_m);
		LCD_Write_Char(s3_c);
		LCD_Write_Char(s3_d);
		LCD_Write_Char(s3_u);
		
		
	  if (comando_listo)
	  {
		  comando_listo = 0;

		  if (comando_rx == '1') // Se compara con el caracér no con el entero 1, ya que comando_rx guarda valores ASCII
		  {
			  cadena_texto("\nValor potenciometros:\n"); // Se imprime en la hiperterminal

			//Se configura igual que en el main el orden de los dígitos de cada contador
			  cadena_texto("S1: ");
			  writeChar(s1_c); 
			  writeChar('.');
			  writeChar(s1_d); 
			  writeChar(s1_u);
			  cadena_texto(" V\n");

			  cadena_texto("S2: ");
			  writeChar(s2_m);
			  writeChar(s2_c);
			  writeChar(s2_d);
			  writeChar(s2_u);
			  cadena_texto("\n");

			  cadena_texto("S3: ");
			  writeChar(s3_m);
			  writeChar(s3_c);
			  writeChar(s3_d);
			  writeChar(s3_u);
			  cadena_texto("\n\n");

			  mostrar_menu(); // Al final la acción siempre se muestra de nuevo el menú
		  }
		  else if (comando_rx == '+')
		  {
			  S3_contador++; // Aumenta el contador
			  mostrar_menu();
		  }
		  else if (comando_rx == '-')
		  {
			  S3_contador--; // Disminuye el contador
			  mostrar_menu();
			// Debido a que S3_contador es se declaró como una variable de 8 bits el contador llega hasta 255
		  }
		  else
		  {
			  cadena_texto("\nEntrada no valida\n\n");
			  mostrar_menu();
			}
		}
	}
}

//************************************************************************************
// NON-INterrupt subroutines
void setup()
{
	cli();
	initADC();
	initUART();
	canal_actual = 3;
	S3_contador = 0;
	sei();
}
//Función importante para poder mostrar los números en el LCD y en la hiperterminal
//esta función separa el número en dígitos, los convierte en ASCII y guarda los caracteres en variables 

void convertir_4_digitos(uint16_t valor,
char *m, char *c, char *d, char *u) //Para que la función devuelva más de un dato se necesita emplear punteros
{
	//Con el puntero me permite guardar el valor en la dirección que le indico, dicha dirección es la variable para cada contador
	*m = '0';
	*c = '0';
	*d = '0';
	*u = '0';

	if (valor >= 1000) { // Lo separa en miles
		*m = (valor / 1000) + '0';
		valor %= 1000; // % es para conseguir el módulo, este sirve para tener el residuo de la división y así seguir separando el número
	}

	if (valor >= 100) { // Lo separa en centenas
		*c = (valor / 100) + '0';
		valor %= 100;
	}

	if (valor >= 10) { // Lo separa en decenas
		*d = (valor / 10) + '0';
		valor %= 10;
	}

	*u = valor + '0'; 
	//A cada valor se le suma el CARACTER '0' debido a que 0 en ASCII es 48 y de esta forma iniciamos los dígitos desde 48 
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

// Menú el cual se desplegará en la pantalla de la hiperterminal
void mostrar_menu(){
	writeChar(' ');
	writeChar('B');
	writeChar(':');
	writeChar(' ');
	cadena_texto("\n Presione 1 para leer los potenciómetros o +/- para aumentar o decrementar S3");
	cadena_texto("\n 1. Leer potenciómetros");
	cadena_texto("\n	+");
	cadena_texto("\n	-");
}

//************************************************************************************
// Interrupt subroutines
//Debo de alternar entre ADC ya que el arduino puede realizar la lectura pero solo cuenta con un solo conversor ISR(ADC_vect)
{
	if (canal_actual == 3) {
		uint8_t low  = ADCL;   // Lectura de la parte baja del ADC
		uint8_t high = ADCH;	//Lectura de la parte alta del ADC

		ADC3_valor = (high << 8) | low; // Corremos los bits para agregar los 2 bits menos significativos
		
		canal_actual = 4;
		ADMUX = (ADMUX & 0xF0) | 4; // Realizo el cambio a ADC4
	}
	else {
		uint8_t low_4  = ADCL;   // Lectura de la parte baja del ADC
		uint8_t high_4 = ADCH;	//Lectura de la parte alta del ADC

		ADC4_valor = (high_4 << 8) | low_4; // Corremos los bits para agregar los 2 bits menos significativos
		
		canal_actual = 3;
		ADMUX = (ADMUX & 0xF0) | 3; // Realizo el cambio a ADC3
	}

	ADCSRA |= (1 << ADSC); // nueva conversión
}
ISR(USART_RX_vect)
{
	char recibido = UDR0; //Recibo los datos del registro

	if (recibido == '\n' || recibido == '\r') return; // Cuando uno presiona la tecla "enter" se envía \r y \n , esta línea evita procesar estos caracteres 

	comando_rx = recibido;	// El valor obtenido se guarda en una variable global en formato ASCII ya que la variable recibido es tipo char
	comando_listo = 1;
}
