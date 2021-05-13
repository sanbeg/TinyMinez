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

class Game
{
public:
  Game();
  Game( uint8_t levelWidth, uint8_t levelHeight );
  
  void      createLevel( uint8_t mineCount );
  bool      uncoverCells( const int8_t x, const int8_t y );
  uint8_t   getCursorX() { return( cursorX ); }
  uint8_t   getCursorY() { return( cursorY ); }
  uint8_t   getLevelWidth() { return( levelWidth ); }
  uint8_t   getLevelHeight() { return( levelHeight ); }
  void      setCursorPosition( const uint8_t x, const uint8_t y );
  uint8_t   getCellValue( const int8_t x, const int8_t y );
  void      serialPrintLevel();

private:
  uint8_t   countNeighbours( const int8_t x, const int8_t y );
  void      clearLevel();

// Attributes

  // number of bombs
  uint8_t   mineCount;
  // number of clicks
  uint16_t  clickCount;
  // level size
  uint8_t   levelWidth;
  uint8_t   levelHeight;
  // cursor position
  uint8_t   cursorX;
  uint8_t   cursorY;
  // level data
  uint8_t   levelData[MAX_GAME_COLS * MAX_GAME_ROWS];
};
