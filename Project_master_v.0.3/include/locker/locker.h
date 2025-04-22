#ifndef LOCKER_H
#define LOCKER_H
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdarg.h>

#include "locker.c"
#include "div/eeprom.h"
#include "protocol_status.h"

#define MAX_LOCKERS 64
#define LOCKER_SIZE 8
#define LOCKER_EEPROM_BASE 512

#define LOCKER_TIMEOUT 2500
#define LOCKER_WAIT 1

typedef struct locker {
	uint8_t locker_id; // 0 -> 63 locker_id -> 64 is outside EEPROM range
	uint8_t alarm_state;
	uint8_t locker_active;
	

	struct locker * previous;
	struct locker * next;

}locker;


typedef enum current_running_prog {
	no_render,
	menu,
	popup,
	eeprom_print
} current_running_prog;

uint8_t load_locker(locker * self, uint8_t locker_id);
uint8_t locker_get_addr(char * addr_buffer, uint8_t locker_id);
void addr_str_to_arr(char * addr_str, uint8_t addr_arr[8]);
uint8_t locker_store_new( char * str_addr, uint8_t * locker_id);

void locker_check_alarm(uint8_t locker_id);
void locker_send_message(uint8_t type, uint8_t len_body, ...);

uint8_t locker_connect(uint8_t locker_id);
uint64_t hex_str_to_uint64(const char *str);

void locker_delete(uint8_t locker_id);
void locker_delete_all();

void locker_update_alarm(uint8_t locker_id, uint8_t alarm_status);
void locker_reset_all_alarms();

uint8_t findString(const char string[], const char substring[]);

extern void popup_add_text(const char * text, uint8_t index);
extern void popup_delete_string_array();
extern void popup_make_string_array(uint8_t len);
extern void popup_render();
extern void init_popup(uint8_t popup_lines);

extern void render();


extern uint8_t background_program[6];

void update_next_check_locker();



void locker_check_message(uint8_t * message);
uint8_t locker_check_recieved(uint8_t * recieved, uint8_t rec_len);

uint8_t load_locker(locker * self, uint8_t locker_id) {
	uint16_t locker_eeprom_id = LOCKER_EEPROM_BASE + (LOCKER_SIZE * locker_id);
	memset(self, 0, sizeof(locker));

	uint8_t locker_active = eepromRead(locker_eeprom_id);
	uint8_t alarm_state = eepromRead(locker_eeprom_id + 1);

	if (locker_active == 0) {
		return 0; // locker is not active
	}

	self->alarm_state = alarm_state;
	self->locker_id = locker_id;
	self->locker_active = locker_active;

	return 1; // found active locker
}

uint8_t locker_get_addr(char * addr_buffer, uint8_t locker_id) {
	uint16_t locker_eeprom_id = LOCKER_EEPROM_BASE + LOCKER_SIZE * locker_id;

	uint8_t locker_state[LOCKER_SIZE];

	eepromReadArr(locker_eeprom_id,locker_state,LOCKER_SIZE);

	if (locker_state[0] == 0) {
		return 0; // fault -> locker is not active
	}
	
	addr_buffer[0] = '\0';

	for (uint8_t i = 7; i >= 2; i--) {
		uint8_t current_addr = 0;
		char current_buffer[3];
		current_addr = locker_state[i]; // the MSB of ADDR is locker_state[7] and LSB is locker_state[2]
		sprintf(current_buffer, "%02X", current_addr);
		strcat(addr_buffer, current_buffer);
	}

	addr_buffer[12] = '\0';



	return 1; // found locker addr

}

uint8_t locker_store_new( char * str_addr, uint8_t * locker_id) { // it is possible to overwrite exsisting lockers with this function

	uint8_t locker_arr[LOCKER_SIZE] = {};

	locker_arr[0] = 1; // locker active
	locker_arr[1] = SKAP_ALARM_INGEN; // set alarmstate no alarm

	uint8_t i = 0;

	for (i = 0; i < MAX_LOCKERS; i++) {
		if (eepromRead(LOCKER_EEPROM_BASE + (LOCKER_SIZE * i)) == 0) {
			break;
		}
	}

	if (i > MAX_LOCKERS) return 1; // if i >= 64 the locker_id is not valid -> the maximum number of lockers is reached

	* locker_id = i; 

	addr_str_to_arr(str_addr, locker_arr);

	eepromWriteArr(LOCKER_EEPROM_BASE + LOCKER_SIZE * (* locker_id), locker_arr, LOCKER_SIZE); // write locker_arr at locker_id location

	return 0;

}

