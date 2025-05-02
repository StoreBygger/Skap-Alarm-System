// master/oled.h

#ifndef oled_h
#define oled_h

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

#include "abcbitmap.h"
#include "oled.c"
#include "div/twi.h"

#define TWI_SPEED_MASTER 500000


#define max(x, y) ((x) < (y) ? (y) : (x))
#define min(x, y) ((x) > (y) ? (y) : (x))
#define TAB_SIZE 16

#define TWI_SPEED 500000
#define OLED_ADDR 0x3C

#define TWI_TIMEOUT 16000
#define TWI_DELAY 1

#define MAX_CURSOR_X 128
#define MAX_CURSOR_Y 8

extern uint8_t cursor[2];
extern uint8_t address_mode;


// control over cursor functions
void cursor_add(uint8_t x,uint8_t y);
uint8_t cursor_getX();
uint8_t cursor_getY();
void cursor_set(uint8_t x, uint8_t y);

// make integers numbers
void str_from_uint16_t(uint16_t num, volatile char * str);

// basic oled functions
void oled_data(uint8_t data);
void oled_cmd(uint8_t cmd);
void oled_move(uint8_t x, uint8_t y);
void oled_move_NoStop(uint8_t x, uint8_t y);
void oled_set_mode(uint8_t mode);
void oled_init();
void oled_config(volatile uint8_t * commands, uint8_t size);

