/*
 * steering.c
 *
 * Created: 2019-11-06 15:16:41
 *  Author: marno874
 */ 


#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "motors.h"


void
pwm_init()
{
	// set PWM port pins as outputs
	DDRD |= (1 << DDD4) | (1 << DDD5);
	
	// make timers reset to BOTTOM when they reach TOP (255)
	TCCR1A |= (1 << COM1A1) | (0 << COM1A0);
	TCCR1A |= (1 << COM1B1) | (0 << COM1B0);

	// enable fast PWM mode
	TCCR1A |= (1 << WGM10) | (0 << WGM11);
	TCCR1B |= (1 << WGM12) | (0 << WGM13);
	
	// configure timers (no prescaling)
	TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
	
	//set pmw to 0
	OCR1A = 0;
	OCR1B = 0;
}

void
dir_init()
{
	// set direction port pins as outputs
	DDRD |= (1 << DDD2) | (1 << DDD3);
}

void
pwm_set(uint8_t left, uint8_t right)
{
	OCR1A = right;
	OCR1B = left;
}

void
dir_set(bool left_forward, bool right_forward)
{
	if(left_forward)
	{
		PORTD |= (1 << 2);
	}
	else
	{
		PORTD &= ~(1 << 2);
	}
	
	if(right_forward)
	{
		PORTD |= (1 << 3);
	}
	else
	{
		PORTD &= ~(1 << 3);
	}
}

void
motors_init()
{
	pwm_init();
	dir_init();
}