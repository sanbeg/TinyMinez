#include <Arduino.h>
#include "TinyMinezGame.h"

#if !defined(__AVR_ATtiny85__)
  #include "SerialHexTools.h"
#endif

/*--------------------------------------------------------*/
Game::Game() : Game( MAX_GAME_COLS, MAX_GAME_ROWS )
{
  // Intro screen
  status = Status::intro;
}

/*--------------------------------------------------------*/
Game::Game( uint8_t levelWidth, uint8_t levelHeight ) : minesCount( 0 ), flagsCount( 0 ), clicksCount( 0 ),
                                                        levelWidth( levelWidth ), levelHeight( levelHeight ),
                                                        cursorX( levelWidth / 2 ), cursorY( levelHeight / 2 ),
                                                        seed( 0 )
{
  // Intro screen
  status = Status::intro;
  // clear level - just in case
  clearLevel();
}

/*--------------------------------------------------------*/
// Creates a level with 'numOfMines' randomly placed mines.
void Game::createLevel( uint8_t numOfMines )
{
  // clear the level
  clearLevel();

  // store number of mines
  minesCount = numOfMines;
  // no flags
  flagsCount = 0;
  // no clicks
  clicksCount = 0;

  // set random seed to the live value
  randomSeed( seed );

  // now place the mines
  while ( numOfMines-- )
  {
    uint8_t pos;

    // find a free mine position
    do
    {
      // get random position
      pos = random( levelWidth * levelHeight );

    } while ( levelData[pos] != empty );

    // place the mine
    levelData[pos] = bomb;
  }

  // now count all bombs in the neightbourhood
  for ( int8_t y = 0; y < levelHeight; y++ )
  {
    for ( int8_t x = 0; x < levelWidth; x++ )
    {
      if ( getCellValue( x, y ) != bomb )
      {
        // count all bombs around x,y
        uint8_t neighbours = countNeighbours( x, y );

        // bombs found?
        if ( neighbours > 0 )
        {
          // store the neighbour count in the lower 4 bits of the cell
          levelData[x + y * levelWidth] |= neighbours;
        }
      }
    } // for x
  } // for y

  // hide all cells
  for ( uint8_t n = 0; n < levelWidth * levelHeight; n++ )
  {
    levelData[n] |= hidden;
  }

  // place cursor in the middle of the level
  cursorX = levelWidth / 2;
  cursorY = levelHeight / 2;
}

/*--------------------------------------------------------*/
// uncovers all tiles adjacent to x,y
bool Game::uncoverCells( const int8_t x, const int8_t y, bool countClick /*= true*/ )
{
  uint8_t value = getCellValue( x, y );

  // any work to do?
  if ( !( value & hidden ) )
  {
    // no work here...
    return( false );
  }

  // should this "click" be counted?
  if ( countClick ) { clicksCount++; }

  // uncover this tile (and remove any flags positioned on this tile)
  setCellValue( x, y, value & ~( hidden | flag ) );

  // is it a bomb?
  if ( value & bomb )
  {
    Serial.println( F("***BOOOOOM***") );
    // GAME OVER...
    return( true );
  }
  else
  {
    // is this tile empty?
    uint8_t value = getCellValue( x, y ) & dataMask;
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
          if ( ( value & hidden ) && !( value & bomb ) )
          {
            // play it again, Sam!
            uncoverCells( x + offsetX, y + offsetY, false );
          }
        }
      }
    }

    // still in the game
    return( false );
  }
}

/*--------------------------------------------------------*/
// uncover all tiles after ***BOOM***
void Game::uncoverAllCells()
{
  for ( uint8_t y = 0; y < levelHeight; y++ )
  {
    for ( uint8_t x = 0; x < levelWidth; x++ )
    {
      setCellValue( x, y, getCellValue( x, y ) & ~hidden );
    }
  }

}
/*--------------------------------------------------------*/
// The game is won, if all fields except the mines are uncovered,
// thus the number of covered fields is less or equal to the number of mines.
// (Less, because there might be a "continue" play mode)
bool Game::isWon()
{
  return( countCellsWithAttribute( hidden ) <= minesCount );
}

