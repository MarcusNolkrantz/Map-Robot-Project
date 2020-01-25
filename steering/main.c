/*
 * main.c
 *
 * Created: 2019-11-07 11:27:36
 *  Author: osklu414
 */

#include "steering.h"


int
main(void)
{
	steering_init();
	while(1) steering_tick();
	return 0;
}