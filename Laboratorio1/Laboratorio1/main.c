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
	
/****************************************/// Function prototypesvoid setup();void initTMR0();/****************************************/// Main Functionint main(void)
{
	setup();

	while (1)
	{
		if(activar_conteo == 1){
			 // Apagar LEDs J1 (PC0–PC3)
			 PORTC &= 0xF0;

			 // Apagar LEDs J2 (PC4, PC5, PB3, PB4)
			 PORTC &= ~((1 << PORTC4) | (1 << PORTC5));
			 PORTB &= ~((1 << PORTB3) | (1 << PORTB4));
		}else{
			
		// ===== CONTADOR J1 (PC0 a PC3) ===== 
		if (activar_conteo_J1 && habilitar_jugadores)
		{
			activar_conteo_J1 = 0;

			if (estado_J1 == 0x00){
			estado_J1 = 0x01;}
			else{
			estado_J1 <<= 1;}
			if (estado_J1 > 0x08){
			estado_J1 = 0x0F;
			display_show(1);
			habilitar_jugadores = 0;}
		}

		// Actualizar LEDs J1 SIN pisar J2
		PORTC = (PORTC & 0xF0) | estado_J1;


		// ===== CONTADOR J2 (PC4, PC5, PB3, PB4) =====
		if (activar_conteo_J2 && habilitar_jugadores)
		{
			activar_conteo_J2 = 0;

			// 1?? Actualizar estado lógico
			estado_J2++;

			// 2?? Caso especial: pasó de 4 ? ganador
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
				// 3?? Estado normal ? one-hot
				// Apagar LEDs de J2
				PORTC &= ~((1 << PORTC4) | (1 << PORTC5));
				PORTB &= ~((1 << PORTB3) | (1 << PORTB4));

				switch (estado_J2)
				{
					case 1: PORTC |= (1 << PORTC4); break;
					case 2: PORTC |= (1 << PORTC5); break;
					case 3: PORTB |= (1 << PORTB3); break;
					case 4: PORTB |= (1 << PORTB4); break;
					default: break;
					estado_J2++;
				}
			}
		}

	}
}
}


/****************************************/// NON-Interrupt subroutinesvoid setup(){
	cli();
	
	CLKPR	= (1 << CLKPCE); //Habilitar la configuración del oscilador CPU
	CLKPR	= (1 << CLKPS2) | (1 << CLKPS1); // 64 PRESCALER -> 1MHz
	
	display_init();		//Inicializar display por medio de la libreria
	
	DDRC = 0xFF;	// Configuro PORTC como salida
	PORTC = 0x00;	// PORTC inicialmente apagado (leds)
	
	DDRC |= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC4);  // PORTC como salida 1 lógico
	PORTC &= ~((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC4));	//Inicialmente apagados
	
	DDRB |= (1 << PORTB3) | (1 << PORTB4);	//Parte de PORTB como salidas (leds)
	PORTB &= ~((1 << PORTB3) | (1 << PORTB4));	//Inicialmente apagados
	
	// Configuración de botones en PORTB
	DDRB &= ~((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2));  // PB0, PB1, PB2 como entradas
	PORTB |= (1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2);    // Activar pull-ups
	
	// Habilitar interrupción en PB1 (PCI0)
	PCICR |= (1 << PCIE0);               // Habilitar interrupciones en PORTB
	PCMSK0 |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2);  // PB0, PB1, PB2
	
	UCSR0B = 0x00;		//Deshabilitar comunicación serial
	
	contador_mostrar_dis = 5;
	estado_J1 = 0;
	estado_J2 = 0;
	
	initTMR0();
	
	sei();
}void initTMR0(){	TCCR0A  = 0;	TCCR0B  |= (1 << CS02) | (1 << CS00);	TCNT0   = 134;	TIMSK0  = (1 << TOIE0);  //Activar la máscara}

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 134;

	if (activar_conteo)
	{
		display_show(contador_mostrar_dis);
		if (contador_mostrar_dis > 0)
		{
			contador_mostrar_dis--;
		}
		else
		{
			activar_conteo = 0;
			habilitar_jugadores = 1;
		}
	}
}

ISR(PCINT0_vect)
{
	if (!(PINB & (1 << PORTB0)))
	{
		if (!activar_conteo && !habilitar_jugadores)
		{
			contador_mostrar_dis = 5;
			activar_conteo = 1;
		}
	}

	if (!(PINB & (1 << PORTB1)))
	{
		if (habilitar_jugadores)
		activar_conteo_J1 = 1;
	}

	if (!(PINB & (1 << PORTB2)))
	{
		if (habilitar_jugadores)
		activar_conteo_J2 = 1;
	}
}
/*
ISR(PCINT0_vect) {
	// Verificar estado actual de los botones
	if (!(PINB & (1 << PORTB0))) {  // Si PB0 está presionado
		activar_conteo = 1;
	}
	if (!(PINB & (1 << PORTB1))) {  // Si PB1 está presionado
		activar_conteo_J1 = 1;
	}
	if (!(PINB & (1 << PORTB2))) {  // Si PB2 está presionado
		activar_conteo_J2 = 1;
	}
}
*/

