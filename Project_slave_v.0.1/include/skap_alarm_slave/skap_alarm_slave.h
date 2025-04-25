#ifndef SKAP_ALARM_SLAVE_H
#define SKAP_ALARM_SLAVE_H
#include "div/button.h"
#include "distance_sensor/distance_sensor.h"
#include "Bluetooth/bluetooth.h"
#include "skap_alarm_slave.c"
#include "div/eeprom.h"
#include "skap_alarm_slave/protocol_status.h"

#define ALARM_THRESHOLD 10

extern uint8_t intO_btn_state;


void update_device_addr();
void read_device_addr(uint8_t * addr_arr);
void init_skap_alarm();
void removeString(char string[], int start, int count);
void addr_str_to_arr(char * addr_str, uint8_t addr_arr[6]);
uint64_t hex_str_to_uint64(const char *str);

uint8_t get_alarm_status();
void send_alarm_status();
void skap_send_message(uint8_t type, uint8_t len, ...);

void skap_check_message(uint8_t * message);
uint8_t skap_check_recieved(uint8_t * recieved, uint8_t rec_len);

void init_skap_alarm() {
	oled_init();
	distance_sensort_init();
	bluetooth_init();



	bluetooth_device_init();
	update_device_addr();

	debug_print("init skap alarm");
	sei();

	set_rx_interrupt();



}

void update_device_addr() {

	if (bluetooth_state != 0) return; // only update if bluetooth device is not connected

	char addr_str[25];
	clear_rx_interrupt(); // clear RXC interrupt handling
	send_AT_command("AT+ADDR?"); // ask for device address
	usart_recieve_string(addr_str, 25);
	removeString(addr_str, 0, 8);

	uint8_t addr_arr[6];
	addr_str_to_arr(addr_str, addr_arr);


	eepromWriteArr(0, addr_arr, 6);

	debug_printf("MAC-Addr: %s", addr_str);
	set_rx_interrupt();
}

void read_device_addr(uint8_t * addr_arr) {

	eepromReadArr(0, addr_arr, 6);
}


void removeString(char string[], int start, int count) {
	uint16_t len = strlen(string);
    // removes by writing the charachters after the charachters to be removed over the now overwritten chrachters
    for (uint16_t i = start; i<len;i++) {
        string[i] = string[i+count];
    }
}

void addr_str_to_arr(char * addr_str, uint8_t addr_arr[6]) { // writes the mac addr from arr[0] to arr[5]
	uint64_t uint_addr = hex_str_to_uint64(addr_str); // get addr num from 

	uint_addr &= 0x0000FFFFFFFFFFFF; // mask addr -> MAC addr is only 12 bytes long

	for (uint8_t j = 0; j < 6; j++) {
		uint8_t num = (uint_addr >> (8*j)) & 0xFF;
		addr_arr[j] = num;
	}
}

uint64_t hex_str_to_uint64(const char *str) {
    uint64_t result = 0;
    while (*str) {
        result <<= 4; // shift 4 bits (1 hex digit)
        char c = *str++;

        if (c >= '0' && c <= '9') result |= (c - '0');
        else if (c >= 'A' && c <= 'F') result |= (c - 'A' + 10);
        else if (c >= 'a' && c <= 'f') result |= (c - 'a' + 10);
        else break; // invalid character
    }
    return result;
}

void skap_send_message(uint8_t type, uint8_t len_body, ...) {
	// All extra args must be type uint8_t

	va_list args;
	va_start(args, len_body);

	uint8_t len = PROTOCOLL_HEADER_SIZE + len_body;

	uint8_t message[len + 1];
	message[0] = 255;
	message[1] = type;
	message[2] = len;
	message[PROTOCOLL_HEADER_SIZE - 1] = 255;

	

	for (uint8_t i = 0; i < len_body; i++) {
		message[PROTOCOLL_HEADER_SIZE + i] = (uint8_t) va_arg(args, int);
	}
	va_end(args);

	usart_send_array(message, len);
}

uint8_t skap_check_recieved(uint8_t * recieved, uint8_t rec_len) {

	uint8_t byte = 0, i = 0;
	


	for (i = 0; i < rec_len; i++) {
		byte = recieved[i];

		if (byte == 255) break; // find the start of header, i = header_start

	}

	if (i >= rec_len) return 1; // invalid, no message found, return fault

	uint8_t message[rec_len - i]; // make message, fitting max body size



	for (uint8_t j = 0; j < PROTOCOLL_HEADER_SIZE; j++) {
		message[j] = recieved[i++];
	}

	uint8_t len, body_len;

	len = message[2];

	if ((i + body_len) > rec_len) return 2; // message is not complete -> return fault

	body_len = len - PROTOCOLL_HEADER_SIZE;


	for (uint8_t j = 0; j < body_len; j++) {
		message[PROTOCOLL_HEADER_SIZE + j] = recieved[i++];
	}

	

	skap_check_message(message);


	return 0;


}


void skap_check_message(uint8_t * message) {

	switch (message[1]) { // sort messages on different message-types

		case ASK_ALARM_STATUS:
			send_alarm_status();
			break;

		case ASK_SKAP_IS_LOCKER:
			skap_send_message(SEND_SKAP_IS_LOCKER,0);
			break;

		default:
			break;

	}

	uint8_t body_len = message[2] - PROTOCOLL_HEADER_SIZE;

	char text[30] = "BD: ";
	memset(text, '\0' , 30);
	for (uint8_t i = 0; i < body_len; i++) {
		char num[5];
		sprintf(num, "%02X:", message[PROTOCOLL_HEADER_SIZE + i]);
		strcat(text, num);
	}

	debug_printf("RM, T= %02X, L= %02X", message[1], message[2]);
	debug_print(text);
}


void send_alarm_status() {
	uint8_t alarm = get_alarm_status();

	skap_send_message(SEND_ALARM_STATUS,1,alarm );
	debug_printf("Alarm status %02X sent", alarm);

	
}

uint8_t get_alarm_status() {
	uint8_t distance = distance_sensor_measure();

	if (distance >= ALARM_THRESHOLD) {
		return SKAP_ALARM_ALARM;
	}

	return SKAP_ALARM_INGEN;
}

void init_int0() {
	GICR |= (1<<INT0);
	MCUCR |= (1<<ISC00);
	PORTD |= (1<<PD2);
	sei();
	
}

ISR (INT0_vect) {
	if (pressed(&PIND, 2, &intO_btn_state)) {
		debug_printf("Distance: %i", distance_sensor_measure());
	}
}





#endif