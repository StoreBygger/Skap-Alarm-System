#ifndef LOCKER_H
#define LOCKER_H
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "locker.c"
#include "div/eeprom.h"
#include "protocol_status.h"
#include "oled/oled.h"



void locker_init();
uint8_t load_locker(locker * self, uint8_t locker_id);
uint8_t locker_get_addr(char * addr_buffer, uint8_t locker_id);
void addr_str_to_arr(char * addr_str, uint8_t addr_arr[8]);
uint8_t locker_store_new( char * str_addr, uint8_t * locker_id);

void locker_check_alarm(uint8_t locker_id);
void locker_send_message(uint8_t type, uint8_t len_body, ...);

uint8_t locker_connect(uint8_t locker_id);
uint64_t hex_str_to_uint64(const char *str);
uint8_t locker_check_addr_exist(char addr[13]);

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
extern void menu_stop();
extern void menu_start();
extern void menu_init();

extern void render();


extern LCN_locker * first_LCN_locker;

extern uint8_t max_cursor_x;
extern uint8_t current_cursor_x;
extern uint8_t max_cursor_y;
extern uint8_t current_cursor_y;


void update_next_check_locker();



void locker_check_message(uint8_t * message);
uint8_t locker_check_recieved(uint8_t * recieved, uint8_t rec_len);

void LCN_init();
uint8_t LCN_check_DISC_recieve(char * data_recieve, uint8_t len);
void LCN_render();
uint8_t LCN_check_new_addr(char * addr);
void LCN_store_new();

void LCN_delete_list(LCN_locker * LCN_list, uint8_t len);
void LCN_delete_item(LCN_locker * LCN_list, uint8_t id); 

void LCN_stop();


void initTimer2();
void timer2_stop();
void timer2_start();


void locker_init() {
	oled_init();
	joystick_init();
	menu_init();
	bluetooth_init();

	if (eepromRead(0) == 0xFF) { // all eeprom is FF when ATmega32 loaded -> check if ATmega is not beeing manually reset
		locker_delete_all(); // delete all lockers
		eepromWrite(0,0xAA); // make sure first byte is not FF
	}

}

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

uint8_t locker_check_addr_exist(char addr[13]) {
	

	for (uint8_t i = 0; i < MAX_LOCKERS; i++) {
		char cmp_addr[13] = {0};
		uint8_t active = locker_get_addr(cmp_addr, i);
		if (active == 1) {
			if (strcmp(addr, cmp_addr) == 0) return 1; // locker addr exist
		}

	}

	return 0; // locker addr dont exist

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

	if (bluetooth_state != BLUETOOTH_DISCONNECTED) return 1; // fault -- bt connected
	char * locker_addr = (char *)malloc(13 * sizeof(char));
	uint8_t locker_active = locker_get_addr(locker_addr, locker_id);

	if (locker_active == 0) {
		free(locker_addr);
		return 1; // fault -> locker is not active
	}

	bluetooth_connect_addr(locker_addr);
	free(locker_addr);
	return 0; // no errors
}

void locker_delete(uint8_t locker_id) {

	for (uint8_t i = 0; i < (LOCKER_SIZE - 6); i++) {

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
			update_next_check_locker();
			break;

		case SEND_SKAP_IS_LOCKER:
			LCN_store_new();
			break;

		default:
			break;

	}
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
		return;
	}

	usart_recieve_string(rec_str, SIZE_RECIEVE_BUFFER);

	bluetooth_state = locker_id + 1; // set bluetooth to correct locker state

	locker_send_message(ASK_ALARM_STATUS, 0);

}


void locker_check_all_alarms() {

	current_check_locker_id = 0;

	update_next_check_locker();
}


void locker_reset_all_alarms() {


	for (uint8_t i = 0; i < MAX_LOCKERS; i++) {
		locker_update_alarm(i, SKAP_ALARM_INGEN);
	}



}

void update_next_check_locker() {

	char text[30];
	sprintf(text, "ID:%i", current_check_locker_id);

	init_popup(3);
	popup_add_text("Checking locker", 0);
	popup_add_text(text, 1); 
	render();
	_delay_ms(500);

	if (current_check_locker_id == 255) return; // if no next locker, return

	

	if (current_check_locker_id >= MAX_LOCKERS) { // if id exceeds max lockers -> all lockers gone through
		current_check_locker_id = 255;
		return;
	}

	uint8_t active = 0;

	uint16_t eeprom_locker_id = LOCKER_EEPROM_BASE + current_check_locker_id * LOCKER_SIZE; // set address to the first locker to read from

	active = eepromRead(eeprom_locker_id); // check if first locker is active
	while (active == 0) {
		current_check_locker_id++; // incremeant locker_id
		eeprom_locker_id = LOCKER_EEPROM_BASE + current_check_locker_id * LOCKER_SIZE; // set address to the first locker to read from
		active = eepromRead(eeprom_locker_id); // check if current locker is active

		if (current_check_locker_id >= MAX_LOCKERS) { // if id exceeds max lockers -> all lockers gone through
			current_check_locker_id = 255;
			return;
		}
		
	}
	init_popup(2);
	sprintf(text, "CONNECTING:%i", current_check_locker_id);
	popup_add_text("Check Locker", 0);
	popup_add_text(text, 1);
	render();

	locker_check_alarm(current_check_locker_id);
	current_check_locker_id++; // up current locker to next locker id


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
            if (count == len2) { 
                return i; // string is found startin in index i
            };
        }
    }
	return 255;
}


