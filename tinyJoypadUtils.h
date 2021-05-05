#pragma once

#include <Arduino.h>

#if defined(__AVR_ATtiny85__)
  #define LEFT_RIGHT_BUTTON A0
  #define UP_DOWN_BUTTON    A3
  #define FIRE_BUTTON        1
#else
  #define LEFT_RIGHT_BUTTON A0
  #define UP_DOWN_BUTTON    A3
  #define FIRE_BUTTON       A1
#endif

bool isLeftPressed();
bool isRightPressed();
bool isUpPressed();
bool isDownPressed();
bool isFirePressed();
void _variableDelay_us( uint8_t delayValue );
void Sound( const uint8_t freq, const uint8_t dur );
void TinyFlip_PrepareDisplayRow( uint8_t y );
void TinyFlip_FinishDisplayRow();

// macro to simplify display handling between ATtiny85 and Ardafruit_SSD1306

#if defined(__AVR_ATtiny85__) /* codepath for ATtiny85 */

  // no buffer required (and available!)
  #define TinyFlip_PrepareBuffer( y )
  // send a byte directly to the SSD1306
  #define TinyFlip_SendPixels( pixels ) SSD1306.ssd1306_send_byte( pixels )
  // display buffer (not necessary)
  #define TinyFlip_DisplayBuffer

#else  /* codepath for any Adafruit_SSD1306 supported MCU */

  // address the display buffer
  #define TinyFlip_PrepareBuffer( y )   uint8_t *buffer = display.getBuffer() + ( y * 128 )
  // write pixels directly to the buffer
  #define TinyFlip_SendPixels( pixels ) *buffer++ = pixels
  // display buffer (not necessary)
  #define TinyFlip_DisplayBuffer display.display()

#endif

