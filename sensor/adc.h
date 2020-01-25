/*
 * adc.h
 *
 * Created: 2019-11-07 13:24:20
 *  Author: marno874
 */ 


#ifndef ADC
#define ADC
#include <stdint.h>
#include <avr/io.h>

#define UPPER_LIMIT 300
#define LOWER_LIMIT 40

void adc_init();
uint16_t adc_read(uint8_t ch);
uint16_t voltage_to_dist(uint16_t voltage);



#endif