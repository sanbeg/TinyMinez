//
// This file works for TinyJoypad compatible devices.
//
// If not compiled for ATTiny85 (meaning __AVR_ATtiny85__ is not defined),
// generic functions are used instead of direct port access, which 
// makes it possible to use an Arduino or Mega2560 (or many others)
// for debugging with serial output or even hardware breakpoints.
//

#include <Arduino.h>
#include "tinyJoypadUtils.h"


#if defined(ARDUINO_AVR_ATTINYX4)
#  include "Control.h"
#  define SND_MASK 1 << PB0
#elif defined(__AVR_ATtiny85__)
#  define SND_MASK 1 << PB4
#else
  // include serial output functions
  #include "SerialHexTools.h"

  // include Adafruit library and immediately create an object
  #include <Adafruit_SSD1306.h>

  #define SND_MASK 1 << PB4

  Adafruit_SSD1306 display( 128, 64, &Wire, -1 );
  uint8_t *adafruitBuffer;
#endif


/*-------------------------------------------------------*/
// function for initializing the TinyJoypad (ATtiny85) and other microcontrollers
void InitTinyJoypad()
{
#if defined(__AVR_ATtiny85__)
  // not using 'pinMode()' here saves ~100 bytes of flash!
  // configure A0, A3 and D1 as input
  DDRB &= ~( ( 1 << PB5) | ( 1 << PB3 ) | ( 1 << PB1 ) );
  // configure A2 as output
  DDRB |= SND_MASK;
#elif defined(ARDUINO_AVR_ATTINYX4)
  control::setup();
  DDRB |= SND_MASK;
#else
  // use 'pinMode()' for simplicity's sake... any other micro controller has enough flash :)
  pinMode( LEFT_RIGHT_BUTTON, INPUT );
  pinMode( UP_DOWN_BUTTON, INPUT );
  pinMode( FIRE_BUTTON, INPUT );
  // configure PB4 as output (Pin D12 on Arduino UNO R3 and Pin D10 on Arduino Mega 2560 )
  DDRB |= SND_MASK;

  // prepare serial port for debugging output
  Serial.begin( 115200 );
#endif
}

#ifdef LEFT_RIGHT_BUTTON
/*-------------------------------------------------------*/
bool isLeftPressed()
{
  uint16_t inputX = analogRead( LEFT_RIGHT_BUTTON );
  return( ( inputX >= 750 ) && ( inputX < 950 ) );
}

/*-------------------------------------------------------*/
bool isRightPressed()
{
  uint16_t inputX = analogRead( LEFT_RIGHT_BUTTON );
  return( ( inputX > 500 ) && ( inputX < 750 ) );
}

/*-------------------------------------------------------*/
bool isUpPressed()
{
  uint16_t inputY = analogRead( UP_DOWN_BUTTON );
  return( ( inputY > 500 ) && ( inputY < 750 ) );
}

/*-------------------------------------------------------*/
bool isDownPressed()
{
  uint16_t inputY = analogRead( UP_DOWN_BUTTON );
  return( ( inputY >= 750 ) && ( inputY < 950 ) );
}


/*-------------------------------------------------------*/
bool isFirePressed()
{
  return( digitalRead( FIRE_BUTTON ) == 0 );
}

#else
bool isLeftPressed()
{
  return control::isPressed(control::BTN_L);
}
bool isRightPressed()
{
    return control::isPressed(control::BTN_R);
}

bool isUpPressed()
{
    return control::isPressed(control::BTN_U);
}

bool isDownPressed()
{
    return control::isPressed(control::BTN_D);
}

bool isFirePressed()
{
    return control::isPressed(control::BTN_A);
}

bool isFlagPressed()
{
    return control::isPressed(control::BTN_B);
}


#endif

/*-------------------------------------------------------*/
// wait until all buttons are released
void waitUntilButtonsReleased()
{
  while( isLeftPressed() || isRightPressed() || isUpPressed() || isDownPressed() || isFirePressed() );
}

/*-------------------------------------------------------*/
// wait until all buttons are released and wait a little delay
void waitUntilButtonsReleased( const uint8_t delay )
{
  waitUntilButtonsReleased();
  _delay_ms( delay );
}

/*-------------------------------------------------------*/
void _variableDelay_us( uint8_t delayValue )
{
  while ( delayValue-- != 0 )
  {
    _delay_us( 1 );
  }
}

