#include <Arduino.h>
#include <avr/pgmspace.h>
#include "textUtils.h"
#include "bitTables.h"
#include "spritebank.h"

#undef TEXT_UTILS_SUPPRESS_LEADING_ZEROES

/*--------------------------------------------------------------*/
// Converts 'value' to 2 decimal digits (0..99)
void convertValueToDigits( uint8_t value, uint8_t *digits )
{
  static uint8_t dividerList[] = { 10, 1, 0 };

  uint8_t *divider = dividerList;

#ifdef TEXT_UTILS_SUPPRESS_LEADING_ZEROES
  // wait for the first divider which is smaller than the value
  while ( *divider > value ) { divider++; }
  // if we reach zero, the divider pointer has to be reset to '1'
  if ( value == 0 ) { divider = &dividerList[1]; }
#endif

  do
  {
    uint8_t digit = '0';
    while( value >= *divider )
    {
      digit++;
      value -= *divider;
    }

    // store digit
    *digits++ = digit;
    // next divider
    divider++;
  }
  while ( *divider != 0 );
}

/*--------------------------------------------------------------*/
// Displays the digits on the screen. 
// To save flash memory, the font ranges only from '0' to '9'.
uint8_t displayText( uint8_t x, uint8_t y )
{
  // find appropriate character in text array (font width is 8 px),
  // row width is 4 characters
  uint8_t value = textBuffer[( x >> 3 ) + y * 4];
  // is it a valid character?
  if ( value != 0 )
  {
    // get the column value
    return( pgm_read_byte( segementedDigits + ( value - '0' ) * 8 + ( x & 0x07 ) ) );
  }

  return( 0x00 );
}

/*--------------------------------------------------------------*/
void clearTextBuffer()
{
  memset( textBuffer, 0x00, sizeof( textBuffer ) );
}

/*--------------------------------------------------------------*/
void printText( uint8_t x, uint8_t *text, uint8_t textLength )
{
  memcpy( textBuffer + x, text, textLength );
}

/*--------------------------------------------------------------*/
void pgm_printText( uint8_t x, uint8_t *text, uint8_t textLength )
{
  memcpy_P( textBuffer + x, text, textLength );

#if !defined(__AVR_ATtiny85__)
  for ( auto n = 0; n < sizeof( textBuffer ); n++ )
  {
    auto value = textBuffer[n];
    Serial.write( value == 0 ? '_' : value );
  }
  Serial.println();
#endif
}

/*--------------------------------------------------------------*/
uint8_t *getTextBuffer()
{
  return( textBuffer );
}
