#include "SelectionOverlay.h"
#include "spritebank.h"

/*--------------------------------------------------------------*/
SelectionOverlay::SelectionOverlay( uint8_t *bitmapChecked, uint8_t *bitmapUnchecked, const uint8_t bitmapWidth, const uint8_t bitmapOffsetX, const uint8_t selection ) :
                                    _bitmapSelected( bitmapChecked ),
                                    _bitmapUnselected( bitmapUnchecked ),
                                    _bitmapWidth( bitmapWidth ),
                                    _bitmapOffsetX( bitmapOffsetX ),
                                    _selection( selection )
{
}

/*--------------------------------------------------------------*/
// Returns the selection overlay for ( x, y ) considering the 
// selected item.
uint8_t SelectionOverlay::getOverlayPixels( const uint8_t x, const uint8_t y )
{
  uint8_t pixels;
  uint8_t *bitmap = ( ( y >> 1 ) == _selection ) ? _bitmapSelected : _bitmapUnselected;

  if ( ( x >= _bitmapOffsetX ) && ( x < _bitmapOffsetX + _bitmapWidth ) )
  {
    if ( ( y & 0x01 ) == 0x00 )
    {
      pixels |= pgm_read_byte( bitmap + x - _bitmapOffsetX );
    }
    else
    {
      pixels |= pgm_read_byte( bitmap + x - _bitmapOffsetX + _bitmapWidth );
    }
  }

  return( pixels );
}

