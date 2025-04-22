#ifndef SCREEN_POPUP_H
#define SCREEN_POPUP_H
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h> 
#include "screen_popup/screen_popup.c"
#include "oled/oled.h"

#define MAX_POPUP_ITEM_WIDTH 23
#define MAX_POPUP_LINE 7


extern char * current_popup_start;
extern uint8_t current_popup_lines;
extern uint8_t current_popup_running;

extern uint8_t current_cursor_x;
extern uint8_t current_cursor_y;

extern uint8_t max_cursor_x;
extern uint8_t max_cursor_y;

extern uint8_t current_running_program;


void popup_add_text(const char * text, uint8_t index);
void popup_delete_string_array();
void popup_make_string_array(uint8_t len);
void popup_render();
void init_popup(uint8_t popup_lines);

void init_popup(uint8_t popup_lines) {
	max_cursor_x = 0;
	max_cursor_y = 0;

	popup_make_string_array(popup_lines);
	current_running_program = 2;

}

void popup_add_text(const char * text, uint8_t index) {
	uint8_t i = 0, text_done = 0;

	if ((current_popup_start == NULL) || (index >= MAX_POPUP_LINE)) return; // avoid NULL-ptr accessing

	while ((i < MAX_POPUP_ITEM_WIDTH - 1)) {

		if (text[i] == '\0') {
			text_done = 1;
		}
		char c = '\0';
		if (text_done == 0) {
			c = text[i];
		}
		*(current_popup_start + index * MAX_POPUP_ITEM_WIDTH + i) = c;
		i++;
	}

}

void popup_make_string_array( uint8_t len) {
	if (current_popup_running == 1) {
		_delay_ms(2000);
		popup_delete_string_array();
	}
	current_popup_running = 1; 
	current_popup_start = (char *)malloc(MAX_POPUP_ITEM_WIDTH * (len) * sizeof(char));
	if (current_popup_start==NULL) return;
	memset(current_popup_start, 0, MAX_POPUP_ITEM_WIDTH * len);  // Clear memory
	current_popup_lines = len ;

}

void popup_delete_string_array() {
	if (current_popup_start != NULL) {
        free(current_popup_start);
        current_popup_start = NULL;
        current_popup_lines = 0;
    }
	current_popup_running = 0; //reset current_popup_running

}

void popup_render() {
	if (current_popup_start == NULL) return; // check if the current popup is not NULL
	oled_clear();

	oled_cmd(0xA7); // inverted mode

	for (uint8_t i = 0; i < current_popup_lines ; i++) {
		char text[MAX_POPUP_ITEM_WIDTH] = {0};
		uint8_t j = 0;
		while ((j < MAX_POPUP_ITEM_WIDTH) && (*(current_popup_start +i * MAX_POPUP_ITEM_WIDTH + j) != '\0')) {
			text[j] = *(current_popup_start + i * MAX_POPUP_ITEM_WIDTH + j);
			j++;
			
		}
		text[j] = '\0';
		

		oled_draw_text(text,64-(j*2),(i==0) ? 0 : (i+1), (i==0) ? 2 : 1,0);
	}

	oled_draw_text((volatile char * ) " PRESS BTN TO CONTINUE", 0,7,1,1);


}







#endif