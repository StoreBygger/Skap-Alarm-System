// master/manu.c

#ifndef MENU_C
#define MENU_C
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h> 

#define MAX_MENU_ITEMS 9 // maximum of pre-programmed menu items
#define MAX_MENU_TEXT_SIZE 30 // max size of menu text
#define MAX_MENUS 7 // max number of menus
#define MAX_MENU_RENDER_ITEMS 6


#define MAX_MENU_STATE 5


typedef struct menu_item  {

	// object id's
	uint8_t type; // type of menu object - to display object correctly
	uint8_t text_id;
	uint8_t item_id; // the ID to identify this menu object - the next menu is defined by item_id
	uint8_t parent_id; // the id of the menu leading to this object
	uint8_t state; // extra data to go along with menu object

	//next and previous struct
	struct menu_item * next;
	struct menu_item * previous;

	// methods
	void (*menu_item_render)(struct menu_item * self, uint8_t x, uint8_t y, uint8_t selected);
	void (*menu_item_pressed)(struct menu_item * self);

}menu_item;

enum menu_item_types {
	header_item,
	plain_text_item,
	menu_pointer_item,
	locker_item,
	function_item,
	back_item,
	locker_header_item
};


// text strings for menu item text
const char menu_text_main_menu[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM = {
	"MAIN MENU",
	"Alarms",
	"Connected Devices",
	"Options",
	"Credits"

};
const char menu_text_alarms[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM = {
	"Alarms",
	"Back",
	"Reset alarms",
	"Get alarm status"
};
const char menu_text_connections[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM = {
	"Connected Devices",
	"Back",
	"Connect new device",
	"Delete all devices"
};
const char menu_text_options[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM = {
	"Options",
	"Back",
	"Debug",
	"Power off"
};
const char menu_text_credits[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM = {
	"Credits",
	"Back",
	"Henrik A Toemt",
	"Morten K Tverberg"
};
const char menu_text_debug[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM = {
	"Debug",
	"Back",
	"CHECK AT",
	"GET ADDR",
	"Connect",
	"Disconnect",
	"init",
	"Print eeprom",
	"Debug locker"
};
// state arrays for menu items
const uint8_t menu_state_main_menu[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM = {
	{ // main menu -> main menu
		0, //	type:	 	0,header 
		0, // 	text_id:	0
		1, // 	item_id:	1
		1, // 	parent_id:	1
		5  // 	state:		5
	},
	{ // main menu -> alarms
		2, // 	type: 		2, new menu
		1, // 	text id: 	1
		2, // 	item id: 	2
		1, // 	parent id: 	1
		2 // 	state: 		2

	},
	{ // main menu -> connected devices
		2, // 	type: 		2, new menu
		2, // 	text id: 	2
		3, // 	item id: 	3
		1, // 	parent id: 	1
		3 // 	state: 		3
	},
	{	// main menu -> options
		2, // 	type: 		2, new menu
		3, // 	text_id: 	3
		4, // 	item_id: 	4
		1, // 	parent_id: 	1
		4, // 	state 		4
	},
	{ // main menu -> credits
		2, // 	type: 		2, new menu
		4, // 	text_id:	4
		5, //	item_id:	5
		1, // 	parent_id:	1
		5  //	state:		5
	}
};
const uint8_t menu_state_alarms[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM = {
	{ // alarm menu -> alarms
		0, //	type:		0, header
		0, // 	text_id:	0
		9, //	item_id:	9
		2, // 	parent_id:	2
		4  // 	state: 		4
	},
	{ // alarm menu -> reset alarms
		4, //	type:		4, func
		2, // 	text_id:	2
		11, //	item_id:	11
		2, // 	parent_id:	2
		0  // 	state: 		0
	},
	{ // alarm menu -> get alarm status
		4, //	type:		4, func
		3, // 	text_id:	3
		12, //	item_id:	12
		2, // 	parent_id:	2
		0  // 	state: 		0
	},
	{ // alarm menu -> back
		5, //	type:		5, back
		1, // 	text_id:	1
		10, //	item_id:	10
		2, // 	parent_id:	2
		0  // 	state: 		0
	}
};
const uint8_t menu_state_connections[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM = {
	{ // menu connections -> connected devices
		0, //	type:		0, header
		0, // 	text_id:	0
		17, //	item_id:	17
		3, // 	parent_id:	3
		4  // 	state: 		4
	},
	{ // menu connections -> connect new device
		4, //	type:		4, func
		2, // 	text_id:	2
		19, //	item_id:	19
		3, // 	parent_id:	3
		0  // 	state: 		0
	},
	{ // menu connections -> delete all devices
		4, //	type:		4, func
		3, // 	text_id:	3
		20, //	item_id:	20
		3, // 	parent_id:	3
		0  // 	state: 		0
	},
	{ // menu connections -> back
		5, //	type:		5, back
		1, // 	text_id:	1
		18, //	item_id:	18
		3, // 	parent_id:	3
		0  // 	state: 		0
	}
};
const uint8_t menu_state_options[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM = {
	{ // menu options -> options
		0, //	type:		0, header
		0, // 	text_id:	0
		25, //	item_id:	25
		4, // 	parent_id:	4
		4  // 	state: 		4
	},
	{ // menu options -> debug
		2, //	type:		2, menu_ptr
		2, // 	text_id:	2
		27, //	item_id:	27
		4, // 	parent_id:	4
		6  // 	state: 		0
	},
	{ // menu options -> power off
		4, //	type:		4, func
		3, // 	text_id:	3
		28, //	item_id:	28
		4, // 	parent_id:	4
		0  // 	state: 		0
	},
	{ // menu options -> back
		5, //	type:		5, back
		1, // 	text_id:	1
		26, //	item_id:	26
		4, // 	parent_id:	4
		0  // 	state: 		0
	},
};
const uint8_t menu_state_credits[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM = {
	{ // menu credits -> credits
		0, //	type:		0, header
		0, // 	text_id:	0
		33, //	item_id:	33
		5, // 	parent_id:	5
		4  // 	state: 		4
	},
	{ // menu credits -> Henrik A Toemt
		1, //	type:		1, plain text
		2, // 	text_id:	2
		35, //	item_id:	35
		5, // 	parent_id:	5
		0  // 	state: 		0
	},
	{ // menu credits -> Morten K Tverberg
		1, //	type:		1, plain text
		3, // 	text_id:	3
		36, //	item_id:	36
		5, // 	parent_id:	5
		0  // 	state: 		0
	},
	{ // menu credits -> back
		5, //	type:		5, back
		1, // 	text_id:	1
		34, //	item_id:	34
		5, // 	parent_id:	5
		0  // 	state: 		0
	}
};
const uint8_t menu_state_debug[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM = {
	{ // menu debug -debug
		0, //	type:		0, header
		0, // 	text_id:	0
		41, //	item_id:	33
		6, // 	parent_id:	5
		8  // 	state: 		8
	},
	{ // menu debug -> check at
		4, //	type:		5, func
		2, // 	text_id:	1
		43, //	item_id:	34
		6, // 	parent_id:	5
		0  // 	state: 		0
	},
	{ // menu debug -> check addr
		4, //	type:		5, func
		3, // 	text_id:	1
		44, //	item_id:	34
		6, // 	parent_id:	5
		0  // 	state: 		0
	},
	{ // menu debug -> disconnect
		4, //	type:		5, func
		5, // 	text_id:	1
		45, //	item_id:	34
		6, // 	parent_id:	5
		0  // 	state: 		0
	},
	{ // menu debug -> init
		4, //	type:		5, func
		6, // 	text_id:	1
		46, //	item_id:	34
		6, // 	parent_id:	5
		0  // 	state: 		0
	},
	{ // menu debug -> read eeprom
		4, // type func
		7, // text id
		47, // item id
		6, // parent id
		0 // state

	},
	{ // menu debug -> debug status
		4,
		8,
		48,
		6,
		0

	},
	{ // menu debug-> back
		5, //	type:		5, back
		1, // 	text_id:	1
		42, //	item_id:	34
		6, // 	parent_id:	5
		0  // 	state: 		0
	}
};
// state arrays for menu headers
const uint8_t menu_state_all_headers[MAX_MENUS][MAX_MENU_STATE] PROGMEM = {
	{ // main menu -> main menu
		0, //	type:	 	0,header 
		0, // 	text_id:	0
		1, // 	item_id:	1
		1, // 	parent_id:	1
		6 // 	state:		5
	},
	{ // alarm menu -> alarms
		0, //	type:		0, header
		0, // 	text_id:	0
		9, //	item_id:	9
		2, // 	parent_id:	2
		4  // 	state: 		4
	},
	{ // menu connections -> connected devices
		0, //	type:		0, header
		0, // 	text_id:	0
		17, //	item_id:	17
		3, // 	parent_id:	3
		4  // 	state: 		4
	},
	{ // menu options -> options
		0, //	type:		0, header
		0, // 	text_id:	0
		25, //	item_id:	25
		4, // 	parent_id:	4
		4  // 	state: 		4
	},
	{ // menu credits -> credits
		0, //	type:		0, header
		0, // 	text_id:	0
		33, //	item_id:	33
		5, // 	parent_id:	5
		4  // 	state: 		4
	},
	{ // menu debug -debug
		0, //	type:		0, header
		0, // 	text_id:	0
		41, //	item_id:	33
		6, // 	parent_id:	5
		4  // 	state: 		4
	},
	{ // menu locker -> locker x
		6, //	type:		6, locker header
		0, // text id
		129, // end locker id's
		64, // locker id ++
		6, // state: 6

	}
};


// state array for locker 
const char menu_text_locker[MAX_MENU_ITEMS][MAX_MENU_TEXT_SIZE] PROGMEM = {
	"Locker %i",
	"Reset alarm",
	"Check alarm",
	"Get address",
	"Delete locker",
	"back"
};
const uint8_t menu_state_locker[MAX_MENU_ITEMS][MAX_MENU_STATE] PROGMEM = {
	{ // menu locker top
		6, //	type:		6, locker header
		0, // text id
		129, // end locker id's
		64, // locker id ++
		6, // state: 5

	},
	{ // menu locker -> reset alarm
		4, 	//	type:		4, func
		1, 	//	text_id:	1
		130,//	item_id:	130
		64,	//	parent_id:	64 + locker_id
		0	//	state:		0	
	},
	{ // menu locker -> check alarm
		4, 	//	type:		4, func
		2, 	//	text_id:	2
		131,//	item_id:	131
		64,	//	parent_id:	64 + locker_id
		0	//	state:		0	
	},
	{ // menu locker -> get address
		4, 	//	type:		4, func
		3, 	//	text_id:	3
		132,//	item_id:	132
		64,	//	parent_id:	64 + locker_id
		0	//	state:		0	
	},
	{ // menu locker -> delete locker
		4, 	//	type:		4, func
		4, 	//	text_id:	4
		133,//	item_id:	133
		64,	//	parent_id:	64 + locker_id
		0	//	state:		0	
	},
	{ // menu locker -> back
		5, 	//	type:		5, back
		5, 	//	text_id:	5
		134,//	item_id:	134
		64,	//	parent_id:	64 + locker_id
		0	//	state:		0	
	}

};

menu_item current_menu_header_item = {0,0,1,1,5};
menu_item * current_menu_header = &current_menu_header_item;
menu_item * current_selected_item = &current_menu_header_item;

uint8_t current_menu_id = 1;
uint8_t current_menu_render_y = 0;

// cursor positions
uint8_t current_cursor_x = 0;
uint8_t current_cursor_y = 0;

uint8_t max_cursor_x = 0;
uint8_t max_cursor_y = 1;

// menu scroll
uint8_t scroll_page = 0;
uint8_t max_scroll_page = 0;







#endif