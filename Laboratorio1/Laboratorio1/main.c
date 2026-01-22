/*
 * Laboratorio1.c
 *
 * Created: 15/01/2026 19:14:10
 * Author : Bryan Samuel Morales Paredes
 * Juego de carreras
 * botón de “Inicio de Carrera”. Al presionar este botón debe
 iniciarse una secuencia de salida que se mostrará en el display de 7 segmentos.
 La secuencia de salida consistirá en un conteo regresivo de 5 a 0 (5,4,3,2,1,0), y durante
 este conteo, ningún jugador podrá incrementar su contador.
 Solo después de finalizar esta secuencia, la carrera comenzará y los jugadores podrán
 presionar sus botones para incrementar sus contadores.
 */ 

// Encabezado (Libraries)#define F_CPU 16000000#include <avr/io.h>#include <avr/interrupt.h>#include "hex_display.h"uint8_t activar_conteo;uint8_t contador_mostrar_dis;uint8_t total_valores;uint8_t habilitar_jugadores;uint8_t estado_J1;uint8_t estado_J2;uint8_t activar_conteo_J1;uint8_t activar_conteo_J2;
uint8_t reset;
/****************************************/// Function prototypesvoid setup();void initTMR0();/****************************************/// Main Function/****************************************/
// Main Function
int main(void)
{
	setup();

	while (1)
	{
		if (reset == 1)
		{
			// Apagar LEDs J2 (PC4, PC5, PB3, PB4)
			PORTC &= ~((1 << PORTC4) | (1 << PORTC5) | (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3));
			PORTB &= ~((1 << PORTB3) | (1 << PORTB4));
		}
		else
		{
// ================== CONTADOR J1 (PC0 a PC3) ================================
			if (activar_conteo_J1 && habilitar_jugadores)
			{
				activar_conteo_J1 = 0;

				if (estado_J1 == 0x00)
				{
					estado_J1 = 0x01;
				}
				else
				{
					estado_J1 <<= 1; //Desplazo el 1 para encender los leds
				}

				if (estado_J1 > 0x08)
				{
					estado_J1 = 0x0F;
					display_show(1);
					habilitar_jugadores = 0;
				}
			}

			// Actualizar LEDs J1 SIN pisar J2
			PORTC = (PORTC & 0xF0) | estado_J1;

//================= CONTADOR J2 (PC4, PC5, PB3, PB4) ===========================
//Esta lógica es diferente por que utilizo diferentes puertos de diferentes bytes, por lo que con mi lógica enciendo individualmente
			if (activar_conteo_J2 && habilitar_jugadores)
			{
				activar_conteo_J2 = 0;

				//Actualizar estado lógico
				estado_J2++; //Aumento el contador

				// Caso especial: pasó de 4 a ganador
				if (estado_J2 > 4)
				{
					habilitar_jugadores = 0;

					// Mostrar ganador (todos ON)
					PORTC |= (1 << PORTC4) | (1 << PORTC5);
					PORTB |= (1 << PORTB3) | (1 << PORTB4);
					display_show(2);

					estado_J2 = 0;
				}
				else
				{
					// Estado normal 

					// Apagar LEDs de J2
					PORTC &= ~((1 << PORTC4) | (1 << PORTC5));
					PORTB &= ~((1 << PORTB3) | (1 << PORTB4));

					switch (estado_J2) //Un switch case con todos los casos del contador
					{
						case 1: PORTC |= (1 << PORTC4); break;
						case 2: PORTC |= (1 << PORTC5); break;
						case 3: PORTB |= (1 << PORTB3); break;
						case 4: PORTB |= (1 << PORTB4); break;
						default: break;
					}
				}
			}
		}
	}
}

/****************************************/
// NON-Interrupt subroutines
void setup()
{
	cli();

	CLKPR = (1 << CLKPCE); // Habilitar la configuración del oscilador CPU
	CLKPR = (1 << CLKPS2) | (1 << CLKPS1); // 64 PRESCALER -> 1MHz

	display_init(); // Inicializar display por medio de la libreria

	DDRC = 0xFF;   // Configuro PORTC como salida
	PORTC = 0x00;  // PORTC inicialmente apagado (leds)

	DDRC |= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC4);
	PORTC &= ~((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC4));

	DDRB |= (1 << PORTB3) | (1 << PORTB4);
	PORTB &= ~((1 << PORTB3) | (1 << PORTB4));

	// Configuración de botones en PORTB
	DDRB &= ~((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2));
	PORTB |= (1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2);

	// Habilitar interrupción en PB0, PB1, PB2
	PCICR |= (1 << PCIE0);
	PCMSK0 |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2);

	UCSR0B = 0x00; // Deshabilitar comunicación serial

	contador_mostrar_dis = 5;
	estado_J1 = 0;
	estado_J2 = 0;

	initTMR0();

	sei();
}

void initTMR0()
{
	TCCR0A = 0;
	TCCR0B |= (1 << CS02) | (1 << CS00);//Configuro el prescaler
	TCNT0 = 134;
	TIMSK0 = (1 << TOIE0); //Habilito la máscara 
}

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 134;

	if (activar_conteo)
	{
		display_show(contador_mostrar_dis);//Función que me permite ir con un puntero entre el arreglo de la libreria
		reset = 1;
		if (contador_mostrar_dis > 0)
		{
			contador_mostrar_dis--; //Realizo la cuenta regresiva
		}
		else
		{
			//Coloco valores a variables y activo banderas empleadas en el main
			activar_conteo = 0; 
			habilitar_jugadores = 1;
			reset = 0;
			estado_J1 = 0;
			estado_J2 = 0;
		}
	}
}

//Interupción con lógica de botones
ISR(PCINT0_vect)
{
	//Se emplea static para que al momento de entrar y salir de la interrupción aun se mantenga el valor
	//se usa FF por que como está configurado con pull-up el estádo "apagado" es 1
	static uint8_t estado_anterior = 0xFF; 
	uint8_t estado_actual = PINB; //Se lee lo que está en el puerto
	uint8_t cambio = estado_anterior ^ estado_actual; //Se emplea un XOR para indicar que cambió el estado indicando el botón que se presionó

	// Botón en PB0; este inicializa el conteo regresivo 
	if ((cambio & (1 << PINB0)) && !(estado_actual & (1 << PINB0)))//!(estado_actual & (1 << PINB0) ==> indica si está encendido o apagado el botón, tiene el NOT debido al pull-up
	{
		if (!activar_conteo && !habilitar_jugadores) //Evita el reinicio del conteo cuando se está ejecutando y cuando los jugadores están jugando
		{
			//Activo las banderas que me sirven para iniciar el conteo 
			contador_mostrar_dis = 5;
			activar_conteo = 1;
		}
	}

	// Botón en PB1
	if ((cambio & (1 << PINB1)) && !(estado_actual & (1 << PINB1)))//Misma lógica que en PB0
	{
		if (habilitar_jugadores)
		{
			activar_conteo_J1 = 1; //Habilito bandera para iniciar el conteo de J1
		}
	}

	// Botón en PB2
	if ((cambio & (1 << PINB2)) && !(estado_actual & (1 << PINB2)))
	{
		if (habilitar_jugadores)
		{
			activar_conteo_J2 = 1; //Habilito bandera para iniciar el conteo de J2
		}
	}
	//Se coloca esta lógica para conocer el estado en el que se encuentran los botones
	//sin esta lógica el XOR no tendría sentido ya que se está comparando con los mismos valores viejos
	estado_anterior = estado_actual; 
}

