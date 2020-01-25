/*
 * uart.c
 *
 * Created: 2019-11-07 11:29:43
 *  Author: osklu414
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdlib.h>

#include "usart.h"



#define BAUD_PRESCALE 25

void
init_usart()
{
	UBRR0H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR0L = (unsigned char)BAUD_PRESCALE;
	
	// set double speed operation
	UCSR0A |= (1<<U2X0);

	// enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	
	// Set frame format: 8 data, 1 stop bit
	UCSR0C = (0<<USBS0)|(3<<UCSZ00);
	
	// Enable the USART receive complete interrupt (USART_RXC)
	UCSR0B |= (1 << RXCIE0);
	// set the global interrupt enable flag
	sei();
}

void usart_transmit(uint8_t data)
{	
	// Wait for empty transmit buffer
	while ( !( UCSR0A & (1<<UDRE0)) );
	// Put data into buffer, sends the data
	UDR0 = data;
}

unsigned char usart_receive(void)
{
	// Wait for data to be received
	while (!(UCSR0A & (1<<RXC0)));
	// Get and return received data from buffer
	return UDR0;
}




void usart_transmit_competition()
{
	usart_transmit(COMPETITION);
}
	

struct rx_state
{
	enum
	{
		NONE = 0,
		IDENTIFIED = 1
	} current_type;
	uint8_t current_field;
	uint8_t field_buffer[6]; // largest transmission size 6 (TBD for sensor)
};
	

ISR(USART0_RX_vect)
{
	// code to be executed when the USART receives a byte here
	static struct rx_state state =
	{
		.current_type = NONE,
		.current_field = 0,
	};
	
	uint8_t rx_byte;
	rx_byte = UDR0; // fetch received byte value into variable
	// UDR0 = rx_byte; // echo back received byte to its transmitter
	
	switch(state.current_type)
	{
		case NONE:
		{
			// identify transmission type
			if(rx_byte == IDENTIFIED)
			{
				sensor_identified();
			}
			else
			{
				state.current_type = rx_byte;
			}
			break;
		}
		
		default:
			break;
	}
}

void usart_transmit_int(int n, int size){
	char str[size];
	sprintf(str, "%d", n);
	
	for (int i = 0; i < size; i++){
		usart_transmit(str[i]);
		_delay_ms(1);
	}
}