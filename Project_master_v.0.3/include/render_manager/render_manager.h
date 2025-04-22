#ifndef RENDER_MANGAGER_H
#define RENDER_MANGAGER_H
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "menu/menu.c"
#include "screen_popup/screen_popup.c"
#include "div/eeprom.h"

extern uint8_t eeprom_read_page;
extern uint8_t background_program[6];

void eeprom_print_arr();

//typedef enum background_prog {
//	None,
//	check_all_locker_alarms
//} background_prog;
//
//typedef enum current_running_prog {
//	no_render,
//	menu,
//	popup,
//	eeprom_print
//} current_running_prog;



// cursor functions
void update_cursor(int8_t x, int8_t y, uint8_t btn);

// render functions
void render();

// joystick functions
void joy_btn_pressed();

void render() {
	switch (current_running_program) {
		case no_render:
			break;

		case menu: // menu program
			menu_render();
			break;

		case popup:
			popup_render();
			break;

		case eeprom_print:
			eeprom_print_arr();
			break;

		default:
			break;
	}
}

void joy_btn_pressed() {
	switch (current_running_program) {

		case no_render:
			break;

		case menu: // menu
			menu_item_pressed(current_selected_item);
			break;

		case popup: // popup
			current_running_program = 1; // set program to menu program
			popup_delete_string_array();
			oled_cmd(0xA6); // non - inverted mode
			render();
			break;

		case eeprom_print: // eeprom print arr on screen
			eeprom_read_page++;
			if (eeprom_read_page >= 64) {
				current_running_program=1;
				eeprom_read_page = 0;
				render();
				break;
			}
			eeprom_print_arr();
			break;

		default:
			break;

	}
	


}

void update_cursor(int8_t x, int8_t y, uint8_t btn) {


	if ((current_cursor_y > 1) && ((current_cursor_y + y) <= max_cursor_y)) { // check if selectorY is between 0 and max list length -- 
		
		current_cursor_y += y;
	} else if ((current_cursor_y == 1) && (y == 1)) {
		current_cursor_y++;
	}

	if ((current_cursor_x > 0) && ((current_cursor_x + x) <= max_cursor_x)) {
		current_cursor_x += x;
	} else if ((current_cursor_x == 0) && (x == 1)) {
		current_cursor_x++;
	}

	if (btn == 1) {
		joy_btn_pressed();
	} else {
		render();
	}
	
}


void eeprom_print_arr() {
	//uint8_t draw_array[8] = {};
	//memset(draw_array,0xFF,8);
	uint16_t start_read = 512 + (8 * eeprom_read_page);
	//eepromReadArr(start_read, draw_array, 8);
	oled_clear();
	locker * new_locker = (locker *) malloc(sizeof(locker));
	uint8_t active = load_locker(new_locker, eeprom_read_page);

	oled_draw_text((volatile char *)"a1:",70,0,1,0);
	oled_draw_num(active, 100,0,1,0);


	oled_draw_text((volatile char *)"a2:",70,1,1,0);
	oled_draw_num(new_locker->locker_active, 100,1,1,0);


	oled_draw_text((volatile char *)"al:",70,2,1,0);
	oled_draw_num(new_locker->alarm_state, 100,2,1,0);


	oled_draw_text((volatile char *)"id:",70,3,1,0);
	oled_draw_num(new_locker->locker_id, 100,3,1,0);
	free(new_locker);

	uint8_t dataArr[8];
	memset(dataArr, 0 ,8);
	eepromReadArr(start_read, dataArr, 8);

	for (uint8_t i = 0; i < 8; i++) {
		char line[15];
		
		uint8_t data = eepromRead(i + start_read);

		

		sprintf(line, " %i: d: %X d2: %X", (i + (start_read)), data, dataArr[i]);
		oled_draw_text(line, 1,i,1,i%2);


		
	}
	
}






#endif