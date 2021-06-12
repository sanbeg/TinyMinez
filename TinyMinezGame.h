#pragma once

#include <Arduino.h>

const uint8_t MAX_GAME_COLS = 12;
const uint8_t MAX_GAME_ROWS = 8;
const uint8_t keyDelay = 133;

enum
{
  empty     = 0x00,
  countMask = 0x0f,
  bomb      = 0x10,
  flag      = 0x20,
  hidden    = 0x40,
  cursor    = 0x80,
  dataMask  = 0x7f,
};

enum class Status
{
  intro,
  difficultySelection,
  prepareGame,
  playGame,
  boom,
  gameOver,
  gameWon,
};

class Game
{
public:
  Game();
  Game( uint8_t levelWidth, uint8_t levelHeight );
  
  void      createLevel( uint8_t numOfMines );
  bool      uncoverCells( const int8_t x, const int8_t y, bool countClick = true );
  void      uncoverAllCells();
  bool      isWon();
  Status    getStatus() { return( status ); }
  void      setStatus( Status newStatus ) { status = newStatus; }
  void      toggleFlag( const int8_t x, const int8_t y );
  uint8_t   getCursorX() { return( cursorX ); }
  uint8_t   getCursorY() { return( cursorY ); }
  uint8_t   getLevelWidth() { return( levelWidth ); }
  uint8_t   getLevelHeight() { return( levelHeight ); }
  void      setCursorPosition( const uint8_t x, const uint8_t y );
  void      setCellValue( const int8_t x, const int8_t y, const uint8_t value );
  uint8_t   getCellValue( const int8_t x, const int8_t y );
  void      incrementSeed() { seed++; }
  long      getSeed() { return( seed ); }
  void      serialPrintLevel();
  void      serialPrintGameStatus();

  uint8_t   getClicksCount() { return( clicksCount ); }
  uint8_t   getFlaggedTilesCount() { return( countCellsWithAttribute( flag ) ); }
  uint8_t   getHiddenTilesCount() { return( countCellsWithAttribute( hidden ) ); }


private:
  uint8_t   countNeighbours( const int8_t x, const int8_t y );
  void      clearLevel();
  uint8_t   countCellsWithAttribute( uint8_t mask );

// Attributes
private:
  // game status (intro, game, boom)
  Status    status;
  // number of bombs
  uint8_t   minesCount;
  // number of set flags
  uint8_t   flagsCount;
  // number of clicks
  uint16_t  clicksCount;
  // level size
  uint8_t   levelWidth;
  uint8_t   levelHeight;
  // cursor position
  uint8_t   cursorX;
  uint8_t   cursorY;
  // seed value
  long      seed;
  // level data
  uint8_t   levelData[MAX_GAME_COLS * MAX_GAME_ROWS];
};