void locker_update_alarm(uint8_t locker_id, uint8_t alarm_status) {


	if (locker_id >= 64) return; // check valid locker_id

	uint16_t locker_eeprom_id = LOCKER_EEPROM_BASE + (locker_id * LOCKER_SIZE);

	uint8_t locker_active = eepromRead(locker_eeprom_id);
	
	if (locker_active == 0) return; // check if locker is active

	eepromWrite(locker_eeprom_id + 1, alarm_status);


	char text1[25];
	char text2[25];
	sprintf(text1, "Locker %i status", locker_id);
	sprintf(text2, "Status: %02X", alarm_status);
	
	init_popup(3);
	popup_add_text(text1, 0);
	popup_add_text(text2, 2);
	render();

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

void addr_str_to_arr(char * addr_str, uint8_t addr_arr[8]) { // writes the mac addr from arr[2] to arr[7]
	uint64_t uint_addr = hex_str_to_uint64(addr_str); // get addr num from 

	uint_addr &= 0x0000FFFFFFFFFFFF; // mask addr -> MAC addr is only 12 bytes long

	for (uint8_t j = 0; j < 6; j++) {
		uint8_t num = (uint_addr >> (8*j)) & 0xFF;
		addr_arr[2+j] = num;
	}
}



uint8_t locker_connect(uint8_t locker_id) {
	char * locker_addr = (char *)malloc(13 * sizeof(char));
	uint8_t locker_active = locker_get_addr(locker_addr, locker_id);

	if (locker_active == 0) {
		return 1; // fault -> locker is not active
	}

	bluetooth_connect_addr(locker_addr);
	free(locker_addr);
	return 0; // no errors
}

void locker_delete(uint8_t locker_id) {

	for (uint8_t i = 0; i < LOCKER_SIZE; i++) {

		eepromWrite(LOCKER_EEPROM_BASE + (locker_id * LOCKER_SIZE) + i, 0);
	}
}
void locker_delete_all() {

	for (uint8_t i = 0; i < MAX_LOCKERS; i++) {
		locker_delete(i);
	}

	init_popup(1);
	popup_add_text("DELETED ALL DEVICES",0);
	render();

}

void locker_send_message(uint8_t type, uint8_t len_body, ...) {
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

uint8_t locker_check_recieved(uint8_t * recieved, uint8_t rec_len) {

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

	

	locker_check_message(message);


	return 0;


}


void locker_check_message(uint8_t * message) {

	switch (message[1]) { // sort messages on different message-types

		case ASK_ALARM_STATUS:
			break;

		case SEND_ALARM_STATUS:
			locker_update_alarm(bluetooth_state - 1, message[PROTOCOLL_HEADER_SIZE]);
			bluetooth_disconnect(); // disconnect locker
			break;

		default:
			break;

	}

	// debug print code: 

	uint8_t body_len = message[2] - PROTOCOLL_HEADER_SIZE;

	char text1[30] = "BD: ";
	memset(text1, '\0' , 30);
	for (uint8_t i = 0; i < body_len; i++) {
		char num[5];
		sprintf(num, "%02X:", message[PROTOCOLL_HEADER_SIZE + i]);
		strcat(text1, num);
	}

	char text[20];
	sprintf(text, "Type: %02X <> Len: %02X", message[1], message[2]);

	init_popup(3);
	popup_add_text("Skap MSG received", 0);
	popup_add_text(text, 1);
	popup_add_text(text1, 2);
	render();
}


void locker_check_alarm(uint8_t locker_id) {

	if (locker_id  >= 64) {
		locker_id -= 64;
	}

	char rec_str[SIZE_RECIEVE_BUFFER];
	memset(rec_str, '\0', SIZE_RECIEVE_BUFFER);

	uint8_t err = locker_connect(locker_id);
	if (err != 0) {
		bluetooth_disconnect();
		update_next_check_locker();
		return;
	}

	usart_recieve_string(rec_str, SIZE_RECIEVE_BUFFER);

	bluetooth_state = locker_id + 1; // set bluetooth to correct locker state

	locker_send_message(ASK_ALARM_STATUS, 0);

	update_next_check_locker();

}


void locker_check_all_alarms() {

	locker_check_alarm(0);
	current_check_locker_id = 1;


}


void locker_reset_all_alarms() {


	for (uint8_t i = 0; i < MAX_LOCKERS; i++) {
		locker_update_alarm(i, SKAP_ALARM_INGEN);
	}



}

void update_next_check_locker() {

	char text[10];
	sprintf(text, "ID:%i", current_check_locker_id);

	init_popup(2);
	popup_add_text("Checking locker", 0);
	popup_add_text(text, 0); 
	locker_check_alarm(current_check_locker_id);
	render();
	_delay_ms(500);

	if (current_check_locker_id == 255) return; // if no next locker, return

	current_check_locker_id++; // up current locker to next locker id

	if (current_check_locker_id >= MAX_LOCKERS) { // if id exceeds max lockers -> all lockers gone through
		current_check_locker_id = 255;
		return;
	}

	uint8_t active = 0;

	uint16_t eeprom_locker_id = LOCKER_EEPROM_BASE + current_check_locker_id * LOCKER_SIZE; // set address to the first locker to read from

	while (active == 0) {

		active = eepromRead(eeprom_locker_id); // check if current locker is active

		eeprom_locker_id += LOCKER_SIZE; // increment to next locker start
		current_check_locker_id++; // incremeant locker_id

		if (current_check_locker_id >= MAX_LOCKERS) { // if id exceeds max lockers -> all lockers gone through
			current_check_locker_id = 255;
			return;
		}
		
	}


}

uint8_t findString(const char string[], const char substring[]) {

    uint8_t len1 = strlen(string);
    uint8_t len2 = strlen(substring);

    // the count is for how many of the correct charachters in str2 it has found in str1
    // times is how many times str2 have been found in str1 
    uint8_t count = 0;

    // loops throug each charachter in str1, and stops when the charachters remainding in st1 is less than charachters in str2
    for (uint8_t i = 0; i<=abs(len1-len2);i++) {

        // loop through each charachter in str2, and checks if the charachters now match up with str1, if not it breaks, and goes
        // to the next charachter in str1
        for (uint8_t j = 0; j < len2; j++) {
            
            //If the charachters in str1 and 2 dont match, it breaks the str2 loop, and sets maching charachters to 0
            if (string[i+j] != substring[j]) { 
                count = 0;
                break;
            };

            // if all charachters match, the count of matching charachters goes up by 1
            count++;

            // when the count of matching charachters equals the length of str2, it means that str2 have been found in str 1
            // one time, and it now adds the starting location to the output array at index of how many times str2 have been
            // found in str1, and resets the count to 0. It increments i by the length of str2 - 1 to make the program skip
            // to the next charachter after the discovered word in str1, -1 because the for loop increments i by 1 afterwards
            if (count == len2) { 
                return 1; // string is fond in 
            };
        }
    }
	return 0;
}












#endif