#ifndef ATTINY85_JOYCON_JOYCON_H
#define ATTINY85_JOYCON_JOYCON_H

#include <stdint.h>

void joycon_init(void);

void joycon_update(void);

void joycon_delay(long milli);

void joycon_setX(uint8_t value);

void joycon_setY(uint8_t value);

void joycon_setXROT(uint8_t value);

void joycon_setYROT(uint8_t value);

void joycon_setZROT(uint8_t value);

void joycon_setSLIDER(uint8_t value);

void joycon_setButtons(uint8_t low, uint8_t high);

void joycon_setValues(uint8_t values[]);

#endif //ATTINY85_JOYCON_JOYCON_H
