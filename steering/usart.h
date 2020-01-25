/*
 * usart.h
 *
 * Created: 2019-11-07 13:24:20
 *  Author: osklu414
 */ 


#ifndef USART_H_
#define USART_H_

#include <stdint.h>


// initialize usart communication
void
usart_init();

// transmit a byte over usart
void
usart_transmit(uint8_t data);

/*
// wait for and receive byte over usart
unsigned char
usart_receive(void);
*/


#endif /* USART_H_ */