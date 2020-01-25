/*
 * adc.c
 *
 * Created: 2019-11-07 13:24:20
 *  Author: marno874
 */ 


#include "adc.h"
#include <math.h>

void adc_init(){
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with '7' will always keep the value
	// of 'ch' between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
	
	// start single conversion
	// write '1' to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes '0' again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

uint16_t voltage_to_dist(uint16_t voltage){
	volatile float inverse = (3*pow(10, -5)*voltage) + 0.4204;   // Linear relationship between voltage and (1/distance) -0,42.
	volatile uint16_t distance = (uint16_t)(1/(inverse - 0.42)); 
	
	if(distance < LOWER_LIMIT || distance > UPPER_LIMIT){
		return 0;
	}
	return distance;
}
