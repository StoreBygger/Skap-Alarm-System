#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H
#include <avr/io.h>
#include "oled_debug/oled_debug.h"


void distance_sensort_init() {


}


uint8_t distance_sensor_measure() {
	
	static uint8_t measured = 0;


	uint8_t distance = measured;

	measured++;

	debug_printf("Measured distance:\t%i", distance);

	return distance;

}






#endif