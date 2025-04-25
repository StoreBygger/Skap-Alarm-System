#ifndef joystick
#define joystick
#include <avr/io.h>
#include <avr/interrupt.h>
#include "div/button.h"



extern uint8_t measure_axis; // 0 - 1= x, 2 - 3 = y
extern uint8_t int_btn_pressed;

extern void update_cursor(int8_t x, int8_t y, uint8_t btn);


void joystick_init();
void check_axis(int16_t VqSet, volatile uint8_t * ASet);
static void initTimer0();
static void initADC();

void init_int0() {
	GICR |= (1<<INT0);
	MCUCR |= (1<<ISC00);
	PORTD |= (1<<PD2);
	sei();
	
}



void joystick_init() {
	init_int0();
	initTimer0();
	initADC();
}

static void initTimer0() { // set timer0 in normal mode, and trigger interupt on overflow vector
	TCCR0 |= (1<<CS02) | (1<<CS00); // 1024 prescaler

	TIMSK |= (1<<TOIE0); // enable Timer0_ovf_vect
  
}


static void  initADC() {
	ADMUX |= (1<<REFS0); // AVCC with external capacitor in AREF
	ADMUX |= (1<<MUX4) | (1<<MUX1) | (1<<MUX0); // ADC1 and ADC3
	//ADMUX |= (1<<MUX0); // ADC 1

	ADCSRA |= (1<<ADIE); // enable ADC interrupt
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // set 128 division factor (50KHz < F_CPU / divison_factor < 200KHz) -- works for 8MHz and 16MHz
	ADCSRA |= (1<<ADEN); // enable ADC


	sei();
  }
  
ISR (TIMER0_OVF_vect) {
	ADCSRA |= 1<< ADSC;
}

ISR (INT0_vect) {
	
	if (pressed(&PIND, 2, &int_btn_pressed)) {
		update_cursor(0,0,1);
	}
}

  
ISR (ADC_vect) {
	static int16_t VqX, VqY, LqX, LqY;

	int16_t Vadc = 0, Vq = 0, VqSet; 
	uint8_t Xset = 0, Yset = 0;

	Vadc = ADCL;
	Vadc |= (ADCH << 8);	

	if ((Vadc & (1<<9)) != 0) {
		Vadc |= (0x3F<<10);
	}

	Vq = (5000L * (Vadc + 1)) / 512;// Calculate Vq, 5000 / 512 ~= 10

	if (measure_axis == 1) { // code for y - axis
		measure_axis = 0; // set measure axis to X
		LqY = VqY; // updata last Y measured
		VqY = (Vq + VqX - 68 ); // set new Y measured
		VqSet = (LqY + VqY) / 2; // apply average

		check_axis(VqSet, &Yset);
		
		ADMUX = (1<<MUX4) | (1<<MUX1) | (1<<MUX0) | (1<<REFS0); // ADC1 and ADC3
	} else {
		measure_axis = 1;
		LqX = VqX;
		VqX = -Vq + 78;
		VqSet = (LqX + VqX) / 2;
		check_axis(VqSet, &Xset);

		ADMUX = (1<<MUX4) | (1<<MUX1) | (1<<REFS0); // ADC1 and ADC2
	}

	TIFR |= (1<<TOV0); // CLEAR INTERUPT FLAG
	ADCSRA |= (1<<ADIF); //Clear adif interrupt flag
	TCNT0 = 0;
}

void check_axis(int16_t VqSet, volatile uint8_t * ASet) {
	int8_t update = 0;
	if ((VqSet > 2000) && (* ASet == 0)) {
		update = 1;
		* ASet = 1;
	} else if ((VqSet < -2000) && (* ASet == 0)) {
		update = -1;
		* ASet = 1;
	} else {
		* ASet = 0;
		return;
	}
	
	if (measure_axis == 1) {
		update_cursor(update, 0,0);
	} else if (measure_axis == 0) {
		update_cursor(0, update,0);
	}
}

void joystick_adc_start() {
	ADCSRA |= (1<<ADEN); // enable ADC
	TCCR0 |= (1<<CS02) | (1<<CS00); // 1024 prescaler
}

void joystick_adc_stop() {
	ADCSRA &= ~(1<<ADEN); // enable ADC
	TCCR0 &= ~((1<<CS02) | (1<<CS00)); // 1024 prescaler

}


#endif