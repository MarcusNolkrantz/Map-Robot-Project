/*
 * sensor.h
 *
 * Created: 2019-11-07 13:24:20
 *  Author: marno874, felli675, edwjo109, matlj387
 */ 

#ifndef sensor
#define sensor

#include <stdint.h>
#include <stdbool.h>
#include "adc.h"
#include "usart.h"
#include "i2cmaster.h"
#include <avr/io.h>
#include <util/delay.h>
#include "i2cmaster.h"
#include <avr/interrupt.h>



//Constants
#define SENSOR_ID 0
#define PRESCALE 1024.0

//Addresses
//8 bits, lsb is 0 for w, 1 for r
#define GYRO_SAD 0xD6	//Gyro I2C adress
#define DATA_REGS 0xA8	//The start of the data registers for the ADAFRUIT-10DOF
#define GYRO_REGS 0xAC 	//
//Conversion constants for the ADAFRUIT-10DOF
#define SENSORS_GRAVITY_EARTH  9.80665F
#define SENSORS_DPS_TO_RADS    0.017453293F          /**< Degrees/s to rad/s multiplier */
#define GYRO_SENSITIVITY_250DPS  0.00875F
#define GYRO_SENSITIVITY_500DPS  0.0175F
#define GYRO_SENSITIVITY_2000DPS 0.070F
#define F_CPU 8000000UL
//Variables


struct
sensor_state
{
    bool identified;
};

struct linked_list {
	struct linked_list *_next;
	uint8_t _data;
};

struct 
sensor_data 
{
	int16_t gyro_angle;
	uint16_t right_distance;
	uint16_t left_distance;
};


/* Functions */
/*Wait for sensor module to be identified*/
void sensor_init();
/*Set module as identified*/
void sensor_identified();
/*Initialize timer*/
uint16_t init_timer();
/*Read gyro from I2C buss and save it as a float*/
void read_gyro(float *rotation);
/*Calculate gyro average offset for n number of measurements*/
void calc_offset(float *gyro, int n);
/*Initialize the reset button*/
void init_btn();
/*Initialize hardware by calling other init functions.*/
void hw_init();
/*Update the angle from the current rotation speed and the time delta.*/
void update_angle(float offset, float *angle);
/*Update sensor data struct if at least one of the sensor values changed and calls send_data function with that struct as argument*/
void sensor_tick(float offset, float angle, struct sensor_data* d);
/*Returns distance to wall from side sensors*/
uint16_t get_dist(int n);
/*Calls usart_transmit function for every byte that should be sent to communication module.*/
void send_data(struct sensor_data* d);

#endif