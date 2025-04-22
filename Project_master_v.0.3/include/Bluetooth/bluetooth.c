#ifndef BLUETOOTH_C
#define BLUETOOTH_C

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "bluetooth.h"

uint8_t recieve_buffer[SIZE_RECIEVE_BUFFER] = {};
uint8_t len_recieve_buffer;

uint8_t bluetooth_state = BLUETOOTH_DISCONNECTED; // 0 = disconnected, 255 = connected, unidentified




#endif