/*
 * steering.c
 *
 * Created: 2019-11-11 14:13:17
 *  Author: osklu414
 */ 

#include "motors.h"
#include "usart.h"
#include "steering.h"

#include <stdlib.h>
#include <math.h>

#include <avr/interrupt.h>
#include <util/delay.h>


static struct steering_state state;

void
steering_init()
{
	motors_init();
	usart_init();
	state.identified = false;
	
	// enable interrupts
	sei();
	
	// transmit module id
	while(!state.identified)
	{
		usart_transmit(STEERING_ID);
		_delay_ms(100);
	}
	
	
}

void
steering_tick()
{
	/* OLD CODE (kept for reference)
	int16_t rot_delta = acos((cos(state.r)*(state.tgt_x - state.x) + sin(state.r)*(state.tgt_y - state.y)) / (sqrt(pow(cos(state.r), 2) + pow(sin(state.r), 2)) * sqrt(pow(state.tgt_x - state.x, 2) + pow(state.tgt_y - state.y, 2))));
	
	// dist controller
	int16_t dist_e = sqrt(pow(state.tgt_x - state.x, 2) + pow(state.tgt_y - state.y, 2));
	int16_t dist_de = dist_e - state.dist_e_last;
	state.dist_e_last = dist_e;
	uint8_t dist_u = state.dist_kp*dist_e + state.dist_kd*dist_de;
					
	// turn controller
	int16_t turn_e = rot_delta;
	int16_t turn_de = turn_e - state.turn_e_last;
	state.turn_e_last = turn_e;
	int16_t turn_u = state.turn_kp*turn_e + state.turn_kd*turn_de;
	
	// TODO: limit u(t):s to values between 0 and 255
	// What is the maximum values they can be? Maybe the only viable solution is to tune P values?
	
	if(abs(rot_delta) > 5)
	{
		uint8_t speed = turn_u;
		// rotate in correct direction
		if(rot_delta >= 0)
		{
			// rotate left
			dir_set(false, true);
			pwm_set(speed, speed);
		}
		else
		{
			// rotate right
			dir_set(true, false);
			pwm_set(speed, speed);
		}
	}
	else
	{
		// max speed depends on distance to target
		uint8_t speed_1 = dist_u;
		uint8_t speed_2 = dist_u - turn_u;
		
		// move towards destination
		dir_set(true, true);
		if(rot_delta >= 0)
		{
			// turn left
			pwm_set(speed_2, speed_1);
		}
		else
		{
			// turn right
			pwm_set(speed_1, speed_2);
		}
	}
	*/
}

void
steering_pwm(uint8_t left, uint8_t right)
{
	pwm_set(left, right);
}

void
steering_dir(bool left_forward, bool right_forward)
{
	dir_set(left_forward, right_forward);
}

void
steering_identified()
{
	state.identified = true;
}