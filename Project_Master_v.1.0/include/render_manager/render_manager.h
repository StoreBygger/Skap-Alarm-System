// master/render_manager.h

#ifndef RENDER_MANGAGER_H
#define RENDER_MANGAGER_H
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "menu/menu.h"
#include "screen_popup/screen_popup.c"
#include "div/eeprom.h"
#include "render_manager.c"


void eeprom_print_arr();
void eeprom_print_stop();
void eeprom_print_start();


// cursor functions
void update_cursor(int8_t x, int8_t y, uint8_t btn);

// render functions
void render();

// joystick functions
void joy_btn_pressed();

uint8_t ren_man_set_program(uint8_t set_program, ...);

void ren_man_stop_current_program();

void render() {
	switch (ren_man_set_program(0)) {
		case pgm_none:
			oled_clear();
			break;

		case pgm_menu: // menu program
			menu_render();
			break;

		case pgm_popup:
			popup_render();
			break;

		case pgm_eeprom_print:
			eeprom_print_arr();
			break;

		case pgm_LCN:
			LCN_render();
			break;

		case pgm_loading:
			loading_render();
			break;

		default:
			break;
	}
}


void joy_btn_pressed() {
	switch (ren_man_set_program(0)) {

		case pgm_none:
			break;

		case pgm_menu: // menu
			menu_item_pressed(current_selected_item);
			break;

		case pgm_popup: // popup
			popup_stop();
			menu_start();
			break;

		case pgm_eeprom_print: // eeprom print arr on screen
			eeprom_print_stop();
			break;

		case pgm_LCN:
			LCN_pressed();
			break;

		case pgm_sleep:
			sleep_stop();
			return;

		default:
			break;

	}
	


}

uint8_t ren_man_set_program(uint8_t set_program, ...) { /* 
	set_program == 0; return current program, 
	set_program == 1; return previous program, 
	set_program == 2; set current_program to previous program
	set_program == 3; set current_program to parameter after set_program
	set_program == 4; set previous program to given program
	*/
	static uint8_t previous_program = pgm_none, current_program = pgm_none;
	if (set_program == 0) return current_program;
	if (set_program == 1) return previous_program;
	if (set_program == 2) {
		if (current_program == previous_program) {
			return current_program;
		}
		uint8_t temp = current_program;
		current_program = previous_program;
		previous_program = temp;
		return current_program;
	}
	

	uint8_t program = pgm_none;
	va_list args;
	va_start(args, set_program);
	program = va_arg(args, int);
	va_end(args);
	if (set_program == 3) {

		if ( previous_program != current_program) {

			previous_program = current_program;
		}
		current_program = program;
	} else if (set_program == 4) {
		previous_program = program;
	}


	return previous_program;
}

void update_cursor(int8_t x, int8_t y, uint8_t btn) {

	if ((y == -1) && (current_cursor_y > 1)) { // up
		current_cursor_y = (current_cursor_y + max_cursor_y) % (max_cursor_y + 1);
	}

	if ((y == 1) && (current_cursor_y < max_cursor_y)) { // down
		current_cursor_y = (current_cursor_y + 1) % (max_cursor_y + 1);	
	}

	if ((x == 1) && (current_cursor_x > 1)) { // left
		current_cursor_x = (current_cursor_x + max_cursor_x) % (max_cursor_x + 1);
	}

	if ((x == -1) && (current_cursor_x < max_cursor_x)) { // right
		current_cursor_x = (current_cursor_x + 1) % (max_cursor_x + 1);	
	}

	if (btn == 1) {
		joy_btn_pressed();
	} else {
		render();
	}
	
}

void eeprom_print_arr() {


	uint8_t eeprom_read_page = current_cursor_y - 1;

	uint16_t start_read = 512 + (8 * eeprom_read_page);
	oled_clear();
	locker * new_locker = (locker *) malloc(sizeof(locker));
	load_locker(new_locker, eeprom_read_page);
	free(new_locker);

	uint8_t dataArr[8];
	memset(dataArr, 0 ,8);
	eepromReadArr(start_read, dataArr, 8);

	char page_text[15];
	sprintf(page_text, "-%i/%i-", eeprom_read_page + 1, MAX_LOCKERS);
	oled_draw_text(page_text, 80,6,2,0);

	for (uint8_t i = 0; i < 8; i++) {
		uint8_t inv = ((eeprom_read_page % 2 == 0) ? (i%2) : !(i%2));
		char line[15];
		
		uint8_t data = eepromRead(i + start_read);
		sprintf(line, "%i:\tdata: %02X", i, data);
		oled_draw_text(line, 1,i,1, inv);
	}

	
}

void eeprom_print_start() {
	ren_man_set_program(3, pgm_eeprom_print);
	current_cursor_y = 1;
	current_cursor_x = 0;
	max_cursor_y = 64;
	joystick_adc_start();
	render();

}

void eeprom_print_stop() {
	menu_start();
}





#endif