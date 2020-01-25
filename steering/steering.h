/*
 * steering.h
 *
 * Created: 2019-11-11 14:13:42
 *  Author: osklu414
 */ 


#ifndef STEERING_H_
#define STEERING_H_

#define STEERING_ID 1
#define F_CPU 8000000UL

#include <stdbool.h>
#include <stdint.h>

#include "motors.h"

// steering module state
struct
steering_state
{
	// whether communication module has identified the steering module or not
	bool identified;
};

// initialize steering module
void
steering_init();

// tick steering module
void
steering_tick();

// set motors' speeds
void
steering_pwm(uint8_t left, uint8_t right);

// set motors' directions
void
steering_dir(bool left_forward, bool right_forward);

// call when steering module has been identified by communication module
void
steering_identified();

#endif /* STEERING_H_ */
