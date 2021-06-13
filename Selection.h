#pragma once

#include <Arduino.h>

///////////////////////////////////////////////
// helper "class" for the difficulty selection
class Selection
{
enum
{
  MIN_SELECTION = 0,
  MAX_SELECTION = 3,
};

public:
  Selection( uint8_t *bitmapChecked, uint8_t *bitmapUnchecked, const uint8_t bitmapWidth, const uint8_t bitmapOffsetX, const uint8_t selection );
  uint8_t getOverlayPixels( const uint8_t x, const uint8_t y );
  void setSelection( const uint8_t selection ) { _selection = selection; }
  uint8_t getSelection() { return( _selection ); }
  void nextSelection() { if ( _selection < MAX_SELECTION ) { _selection++; } }
  void previousSelection() { if ( _selection > MIN_SELECTION ) { _selection--; } }

private:
  uint8_t *_bitmapSelected;
  uint8_t *_bitmapUnselected;
  uint8_t _bitmapWidth;
  uint8_t _bitmapOffsetX;
  uint8_t _selection;
};
