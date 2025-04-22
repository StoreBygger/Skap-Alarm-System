#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "skap_alarm_slave/skap_alarm_slave.h"

uint8_t btn = 0;

int freeRam() {
    extern int __heap_start, *__brkval;
    volatile int stackPointer;
    return (int) &stackPointer - (int) (__brkval == 0 ? &__heap_start : __brkval);
}



int main(void) {
	

	init_skap_alarm();
	init_int0();

	uint8_t test_message[] = {255,SEND_ALARM_STATUS,6,255,4};

	debug_printf("err: %i", skap_check_recieved(test_message, 10));



	while (1) {



	}
	return 0;
}