/*
 * uart.c
 *
 * Created: 2019-11-07 11:29:43
 *  Author: osklu414
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdlib.h>

#include "usart.h"
#include "steering.h"

#define BAUD_PRESCALE 25

void
usart_init()
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
}

void usart_transmit(uint8_t data)
{	
	// Wait for empty transmit buffer
	while ( !( UCSR0A & (1<<UDRE0)) );
	// Put data into buffer, sends the data
	UDR0 = data;
}

/*
unsigned char usart_receive(void)
{
	// Wait for data to be received
	while (!(UCSR0A & (1<<RXC0)));
	// Get and return received data from buffer
	return UDR0;
}
*/

// USART rx state
struct rx_state
{
	enum
	{
		NONE = 0,
		PWM = 1,
		DIR = 2,
		IDENTIFIED = 3,

	} current_type;
	uint8_t current_field;
	uint8_t field_buffer[6]; // largest transmission is 6 bytes
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
				steering_identified();
			}
			else
			{
				state.current_type = rx_byte;
			}
			break;
		}
		case PWM:
		{
			state.field_buffer[state.current_field++] = rx_byte; // fill field buffer
			// check if last field reached
			if(state.current_field == 2)
			{
				// call callback with new pwm
				steering_pwm(state.field_buffer[0], state.field_buffer[1]);
				// reset state
				state.current_type = NONE;
				state.current_field = 0;
			}
			break;
		}
		case DIR:
		{
			state.field_buffer[state.current_field++] = rx_byte; // fill field buffer
			// check if last field reached
			if(state.current_field == 2)
			{
				// call callback with new dir
				steering_dir((bool)state.field_buffer[0], (bool)state.field_buffer[1]);
				// reset state
				state.current_type = NONE;
				state.current_field = 0;
			}
			break;
		}
		case IDENTIFIED:
		{
			state.current_type = NONE;
			state.current_field = 0;
			break;
		}
	}
}