/*--------------------------------------------------------*/
// toggle flag - but only on covered tiles
void Game::toggleFlag( const int8_t x, const int8_t y )
{
  uint8_t cellValue = getCellValue( x, y );
  if ( cellValue & hidden )
  {
    setCellValue( x, y, cellValue ^ flag );
  }
}

/*--------------------------------------------------------*/
// We can safely count the 3x3 neighbourhood, because the center
// position is not a bomb - otherwise we would already be dead ;)
uint8_t Game::countNeighbours( const int8_t x, const int8_t y )
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
void Game::setCellValue( const int8_t x, const int8_t y, const uint8_t value )
{
  if (    ( x >= 0 ) && ( x < levelWidth )
       && ( y >= 0 ) && ( y < levelHeight )
     )
  {
    levelData[x + y * levelWidth] = value;
  }
}


/*--------------------------------------------------------*/
// Access function to handle border management
uint8_t Game::getCellValue( const int8_t x, const int8_t y )
{
  uint8_t cellValue = empty;

  if (    ( x >= 0 ) && ( x < levelWidth )
       && ( y >= 0 ) && ( y < levelHeight )
     )
  {
    cellValue = levelData[x + y * levelWidth];
  }

  return( cellValue );
}

/*--------------------------------------------------------*/
void Game::setCursorPosition( const uint8_t x, const uint8_t y )
{
  // store new positions
  cursorX = x;
  cursorY = y;

  // remove old cursor (wherever it was)
  for ( uint8_t n = 0; n < levelWidth * levelHeight; n++ )
  {
    levelData[n] &= dataMask;
  }

  // set new cursor
  levelData[x + y * levelWidth] |= cursor;
}

/*--------------------------------------------------------*/
  // clear level - just in case
void Game::clearLevel()
{
  minesCount = 0;
  flagsCount = 0;
  clicksCount = 0;
  // nothing is to be found here
  memset( levelData, empty, MAX_GAME_COLS * MAX_GAME_ROWS );
}

/*--------------------------------------------------------*/
// dump the level to the serial port
void Game::serialPrintLevel()
{
#if !defined(__AVR_ATtiny85__)
  Serial.print( F("markedMines = ") ); Serial.print( getFlaggedTilesCount() );
  Serial.print( F(", hiddenTiles = ") ); Serial.print( getHiddenTilesCount() );
  Serial.print( F(", clicksCount = ") ); Serial.print( getClicksCount() );
  Serial.print( F(", seed = ") ); Serial.println( getSeed() );
  
  for ( uint8_t y = 0; y < levelHeight; y++ )
  {
    hexdumpResetPositionCount();
    hexdumpToSerial( levelData + y + levelWidth, levelWidth, false, true );
  }
  Serial.println();
#endif
}

/*--------------------------------------------------------*/
// prints the current game status to serial output
void Game::serialPrintGameStatus()
{
  switch( status )
  {
    case Status::intro:
      Serial.println( F("Status: intro") );
      break;
    case Status::difficultySelection:
      Serial.println( F("Status: difficultySelection") );
      break;
    case Status::prepareGame:
      Serial.println( F("Status: prepareGame") );
      break;
    case Status::playGame:
      Serial.println( F("Status: playGame") );
      break;
    case Status::boom:
      Serial.println( F("Status: boom") );
      break;
    case Status::gameOver:
      Serial.println( F("Status: gameOver") );
      break;
    case Status::gameWon:
      Serial.println( F("Status: gameWon") );
      break;
    default:
      Serial.println( F("Statis: <unknown status>") );
      break;
  }
}

/*--------------------------------------------------------*/
uint8_t Game::countCellsWithAttribute( uint8_t mask )
{
  uint8_t count = 0;

  for ( uint8_t n = 0; n < levelWidth * levelHeight; n++ )
  {
    // does this cell meet the condition?
    if ( ( levelData[n] & mask ) != 0 )
    {
      count++;
    }
  }

  return( count );
}