/*-------------------------------------------------------*/
// This code was originaly borrowed from Daniel C's Tiny-invaders :)
// Code optimization by sbr
void Sound( const uint8_t freq, const uint8_t dur )
{
  for ( uint8_t t = 0; t < dur; t++ )
  {
    if ( freq!=0 ){ PORTB = PORTB|SND_MASK; }
    _variableDelay_us( 255 - freq );
    PORTB = PORTB&(~SND_MASK);
    _variableDelay_us( 255 - freq );
  }
}

/*-------------------------------------------------------*/
void InitDisplay()
{
#if defined(__AVR_ATtiny85__) || defined(ARDUINO_AVR_ATTINYX4) /* codepath for ATtiny85 */
  SSD1306.ssd1306_init();
#else
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // Address 0x3D for 128x64
  if( !display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { 
    Serial.println(F("SSD1306 allocation failed")); for(;;);
  }
#endif
}

/*-------------------------------------------------------*/
// This code will init the display for row <y>
void TinyFlip_PrepareDisplayRow( uint8_t y )
{
#if defined(__AVR_ATtiny85__)||defined(ARDUINO_AVR_ATTINYX4)  /* codepath for ATtiny85 */
    // initialize image transfer to segment 'y'
    SSD1306.ssd1306_send_command(0xb0 + y);
  #ifdef _USE_SH1106_
    // SH1106 internally uses 132 pixels/line,
    // output is (always?) centered, so we need to start at position 2
    SSD1306.ssd1306_send_command(0x02);
    SSD1306.ssd1306_send_command(0x10);  
  #else
    // classic SSD1306 supports only 128 pixels/line, so we start at 0
    SSD1306.ssd1306_send_command(0x00);
    SSD1306.ssd1306_send_command(0x10);  
  #endif    
    SSD1306.ssd1306_send_data_start();

#else  /* codepath for any Adafruit_SSD1306 supported MCU */

  // address the display buffer
  adafruitBuffer = display.getBuffer() + ( y * 128 );
#endif
}

/*-------------------------------------------------------*/
void TinyFlip_SendPixels( uint8_t pixels )
{
#if defined(__AVR_ATtiny85__)||defined(ARDUINO_AVR_ATTINYX4) /* codepath for ATtiny85 */
  // send a byte directly to the SSD1306
  SSD1306.ssd1306_send_byte( pixels );

#else  /* codepath for any Adafruit_SSD1306 supported MCU */
  // write pixels directly to the buffer
  *adafruitBuffer++ = pixels;
#endif
}

/*-------------------------------------------------------*/
// This code will finish a row (only on Tiny85)
void TinyFlip_FinishDisplayRow()
{
#if defined(__AVR_ATtiny85__)||defined(ARDUINO_AVR_ATTINYX4)
  // this line appears to be optional, as it was never called during the intro screen...
  // but hey, we still have some bytes left ;)
  SSD1306.ssd1306_send_data_stop();
#endif
}

/*-------------------------------------------------------*/
void TinyFlip_DisplayBuffer()
{
#if !(defined(__AVR_ATtiny85__)||defined(ARDUINO_AVR_ATTINYX4)) /* codepath for any Adafruit_SSD1306 supported MCU */
  // display buffer (not necessary)
  display.display();
#endif
}

/*-------------------------------------------------------*/
// Outputs the screen as one hex byte per pixel. To get an actual image perform the following steps:
// (1) The output can be converted to binary with 'https://tomeko.net/online_tools/hex_to_file.php?lang=en' online.
// (2) Then import the file with IrfanView (https://www.irfanview.com/): Open as -> RAW file...
// (3) Set Image width to 64 and Image height to 128, 8 BPP -> OK
// (4) Rotate and mirror the result as needed :)
void TinyFlip_SerialScreenshot()
{
#if !(defined(__AVR_ATtiny85__)||defined(ARDUINO_AVR_ATTINYX4)) /* codepath for any Adafruit_SSD1306 supported MCU */
  // print a short header
  Serial.println(F("\r\nTinyMinez screenshot"));
  // output the full buffer as a hexdump to the serial port
  printScreenBufferToSerial( display.getBuffer(), 128, 8 );
#endif
}
