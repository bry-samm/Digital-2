/*
 * SPI_conf.h
 *
 * Created: 29/01/2026 00:06:00
 *  Author: bsmor
 */ 


#ifndef SPI_CONF_H_
#define SPI_CONF_H_

#include <stdio.h>
#include <avr/io.h>

typedef enum
{
	SPI_DATA_ORDER_MSB = 0b00000000,
	SPI_DATA_ORDER_LSB = 0b00100000
	
	}Spi_Data_Order;


typedef enum
{
	SPI_CLOCK_FIRST_EDGE = 0b00000000,
	SPI_CLOCK_LAST_EDGE  = 0b00000100 
	}Spi_Clock_Phase;
	
typedef enum
{
	SPI_CLOCK_IDLE_HIGH = 0b00001000,
	SPI_CLOCK_IDLE_LOW  = 0b00000000
	}Spi_Clock_Polarity;

typedef enum
{
	SPI_MASTER_OSC_DIV2		= 0b01010000,
	SPI_MASTER_OSC_DIV4		= 0b01010001,
	SPI_MASTER_OSC_DIV8		= 0b01010010,
	SPI_MASTER_OSC_DIV16	= 0b01010011,
	SPI_MASTER_OSC_DIV32	= 0b01010100,
	SPI_MASTER_OSC_DIV64	= 0b01010101,
	SPI_MASTER_OSC_DIV128	= 0b01010110,
	SPI_SLAVE_SS			= 0b01000000
	}Spi_Type;

void spiInt(Spi_Type, Spi_Data_Order, Spi_Clock_Polarity, Spi_Clock_Phase);
void spiWrite(uint8_t dat);
unsigned spiDataReady();
uint8_t spiRead(void);

uint8_t spiTransfer(uint8_t data);


#endif /* SPI_CONF_H_ */