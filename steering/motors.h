/*
 * motors.h
 *
 * Created: 2019-11-07 11:26:39
 *  Author: osklu414
 */ 


#ifndef MOTORS_H_
#define MOTORS_H_

#include <stdint.h>
#include <stdbool.h>


// initialize PWM and direction ports
void
motors_init();

// send pwm signals to motors
void
pwm_set(uint8_t left, uint8_t right);

// change direction signals
void
dir_set(bool left_forward, bool right_forward);


#endif /* MOTORS_H_ */