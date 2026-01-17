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

// Encabezado (Libraries)#define F_CPU 16000000#include <avr/io.h>#include <avr/interrupt.h>uint8_t activar_conteo;uint8_t contador_mostrar_dis;uint8_t total_valores;uint8_t valores_display[6] = {0x37, 0x1B, 0x3D, 0x6D, 0x18, 0x7E};

	
/****************************************/// Function prototypesvoid setup();void initTMR0();/****************************************/// Main Functionint main(void)
{
	setup();  // Configurar
	
	while (1)
	{
	}
}

/****************************************/// NON-Interrupt subroutinesvoid setup(){
	cli();
	
	CLKPR	= (1 << CLKPCE); //Habilitar la configuración del oscilador CPU
	CLKPR	= (1 << CLKPS2) | (1 << CLKPS1); // 64 PRESCALER -> 1MHz
	
	DDRD = 0xFF;     // Configuro PORTD como salida
	PORTD = 0x00;    // PORTD inicialmente apagado (LEDS)
	
	// Configuración de botones en PORTB
	DDRB &= ~((1 << PORTB0));  // PB0 como entradas
	PORTB |= (1 << PORTB0);    // Activar pull-ups
	
	// Habilitar interrupción en PB1 (PCI0)
	PCICR |= (1 << PCIE0);               // Habilitar interrupciones en PORTB
	PCMSK0 |= (1 << PCINT0);  // PB0 y PB1 
	
	UCSR0B = 0x00;
	
	total_valores = 6;
	contador_mostrar_dis = 0;
	
	initTMR0();
	
	sei();
}void initTMR0(){	TCCR0A  = 0;	TCCR0B  |= (1 << CS02) | (1 << CS00);	TCNT0   = 134;	TIMSK0  = (1 << TOIE0);  //Activar la máscara}

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 134;
	if (activar_conteo ==1)
	{
		PORTD = valores_display[contador_mostrar_dis];
		contador_mostrar_dis++;
		if (contador_mostrar_dis >= total_valores)
		{
			contador_mostrar_dis = 0;
			activar_conteo = 0;
		}
	}
}


ISR(PCINT0_vect) {
	// Verificar estado actual de los botones
	if (!(PINB & (1 << PORTB0))) {  // Si PB0 está presionado
		activar_conteo = 1;
	}
}
