#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include "wiring.h"
#include "joycon.h"

void init_timer1() {
    PLLCSR = 0;
    PLLCSR |= 1 << LSM;
    TCCR1 |= 7;
    TIMSK |= 1 << TOIE1;
    sei();
}

void setup() {
    init_timer1();
    pinMode(1, OUTPUT);
    joycon_init();
}

int main(void) {
    setup();
    //uint8_t value;
    while (1) {
        //value = millis() / 100;
        //joycon_setX(value);
        //joycon_setY(value);
        joycon_delay(50);
        PORTB ^= 1 << PB1;
    }

    return 0;
};