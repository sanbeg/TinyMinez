#ifndef CONTROL_
#define CONTROL_

#include <stdint.h>

namespace control {

enum ButtonPins {
  /*
  BTN_L = 0,
  BTN_U = 1,
  BTN_R = 2,
  BTN_A = 3,
  BTN_B = 5,
  BTN_D = 7,
  */
  BTN_D = 7,
  BTN_L = 5,
  BTN_U = 3,
  BTN_R = 2,
  BTN_A = 1,
  BTN_B = 0,
};

void setup();
bool isPressed(int);
bool consumePress(int);
const char * debug(char *);
};

#endif