void locker_debug_status() {

	char text[30];

	init_popup(3);
	popup_add_text("Locker Debug", 0);
	sprintf(text, "bt state: %i", bluetooth_state);
	popup_add_text(text, 1);
	sprintf(text, "ch locker: %i", current_check_locker_id);
	popup_add_text(text, 2);

	render();

}

void LCN_init() {
	
	menu_stop();
	joystick_adc_start();
	current_running_program = pgm_LCN;
	max_cursor_x = 2;

	current_cursor_x = 1;
	current_cursor_y = 1;

	joystick_adc_start();
	clear_rx_interrupt();

	LCN_delete_list(LCN_locker_list, MAX_DISC); // delete previous list
	LCN_last_id = 0; // reset last id

	oled_clear();
	char recieved[LCN_LEN_RECIEVE];
	uint8_t err = check_AT_command("AT+DISC?", "OK+DISCS", recieved);

	if (err != 0) {
		LCN_stop();
	}
	
	oled_draw_text("SEARCHING DEVICES",31,0,2,0);
	
	char recieve_arr[LCN_LEN_RECIEVE];
	uint8_t i = 0, found = 0;
	do {
		char text[30];
		sprintf(text, "FOUND %i DEVICES", found);
		oled_draw_text(text, 32,4,2,0);
		usart_recieve_string_wait(recieve_arr, LCN_LEN_RECIEVE, 10000UL);
		if (findString(recieve_arr, "OK+DISCE") != 255) {
			break;
		}
		uint8_t err = LCN_check_DISC_recieve(recieve_arr, LCN_LEN_RECIEVE);
		if (err == 0) found++;

		
		i++;

		
	} while ((findString(recieve_arr, "DISCE") == 255));
	

	LCN_render();


	set_rx_interrupt();

}

uint8_t LCN_check_DISC_recieve(char * data_recieve, uint8_t len) {
	uint8_t LCN_id = LCN_last_id + 1;

	if (LCN_id > MAX_DISC) return 1; // fault -> last id not valid

	char * addr = (char *) malloc(13 * sizeof(char));

	for (uint8_t i = 0; i < 12; i++) {
		addr[i] = data_recieve[8 + i]; // found ->OK+DIS + OK+DISx: <- 10 + i 
	}

	addr[12] = '\0';

	if (LCN_check_new_addr(addr) == 1) {
		free(addr);
		return 1; // fault ->addr already exists	
	}

	char * name = (char *) malloc(LCN_LEN_NAME * sizeof(char));

	for (uint8_t i = 0; i < LCN_LEN_NAME; i++) {
		char c_name = data_recieve[8 + 12 + 8 + i]; // found ->OK+DIS + OK+DISx: + ADDR + OK+NAME: <- 9 + i 
		name[i] = c_name;

		if (c_name == '\r') {
			name[i] = '\0';
			break;
		}
	}

	addr[12] = '\0'; // end string correctly
	name[LCN_LEN_NAME - 1] = '\0';

	LCN_locker_list[LCN_id - 1].addr = addr;
	LCN_locker_list[LCN_id - 1].name = name;
	LCN_locker_list[LCN_id - 1].LCN_id = LCN_id;

	LCN_last_id = LCN_id;
	return 0;
	
}

void LCN_render() {
	oled_clear();
	oled_draw_text("Connect New Device", 1, 0, 2, 0);
	max_cursor_y = 0;
	

	for (uint8_t i = 0; i < MAX_DISC; i++) {
		LCN_locker render_locker = LCN_locker_list[i];
		uint8_t LCN_id = render_locker.LCN_id;

		if (LCN_id != 0) {
			char text[10 + LCN_LEN_NAME];

			uint8_t selected = current_cursor_y == LCN_id;

			char screen_show[LCN_LEN_NAME] = "\0";
			if ((current_cursor_x == 2) && selected) {
				strcat(screen_show, "addr: ");
				strcat(screen_show, render_locker.addr);
			} else {
				strcat(screen_show, "name: ");
				strcat(screen_show, render_locker.name);
			}

			

			sprintf(text, "%i-%s", LCN_id, screen_show);

			oled_draw_text(text, 1,i + 2,1,selected);
			max_cursor_y++;
		}
		
	}
}

