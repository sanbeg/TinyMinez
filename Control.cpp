#include "Control.h"

//#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#ifndef PORTA
# error No button layout is configured for your device
#endif

namespace control {

static volatile uint8_t pins = 0xff;

ISR(PCINT0_vect) {
  control::pins = PINA;
}


void setup() {
  /* we could compute this mask from all of the buttons, but we're listening on all of
    port A, except the i2c pins.  The enum really just allows us to swap button pins
    amongst each other.
  */
  const uint8_t mask = 0b10101111;

  DDRA &= ~mask;
  PORTA |= mask;
  GIMSK |= 1 << PCIE0;
  PCMSK0 |= mask;
}

bool isPressed(int button) {
  return (pins & (uint8_t)(1 << button)) == 0;
}

bool consumePress(int button) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    bool rv = (pins & (1 << button)) == 0;
    pins |= (1 << button);
    return rv;
  }
}

const char * debug(char * btn_text) {
  if (pins & (1 << BTN_L)) btn_text[0] = ' '; else btn_text[0] = 'L';
  if (pins & (1 << BTN_U)) btn_text[1] = ' '; else btn_text[1] = 'U';
  if (pins & (1 << BTN_R)) btn_text[2] = ' '; else btn_text[2] = 'R';
  if (pins & (1 << BTN_A)) btn_text[3] = ' '; else btn_text[3] = 'A';
  if (pins & (1 << BTN_B)) btn_text[4] = ' '; else btn_text[4] = 'B';
  if (pins & (1 << BTN_D)) btn_text[5] = ' '; else btn_text[5] = 'D';
  btn_text[6] = 0;
  return btn_text;
}
}
