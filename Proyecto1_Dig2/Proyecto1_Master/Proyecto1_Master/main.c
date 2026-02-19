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
#include <math.h> //Incluido para realiza operaciones matematicas
#include "I2C_conf/I2C_conf.h"
#include "LCD_control/LCD_control.h"
#include "initUART/initUART.h"

#define slave1 0x30
#define slave2 0x40

#define slave1R (0x30 << 1) | 0x01 //Pongo el último bit en 1 para lectira
#define slave1W (0x30 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir

#define slave2R (0x40 << 1) | 0x01 //Pongo el último bit en 1 para lectira
#define slave2W (0x40 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir

uint8_t bufferI2C = 0;
uint8_t color = 0;

volatile int16_t angulo_x;
volatile int16_t angulo_y;
volatile uint8_t autorizado = 0;
volatile uint8_t movimiento_motor = 0;


volatile uint16_t angulox_IO = 0;
volatile uint16_t anguloy_IO = 0;
volatile uint16_t distancia_IO = 0;
volatile uint8_t disparo_IO = 0;
volatile uint8_t autorizacion_IO = 0;

volatile uint8_t datos_listos = 0;


// ================== MELMAN =============================

// --- DIRECCIONES I2C ---
#define MPU_ADDR 0x68
#define MPU_W (MPU_ADDR << 1)
#define MPU_R ((MPU_ADDR << 1) | 1)

// ----Dirección un esclvo----------
// #define slave1 0x30
// #define slave1R (0x30 << 1) | 0x01 //Pongo el último bit en 1 para lectura
// #define slave1W (0x30 << 1) & 0b11111110 //Pongo el último bit en 0 para escribir

// --- REGISTROS ---
#define PWR_MGMT_1   0x6B
#define ACCEL_XOUT_H 0x3B

// --- VARIABLES GLOBALES PARA POSICIÓN ---
// Se declaran fuera para que guarden su valor entre ciclos
float vel_X = 0, vel_Y = 0, vel_Z = 0;
float pos_X = 0, pos_Y = 0, pos_Z = 0;

//************************************************************************************
// Function prototypes
void setup();
void refreshPORT(uint8_t valor);
void convertir_3_digitos(uint16_t numero, char *c, char *d, char *u);
void writeChar(char caracter);
void cadena_texto(char* texto);

void enviar_comando();
void comunicar_distancia();

void imprimirNumero(uint16_t num);
//====================== MELMAN ==================================
// --- PROTOTIPOS ---
void MPU6050_Init(void);
void angulo_giro(void);
void Enviar_Numero(int16_t numero);
void Enviar_angulos(int16_t anguloX, int16_t anguloY);

