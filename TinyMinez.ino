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
#define _SERIAL_SCREENSHOT_TRIGGER_CONDITION_ ( isDownPressed() )

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

const uint8_t GAME_ROWS = 8;
const uint8_t GAME_COLS = 12;

uint8_t level[GAME_ROWS * GAME_COLS];

enum
{
  empty     = 0x00,
  countMask = 0x0f,
  bomb      = 0x10,
  hidden    = 0x40,
  cursor    = 0x80,
  dataMask  = 0x7f,

  keyDelay  = 133,
};

uint8_t cursorX = 0;
uint8_t cursorY = 0;


/*--------------------------------------------------------*/
void setup()
{
#if defined(__AVR_ATtiny85__)
  SSD1306.ssd1306_init();
  // not using 'pinMode()' here saves ~100 bytes of flash!
  // configure A0, A3 and D1 as input
  DDRB &= ~( ( 1 << PB5) | ( 1 << PB3 ) | ( 1 << PB1 ) );
  // configure A2 as output
  DDRB |= ( 1 << PB4 );
#else
  // DEBUG version on controller with serial ports
  Serial.begin( 115200 );
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // use 'pinMode()' for simplicity's sake... any other micro controller has enough flash :)
  pinMode( LEFT_RIGHT_BUTTON, INPUT );
  pinMode( UP_DOWN_BUTTON, INPUT );
  pinMode( FIRE_BUTTON, INPUT );
#endif
}

