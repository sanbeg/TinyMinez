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

#if !defined(__AVR_ATtiny85__)
  #include "SerialHexTools.h"
#endif
#include "spritebank.h"
#include "smallFont.h"
#include "tinyJoypadUtils.h"
#include "textUtils.h"
#include "soundFX.h"
#include "RLEdecompression.h"
#include "TinyMinezGame.h"

const unsigned char PROGMEM txtAllMinesFound[] = "CONGRATS\0\0ALL\0\0\0\0MINES\0\0FOUND!\0\0";

// the game object containing all logic and data
Game game;

// it's difficult to spot the cursor, so let it flash (frame time is ~50ms)
const uint8_t cursorMaxFlashCount = 24;
// flash if count is greater or equal the threshold
const uint8_t cursorFlashThreshold = cursorMaxFlashCount / 2;
// increase this counter on every display;
uint8_t cursorFlashCount = 0;

/*--------------------------------------------------------*/
void setup()
{
  // initialize the pins (and serial port if present)
  InitTinyJoypad();
  // perform display initialization
  InitDisplay();
}

/*--------------------------------------------------------*/
void loop()
{
  game.setStatus( Status::intro );

  // game main loop
  while ( true )
  {
    // always incement seed
    game.incrementSeed();

    switch ( game.getStatus() )
    {
      /////////////////////////////
      // intro screen
      case Status::intro:
      {
        // display intro screen
        Tiny_Flip( false );

        // check if button pressed
        if ( isFirePressed() )
        {
          game.setStatus( Status::prepareGame );

          // increment seed while waiting - this way we get a good enough random seed
          while ( isFirePressed() ) { game.incrementSeed(); }
        }

        break;
      }

      /////////////////////////////
      // prepare new game
      case Status::prepareGame:
      {
        // create a new level depending on the difficulty (TODO)
        game.createLevel( 10 );
        // dump the level to serial
        game.serialPrintLevel();
        // start the game
        game.setStatus( Status::playGame );
        break;
      }

      /////////////////////////////
      // play the game
      case Status::playGame:
      {
        bool playerAction = false;

        while ( ( game.getStatus() != Status::gameWon ) && ( game.getStatus() != Status::boom ) )
        {
          // increase random seed
          game.incrementSeed();

          // get current cursor position
          uint8_t cursorX = game.getCursorX();
          uint8_t cursorY = game.getCursorY();

          // any buttons pressed?
          if ( isLeftPressed() && ( cursorX > 0 ) )
          {
            cursorX--;
            // wait a moment
            playerAction = true;
          }
          if ( isRightPressed() && ( cursorX < game.getLevelWidth() - 1 ) )
          {
            cursorX++;
            // wait a moment
            playerAction = true;
          }
          if ( isUpPressed() && ( cursorY > 0 ) )
          {
            cursorY--;
            // wait a moment
            playerAction = true;
          }
          if ( isDownPressed() && ( cursorY < game.getLevelHeight() - 1 ) )
          {
            cursorY++;
            // wait a moment
            playerAction = true;
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
              if ( count < 255 ) { count++; 
              }
            // wait unit the button is released
            } while ( isFirePressed() );

            // was fire pressed longer than 3 rounds(~400ms)?
            if ( count > 3 )
            {
              // set or remove a flag symbol
              game.toggleFlag( cursorX, cursorY );
            }
            else
            {
              // uncover this cell and all adjacent cells (if this cell is empty)
              if ( game.uncoverCells( cursorX, cursorY ) )
              {
                // something bad did happen...
                game.setStatus( Status::boom );
              }
              else
              {
                // are all non mine fields uncovered?
                if ( game.isWon() ) 
                { 
                  // game won!
                  game.setStatus( Status::gameWon );
                }
              }
            }
            // wait a moment
            playerAction = true;
          }

          // update cursor flash count
          cursorFlashCount++;
          if ( cursorFlashCount >= cursorMaxFlashCount ) { cursorFlashCount = 0; }

          // set cursor to the new position
          game.setCursorPosition( cursorX, cursorY );

          // draw board
          Tiny_Flip( false );

          // only delay if there were any changes (important for getting a good seed)
          if ( playerAction )
          {
            // reset cursor to visible
            cursorFlashCount = 0;
            // no forced update required
            playerAction = false;
            // wait a moment
            _delay_ms( keyDelay );
        
            // dump the level to serial
            game.serialPrintLevel();
          }
        }
        break;
      }

      /////////////////////////////
      // show game over screen (aka BOOM screen)
      case Status::boom:
      {
        // display ***BOOM*** screen and flash 
        for ( uint8_t flash = 0; flash < 10; flash++ ) { Tiny_Flip( flash == 0 ); _delay_ms( 100 ); }

        // uncover all cells
        //game.uncoverAllCells();
        game.setStatus( Status::gameOver );

        while ( !isFirePressed() )
        { 
          // show the board with all tiles uncoverted
          Tiny_Flip( true );
          // update cursor flash count
          cursorFlashCount++;
          if ( cursorFlashCount >= cursorMaxFlashCount ) { cursorFlashCount = 0; }
        }

        // wait until fire is released
        while ( isFirePressed() );

        // return to the title screen
        game.setStatus( Status::intro );

        break;
      }

      /////////////////////////////
      // all mines found, congratulations!
      case Status::gameWon:
      {
        // display game won screen
        Tiny_Flip( false );
        // play a tune
        // ...
        // wait for button
        waitForFireButtonPressedAndReleased();

        // switch to intro screen
        game.setStatus( Status::intro );
      }

    } // switch

  } // while ( true )
}

