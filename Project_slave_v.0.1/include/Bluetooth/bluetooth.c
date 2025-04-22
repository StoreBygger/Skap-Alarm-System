#ifndef BLUETOOTH_C
#define BLUETOOTH_C

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define MAX_LEN_ANSWER 25
#define BAUD_UBRR 51

#define BT_MSG_REC_COUNT 7810 // 1ms wait time

#define SIZE_RECIEVE_BUFFER 25

uint8_t recieve_buffer[SIZE_RECIEVE_BUFFER] = {};
uint8_t len_recieve_buffer;

uint8_t bluetooth_state = 0; // 0 = disconnected, 1 = connected




#endif