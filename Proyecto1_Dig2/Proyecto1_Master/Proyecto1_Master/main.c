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

//====================== MELMAN ==================================
// --- PROTOTIPOS ---
void MPU6050_Init(void);
void angulo_giro(void);
void Enviar_Numero(int16_t numero);
void Enviar_angulos(int16_t anguloX, int16_t anguloY);

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
		
		convertir_3_digitos(bufferI2C, &s1_c, &s1_d, &s1_u);
		
		LCD_Set_Cursor(1,1);
		LCD_Write_String("Dis:");
		
		LCD_Set_Cursor(6,1);
		LCD_Write_String("Ang:");
  
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
			if(bufferI2C <= 20)
			I2C_Master_Write('N');
			else
			I2C_Master_Write('F');
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
	I2C_Master_Start();
	I2C_Master_Write(slave2W);
	I2C_Master_RepeatedStart();
	I2C_Master_Write(slave2R);
	
	I2C_Master_Read(&bufferI2C, 0);
	I2C_Master_Stop(); //Finalizamos
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
		LCD_Set_Cursor(6,2);
		LCD_Write_String("->");
	}else if (ang_x >= 30){
		LCD_Set_Cursor(6,2);
		LCD_Write_String("<-");
	}else{
		LCD_Set_Cursor(6,2);
		LCD_Write_String("-");
	}
	
    cadena_texto(" deg | Y: "); 
	Enviar_Numero((int16_t)ang_y);
    cadena_texto(" deg\r\n");
	//Enviar_angulos((int16_t) ang_x, (int16_t) ang_y); 	
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
//************************************************************************************
// Interrupt subroutines