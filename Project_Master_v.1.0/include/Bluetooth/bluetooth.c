#ifndef BLUETOOTH_C
#define BLUETOOTH_C

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define MAX_LEN_ANSWER 25
#define BAUD_UBRR 51


#define BTPORT PORTB
#define BTDDR DDRB
#define BTPIN PB0

#define SIZE_RECIEVE_BUFFER 25

#define BLUETOOTH_CONNECTED_UNIDENTIFIED 255
#define BLUETOOTH_DISCONNECTED 0

uint8_t recieve_buffer[SIZE_RECIEVE_BUFFER] = {};
uint8_t len_recieve_buffer;

uint8_t bluetooth_state = BLUETOOTH_DISCONNECTED; // 0 = disconnected, 255 = connected, unidentified




#endif