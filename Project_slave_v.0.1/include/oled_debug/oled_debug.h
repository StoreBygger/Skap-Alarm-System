#ifndef OLED_DEBUG_H
#define OLED_DEBUG_H
#include "oled/oled.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


#define DEBUG_MAX_LINES 7

void debug_print(char * text) {
	static uint8_t current_line = 0;
	oled_clear_area(0, current_line, MAX_CURSOR_X, 1);

	oled_draw_text(text, 1, current_line, 1, current_line % 2);
	if (current_line < DEBUG_MAX_LINES) {
		oled_clear_area(0,current_line + 1, MAX_CURSOR_X, 1);
		oled_draw_text("slave~# ", 1,current_line + 1, 1, 0);
	} else {
		oled_clear_area(0, 0,MAX_CURSOR_X,1);
		oled_draw_text("slave~# ", 1,0, 1, 0);
	}
	

	current_line++;

	if (current_line > DEBUG_MAX_LINES) {
		current_line = 0;
	}

}

void debug_printf(char * text, ...) {
	va_list args;
	va_start(args, text);
	char line[strlen(text) * 2];
	vsprintf(line, text, args);
	debug_print(line);
	va_end(args);


}

#endif