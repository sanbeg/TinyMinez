/*
   SSD1306xLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x64 displays

   @created: 2014-08-12
   @author: Neven Boyanov

   Source code available at: https://bitbucket.org/tinusaur/ssd1306xled

   Modified by Tejashwi Kalp Taru, with the help of TinyI2C (https://github.com/technoblogy/tiny-i2c/)
   Modified code available at: https://github.com/tejashwikalptaru/ssd1306xled

   Modified by Steve Sanbeg
*/

// ----------------------------------------------------------------------------


#include <stdlib.h>
#include <avr/io.h>

#include <avr/pgmspace.h>

#include "ssd1306xled.h"
#include "WireWrap.h"

#if 0
#include "font6x8.h"
#include "font8x16.h"
#endif

#define SSD1306_COMMAND 0x00
#define SSD1306_DATA 0x40

// ----------------------------------------------------------------------------

// Some code based on "IIC_wtihout_ACK" by http://www.14blog.com/archives/1358

const uint8_t ssd1306_init_sequence [] PROGMEM = {	// Initialization Sequence
  0xAE,			// Set Display ON/OFF - AE=OFF, AF=ON
  0xD5, 0xF0,		// Set display clock divide ratio/oscillator frequency, set divide ratio
  0xA8, 0x3F,		// Set multiplex ratio (1 to 64) ... (height - 1)
  0xD3, 0x00,		// Set display offset. 00 = no offset
  0x40 | 0x00,	// Set start line address, at 0.
  0x8D, 0x14,		// Charge Pump Setting, 14h = Enable Charge Pump
  0x20, 0x00,		// Set Memory Addressing Mode - 00=Horizontal, 01=Vertical, 10=Page, 11=Invalid
  0xA0 | 0x01,	// Set Segment Re-map
  0xC8,			// Set COM Output Scan Direction
  0xDA, 0x12,		// Set COM Pins Hardware Configuration - 128x32:0x02, 128x64:0x12
  0x81, 0x3F,		// Set contrast control register
  0xD9, 0x22,		// Set pre-charge period (0x22 or 0xF1)
  0xDB, 0x20,		// Set Vcomh Deselect Level - 0x00: 0.65 x VCC, 0x20: 0.77 x VCC (RESET), 0x30: 0.83 x VCC
  0xA4,			// Entire Display ON (resume) - output RAM to display
  0xA6,			// Set Normal/Inverse Display mode. A6=Normal; A7=Inverse
  0x2E,			// Deactivate Scroll command
  0xAF,			// Set Display ON/OFF - AE=OFF, AF=ON
};

#if 0 //sequence from attiny arcade; see if this is different when used -w- game.
const uint8_t ata_init_seq []  PROGMEM = {
  0xAE, // display off
  0x00, // Set Memory Addressing Mode
  0x10, // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
  0x40, // Set Page Start Address for Page Addressing Mode,0-7
  0x81, // Set COM Output Scan Direction
  0xCF, // ---set low rowumn address
  0xA1, // ---set high rowumn address
  0xC8, // --set start line address
  0xA6, // --set contrast control register
  0xA8,
  0x3F, // --set segment re-map 0 to 127
  0xD3, // --set normal display
  0x00, // --set multiplex ratio(1 to 64)
  0xD5, //
  0x80, // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
  0xD9, // -set display offset
  0xF1, // -not offset
  0xDA, // --set display clock divide ratio/oscillator frequency
  0x12, // --set divide ratio
  0xDB, // --set pre-charge period
  0x40, //
  0x20, // --set com pins hardware configuration
  0x02,
  0x8D, // --set vcomh
  0x14, // 0x20,0.77xVcc
  0xA4, // --set DC-DC enable
  0xA6, //
  0xAF, // --turn on oled panel
};
#endif

// Program:    5248 bytes
namespace {
  using wirerap::WireWrap;
}

SSD1306Device::SSD1306Device(void) {}

void SSD1306Device::begin() {
  WireWrap::init();
#ifndef TINY4KOLED_QUICK_BEGIN
  while (!WireWrap::start(SSD1306_SA)) {
    delay(10);
  }
  WireWrap::stop();
#endif
}

