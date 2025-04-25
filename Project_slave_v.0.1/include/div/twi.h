#ifndef TWI_H
#define TWI_H
#include <avr/io.h>
#include <util/delay.h>



#define TWI_SPEED 100000

#define TWI_TIMEOUT 16000
#define TWI_DELAY 1


#define DATA_RECIEVE_ACK 0x50
#define DATA_RECIEVE_NACK 0x58

// I2C functions
void TWI_Init(uint32_t speed);
void TWI_start();
void TWI_restart();
void TWI_TData(uint8_t data);
uint8_t TWI_RData(uint8_t ack);
void TWI_stop();

void twi_wait();

void TWI_Init(uint32_t speed) {
	PORTC |= (1<<PC0) | (1<<PC1); // set Pull up on I2C lines

	uint32_t Br = ( (F_CPU / speed) - 16) / 2; // calculate TWI frequency
	TWBR = Br; // set TWI frequency

}

void TWI_start() {
	TWCR = (1<<TWSTA) | (1<<TWINT) | (1<<TWEN); // send START message
	twi_wait(); // wait for message to be sent
}

void TWI_restart() { // switch slave to not addressed mode
	TWCR = (1<<TWSTA) | (1<<TWINT) | (1<<TWEN); // send START message
	twi_wait();// wait foTWINTr message to be sent
}

void TWI_stop() {
	TWCR = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN); // send STOP message
}

void TWI_SLA(uint8_t addr, uint8_t read) {
	TWDR = (addr<<1) | (read<<0); // load SLA_W/R
	TWCR = (1<<TWINT) | (1<<TWEN); // send message
	twi_wait();// wait for message to be sent
}

void TWI_TData(uint8_t data) {
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN); // send message
	twi_wait(); // wait for message to be sent

}

uint8_t TWI_RData(uint8_t ack) {
	uint8_t response = (ack  == 1 ? DATA_RECIEVE_ACK : DATA_RECIEVE_NACK);

	TWCR = (1<<TWINT) | (1<<TWEN) | (ack << TWEA); // send message
	twi_wait(); // wait for message to be sent

	uint16_t i = 0;
	while ((TWSR & 0xF8) != response) {
		i++;
		_delay_ms(TWI_DELAY);

		if (i > TWI_TIMEOUT) {
			return 0;
		}
	}

	return TWDR;

}

void twi_wait() {
	uint16_t i = 0;
	while (!(TWCR & (1<<TWINT))) {
		i++;

		if (i>TWI_TIMEOUT) {
			TWI_stop();
			return;
		}
	}
}





#endif