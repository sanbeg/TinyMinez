//   >>>>>  T-I-N-Y  M-I-N-E-Z v0.1 for ATTINY85  GPLv3 <<<<
//						Tinyjoypad rev2 compatible
//                   Programmer: Sven B 2021
//              Contact EMAIL: 

// The code works at 16MHZ internal
// and uses ssd1306xled Library for SSD1306 oled display 128x64.
//
// To stuff all code and data into the 8192 bytes of the ATtiny85
// the ATTinyCore (v1.5.2) by Spence Konde is recommended.
// The core is available at github: [https://github.com/SpenceKonde/ATTinyCore], just add the
// following board manager to the Arduino IDE: [http://drazzy.com/package_drazzy.com_index.json]
// Please enable LTO (link time optimization) and disable 'millis()' and
// 'micros()'.

// show an 8x8 grid overlay
//#define _SHOW_GRID_OVERLAY
// enable serial screenshot
//#define _ENABLE_SERIAL_SCREENSHOT_
// perform a serial screenshot if this condition is true:
//#define _SERIAL_SCREENSHOT_TRIGGER_CONDITION_ ( isDownPressed() )

#if defined(__AVR_ATtiny85__)
  #include <ssd1306xled.h>
#else
  #include "SerialHexTools.h"
  #include <Adafruit_SSD1306.h>
  Adafruit_SSD1306 display( 128, 64, &Wire, -1 );
#endif
#include "spritebank.h"
#include "smallFont.h"
#include "tinyJoypadUtils.h"
#include "textUtils.h"
#include "soundFX.h"
#include "TinyMinezGame.h"

Game game;

/*--------------------------------------------------------*/
void setup()
{
  // initialize the pins (and serial port if present)
  InitTinyJoypad();

  InitDisplay;
}

/*--------------------------------------------------------*/
void loop()
{
  game.createLevel( 10 );

  game.serialPrintLevel();

  uint16_t seed;

  bool updateBoard = true;

  while ( !game.isWon() )
  {
    uint8_t cursorX = game.getCursorX();
    uint8_t cursorY = game.getCursorY();
    
    // increase random seed
    seed++;
    
    // buttons pressed?
    if ( isLeftPressed() && ( cursorX > 0 ) )
    {
      cursorX--;
      updateBoard = true;
    }
    if ( isRightPressed() && ( cursorX < game.getLevelWidth() - 1 ) )
    {
      cursorX++;
      updateBoard = true;
    }
    if ( isUpPressed() && ( cursorY > 0 ) )
    {
      cursorY--;
      updateBoard = true;
    }
    if ( isDownPressed() && ( cursorY < game.getLevelHeight() - 1 ) )
    {
      cursorY++;
      updateBoard = true;
    }
    if ( isFirePressed() )
    {
      uint8_t count = 0;
      // let's check how long fire is pressed
      do
      {
        // wait a moment
        _delay_ms( keyDelay );
        // count this!
        if ( count < 255 )
        {
          count++;
        }
      // wait unit the button is released
      } while ( isFirePressed() );

      // was fire pressed longer than 400ms?
      if ( count > 3 )
      {
        game.toggleFlag( cursorX, cursorY );
      }
      else
      {
        game.uncoverCells( cursorX, cursorY );
      }
      // board requires drawing
      updateBoard = true;
    }

    // only draw board if there are any changes (important for getting a good seed)
    if ( updateBoard )
    {
      // set cursor to the new position
      game.setCursorPosition( cursorX, cursorY );
      // draw board
      Tiny_Flip();
      // no forced update required
      updateBoard = false;
      // wait a moment
      _delay_ms( keyDelay );
    }
  }

  Serial.println( F("Hooray!") );
}

/*--------------------------------------------------------*/
void Tiny_Flip()
{
  uint8_t statusPaneOffset = 0; 

  for ( uint8_t y = 0; y < 8; y++)
  {
    TinyFlip_PrepareDisplayRow( y );

    // allocate a buffer in RAM (if necessary)
    TinyFlip_PrepareBuffer( y );
    
    // the first 96 columns are used to display the dungeon
    for ( uint8_t x = 0; x < 96; x++ )
    {
      uint8_t spriteColumn = x & 0x07;
      uint8_t cellValue = game.getCellValue( x >> 3, y );

      uint8_t pixels = getSpriteData( cellValue, spriteColumn );
      // invert the tile with the cursor above it
      if ( cellValue & 0x80 ) { pixels ^= 0xff; }

      TinyFlip_SendPixels( pixels );
    } // for x

    // display the dashboard here
    for ( uint8_t x = 0; x < 32; x++)
    {
      uint8_t pixels = x;
      TinyFlip_SendPixels( pixels );
      statusPaneOffset++;
    }
    
    TinyFlip_FinishDisplayRow();
  } // for y

  // display the whole screen at once
  TinyFlip_DisplayBuffer;

#if !defined(__AVR_ATtiny85__)
  #ifdef _ENABLE_SERIAL_SCREENSHOT_
    if ( _SERIAL_SCREENSHOT_TRIGGER_CONDITION_ )
    {
      // print a short header
      Serial.println(F("\r\nTinyDungeon screenshot"));
      // output the full buffer as a hexdump to the serial port
      printScreenBufferToSerial( display.getBuffer(), 128, 8 );
    }
  #endif
#endif
}

/*--------------------------------------------------------*/
uint8_t getSpriteData( uint8_t cellValue, uint8_t spriteColumn )
{
  // remove cursor
  cellValue &= dataMask;

  if ( cellValue & flag )
  {
    // a flag was planted here!
    return( pgm_read_byte( flag8x8 + spriteColumn ) );
  }
  if ( cellValue & hidden )
  {
    // this cellValue is still covered
    return( pgm_read_byte( tile8x8 + spriteColumn ) );
  }
  if ( cellValue == bomb )
  {
    // a bomb!
    return( pgm_read_byte( bomb8x8 + spriteColumn ) );
  }
  if ( ( cellValue >=1 ) && ( cellValue <= 9 ) )
  {
    // draw according digit
    return( pgm_read_byte( digits + cellValue * 8 + spriteColumn ) );
  }

  // obviously empty ;)
  return( pgm_read_byte( empty8x8 + spriteColumn ) );
}