void enviar_paquete_ESP32(void);
//************************************************************************************
// Main Function
int main(void)
{
	setup();
	//En estas variables se guardan los números modificados por la función convertir_3_digitos
	char s1_m, s1_c, s1_d, s1_u;
	
	while (1)
	{
		
		angulo_giro();
		comunicar_distancia();
		
		enviar_paquete_ESP32();
		
		convertir_3_digitos(bufferI2C, &s1_c, &s1_d, &s1_u);
		
		LCD_Set_Cursor(1,1);
		LCD_Write_String("Dis");
		
		LCD_Set_Cursor(5,1);
		LCD_Write_String("Ang");
		
		LCD_Set_Cursor(9,1);
		LCD_Write_String("Gir");
		
		LCD_Set_Cursor(13,1);
		LCD_Write_String("Col");
		

		LCD_Set_Cursor(1,2);
		LCD_Write_Char(s1_c);
		LCD_Write_Char(s1_d);
		LCD_Write_Char(s1_u);
		writeChar(s1_c);
		writeChar(s1_d);
		writeChar(s1_u);

		PORTB ^= (1 << PORTB5);
		
	}
}
//************************************************************************************
// NON-INterrupt subroutines
void setup()
{
	initLCD8bits();
	initUART();
	MPU6050_Init();
	//Puerto de salida para leds (en este caso sería el LCD)
	
	DDRB |= (1 << DDB5); //Inicializo led del arduino, esto solo me va a servir para ver si si se está comunicando
	PORTB &= ~(1 << PORTB5);
	
	I2C_Master_Init(100000, 1); //Inicializar como MAster F_SCL 100kHz, prescaler 1
		
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

void enviar_comando()
{
	if(I2C_Master_Start())
	{
		if(I2C_Master_Write(slave2W))
		{
			// Primero enviar N o F
			if(bufferI2C <= 20){
				I2C_Master_Write('N');
				movimiento_motor = 1;
			}
			else{
				I2C_Master_Write('F');
				movimiento_motor = 0;
			}

			//evaluar color 
			if(color == 1){
				LCD_Set_Cursor(13,2);
				LCD_Write_String("R");
				I2C_Master_Write('X');
				autorizado = 0;
			}else if (color == 2 || disparo_IO == 1){
				LCD_Set_Cursor(13,2);
				LCD_Write_String("V");
				I2C_Master_Write('D');
				autorizado = 1;
			}else if (color == 3){
				LCD_Set_Cursor(13,2);
				LCD_Write_String("A");
				autorizado = 0;
		}
		
		}
		I2C_Master_Stop();
	}
	else
	{
		I2C_Master_Stop();
		cadena_texto("Error: escalvo no responde\r\n");
	}
}


void comunicar_distancia()
{
	// ===== Leer distancia del ultrasonico (0x40) =====
	I2C_Master_Start();
	I2C_Master_Write(slave2W);
	I2C_Master_RepeatedStart();
	I2C_Master_Write(slave2R);
	I2C_Master_Read(&bufferI2C, 0);
	I2C_Master_Stop();

	// ===== Leer color del esclavo 0x30 =====
	I2C_Master_Start();
	I2C_Master_Write(slave1W);
	I2C_Master_RepeatedStart();
	I2C_Master_Write(slave1R);
	I2C_Master_Read(&color, 0);
	I2C_Master_Stop();

	enviar_comando();
}

//======================= MELMAN =====================================
// Función para calcular el angulo de giro
void angulo_giro(void)
{
    uint8_t high, low;
    int16_t Ax, Ay, Az;
    double ang_x, ang_y;

    // --- Lectura I2C ---
    I2C_Master_Start();
	I2C_Master_Write(MPU_W); 
	I2C_Master_Write(ACCEL_XOUT_H);
    I2C_Master_RepeatedStart(); 
	I2C_Master_Write(MPU_R);
    
    I2C_Master_Read(&high, 1); 
	I2C_Master_Read(&low, 1); Ax = (high << 8) | low;
    I2C_Master_Read(&high, 1); 
	I2C_Master_Read(&low, 1); Ay = (high << 8) | low;
    I2C_Master_Read(&high, 1); 
	I2C_Master_Read(&low, 0); Az = (high << 8) | low; 
    I2C_Master_Stop();

    // --- Matemáticas para hacer el calculo del angulo en los ejes
	ang_x = atan2(Ay, Az) * 57.296;
	ang_y = atan2(-Ax, Az) * 57.296;
	
    // Envio de datos por el serial para verificar funcionamienot
    cadena_texto("ANGULO -> X: "); 
	Enviar_Numero((int16_t)ang_x);
	if (ang_x <= -30){
		LCD_Set_Cursor(5,2);
		LCD_Write_String("->");
	}else if (ang_x >= 30){
		LCD_Set_Cursor(5,2);
		LCD_Write_String("<-");
	}else{
		LCD_Set_Cursor(5,2);
		LCD_Write_String("--");
	}
	if (ang_y <= -45){
		LCD_Set_Cursor(9,2);
		LCD_Write_String("Izq");
		}else if (ang_y >= 45){
		LCD_Set_Cursor(9,2);
		LCD_Write_String("Der");
		}else{
		LCD_Set_Cursor(9,2);
		LCD_Write_String("---");
	}
	
	angulo_x = ang_x;
	angulo_y = ang_y;
	
    cadena_texto(" deg | Y: "); 
	Enviar_Numero((int16_t)ang_y);
    cadena_texto(" deg\r\n");
	Enviar_angulos((int16_t) ang_x, (int16_t) ang_y); 	
}

void Enviar_angulos(int16_t anguloX, int16_t anguloY)
{
	 I2C_Master_Start();
	 
	 if(I2C_Master_Write(slave1W))
	 { 
		 /*
		 I2C_Master_Write(anguloX); // Byte 1: Angulo X
		 I2C_Master_Write(anguloY); // Byte 2: Angulo Y
		 I2C_Master_Stop();
		 */
		 I2C_Master_Write((uint8_t)(anguloX >> 8)); // byte alto
		 I2C_Master_Write((uint8_t)(anguloX & 0xFF)); // byte bajo

		 // Ángulo Y
		 I2C_Master_Write((uint8_t)(anguloY >> 8));
		 I2C_Master_Write((uint8_t)(anguloY & 0xFF));

		 I2C_Master_Stop();
	 }
	 else
	 {
		 // Si el esclavo no responde, liberar el bus para no colgar el programa
		 I2C_Master_Stop();
		 cadena_texto("Error: Esclavo no responde\r\n");
	 }
}
// Funciones para inicar y enviar los numeros por serial 
void MPU6050_Init(void) {
    I2C_Master_Start(); I2C_Master_Write(MPU_W); 
    I2C_Master_Write(PWR_MGMT_1); I2C_Master_Write(0x00);
    I2C_Master_Stop(); _delay_ms(10);
}

void Enviar_Numero(int16_t numero) {
    char temp[7]; uint8_t i = 0;
    if (numero == 0) { writeChar('0'); return; }
    if (numero < 0) { writeChar('-'); numero = -numero; }
    while (numero > 0) { temp[i++] = (numero % 10) + '0'; numero /= 10; }
    while (i > 0) { writeChar(temp[--i]); }
}

void enviar_paquete_ESP32(void)
{
	cadena_texto("<");

	Enviar_Numero(angulo_x);
	cadena_texto(",");

	Enviar_Numero(angulo_y);
	cadena_texto(",");

	Enviar_Numero(bufferI2C);
	cadena_texto(",");

	Enviar_Numero(autorizado);
	cadena_texto(",");
	
	Enviar_Numero(movimiento_motor);
	cadena_texto(",");

	cadena_texto(">\n");
}

//************************************************************************************
// Interrupt subroutines

#define BUFFER_SIZE 40

volatile char buffer[BUFFER_SIZE];
volatile uint8_t index = 0;

ISR(USART_RX_vect) {

	char recibido = UDR0;

	if (recibido == '\n' || recibido == '\r') {

		buffer[index] = '\0';

		uint16_t valores[5] = {0};
		uint8_t val_idx = 0;
		uint16_t temp = 0;

		for (uint8_t i = 0; i <= index; i++) {

			char c = buffer[i];

			if (c >= '0' && c <= '9') {
				temp = temp * 10 + (c - '0');
			}
			else if (c == ',' || c == '\0') {

				if (val_idx < 5) {
					valores[val_idx++] = temp;
					temp = 0;
				}
			}
		}

		// ===== GUARDAR CADA VALOR EN SU VARIABLE =====

		if (val_idx >= 5) {

			angulox_IO     = valores[0];
			anguloy_IO     = valores[1];
			distancia_IO   = valores[2];
			disparo_IO     = valores[3];
			autorizacion_IO = valores[4];

			datos_listos = 1;
		}

		index = 0;
	}
	else {

		if (index < BUFFER_SIZE - 1) {
			buffer[index++] = recibido;
		}
		else {
			index = 0;  // Protección overflow
		}
	}
}


/*
#define BUFFER_SIZE 20

volatile char buffer[BUFFER_SIZE];
volatile uint8_t index = 0;
volatile uint8_t paquete_listo = 0;

ISR(USART_RX_vect)
{
	char c = UDR0;

	if (c == '\n')  // fin de paquete
	{
		buffer[index] = '\0';
		paquete_listo = 1;
		index = 0;
	}
	else
	{
		if (index < BUFFER_SIZE - 1)
		{
			buffer[index++] = c;
		}
		else
		{
			index = 0;  // protección overflow
		}
	}
}
*/