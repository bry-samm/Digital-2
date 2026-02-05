/*
 * SPI_conf.c
 *
 * Created: 29/01/2026 00:05:49
 *  Author: bsmor
 */ 
 #define F_CPU 16000000
 #include <stdio.h>
 #include <avr/io.h>
 #include "SPI_conf.h"
 
 void spiInt(Spi_Type sType, Spi_Data_Order sDataOrder, Spi_Clock_Polarity sClockPolarity, Spi_Clock_Phase sClockPhase)
//Configuro para ser maestro o esclavo
//================= Si soy MAESTRO ==================
	// SS -> 
	// MOSI -> OUT
	// MISO -> IN
	// SCK -> OUT
	
//COnfiguración actual
	// PB2 -> SS
	// PB3 -> MOSI
	// PB4 -> MISO
	// PB5 -> SCK

{
	if(sType & (1 << MSTR)) //Si es modo maestro
	//Recordar que SS (slave select) no lo tengo que configurar
	{
		DDRB |= (1 << DDB3) | (1 << DDB5) | (1 << DDB2); //Configuro pines para MOSI como salida, SCK, NEGADO_SS
		DDRB &= ~(1 << DDB4);  //MISO como entrada 
		SPCR |= (1 << MSTR);   // modo maestro
		
		uint8_t temp = sType & 0b00000111;
		switch(temp){
			case 0: // DIV2
				SPCR &= ~((1 << SPR1) | (1 << SPR0));
				SPSR  |= (1 << SPI2X);
			break;
			case 1: // DIV4
				SPCR &= ~((1 << SPR1) | (1 << SPR0));
				SPSR  &= ~(1 << SPI2X);
			break;
			case 2: // DIV8
				SPCR |= (1 << SPR0);
				SPCR &= ~(1 << SPR1);
				SPSR  |= (1 << SPI2X);
			break;
			case 3: // DIV16
				SPCR &= ~(1 << SPR1);
				SPCR |= (1 << SPR0);
				SPSR  &= ~(1 << SPI2X);
			break; 
			case 4: // DIV32
				SPCR |= (1 << SPR1);
				SPCR &= ~(1 << SPR0);
				SPSR  |= (1 << SPI2X);
			break;
			case 5: // DIV64
				SPCR |= (1 << SPR1);
				SPCR &= ~(1 << SPR0);
				SPSR  &= ~(1 << SPI2X);
			break;
			case 6: // DIV128
				SPCR |= (1 << SPR1) | (1 << SPR0);
				SPSR  &= ~(1 << SPI2X);
			break;
		}
	}
	else // Si es modo esclavo
	{
		//Configurar MOSI, SCK y SS
		DDRB |= (1 << DDB4);  //MISO como salida	
		DDRB &= ~((1 << DDB3) | (1 << DDB5) | (1 << DDB2)); //Configuro pines para MOSI como entrada, SCK, SS
		SPCR &= ~(1 << MSTR);  //Modo esclavo
	}
	//Habilitar SPI, Data Order, Clock Polarity, Clock Phase
	SPCR |= (1 << SPE) | sDataOrder | sClockPolarity | sClockPhase;
}

/*
void spiWrite(uint8_t dat)
{
	SPDR = dat;                         // inicia transmisión
	//while (!(SPSR & (1 << SPIF)));      // espera a que termine
}
*/

static void spiReceiveWait(void)
{
	while (!(SPSR & (1 << SPIF))); //Esperar hasta que la carga de información esté completa
}

void spiWrite(uint8_t dat) // Escribe la información al SPI bus
{
	SPDR = dat;
}

unsigned spiDataReady() // Confirma si la data está lista para ser leida
{
	if(SPSR & (1 << SPIF)) //Verifica que la bandera esté activa
	return 1;
	else 
	return 0;
}

uint8_t spiRead(void) //Lee la data
{
	while (!(SPSR & (1 << SPIF))); //Espera que la data este completa, cuando se active la bandera	
	return (SPDR); // Lee la data recibida del buffer
}

uint8_t spiTransfer(uint8_t data)
{
	SPDR = data;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

