/*
 * usart.h
 *
 * Created: 2019-11-07 13:24:20
 *  Author: osklu414
 */ 


#ifndef USART_H_
#define USART_H_


enum tx_type
{
	MEASUREMENT = 7,
	COMPETITION = 2
};


void
init_usart();


void
usart_transmit(uint8_t data);

unsigned char
usart_receive(void);


void
usart_transmit_competition();

void 
usart_transmit_int(int n, int size);


#endif /* USART_H_ */