void LCN_delete_list(LCN_locker * LCN_list, uint8_t len) {
	for (uint8_t i = 0; i<len; i++) {
		LCN_delete_item(LCN_list, i);
	}
}

void LCN_delete_item(LCN_locker * LCN_list, uint8_t id) {
	free(LCN_list[id].addr);
	LCN_list[id].addr = NULL;

	free(LCN_list[id].name);
	LCN_list[id].name = NULL;

	LCN_list[id].LCN_id = 0;
}

uint8_t LCN_check_new_addr(char * addr) {

	for (uint8_t i = 0; i < MAX_DISC; i++) {
		
		if (strcmp(addr, LCN_locker_list[i].addr) == 0) return 1; // error -> address already exists
	}

	return 0;
}

void LCN_pressed() {

	oled_clear();

	if (LCN_last_id == 0) {
		LCN_stop();
		return;
	} // no items in LCN list -> probebly list not ready

	clear_rx_interrupt();

	char rec_str[SIZE_RECIEVE_BUFFER];
	memset(rec_str, '\0', SIZE_RECIEVE_BUFFER);

	bluetooth_connect_addr(LCN_locker_list[current_cursor_y-1].addr);

	uint8_t err = 0;
	if (err != 0) {
		LCN_stop();
		return;
	}

	usart_recieve_string(rec_str, SIZE_RECIEVE_BUFFER);

	bluetooth_state = MAX_LOCKERS + current_cursor_y + 1; // set bluetooth to correct locker state

	uint8_t recieve[30];
	memset(recieve, 0, 30); // reset recieve
	locker_send_message(ASK_SKAP_IS_LOCKER, 0);

	usart_recieve_string_wait((char *) recieve, 30, 100); // wait ~1s maximum
	err = locker_check_recieved(recieve, 30);

	if (err != 0) {
		LCN_stop();
		return;
	}

	set_rx_interrupt();
}


void LCN_store_new() {

	

	if ((bluetooth_state < MAX_LOCKERS) || ((bluetooth_state > (MAX_LOCKERS + 1 + MAX_DISC )))) {
		LCN_stop();
		init_popup(1);
		popup_add_text("LOCKER OUTSIDE PARAMS",0);
		return;
	} // error -> wrong bluetooth state

	uint8_t LCN_id = bluetooth_state - 1 - 64;

	uint8_t locker_id = 0;

	uint8_t addr_found = locker_check_addr_exist(LCN_locker_list[LCN_id - 1].addr);

	if (addr_found != 0) {
		LCN_stop();
		init_popup(2);
		popup_add_text("LOCKER ALREADY EXISTS",0);
		popup_add_text(LCN_locker_list[LCN_id - 1].name,1);
		render();
		return;
	}
	locker_store_new(LCN_locker_list[LCN_id - 1].addr,&locker_id);
	
	LCN_stop();
	init_popup(2);
	popup_add_text("LOCKER PAIRED",0);
	popup_add_text(LCN_locker_list[LCN_id - 1].name,1);
	render();
	
}

void LCN_stop() {
	_delay_ms(1000);
	LCN_delete_list(LCN_locker_list, MAX_DISC);
	bluetooth_disconnect();
	menu_start();
}


void initTimer2() { // set timer2 in normal mode, and trigger interupt on overflow vector
	/*
		Because with 8MHz -> OCn frequency with OCR2 = 254 -> frequency = 30Hz -> too low for the ear to hear
	*/
	OCR2 = 10;
	DDRD |= (1<<PD7); // PD7 / OC2 output
	TCCR2 |= (1<<WGM21); // ctc mode
	//TCCR2 |= (1<<COM20);

	TIMSK |= (1<<OCIE2);
	sei();
}

void timer2_start() {
	TCCR2 |= (1<<CS22) | (1<<CS21) | (1<<CS20); // enable prescalers
}

void timer2_stop() {
	TCCR2 &= ~((1<<CS22) | (1<<CS21) | (1<<CS20)); // disable prescalers
}

ISR (TIMER2_COMP_vect) {
	static uint8_t state = 0;

	state++;

	if (state < 128) {
		PORTD ^= (1<<PD7);
		
		OCR2 += 1;
		if (OCR2 >= 15) OCR2 = 1;

	} else {
		PORTD &= ~(1<<PD7);
		OCR2 = 10;
	}

}

void initTimer1() {

	DDRD |= (1<<PD5); //OC1A output
	TCCR1A |= (1<<COM1A1); // toggle OC1A on compare match
	TCCR1B |= (1<<WGM12); // ctc mode
	
}

void timer1_start(uint16_t count) {
	OCR1A = count;
	TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
}

void timer1_stop() {
	TCCR1B &= ~((1<<CS12) | (1<<CS10));
}


#endif