void SSD1306Device::ssd1306_init(void)
{
  begin();
  ssd1306_send_command_start();
  #if 1
  for (uint8_t i = 0; i < sizeof (ssd1306_init_sequence); i++) {
    ssd1306_send_byte(pgm_read_byte(&ssd1306_init_sequence[i]));
  }
  #else
    for (uint8_t i = 0; i < sizeof (ata_init_seq); i++) {
    ssd1306_send_byte(pgm_read_byte(&ata_init_seq[i]));
  }
  #endif
  ssd1306_send_command_stop();
  ssd1306_fillscreen(0);
}

void SSD1306Device::ssd1306_send_command_start(void) {
  WireWrap::stop();
  WireWrap::start(SSD1306_SA);
  WireWrap::write(SSD1306_COMMAND);
}

void SSD1306Device::ssd1306_send_command_stop() {
  WireWrap::stop();
}

void SSD1306Device::ssd1306_send_command(uint8_t command) {
  ssd1306_send_command_start();
  ssd1306_send_byte(command);
  ssd1306_send_command_stop();
}

void SSD1306Device::ssd1306_send_byte(uint8_t byte) {
  WireWrap::write(byte);
}

void SSD1306Device::ssd1306_send_data_start(void) {
  WireWrap::stop();
  WireWrap::start(SSD1306_SA);
  WireWrap::write(SSD1306_DATA);
}

void SSD1306Device::ssd1306_send_data_stop() {
  WireWrap::stop();
}

void SSD1306Device::ssd1306_fillscreen(uint8_t fill) {
  ssd1306_setpos(0, 0);
  ssd1306_send_data_start();	// Initiate transmission of data
  #if 1
    for (uint16_t i = 0; i < 128 * 8 / 4; i++) {
  	ssd1306_send_byte(fill);
  	ssd1306_send_byte(fill);
  	ssd1306_send_byte(fill);
  	ssd1306_send_byte(fill);
    }
  #else
  for (int i = 0; i < 128 * 8; ++i)      ssd1306_send_byte(fill);
  #endif
  ssd1306_send_data_stop();	// Finish transmission
}

void SSD1306Device::ssd1306_setpos(uint8_t x, uint8_t y)
{
  ssd1306_send_command_start();
  ssd1306_send_byte(0xb0 | (y & 0x07));
  ssd1306_send_byte(0x10 | ((x & 0xf0) >> 4));
  ssd1306_send_byte(x & 0x0f); // | 0x01
  ssd1306_send_command_stop();
}
#if 0
void SSD1306Device::ssd1306_char_font6x8(char ch) {
  uint8_t i;
  uint8_t c = ch - 32;
  ssd1306_send_data_start();
  for (i = 0; i < 6; i++)
  {
    ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font6x8[c * 6 + i]));
  }
  ssd1306_send_data_stop();
}

void SSD1306Device::ssd1306_string_font6x8(char *s) {
  while (*s) {
    ssd1306_char_font6x8(*s++);
  }
}
#endif
void SSD1306Device::ssd1306_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]) {
  uint16_t j = 0;
  uint8_t y, x;
  if (y1 % 8 == 0) y = y1 / 8;
  else y = y1 / 8 + 1;
  for (y = y0; y < y1; y++)
  {
    ssd1306_setpos(x0, y);
    ssd1306_send_data_start();
    for (x = x0; x < x1; x++)
    {
      ssd1306_send_byte(pgm_read_byte(&bitmap[j++]));
    }
    ssd1306_send_data_stop();
  }
}
#if 0
void SSD1306Device::ssd1306_char_f8x16(uint8_t x, uint8_t y, const char ch[])
{
  uint8_t c, j, i = 0;
  while (ch[j] != '\0')
  {
    c = ch[j] - 32;
    if (x > 120)
    {
      x = 0;
      y++;
    }
    ssd1306_setpos(x, y);
    ssd1306_send_data_start();
    for (i = 0; i < 8; i++)
    {
      ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font8x16[c * 16 + i]));
    }
    ssd1306_send_data_stop();
    ssd1306_setpos(x, y + 1);
    ssd1306_send_data_start();
    for (i = 0; i < 8; i++)
    {
      ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font8x16[c * 16 + i + 8]));
    }
    ssd1306_send_data_stop();
    x += 8;
    j++;
  }
}

#endif

SSD1306Device SSD1306;

// ----------------------------------------------------------------------------