/*--------------------------------------------------------*/
void loop()
{
  createLevel( level, GAME_COLS, GAME_ROWS, 10 );
  serialPrintLevel( level, GAME_COLS, GAME_ROWS );

  uint16_t seed;

  bool updateBoard = true;

  while ( 1 )
  {
    // increase random seed
    seed++;
    
    // buttons pressed?
    if ( isLeftPressed() && ( cursorX > 0 ) )
    {
      cursorX--;
      updateBoard = true;
    }
    if ( isRightPressed() && ( cursorX < GAME_COLS - 1 ) )
    {
      cursorX++;
      updateBoard = true;
    }
    if ( isUpPressed() && ( cursorY > 0 ) )
    {
      cursorY--;
      updateBoard = true;
    }
    if ( isDownPressed() && ( cursorY < GAME_ROWS - 1 ) )
    {
      cursorY++;
      updateBoard = true;
    }
    if ( isFirePressed() )
    {
      uncoverCells( cursorX, cursorY );
      // wait unit the button is released
      //while( isFirePressed() ) { seed++; }
      // board requires drawing
      updateBoard = true;
    }

    // only draw board if there are any changes (important for getting a good seed)
    if ( updateBoard )
    {
      // set cursor to the new position
      setCursorPosition( cursorX, cursorY );
      // draw board
      Tiny_Flip();
      // no forced update required
      updateBoard = false;
      // wait a moment
      _delay_ms( keyDelay );
    }
  }
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
      uint8_t *cell = &level[( x >> 3 ) + y * GAME_COLS];

      uint8_t pixels = getSpriteData( *cell, spriteColumn );
      // invert the tile with the cursor above it
      if ( *cell & 0x80 ) { pixels ^= 0xff; }

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
// Creates a level with 'mineCount' randomly placed mines.
void createLevel( uint8_t *level, uint8_t cols, uint8_t rows, uint8_t mineCount )
{
  uint8_t pos;

  // clear the level
  memset( level, empty, cols * rows );

  // now place the mines
  for ( int n = 0; n < mineCount; n++ )
  {
    // find a free mine position
    do
    {
      // get random position
      pos = random( cols * rows );

    } while ( level[pos] != empty );

    // place the mine
    level[pos] = bomb;
  }

  // now count all bombs in the neightbourhood
  for ( int8_t y = 0; y < GAME_ROWS; y++ )
  {
    for ( int8_t x = 0; x < GAME_COLS; x++ )
    {
      if ( getCellValue( x, y ) != bomb )
      {
        // count all bombs around x,y
        uint8_t neighbours = countNeighbours( x, y );

        // bombs found?
        if ( neighbours > 0 )
        {
          // store the neighbour count in the lower 4 bits of the cell
          level[x + y * GAME_COLS] |= neighbours;
        }
      }
    } // for x
  } // for y

  // hide all cells
  for ( uint8_t n = 0; n < cols * rows; n++ )
  {
    level[n] |= hidden;
  }
}

/*--------------------------------------------------------*/
// dump the level to the serial port
void serialPrintLevel( uint8_t *level, uint8_t cols, uint8_t rows )
{
#if !defined(__AVR_ATtiny85__)
 for ( uint8_t y = 0; y < rows; y++ )
  {
    hexdumpResetPositionCount();
    hexdumpToSerial( level + y * cols, cols, false, true );
  }
#endif
}

/*--------------------------------------------------------*/
uint8_t getSpriteData( uint8_t cellValue, uint8_t spriteColumn )
{
  // remove cursor
  cellValue &= dataMask;

  if ( cellValue & hidden )
  {
    // this cell is still covered
    return( pgm_read_byte( tile8x8 + spriteColumn ) );
  }
  if ( cellValue == bomb )
  {
    // a bomb!
    return( pgm_read_byte( bomb8x8 + spriteColumn ) );
  }
  else if ( ( cellValue >=1 ) && ( cellValue <= 9 ) )
  {
    // draw according digit
    return( pgm_read_byte( digits + cellValue * 8 + spriteColumn ) );
  }

  // obviously empty ;)
  return( pgm_read_byte( empty8x8 + spriteColumn ) );
}

/*--------------------------------------------------------*/
// uncovers all tiles adjacent to x,y
bool uncoverCells( const int8_t x, const int8_t y )
{
  // is it a bomb?
  if ( getCellValue( x, y ) == bomb )
  {
    // GAME OVER...
    return( true );
  }
  else
  {
    // uncover this tile
    level[x + y * GAME_COLS] &= ~hidden;
    // is this tile empty?
    uint8_t value = level[x + y * GAME_COLS] & dataMask;
    if ( value == empty )
    {
      // uncover surrounding area (if )
      for ( int8_t offsetY = -1; offsetY <= 1; offsetY++ )
      {
        for ( int8_t offsetX = -1; offsetX <= 1; offsetX++ )
        {
          // let's have a look
          value = getCellValue( x + offsetX, y + offsetY ) & dataMask;
          // covered, but no bomb there?
          if (    ( value & hidden )
              && !( value & bomb )
            )
          {
            // 
            uncoverCells( x + offsetX, y + offsetY );
          }
        }
      }
    }

    //uncoverArea( x, y );
    // still in the game
    return( false );
  }
}

/*--------------------------------------------------------*/
// We can safely count the 3x3 neighbourhood, because the center
// position is not a bomb - otherwise we would already be dead ;)
uint8_t countNeighbours( const int8_t x, const int8_t y )
{
  uint8_t neighbours = 0;

  for ( int8_t offsetY = -1; offsetY <= 1; offsetY++ )
  {
    for ( int8_t offsetX = -1; offsetX <= 1; offsetX++ )
    {
      if ( getCellValue( x + offsetX, y + offsetY ) & bomb )
      {
        neighbours++;
      }
    }
  }

  return( neighbours );
}

/*--------------------------------------------------------*/
// Access function to handle border management
uint8_t getCellValue( const int8_t x, const int8_t y )
{
  uint8_t cellValue = empty;

  if (    ( x >= 0 ) && ( x < GAME_COLS )
       && ( y >= 0 ) && ( y < GAME_ROWS )
     )
  {
    cellValue = level[x + y * GAME_COLS];
  }

  return( cellValue );
}

/*--------------------------------------------------------*/
void setCursorPosition( const uint8_t &x, const uint8_t &y )
{
  // remove old cursor (wherever it was)
  for ( uint8_t n = 0; n < GAME_COLS * GAME_ROWS; n++ )
  {
    level[n] &= dataMask;
  }

  // set new cursor
  level[x + y * GAME_COLS] |= cursor;
}