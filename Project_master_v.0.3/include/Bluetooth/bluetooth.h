#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include "uart.h"
#include "locker/protocol_status.h"
#include "bluetooth.c"
#include "oled/oled.h"




extern void popup_add_text(const char * text, uint8_t index);
extern void popup_delete_string_array();
extern void popup_make_string_array(uint8_t len);
extern void popup_render();

extern uint8_t current_running_program;
extern uint8_t max_cursor_x;
extern uint8_t max_cursor_y;

extern uint8_t recieve_buffer[SIZE_RECIEVE_BUFFER];
extern uint8_t len_recieve_buffer;
extern uint8_t bluetooth_state;

extern void locker_update_alarm(uint8_t locker_id, uint8_t alarm_status);
extern void update_next_check_locker();


extern void locker_check_message(uint8_t * message);
extern uint8_t locker_check_recieved(uint8_t * recieved, uint8_t rec_len);
extern uint8_t findString(const char string[], const char substring[]);

void send_AT_command(const char* command);
void bluetooth_init();
void bluetooth_device_init();
void bluetooth_check_AT();
void bluetooth_check_ADDR();
uint8_t check_AT_command(const char* command, char * expected_answer, char * recieved_answer);

void bluetooth_disconnect();

void bluetooth_err(char * recieved, char * excpected);

void bluetooth_rec_finished();

uint8_t string_compare(char * string_1, char * string_2, uint8_t len);

uint8_t bluetooth_check_notify();
void bluetooth_rec_finished();

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

	BTDDR |= (1<<BTPIN);
	BTPORT |= (1<<BTPIN);
	usart_init(BAUD_UBRR);
	set_rx_interrupt(); // enable RX complete interrupt

}


