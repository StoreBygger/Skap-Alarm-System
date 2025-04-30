#ifndef BLUETOOTH_H
#define BLUETOOTH_H
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "uart.h"
#include "bluetooth.c"
#include "skap_alarm_slave/protocol_status.h"


extern void send_alarm_status();

extern void skap_check_message(uint8_t * message);
extern uint8_t skap_check_recieved(uint8_t * recieved, uint8_t rec_len);


void send_AT_command(const char* command);
void bluetooth_init();
void bluetooth_device_init();
void initTimer1();
void timer1_stop();
void timer1_start();
void bluetooth_check_AT();
void bluetooth_check_ADDR();
uint8_t check_AT_command(const char* command, char * expected_answer, char * recieved_answer);


void bluetooth_err(char * recieved, char * excpected);

void bluetooth_rec_finished();

uint8_t string_compare(char * string_1, char * string_2, uint8_t len);

uint8_t bluetooth_check_notify();


void clear_recieve();


void send_AT_command(const char* command) {
    usart_send_string(command);
    usart_send_string("\r\n");  // Append CR+LF
	_delay_ms(10); // making sure commands dont come to fast after each other
}

uint8_t check_AT_command(const char* command, char * expected_answer, char * recieved_answer) {
	clear_rx_interrupt();
	send_AT_command(command);
	usart_recieve_string(recieved_answer, MAX_LEN_ANSWER);
	set_rx_interrupt();

	return (string_compare(expected_answer, recieved_answer, MAX_LEN_ANSWER));

}

void bluetooth_init() {

	usart_init(BAUD_UBRR);
	initTimer1();
	set_rx_interrupt(); // enable RX complete interrupt

}
void initTimer1() { // set timer1 in normal mode, and trigger interupt on overflow vector
	OCR1A = BT_MSG_REC_COUNT;
	TCCR1B |= (1<<WGM12); // Timer 1, CTC, OCR1A

	// dont start timer 1 before UART RXC interrupt happens

	TIMSK |= (1<<OCIE1A);
	sei();
}

void timer1_start() {
	TCNT1 = 0;
	TCCR1B |= (1<<CS12) | (1<<CS10);
}

void timer1_stop() {
	TCCR1B &= ~((1<<CS12) | (1<<CS10));
	TCNT1 = 0;
}

void bluetooth_device_init() {
	clear_rx_interrupt();

	char * recieved_answer = (char *)malloc(MAX_LEN_ANSWER * sizeof(char));

	
	uint8_t err = 0;

	// clear last connected addr
	err = 0;
	err = check_AT_command("AT+CLEAR","OK+CLEAR", recieved_answer);
	if (err != 0) {
		bluetooth_err("BT clear", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);

	// set BT baud rate
	err = check_AT_command("AT+BAUD0","OK+Set:0", recieved_answer);
	if (err != 0) {
		bluetooth_err("Baud rate", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);

	// set BT work mode
	err = 0;
	err = check_AT_command("AT+MODE0","OK+Set:0", recieved_answer);
	if (err != 0) {
		bluetooth_err("work mode", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);

	// set set BT notify-mode -> recieve OK+CONN when connected and OK+LOST when lost connection
	err = 0;
	err = check_AT_command("AT+NOTI1","OK+Set:1", recieved_answer);
	if (err != 0) {
		bluetooth_err("BT notify", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);


	// set set BT name 
	err = 0;
	err = check_AT_command("AT+NAMESkapSlave","OK+Set:SkapSlave", recieved_answer);
	if (err != 0) {
		bluetooth_err("BT name", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);


	// set set BT role -> peripheral
	err = 0;
	err = check_AT_command("AT+ROLE0","OK+Set:0", recieved_answer);
	if (err != 0) {
		bluetooth_err("BT role", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);

	// set set BT imme = 0 -> connect immeadetly
	err = 0;
	err = check_AT_command("AT+IMME0","OK+Set:0", recieved_answer);
	if (err != 0) {
		bluetooth_err("BT imme", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);

	
	free(recieved_answer);

	set_rx_interrupt(); // enable RX complete interrupt
}

void bluetooth_err(char * message, char * excpected) {
	


}


void bluetooth_check_AT() {
	char recieved_answer[MAX_LEN_ANSWER];
	char expected_answer[MAX_LEN_ANSWER] = "OK";

	uint8_t err = check_AT_command("AT", expected_answer, recieved_answer);
	if (err != 0) {
		bluetooth_err(recieved_answer, expected_answer);
	}
	
}

void bluetooth_check_ADDR() {
	char * recieved_answer = (char *)malloc((MAX_LEN_ANSWER + 10) * sizeof(char));
	char expected_answer[MAX_LEN_ANSWER] = "AT+GETADDR";

	uint8_t err = check_AT_command("AT+ADDR?", expected_answer, recieved_answer);


	if (err != 0) {
		bluetooth_err(recieved_answer, expected_answer);
	}

	free(recieved_answer);
}


uint8_t string_compare(char * string_1, char * string_2, uint8_t len) {

	uint8_t i = 0;

	while (i < len) {

		if (string_1[i] != string_2[i]) return 1;

		if ((string_1[i] == '\0') && ((string_2[i] == '\0') || (string_2[i] == '\r'))) return 0;
		i++;
	}

	return 0;
}


void bluetooth_rec() {


}


ISR (USART_RXC_vect) {
	uint8_t data = UDR; // read data
	recieve_buffer[len_recieve_buffer] = data; // put data in recieve buffer
	
	len_recieve_buffer++; // incremeant recieve buffer

	if (len_recieve_buffer > SIZE_RECIEVE_BUFFER) { // if recieve_buffer is full, reset recieve buffer
		clear_recieve();
		
	}

	timer1_start(); // start countdown timer
} 

ISR (TIMER1_COMPA_vect) {
	timer1_stop(); // stop countdown timer
	
	bluetooth_rec();
	uint8_t noti = 0;
	noti = bluetooth_check_notify();

	if (noti != 1) {
		bluetooth_rec_finished();
	}
	
	
}

void clear_recieve() {
	memset(recieve_buffer, 0, SIZE_RECIEVE_BUFFER);
	len_recieve_buffer = 0;
}

void bluetooth_rec_finished() {


	uint8_t err = skap_check_recieved(recieve_buffer, len_recieve_buffer);

	if (err == 0) {
		clear_recieve();
	}
	



	

}

uint8_t bluetooth_check_notify() {
	if (string_compare((char *) recieve_buffer, "OK+CONN", 8) == 0) {
		bluetooth_state = 1; // set BT state to 1 / connected
		bluetooth_rec();
		clear_recieve();
		return 1;
	}

	if (string_compare((char * ) recieve_buffer, "OK+LOST", 8) == 0) {
		bluetooth_state = 0; // set BT state to 0 / disconnected
		bluetooth_rec();
		clear_recieve();
		return 1;
	}

	return 0;
}



#endif