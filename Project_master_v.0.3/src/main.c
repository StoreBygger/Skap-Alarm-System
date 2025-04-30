#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "div/button.h"
#include "render_manager/render_manager.h"




int main(void) {
	_delay_ms(100); // make sure ATmega starts properly
	
	locker_init();




	
	while (1) {
		

	}
	return 0;
}