/*--------------------------------------------------------*/
void Tiny_Flip( bool invert )
{
  Status gameStatus = game.getStatus();

  // prepare text buffer for statistics (only displayed during the game)
  clearTextBuffer();
  uint8_t *textBuffer = getTextBuffer();

  if ( gameStatus == Status::gameWon )
  {
    Serial.println( F("GameStatus = gameWon") );
    // Congratulate player...
    memcpy_P( textBuffer, txtAllMinesFound, 32 );
  }
  else
  {
    // prepare statistics when game is running
    convertValueToDigits( game.getFlaggedTilesCount(), textBuffer + 1 + 1 * 4 );
    convertValueToDigits( game.getHiddenTilesCount(), textBuffer + 1 + 4 * 4 );
    convertValueToDigits( game.getClicksCount(), textBuffer + 1 + 7 * 4 );
  }

  // optional bitmap buffer pointer
  uint8_t *compressedBitmap;

  // only invert cursor if flash count is less than threshold
  uint8_t cursor = ( cursorFlashCount < cursorFlashThreshold ) ? 0xff : 0x00;

  // there are 8 rows of 8 pixels each
  for ( uint8_t y = 0; y < 8; y++)
  {
    TinyFlip_PrepareDisplayRow( y );

    switch ( gameStatus )
    {
      case Status::intro:
      case Status::prepareGame:
      case Status::boom:
      {
        // select bitmap
        if ( y == 0 )
        {
          compressedBitmap = TitleScreen;
          if ( gameStatus == Status::boom ) { compressedBitmap = BOOM; }
        }

        // display the full line
        compressedBitmap = displayBitmapRow( y, compressedBitmap, invert );
        break;
      }

      case Status::playGame:
      case Status::gameOver:
      {
        // invert image?
        uint8_t invertValue = invert ? 0xff : 0x00;

        // the first 96 columns are used to display the dungeon
        for ( uint8_t x = 0; x < 96; x++ )
        {
          uint8_t spriteColumn = x & 0x07;
          uint8_t cellValue = game.getCellValue( x >> 3, y );

          uint8_t pixels = getSpriteData( cellValue, spriteColumn );
          // invert the tile with the cursor above it
          if ( cellValue & 0x80 ) { pixels ^= cursor; }
          // invert anyway?
          pixels ^= invertValue;

          TinyFlip_SendPixels( pixels );
        } // for x

        // display the dashboard here
        for ( uint8_t x = 0; x < 32; x++)
        {
          uint8_t pixels = pgm_read_byte( dashBoard + x + y * 32 )
                         | displayText( x, y );
          TinyFlip_SendPixels( pixels );
        }
        break;
      }

      case Status::gameWon:
      {
        // display the zoomed text buffer
        for ( uint8_t x = 0; x < 128; x++)
        {
          uint8_t pixels = displayZoomedText( x, y );
          TinyFlip_SendPixels( pixels );
        }
        break;
      }

      default:
      {
        Serial.println( F("*** default ***") );
        break;
      }

    } // switch
    
    TinyFlip_FinishDisplayRow();
  } // for y

  // display the whole screen at once
  TinyFlip_DisplayBuffer();

#if !defined(__AVR_ATtiny85__)
  #ifdef _ENABLE_SERIAL_SCREENSHOT_
    if ( _SERIAL_SCREENSHOT_TRIGGER_CONDITION_ )
    {
      // print a short header
      Serial.println(F("\r\nTinyMinez screenshot"));
      // output the full buffer as a hexdump to the serial port
      printScreenBufferToSerial( display.getBuffer(), 128, 8 );
    }
  #endif
#endif
}

/*--------------------------------------------------------*/
// Displays the row of the given bitmap and inverts it if required
// The bitmap is expected to be RLE encoded. The function returns
// the pointer of the next compressed chunk of image data.
uint8_t* displayBitmapRow( const uint8_t y, const uint8_t *bitmap, const bool invert )
{
  uint8_t xorValue = ( invert ? 0xff : 0x00 );

  // we will repurpose the text buffer to save valuable RAM
  uint8_t *chunkBuffer = getTextBuffer();

  // uncompress chunk and save next address
  uint8_t *render = pgm_RLEdecompress( bitmap, chunkBuffer, 128 );

  // display the full line
  for ( uint8_t x = 0; x < 128; x++ )
  {
    uint8_t pixels = ( *chunkBuffer++ ) ^ xorValue;
    TinyFlip_SendPixels( pixels );
  } // for x

  // return the current decompression pointer
  return( render );
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

/*--------------------------------------------------------*/
void waitForFireButtonPressedAndReleased()
{
  // wait until fire is pressed
  while ( !isFirePressed() ) { game.incrementSeed(); }
  // wait until fire is released
  while ( isFirePressed() ) { game.incrementSeed(); }
}