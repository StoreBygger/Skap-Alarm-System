#ifndef LOCKER_C
#define LOCKER_C
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define MAX_LOCKERS 64
#define MAX_DISC 6
#define LOCKER_SIZE 8
#define LOCKER_EEPROM_BASE 512

#define LOCKER_TIMEOUT 2500
#define LOCKER_WAIT 1

#define LCN_LEN_RECIEVE 40
#define LCN_LEN_NAME 20

typedef struct locker {
	uint8_t locker_id; // 0 -> 63 locker_id -> 64 is outside EEPROM range
	uint8_t alarm_state;
	uint8_t locker_active;
	

	struct locker * previous;
	struct locker * next;

}locker;


typedef enum current_running_prog {
	pgm_none,
	pgm_menu,
	pgm_popup,
	pgm_eeprom_print,
	pgm_LCN,
	pgm_sleep
} current_running_prog;

typedef struct LCN_locker {

	uint8_t LCN_id; // 1 - 6 ID
	char * addr; // address
	char * name; // name

} LCN_locker;

uint8_t current_check_locker_id = 255;

LCN_locker LCN_locker_list[MAX_DISC];
uint8_t LCN_last_id = 0;

uint8_t alarm_sound = 1;



#endif