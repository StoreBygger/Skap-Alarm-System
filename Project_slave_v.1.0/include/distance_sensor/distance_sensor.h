// slave/distance_sensor.h

#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H
#include <avr/io.h>
#include "div/twi.h"

#define DISTANCE_SENSOR_ADDR 0x57
#define TWI_SPEED 100000

void distance_sensort_init() {

	TWI_Init(TWI_SPEED);


}


int32_t distance_sensor_measure() {
	
	uint8_t response[3];
	_delay_ms(20);

	TWI_start();
	TWI_SLA(DISTANCE_SENSOR_ADDR, 0);
	TWI_TData(0x01);
	TWI_stop();

	_delay_ms(200);

	TWI_start();
	TWI_SLA(DISTANCE_SENSOR_ADDR, 1);
	response[0] = TWI_RData(1);
	response[1] = TWI_RData(1);
	response[2] = TWI_RData(0);
	TWI_stop();

	uint32_t measured = ((uint32_t)response[0] << 16) | ((uint32_t)response[1] << 8) | ((uint32_t)response[2] << 0);

	int distance = measured / 1000000.0 * 100.0;


	_delay_ms(20);

	return distance;

}






#endif