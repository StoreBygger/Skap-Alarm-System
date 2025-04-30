#ifndef MENU_H
#define MENU_H

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h> 

#include "oled/oled.c"
#include "joystick/joystick.c"
#include "Bluetooth/bluetooth.h"
#include "locker/locker.h"
#include "menu.c"





extern menu_item * current_menu_header;
extern menu_item * current_selected_item;

extern uint8_t current_menu_id;
extern uint8_t current_menu_render_y;


// cursor positions
extern uint8_t current_cursor_x;
extern uint8_t current_cursor_y;

extern uint8_t max_cursor_x;
extern uint8_t max_cursor_y;

// menu scroll
extern uint8_t scroll_page;
extern uint8_t max_scroll_page;

extern const char menu_text_main_menu[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM; 		// menu_to_get_index = 1
extern const char menu_text_alarms[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE]; 				// menu_to_get_index = 2
extern const char menu_text_connections[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM;	// menu_to_get_index = 3
extern const char menu_text_options[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM;		// menu_to_get_index = 4
extern const char menu_text_credits[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM;		// menu_to_get_index = 5
extern const char menu_text_debug[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM;			// menu_to_get_index = 6

extern const uint8_t menu_state_main_menu[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM ;
extern const uint8_t menu_state_alarms[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM;
extern const uint8_t menu_state_connections[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM;
extern const uint8_t menu_state_options[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM;
extern const uint8_t menu_state_credits[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM;
extern const uint8_t menu_state_debug[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM;

extern const char menu_text_locker[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM; // menu_to_get index = 64
extern const uint8_t menu_state_locker[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM;

extern const uint8_t menu_state_all_headers[MAX_MENUS][MAX_MENU_STATE];



extern uint8_t current_running_program; // defines the current running program


extern void popup_add_text(const char * text, uint8_t index);
extern void popup_delete_string_array();
extern void popup_make_string_array(uint8_t len);
extern void popup_render();

extern char * current_popup_start;
extern uint8_t current_popup_lines;

extern void locker_reset_all_alarms();

extern void eeprom_print_start();



// load from PROGMEM functions
void menu_load_menu_text(char * textbuffer, uint8_t menu_to_get_index, uint8_t text_to_get_index);
void menu_load_menu_state(uint8_t * state_buffer, uint8_t menu_to_get_index, uint8_t item_to_get_id);
void menu_load_menu_header(uint8_t * state_buffer, uint8_t menu_to_get_index);

// menu construction functions
void menu_make_menu(menu_item * start_pointer, uint8_t menu_id);
void menu_update_menu_item(menu_item * menu_item_to_update, uint8_t * state);
void menu_make_menu_item(menu_item * menu_item_pointer, uint8_t menu_id, uint8_t item_to_get_id);
void menu_insert_menu_item(menu_item * item_to_insert, menu_item * item_before);
void menu_free_menu (menu_item * menu_pointer);
uint8_t menu_insert_lockers(menu_item * item_before, uint8_t menu_id, menu_item ** item_after);
void menu_make_locker_item(menu_item * menu_item_pointer, uint8_t parent_id, locker * self);

// render functions 
void menu_render();
void menu_item_render(	menu_item * self, 	uint8_t x, uint8_t y, uint8_t selected);
void menu_header_render(menu_item * self, 	uint8_t x, uint8_t y, uint8_t selected);
void menu_text_render(	menu_item * self, 	uint8_t x, uint8_t y, uint8_t selected);
void menu_locker_render(menu_item * self, uint8_t x, uint8_t y, uint8_t selected);
void menu_locker_header_render(menu_item * self, uint8_t x, uint8_t y, uint8_t selected);

// pressed functions
void menu_item_pressed(menu_item * self);
void menu_func_pressed(menu_item * self);
void menu_reset_alarms();
void menu_locker_delete(menu_item * self);
void menu_locker_get_addr(menu_item * self);
void menu_locker_check_alarm(menu_item * self);

void menu_init();
void menu_start();
void menu_stop();

// cursor functions
extern void update_cursor(int8_t x, int8_t y, uint8_t btn);


void menu_load_menu_text(char * textbuffer, uint8_t menu_to_get_index, uint8_t text_to_get_index) {
	// function that loads the text for the current menu
	const char * menu_text_ptr;

	switch (menu_to_get_index) {
		case 1:
			menu_text_ptr = menu_text_main_menu[text_to_get_index];
			break;

		case 2:
			menu_text_ptr = menu_text_alarms[text_to_get_index];
			break;
		case 3:
			menu_text_ptr = menu_text_connections[text_to_get_index];
			break;
		case 4:
			menu_text_ptr = menu_text_options[text_to_get_index];
			break;
		case 5:
			menu_text_ptr = menu_text_credits[text_to_get_index];
			break;

		case 6:
			menu_text_ptr = menu_text_debug[text_to_get_index];
			break;

		case 64 ... 128:
			menu_text_ptr = menu_text_locker[text_to_get_index];
			break;

		default:
			return;
			break;

	}

	strcpy_P(textbuffer, (PGM_P)menu_text_ptr);
}
void menu_load_menu_state(uint8_t * state_buffer, uint8_t menu_to_get_index, uint8_t item_to_get_id) {
	// function that loads the text for the current menu
	const uint8_t * menu_state_ptr;
	

	switch (menu_to_get_index ) {
		case 1:
			menu_state_ptr = &menu_state_main_menu[item_to_get_id][0];
			break;

		case 2:
			menu_state_ptr = &menu_state_alarms[item_to_get_id][0];
			break;
		case 3:
			menu_state_ptr = &menu_state_connections[item_to_get_id][0];
			break;
		case 4:
			menu_state_ptr = &menu_state_options[item_to_get_id][0];
			break;
		case 5:
			menu_state_ptr = &menu_state_credits[item_to_get_id][0];
			break;

		case 6:
			menu_state_ptr = &menu_state_debug[item_to_get_id][0];
			break;

		case 64 ... 128:
			menu_state_ptr = &menu_state_locker[item_to_get_id][0];
			break;
		
		default:
			return;
			break; 

	}

	for (uint8_t i = 0; i < MAX_MENU_STATE; i++) {
		*(state_buffer + i) = pgm_read_byte(menu_state_ptr + i);
	}
}
void menu_load_menu_header(uint8_t * state_buffer, uint8_t menu_to_get_index) {

	if ((menu_to_get_index >= 64) && (menu_to_get_index < 128)) { // if menu is locker_menu, menu_to_get index is 7 in the menu_state_all_headers
		menu_to_get_index = 7;
	}

	const uint8_t * state = menu_state_all_headers[--menu_to_get_index]; // subtract one before loading in menu header

	for (uint8_t i = 0; i < MAX_MENU_STATE; i++) {
		*(state_buffer + i) = pgm_read_byte(state + i);
	}
}

void menu_update_menu_item(menu_item * menu_item_to_update, uint8_t * state) {
	/*
		Updates the given menu_item with the parameters given in the state array
	*/
	menu_item_to_update->type = state[0];
	menu_item_to_update->text_id = state[1];
	menu_item_to_update->item_id = state[2];
	menu_item_to_update->parent_id = state[3];
	menu_item_to_update->state = state[4];
}

void menu_make_menu(menu_item * start_pointer, uint8_t menu_id) {
	/* 
		Makes a menu, with the first item at start_pointer
		Makes the menu with menu_id, all items will get parent_id  = menu_id 

		
	*/

	uint8_t header_state[MAX_MENU_STATE];
	menu_load_menu_header(header_state, menu_id );
	menu_update_menu_item(start_pointer, header_state);

	menu_free_menu(start_pointer);
	menu_make_menu_item(start_pointer, menu_id,0);

	

	uint8_t menu_len = start_pointer->state;
	uint8_t inserted = 0;

	menu_item * previous_item = start_pointer;

	inserted += menu_insert_lockers(current_menu_header,menu_id, &previous_item);

	current_menu_header->state = menu_len + inserted;

	
	for(uint8_t i = 1; i < menu_len; i++) {
		menu_item * new_item = (menu_item *)malloc(sizeof(menu_item));
		if(!new_item) return;

		menu_make_menu_item(new_item, menu_id, i);
		menu_insert_menu_item(new_item, previous_item);
		previous_item = new_item;
	}
	previous_item->next = NULL;
	current_cursor_y = 1;
}

uint8_t menu_insert_lockers(menu_item * item_before, uint8_t menu_id, menu_item ** item_after) {

	if ((menu_id != 2) && (menu_id != 3)) { // cath if the menu should not have menu_id's
		*item_after = item_before;
		return 0;
	}


	menu_item * previous_item = item_before; 

	uint8_t locker_draw = 0;

	for (uint8_t i = 0; i < MAX_LOCKERS; i++) {
		locker * new_locker = (locker * )malloc(sizeof(locker));
		uint8_t locker_active = load_locker(new_locker, i);


		if (locker_active == 1) {

			if ((menu_id == 2) && (new_locker->alarm_state == SKAP_ALARM_INGEN)) continue; // only render lockers with current alarm in alarm menu

			menu_item * new_item = (menu_item *)malloc(sizeof(menu_item));
			menu_make_locker_item(new_item,menu_id,new_locker);
			menu_insert_menu_item(new_item, previous_item);

			previous_item = new_item;
			locker_draw++;
		}


		free(new_locker);

	}
	*item_after = previous_item;

	return locker_draw;

}


void menu_free_menu (menu_item * menu_pointer) {

	// loop to the first menu item:
	while ((menu_pointer->previous) != NULL) {
		menu_pointer = menu_pointer->previous;
	}

	menu_item * temp;
	while (menu_pointer->next != NULL) {
        temp = menu_pointer->next; // Store the next item
        free(menu_pointer->next);  // Free the current next item
        menu_pointer->next = temp->next; // Update the link
    }
}



void menu_make_menu_item(menu_item * menu_item_pointer, uint8_t menu_id, uint8_t item_to_get_id) {
	uint8_t state[MAX_MENU_STATE];

	menu_load_menu_state(state, menu_id, item_to_get_id);

	if ((menu_id >= 64) && (menu_id < 128)) { // change menu id correctly when menu_id is locker
		state[3] = menu_id;
	}

	menu_item_pointer->type = state[0];
	menu_item_pointer->text_id = state[1];
	menu_item_pointer->item_id = state[2];
	menu_item_pointer->parent_id = state[3];
	menu_item_pointer->state = state[4];


	menu_item_pointer->previous = NULL;
	menu_item_pointer->next = NULL;

	menu_item_pointer->menu_item_pressed = menu_item_pressed;
	menu_item_pointer->menu_item_render = menu_item_render;

}

void menu_insert_menu_item(menu_item * item_to_insert, menu_item * item_before) {

	menu_item * next_item = NULL;
	
	if((item_before->next) != NULL) { // check if there is an item before the item_before (if it is not NULL)
		next_item = item_before->next;
		next_item->previous = item_to_insert;
	}
	
	item_to_insert->next = next_item;
	item_to_insert->previous = item_before;

	item_before->next = item_to_insert;
}

void menu_render() {

	oled_clear();
	current_menu_render_y = 0;

	max_cursor_y = 0;

	if (!current_menu_header) return;

	menu_item * render_item = current_menu_header;
	menu_item_render(render_item, 1, current_menu_render_y, 0);

	uint8_t item = 0;
	uint8_t menu_len = current_menu_header->state;

	scroll_page = (current_cursor_y - 1) / MAX_MENU_RENDER_ITEMS;


	for (uint8_t i = 0; i<(scroll_page * MAX_MENU_RENDER_ITEMS); i++) {
		render_item = render_item->next;
		max_cursor_y++;
	}
	render_item = render_item->next; // skip over the header if it is the first page

	uint8_t select = 0;

	while ((render_item != NULL) && ( item < MAX_MENU_RENDER_ITEMS)) {
		
		select = ((current_menu_render_y + (scroll_page * MAX_MENU_RENDER_ITEMS)) == (current_cursor_y + 1));
		menu_item_render(render_item, 1, current_menu_render_y, select);
		render_item = render_item->next;
		item++;	
		max_cursor_y++;

		if (max_cursor_y >= menu_len) break;
		

		
	}
	oled_draw_num(menu_len,64,7,0,0);
	oled_draw_num(max_cursor_y,64,6,0,0);
	max_cursor_y++;
	max_scroll_page = max_cursor_y / (MAX_MENU_ITEMS + 1);

}
void menu_item_render(menu_item * self, uint8_t x, uint8_t y, uint8_t selected ) {

	if (selected) {
		current_selected_item = self;
	}
	switch(self->type) {
		case header_item: // header
			menu_header_render(self, x, y,selected);
			break;
		
		case menu_pointer_item: // menu pointer
			menu_text_render(self, x, y,selected);
			break;

		case locker_item: // locker
			menu_locker_render(self, x, y, selected);
			break;

		case function_item: // function
			menu_text_render(self, x, y, selected);
			break;

		case back_item: // back
			menu_text_render(self, x, y ,selected);
			break;

		case locker_header_item: // locker header
			menu_locker_header_render(self, x, y,selected);
			break;

		default:
			menu_text_render(self, x, y,selected);
			break;

	}
}

void menu_header_render(menu_item * self, uint8_t x, uint8_t y, uint8_t selected) {
	char text[MAX_MENU_TEXT_SIZE];
	menu_load_menu_text(text, self->parent_id, self->text_id);
	oled_draw_text(text,x,y,2,selected);
	current_menu_render_y += 2; // add current_menu_render_y by 2 -> by textsize
}

void menu_locker_header_render(menu_item * self, uint8_t x, uint8_t y, uint8_t selected) {

	char text[MAX_MENU_TEXT_SIZE];
	char buffer[MAX_MENU_TEXT_SIZE];
	menu_load_menu_text(buffer, self->parent_id, self->text_id);
	sprintf(text, buffer, self->parent_id - 64);
	oled_draw_text(text,x,y,2,selected);
	oled_draw_num(self->state,cursor[0]+6,y,2,0);
	current_menu_render_y += 2; // add current_menu_render_y by 2 -> by textsize
	
}
void menu_text_render(menu_item * self, uint8_t x, uint8_t y, uint8_t selected) {
	char text[MAX_MENU_TEXT_SIZE];
	menu_load_menu_text(text, self->parent_id, self->text_id);
	oled_draw_text(text,x,y,1,selected);
	current_menu_render_y++;
}

void menu_locker_render(menu_item * self, uint8_t x, uint8_t y, uint8_t selected) {

	uint8_t locker_id = self->item_id - 64;
	char text[25];

	sprintf(text, "Locker %i", locker_id);
	
	if (self->state != SKAP_ALARM_INGEN) {
		strcat(text, "\t\tALARM");
	}

	oled_draw_text(text,x,y,1,selected);
	current_menu_render_y++;
}


void menu_item_pressed(menu_item * self) {
	switch (self->type) {
		case menu_pointer_item: // new menu
			menu_make_menu(current_menu_header, self->state);
			menu_render();
			break;

		
		case locker_item: // locker
			menu_make_menu(current_menu_header, self->item_id);
			menu_render();
			break;


		case function_item: // function
			menu_func_pressed(self);
			break;

		case back_item: // back
			menu_make_menu(current_menu_header, 1);
			menu_render();
			break;

		



		default:
			break;
	}
}

void menu_func_pressed(menu_item * self) {

	switch (self->item_id) {

		case 11: // reset -> reset all alarms
			menu_reset_alarms();
			break;

		case 12: // reset -> get alarm status
			locker_check_all_alarms();
			break;

		case 19: // connect new device
			LCN_init();
			break;

		case 20: // delete all devices
			locker_delete_all();
			break;

		case 28:
			sleep_start();
			break;

		case 43:
			bluetooth_check_AT();
			break;

		case 44:
			bluetooth_check_ADDR();
			break;

		case 45:
			bluetooth_disconnect();
			break;
		
		case 46:
			bluetooth_device_init();
			break;

		case 47:
			menu_stop();
			eeprom_print_start();
			break;


		case 48:
			locker_debug_status();
			break;

		case 130: // locker reset alarm
			locker_update_alarm(self->parent_id - 64, SKAP_ALARM_INGEN);
			break;

		case 131: // locker check alarm
			menu_locker_check_alarm(self);
			break;


		case 132: // locker get address
			menu_locker_get_addr(self);
			break;

		case 133: // locker delete
		 	menu_locker_delete(self);
			break;

		default:
			break;
	}
}

void menu_reset_alarms() {
	locker_reset_all_alarms();

}

void menu_init() {
	menu_make_menu(current_menu_header,1);
	menu_render();
}


void menu_make_locker_item(menu_item * menu_item_pointer, uint8_t parent_id, locker * self) {

	menu_item_pointer->type = 3; // type == locker
	menu_item_pointer->text_id = 0;
	menu_item_pointer->item_id = self->locker_id + 64;
	menu_item_pointer->parent_id = parent_id;
	menu_item_pointer->state = self->alarm_state;


	menu_item_pointer->previous = NULL;
	menu_item_pointer->next = NULL;

	menu_item_pointer->menu_item_pressed = menu_item_pressed;
	menu_item_pointer->menu_item_render = menu_item_render;
}

void menu_locker_delete(menu_item * self) {
	locker_delete(self->parent_id - 64); // delete current locker
	init_popup(1);
	char text[20];
	sprintf(text, "Deleted locker %i", self->parent_id-64);
	popup_add_text(text, 0);
	menu_make_menu(current_menu_header, 1); // make menu item
	render();

}

void menu_locker_get_addr(menu_item * self) {

	uint8_t locker_id = self->parent_id - 64;
	char locker_addr[13];
	uint8_t found = locker_get_addr(locker_addr, locker_id);

	char text[20];
	uint8_t j = 0;

	for(uint8_t i = 0; i < 17; i++) {
		if (i % 3 == 2) {
			text[i] = ':';
		} else {
			text[i] = locker_addr[j++];
		}
	}
	text[17] = '\0';

	char header[19];
	if (found != 0) {
		sprintf(header, "Locker %i address", locker_id);
		init_popup(3);
		popup_add_text(header, 0);
		popup_add_text("MAC-address:",1);
		popup_add_text(text, 2);
	} else {
		sprintf(header, "Locker %i address", locker_id);
		init_popup(2);
		popup_add_text(header, 0);
		popup_add_text("NOT FOUND",1);
	}
	
	render();

	
}

void menu_locker_check_alarm(menu_item * self) {

	uint8_t locker_id = self->parent_id - 64;

	locker_check_alarm(locker_id);


}

void menu_start() {
	joystick_adc_start();
	current_running_program = pgm_menu;
	menu_make_menu(current_menu_header, 1);
	render();
}

void menu_stop() {
	joystick_adc_stop();
	current_running_program = pgm_none;
	menu_free_menu(current_menu_header);
	render();
}



#endif