// Oled display functions
void oled_clear();
void oled_clear_area(uint8_t x, uint8_t page_start, uint8_t width, uint8_t page_height);
void oled_draw_letter(uint8_t letter, uint8_t  x, uint8_t  y, uint8_t textsize, uint8_t invert);
void oled_draw_text(volatile char * string, uint8_t x, uint8_t y, uint8_t textsize, uint8_t invert);
void oled_draw_bitmap(volatile uint8_t * bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void oled_draw_num(int num, uint8_t x, uint8_t y, uint8_t textsize, uint8_t invert);


uint8_t get_letter_bitmap(uint8_t letter, volatile uint8_t * buffer, uint8_t invert);


void cursor_add(uint8_t x,uint8_t y) {
	x += cursor[0];
	y += cursor[1];

	if (x > MAX_CURSOR_X) {
		y += 1;
		x = 0;
		oled_move(x,y);
		
	}
	if (y > MAX_CURSOR_Y) {
		y = 0;
		x = 0;
		oled_move(x,y);
	}

	cursor[0] = x;
	cursor[1] = y;
}

void cursor_set(uint8_t x, uint8_t y) {
	cursor[0] = x;
	cursor[1] = y;
}

uint8_t cursor_getX() {
	return cursor[0];
}

uint8_t cursor_getY() {
	return cursor[1];
}

void oled_data(uint8_t data) {
	TWI_start();
	TWI_SLA(OLED_ADDR,0);
	TWI_TData(0x40); // data mode
	TWI_TData(data);
	TWI_stop();
	cursor_add(1,0);
}

void oled_cmd(uint8_t cmd) {
	TWI_start();
	TWI_SLA(OLED_ADDR,0);
	TWI_TData(0x00); // command mode
	TWI_TData(cmd);
	TWI_stop();
}

void oled_move(uint8_t x, uint8_t y) {
	oled_move_NoStop(x,y);
	TWI_stop();
	cursor[0] = x;
	cursor[1] = y;
	
}

void oled_move_NoStop(uint8_t x, uint8_t y) {
	TWI_start();
	TWI_SLA(OLED_ADDR, 0);
	TWI_TData(0x00);
	TWI_TData(0xB0 + y);
	TWI_TData(0x00 + (x & 0x0F));
	TWI_TData(0x10 + ((x>>4) & 0x0F));
	cursor[0] = x;
	cursor[1] = y;
	
}


void str_from_uint16_t(uint16_t num, volatile char * str) {
	uint8_t i = 0;
	uint8_t temp[6]; // largest to acces 2^16 65xxx

	if (num == 0) {
		* (str)= '0';
		i++;
	} else {
		
		while (num != 0) {
			temp[i] = (num%10) + '0';
			num = num/10;
			i++;
		}

		for (uint8_t j = 0; j < i; j++ ) {
			* (str + j) = temp[i-j-1];
		}
	}
	* (str + i) = '\0';

}

uint8_t letter_special_char(uint8_t letter, uint8_t * x, uint8_t * y) {

	switch (letter) {

		case '\n':
			//*x = 0;
			//*y += 1;
			//cursor_set(*x, *y);
			//return 1;
			break;

		

		default:
			break;
	}

	return 0;
}

uint8_t get_letter_bitmap(uint8_t letter, volatile uint8_t * buffer, uint8_t invert) {
	uint8_t j = 0, bmp = 4, nx = 0;

	switch (letter) {

		case '\t':
			nx = TAB_SIZE - (cursor_getX() % TAB_SIZE);
			memset((uint8_t *) buffer, 0xFF * invert, nx);
			return nx;
			break;

		case 'A' ... 'Z':
			j = letter - 'A';
			bmp = 0;
			break;
		
		case 'a' ... 'z':
			j = letter - 'a';
			bmp = 1;
			break;
			
		case '0' ... '9':
			j = letter - '0';
			bmp = 2;
			break;
		
		case 0xC6:
			j = 26;
			bmp = 0;
			break;
		
		case 0xD8:
			j = 27;
			bmp = 0;
			break;
		
		case 0xC5:
			j = 26;
			bmp = 0;
			break;

		case 0xE6:
			j = 26;
			bmp = 1;
			break;
		
		case 0xF8:
			j = 27;
			bmp = 1;
			break;
		
		case 0xE5:
			j = 26;
			bmp = 1;
			break;


		


		default:
			for (uint8_t k = 0; k<26;k++) {
				uint8_t c = pgm_read_byte(&specialVal + k);
				if (letter == c) {
					j = k;
					bmp = 3;
					break;
				}
			}
			break;
	}

	return load_bitmap(bmp, buffer, j, invert);
}

void double_bitmap(volatile uint8_t * bitmap, uint8_t size, volatile uint8_t * buffer) {
	
	for (uint8_t i = 0; i < size; i++) { // iterates through the bitmap
		uint8_t byte = bitmap[i]; // gets the current byte of the iteration of the bitmap
		uint8_t byte_upper = (byte & 0xF0) >> 4; // seperate the lower and upper part of the byte, and bitshift them to be the 4 most LSB
		uint8_t byte_lower =  byte & 0x0F; 

		uint8_t line_upper = 0, line_lower = 0;

		for (uint8_t j = 0; j < 4; j++) { // iterate through the byte, j is the position if the current byte, from LSB to MSB
			uint8_t bit_upper = (byte_upper>>j) & 1; // get one single bit
			uint8_t bit_lower = (byte_lower>>j) & 1; // get one single bit

			line_upper |= (bit_upper << (j*2)) | (bit_upper << ((j*2) + 1));
			line_lower |= (bit_lower << (j*2)) | (bit_lower << ((j*2) + 1));

		}
		

		buffer[i] = line_lower;
		buffer[i + size] = line_upper;
	}
}

void oled_draw_letter(uint8_t letter, uint8_t x, uint8_t y, uint8_t textsize, uint8_t invert) {
	// letter is char of letter, x and y is position of letter, size is textsize, 1 = normal, 2 = double

	//if (letter_special_char(letter,&x, &y) == 1) return; // if letter is special char, deal with it and return

	uint8_t bitmap[TAB_SIZE] = {
		0b01111110,
		0b01001010,
		0b01100110,
		0b01011010,
		0b01111110
	};
	uint8_t buffer[20];
	memset(buffer, 0 ,20);
	
	uint8_t ch_size = get_letter_bitmap(letter, bitmap, invert);

	if (textsize == 2) {
		double_bitmap(bitmap, ch_size, buffer);
		oled_draw_bitmap(buffer,  x, y , ch_size,textsize);
		return;
	} 
	oled_draw_bitmap(bitmap,  x, y , ch_size,textsize);

	

}
void oled_draw_text(volatile char * string, uint8_t x, uint8_t y, uint8_t textsize, uint8_t invert) {
	uint8_t c = string[0], i = 1;
	uint8_t cx = x, cy = y;

	while (c !='\0') {

		oled_draw_letter(c, cx, cy, textsize, invert);
		c = string[i];
		i++;
		cx = cursor_getX();

		if (cx > 127) {
			cx = x;
			cy += textsize;
			textsize = 1;
		}
	}
}

void oled_draw_num(int num, uint8_t x, uint8_t y, uint8_t textsize, uint8_t invert) {
	char buffer[10]; // the larges number it can make is 10 digits long 
	sprintf(buffer, "%i", num);
	oled_draw_text(buffer, x, y ,textsize, invert);
}

void oled_config(volatile uint8_t * commands, uint8_t size) {
	for (uint8_t i = 0; i < size; i++) {
		oled_cmd(commands[i]);
	}
}


void oled_clear() {
	
	for (uint8_t page = 0; page < 8; page++ ) {
		oled_move_NoStop(0, page);
		TWI_start();
		TWI_SLA(OLED_ADDR, 0);
		TWI_TData(0x40);
		for (uint8_t col = 0; col < 128; col++) {
			TWI_TData(0x00);
		}
		
	}
	TWI_stop();
	
}

void oled_set_mode(uint8_t mode) {
	// 0 = Horizontal addressing mode
	// 1 = Vertical addressing mode
	// 2 = page addressing mode (reset)
	// 3 = invalid

	if (mode > 2) {return; } // do nothing if mode is invalid
	uint8_t cmds[2] = {0x20, mode}; // 0x20 = set oled addressing mode
	oled_config(cmds, 2);
	address_mode = mode; // change global address mode setting
}

void oled_init() {
	TWI_Init(TWI_SPEED);
	oled_set_mode(2); // set page addressing mode
	oled_move(0,0); // move cursor first

	oled_cmd(0x81); // 0x81 set Contrast 
	oled_cmd(0xFF); //  0xFFset contras to this level, valid range: 01h to FFh

	

	oled_cmd(0x40); // 0x40 ensure first line in RAM is first line in display
	oled_cmd(0xA1); // 0xA1 ensure oled content starts correct way ( 0xA1 to flip)
	
	oled_cmd(0xA6); // 0xA6 ensure oled is not- inverted (0xA7 to invert)

	oled_cmd(0xA8); // 0xA8 enter MUX setting
	oled_cmd(0x3F); // 0x3F set 64PX MUX

	oled_cmd(0xB0); // 0xB0 set page addressing mode

	oled_cmd(0xC8); // 0xC0 set COM output scan direction 0x C( to flip side) 0xC0 / 0xC8

	oled_cmd(0xD3); // 0xD3 set Display offset vertical shift
	oled_cmd(0x00); // 0x00 set to 0

	oled_cmd(0xD5); // 0xD5 enter display clock divide 
	oled_cmd(0xF0); // 0xF0 set to maximum frequency

	oled_cmd(0xD9); // 0xD9 enter set pre charge period
	oled_cmd(0xF1); // 0xF1 longer pre-charge - 0x22 for more default, and more power friendly

	oled_cmd(0xDA); // 0xDa enter set com pins hardware configuration
	oled_cmd(0b00010010); // 0b00100010 works and 0b00010010 works and 0b00000010 works

	oled_cmd(0xDB); // 0xDB set VCOMH deselect level
	oled_cmd(0x20); // 0x20 0.77 * Vcc (standard)

	oled_cmd(0x8D); // 0x8D charge pump settings
	oled_cmd(0x14); // 0x14 7.5V charge pump

	oled_cmd(0xAF); // 0xAF display ON

	oled_clear();

}


void oled_draw_bitmap(volatile uint8_t * bitmap, uint8_t x, uint8_t page_start, uint8_t width, uint8_t page_height) {
	uint8_t  page_end = page_start + page_height - 1, i = 0, j = 0;

	if (page_end > 7) {
		page_start = 7 - page_height;
		page_end = 7;
	}
	
	for (uint8_t page = page_start; page <= page_end; page++) {
		oled_move_NoStop(x, page); 
		TWI_start();
		TWI_SLA(OLED_ADDR, 0);
		TWI_TData(0x40);

		for (i = 0; i <  width; i++) {

			TWI_TData(bitmap[i + j * width]);
		}
		j++;
	}
	cursor_set(x + width, page_end);
	TWI_stop();
	
}


void oled_clear_area(uint8_t x, uint8_t page_start, uint8_t width, uint8_t page_height) {
	uint8_t  page_end = page_start + page_height - 1;

	if (x >= 128 || page_start >= 8) return;

	if (x + width > 128) {
		width = 128 - x;
	}

	if (page_end > 7) {
		page_start = 7 - page_height;
		page_end = 7;
	}
	for (uint8_t page = page_start; page <= page_end; page++) {
		oled_move_NoStop(x, page); 
		TWI_start();
		TWI_SLA(OLED_ADDR, 0);
		TWI_TData(0x40);
		for (uint8_t col = 0; col < width; col++) {
			TWI_TData(0x00);
		}
		TWI_stop();
	}

	
}

void oled_draw_loading(uint8_t x, uint8_t y, uint8_t state, uint8_t max_state) {
	


	uint8_t bitmap[10] = {0};
	memset(bitmap, 0xFF,10);

	uint8_t mx = x + max_state * 15;

	for (uint8_t i = 0; i < state; i++) {
		oled_draw_bitmap(bitmap,x+i*15, y,10,1);
		oled_draw_bitmap(bitmap,mx-i*15, y+1,10,1);
	}
}

#endif





