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
#define F_CPU 16000000

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "I2C_conf/I2C_conf.h"
#include "LCD_control/LCD_control.h"
#include "initUART/initUART.h"

#define slave1 0x30
#define slave2 0x40

#define slave1R (0x30 << 1) | 0x01 //Pongo el último bit en 1 para lectira
#define slave1W (0x30 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir

uint8_t direccion;
uint8_t temp;
uint8_t bufferI2C = 0;


//************************************************************************************
// Function prototypes
void setup();
void refreshPORT(uint8_t valor);
void convertir_3_digitos(uint16_t numero, char *c, char *d, char *u);
void writeChar(char caracter);
void cadena_texto(char* texto);

//************************************************************************************
// Main Function
int main(void)
{
	setup();
	initLCD8bits();
	//Puerto de salida para leds (en este caso sería el LCD)
	
	DDRB |= (1 << DDB5); //Inicializo led del arduino, esto solo me va a servir para ver si si se está comunicando 
	PORTB &= ~(1 << PORTB5);
	
	I2C_Master_Init(100000, 1); //Inicializar como MAster F_SCL 100kHz, prescaler 1
	
	//En estas variables se guardan los números modificados por la función convertir_3_digitos
	char s1_m, s1_c, s1_d, s1_u;
	
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
		if (!I2C_Master_RepeatedStart()){ // Si no recibe nada reinicia
			I2C_Master_Stop();// SI no recibo nada termino la comunicación con return
			return;
		}
		if(!I2C_Master_Write(slave1R)){ //Si si se da la comunicación entonces leer 
			I2C_Master_Stop();
			return;
		}
		//Esto se ejecuta en el caso que si se da la comunicación
		I2C_Master_Read(&bufferI2C, 0); //Esto es solo para recibir un solo byte de info ya que no se manda ACK
		
		//Esto es para recopilar varios bytes ya que si se manda ACK
		//I2C_Master_Read(&bufferI2C1, 1);
		//I2C_Master_Read(&bufferI2C2, 1);
		//I2C_Master_Read(&bufferI2C3, 1);
		
		I2C_Master_Stop(); //Finalizamos
		
		PORTB &= ~(1 << PORTB5); //Apago el led para indicar que ya no hay comunicación
		
		//refreshPORT(bufferI2C); //Muestro los valores
		
		convertir_3_digitos(bufferI2C, &s1_c, &s1_d, &s1_u);
		
		LCD_Set_Cursor(1,1);
		LCD_Write_String("Dis:");

		LCD_Set_Cursor(1,2);
		LCD_Write_Char(s1_c);
		LCD_Write_Char(s1_d);
		LCD_Write_Char(s1_u);
		
	}
}
//************************************************************************************
// NON-INterrupt subroutines
void setup()
{
	sei();
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

//************************************************************************************
// Interrupt subroutines

/*

//========================= ENCABEZADOS =========================
#define F_CPU 16000000

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "I2C_conf/I2C_conf.h"
#include "LCD_control/LCD_control.h"

//========================= DIRECCIONES =========================
#define SLAVE1  0x30
#define SLAVE2  0x40

#define SLA_W(addr)   ((addr << 1) | 0)
#define SLA_R(addr)   ((addr << 1) | 1)

//========================= PROTOTIPOS ==========================
void setup(void);
uint8_t leer_esclavo(uint8_t addr, uint8_t *dato);
uint8_t enviar_motor(uint8_t addr, uint8_t motor);
void convertir_3_digitos(uint16_t numero, char *c, char *d, char *u);

//========================= MAIN ================================
int main(void)
{
	setup();
	initLCD8bits();

	DDRB |= (1 << DDB5);   // LED debug
	PORTB &= ~(1 << PORTB5);

	I2C_Master_Init(100000, 1); // 100kHz

	char c, d, u;
	uint8_t distancia1 = 0;
	uint8_t distancia2 = 0;

	while (1)
	{
		PORTB |= (1 << PORTB5);

		//=========== LEER ESCLAVO 1 ===========
		if (leer_esclavo(SLAVE1, &distancia1))
		{
			convertir_3_digitos(distancia1, &c, &d, &u);

			LCD_Set_Cursor(1,1);
			LCD_Write_String("S1:");

			LCD_Set_Cursor(1,2);
			LCD_Write_Char(c);
			LCD_Write_Char(d);
			LCD_Write_Char(u);
		}

		_delay_ms(5);

		//=========== LEER ESCLAVO 2 ===========
		if (leer_esclavo(SLAVE2, &distancia2))
		{
			convertir_3_digitos(distancia2, &c, &d, &u);

			LCD_Set_Cursor(9,1);
			LCD_Write_String("S2:");

			LCD_Set_Cursor(9,2);
			LCD_Write_Char(c);
			LCD_Write_Char(d);
			LCD_Write_Char(u);
		}

		_delay_ms(5);

		//=========== ENVIAR COMANDO MOTOR ===========
		enviar_motor(SLAVE1, 1);   // motor ON esclavo 1
		//_delay_ms(500);
		//enviar_motor(SLAVE1, 0);   // motor OFF esclavo 1

		PORTB &= ~(1 << PORTB5);
		_delay_ms(500);
	}
}

//========================= FUNCIONES ============================

void setup(void)
{
	sei();
}

//----------- LEER DISTANCIA -----------
uint8_t leer_esclavo(uint8_t addr, uint8_t *dato)
{
	if (!I2C_Master_Start()) goto error;
	if (!I2C_Master_Write(SLA_W(addr))) goto error;
	if (!I2C_Master_Write('R')) goto error;

	if (!I2C_Master_RepeatedStart()) goto error;
	if (!I2C_Master_Write(SLA_R(addr))) goto error;

	I2C_Master_Read(dato, 0); // 1 byte, NACK
	I2C_Master_Stop();
	return 1;

	error:
	I2C_Master_Stop();
	return 0;
}

//----------- ENVIAR COMANDO MOTOR -----------
uint8_t enviar_motor(uint8_t addr, uint8_t motor)
{
	if (!I2C_Master_Start()) goto error;
	if (!I2C_Master_Write(SLA_W(addr))) goto error;
	if (!I2C_Master_Write('M')) goto error;
	if (!I2C_Master_Write(motor)) goto error;

	I2C_Master_Stop();
	return 1;

	error:
	I2C_Master_Stop();
	return 0;
}

//----------- CONVERTIR A ASCII -----------
void convertir_3_digitos(uint16_t numero, char *c, char *d, char *u)
{
	*c = '0';
	*d = '0';
	*u = '0';

	if (numero >= 100) {
		*c = (numero / 100) + '0';
		numero %= 100;
	}
	if (numero >= 10) {
		*d = (numero / 10) + '0';
		numero %= 10;
	}
	*u = numero + '0';
}
*/