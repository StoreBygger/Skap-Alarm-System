// button.h

#ifndef BUTTON_H
#define BUTTON_H

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

uint8_t debounce(volatile uint8_t * pinName, uint8_t pinNumber) 
{
	while (bit_is_clear(* pinName, pinNumber))
	{
		_delay_ms(10);
		if (bit_is_clear(* pinName, pinNumber))
		{
			return 1;
		};
	};
	return 0;
}

uint8_t pressed(volatile uint8_t * pinName, uint8_t pinNumber, volatile uint8_t * buttonPressedCounter)
{
	if (debounce(pinName, pinNumber))
	{
		if (* buttonPressedCounter == 0)
		{
			* buttonPressedCounter = 1;
			return 1;
		};
	}
	else
	{
		* buttonPressedCounter = 0;
	};
	
	return 0;
};

#endif
