// eeprom.h

#ifndef EEPROM_H
#define EEPROM_H

#include <avr/io.h>
#include <avr/interrupt.h>

void eepromWrite(uint16_t address, uint8_t data) {
	cli(); // disable interrupts to write correctly
	while (EECR & (1<<EEWE)); // wait for completion of previous data write

	EEAR = address; // set up address to register to write to
	EEDR = data; // send data to selected register

	EECR |= (1<<EEMWE); // Write logical 1 on EEMWE

	EECR |= (1<<EEWE); // start data write
	sei();
}

uint8_t eepromRead(uint16_t address) {
	while (EECR & (1<<EEWE)); // wait for completion of previous data write

	EEAR = address; // set up address to register to read from

	EECR |= (1<<EERE); // start eeprom read
	return EEDR; // return data
}

void eepromWriteArr(uint16_t startAddress, volatile uint8_t * arr, uint16_t len) {

	for (uint16_t i = 0; i<len; i++) {
		if (startAddress + i >= 1024) { break; } // If address is out of eeprom range - break loop

		eepromWrite(startAddress + i, arr[i]); // write data at correct address, incremented by i
	}
}

void eepromReadArr(uint16_t startAddress,  volatile uint8_t * arr, uint16_t len) {

	for (uint16_t i = 0 ; i<len; i++) {

		if (startAddress + i >= 1024) { break; } // If address is out of eeprom range - break loop

		arr[i] = eepromRead(startAddress + i); // read data from correct address, incremented by i
	}
}

#endif