void bluetooth_device_init() {

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
	err = check_AT_command("AT+NAMESkapMaster","OK+Set:SkapMaster", recieved_answer);
	if (err != 0) {
		bluetooth_err("BT name", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);


	// set set BT role -> central
	err = 0;
	err = check_AT_command("AT+ROLE1","OK+Set:1", recieved_answer);
	if (err != 0) {
		bluetooth_err("BT role", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);

	// set set BT imme -> do not connect immeadetly
	err = 0;
	err = check_AT_command("AT+IMME1","OK+Set:1", recieved_answer);
	if (err != 0) {
		bluetooth_err("BT role", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);

	// set set BT show -> show name after OK+DISC ADDR recieved
	err = 0;
	err = check_AT_command("AT+SHOW1","OK+Set:1", recieved_answer);
	if (err != 0) {
		bluetooth_err("BT show", recieved_answer);
	}
	memset(recieved_answer, 0,MAX_LEN_ANSWER);
	
	free(recieved_answer);

	set_rx_interrupt(); // enable RX complete interrupt
}

void bluetooth_err(char * message, char * excpected) {
	max_cursor_x = 0;
	max_cursor_y = 0;

	popup_make_string_array(5);
	popup_add_text("BT ERROR", 0);
	popup_add_text("In message:",1);
	popup_add_text(message,2);
	popup_add_text("And got answer:", 3);
	popup_add_text(excpected, 4);
	current_running_program = 2;

	popup_render();
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
	char expected_answer[MAX_LEN_ANSWER + 10] = "AT+GETADDR";

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

		if (((string_1[i] == '\0') || (string_1[i] == '\r')) && ((string_2[i] == '\0') || (string_2[i] == '\r'))) return 0;
		i++;
	}

	return 0;
}



void bluetooth_connect_addr(char  addr[13]) {
	char send_cmd[MAX_LEN_ANSWER] = "AT+CON";
	char exp_cmd[MAX_LEN_ANSWER] = "OK+CONNA";
	char rec_cmd[MAX_LEN_ANSWER];


	strcat(send_cmd, addr);

	uint8_t err = check_AT_command(send_cmd, exp_cmd, rec_cmd);

	if (err != 0) {
		max_cursor_x = 0;
		max_cursor_y = 0;
		popup_make_string_array(5);
		popup_add_text("BT CONNECT", 0);
		popup_add_text("ADDR", 1);
		popup_add_text(addr,2);
		popup_add_text("Recieved:",3);
		popup_add_text(rec_cmd,4);
		current_running_program = 2;
	
		popup_render();
	}
	

}

void bluetooth_rec() {
	max_cursor_x = 0;
	max_cursor_y = 0;
	char line[50];

	for (uint8_t i = 0; i<8; i++) {
		char num[5];
		sprintf(num, "%02X-", recieve_buffer[i]);
		strcat(line, num);

		if (i > len_recieve_buffer) break;

	}


	popup_make_string_array(3);
	popup_add_text("BT RECIEVED", 0);
	popup_add_text(line, 1);
	recieve_buffer[SIZE_RECIEVE_BUFFER- 1] = '\0';
	popup_add_text((char *) recieve_buffer, 2);
	current_running_program = 2;

	popup_render();
}

void bluetooth_disconnect() {
	BTPORT &= ~(1<<BTPIN);
	_delay_ms(500);
	BTPORT |= (1<<BTPIN);
	bluetooth_state = BLUETOOTH_DISCONNECTED; // reset BT state to disconnected, OK+LOST does not get sent when BT is powered off
	
}

ISR (USART_RXC_vect) {
	uint8_t data = UDR; // read data
	recieve_buffer[len_recieve_buffer++] = data; // put data in recieve buffer
	

	if (len_recieve_buffer > SIZE_RECIEVE_BUFFER - 1) { // if recieve_buffer is full, reset recieve buffer
		clear_recieve();
		return;
		
	}

	uint8_t noti = 0;
	noti = bluetooth_check_notify();

	if (noti != 1) {
		bluetooth_rec_finished();
	}

} 


void clear_recieve() {
	bluetooth_rec();
	memset(recieve_buffer, 0, SIZE_RECIEVE_BUFFER);
	len_recieve_buffer = 0;
	oled_draw_text((volatile char * ) "Cleared buffer", 65, 7, 1, 1);
	
}

void bluetooth_rec_finished() {


	uint8_t err = locker_check_recieved(recieve_buffer, len_recieve_buffer);

	if (err == 0) {
		clear_recieve();
	} 

	



	

}

uint8_t bluetooth_check_notify() {
	

	if (findString((char * ) recieve_buffer, "OK+LOST") != 255) { // BT connection lost
		bluetooth_state = BLUETOOTH_DISCONNECTED; // set BT state to 0 / disconnected

		clear_recieve();
		update_next_check_locker();
		return 1;
	}

	if (findString((char * ) recieve_buffer, "OK+CONNF") != 255) { // BT connection Failure

		oled_draw_text((volatile char *) "OK+CONNF", 64,6,1,1);
		_delay_ms(500);
		bluetooth_state = BLUETOOTH_DISCONNECTED; // set BT state to 0 / disconnected
		clear_recieve();
		update_next_check_locker();
		return 1;
	}

	if (findString((char * ) recieve_buffer, "OK+CONNE") != 255) { // BT connection error
		
		oled_draw_text((volatile char *) "OK+CONNE", 64,6,1,1);
		_delay_ms(500);
		bluetooth_state = BLUETOOTH_DISCONNECTED; // set BT state to 0 / disconnected
		clear_recieve();
		update_next_check_locker();
		return 1;
	}

	if (findString((char * ) recieve_buffer, "OK+CONNA") != 255) { // BT connection accepting conn. requests
		
		oled_draw_text((volatile char *) "OK+CONNA", 64,6,1,1);
		_delay_ms(500);
		clear_recieve();
		return 1;
	}

	if (findString((char *) recieve_buffer, "OK+CONN\r\n") != 255) { // BT connection mad
		oled_draw_text((volatile char *) "OK+CONN", 64,6,1,1);
		_delay_ms(500);

		if (bluetooth_state == BLUETOOTH_DISCONNECTED) bluetooth_state = BLUETOOTH_CONNECTED_UNIDENTIFIED; // only set bluetooth state if it is previously
		clear_recieve();
		
		return 1;
	} 


	oled_draw_text((volatile char *) "No Noti", 64,6,1,1);

	return 0;
}



#endif