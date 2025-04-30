#ifndef UART_H
#define UART_H
#include <avr/io.h>

#define UART_TIMEOUT 16
#define UART_DELAY 1

#define SIZE_RECIEVE_BUFFER1 25

extern uint8_t recieve_buffer[SIZE_RECIEVE_BUFFER1];
extern uint8_t len_recieve_buffer;

void usart_init(uint16_t UBRR) {


	UBRRH = (uint8_t)(UBRR >> 8); 
    UBRRL = (uint8_t)UBRR;
	UCSRC = (1<<UCSZ1) | (1<<UCSZ0) | (1<<URSEL); // ursel to acces UCSRC not UBRRH -- 8bit mode asynchronus mode
	
	UCSRB = (1<<RXEN) | (1<<TXEN); // enable UART
}

void usart_transmit_data(uint8_t data) {
	while ((UCSRA & (1<<UDRE)) == 0); // wait for empty transmit buffer

	UDR = data;
}

uint8_t usart_recieve_data(uint32_t timeout) {
	uint16_t i = 0;
	while (!(UCSRA & (1 << RXC))) { // wait for data to be recieved
		i++;
		_delay_ms(UART_DELAY);
		if (i > timeout) {
			return 0;
		}
	}

	return UDR;
}


void usart_recieve_string(volatile char* buffer, uint8_t buffer_size) {

	uint8_t i = 0;

	while (i < buffer_size - 1) {
		buffer[i] = (char) usart_recieve_data(UART_TIMEOUT);

		if (buffer[i] == '\r') {  // Ignore \r
            continue;
        }

		if (buffer[i] == '\n') {
			break;
		}
		i++;
	}
	buffer[i] = '\0';
}
void usart_recieve_string_wait(volatile char* buffer, uint8_t buffer_size, uint32_t timeout) {
	uint8_t i = 0;

	while (i < buffer_size - 1) {
		buffer[i] = usart_recieve_data(timeout);

		if (buffer[i] == '\r') {  // Ignore \r
            continue;
        }

		if (buffer[i] == '\n') {
			break;
		}
		i++;
	}
	buffer[i] = '\0';
}

void usart_send_string(const char* str) {
    while (*str) {
        usart_transmit_data(*str);
        str++;
    }
}
void usart_send_array(uint8_t * array, uint8_t len) {

	for (uint8_t i = 0; i < len; i++) {
		usart_transmit_data(array[i]);
	}
}

void set_rx_interrupt() {
	memset(recieve_buffer, 0 , SIZE_RECIEVE_BUFFER1);
	len_recieve_buffer = 0;
	UCSRB |= (1<<RXCIE);
}
void clear_rx_interrupt() {
	memset(recieve_buffer, 0 , SIZE_RECIEVE_BUFFER1);
	len_recieve_buffer = 0;
	UCSRB &= ~(1<<RXCIE);
}

#endif