#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "div/button.h"
#include "render_manager/render_manager.c"

uint8_t btn = 0;

int freeRam() {
    extern int __heap_start, *__brkval;
    volatile int stackPointer;
    return (int) &stackPointer - (int) (__brkval == 0 ? &__heap_start : __brkval);
}



int main(void) {
	_delay_ms(100);
	uint8_t locker_id = 0;
	


	oled_init();
	joystick_init();
	menu_init();
	bluetooth_init();

	locker_delete_all();

	locker_store_new("665544332211", &locker_id);
	locker_store_new("778899AABBCC", &locker_id);
	locker_store_new("6C79B8BB36C5", &locker_id);

	


	

	
	
	
	
	
	while (1) {

	}
	return 0;
}