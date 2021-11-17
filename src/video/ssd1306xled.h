/*
 * SSD1306xLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x64 displays
 *
 * @created: 2014-08-12
 * @author: Neven Boyanov
 *
 * Source code available at: https://bitbucket.org/tinusaur/ssd1306xled
 *
 * Modified by Tejashwi Kalp Taru, with the help of TinyI2C (https://github.com/technoblogy/tiny-i2c/)
 * Modified code available at: https://github.com/tejashwikalptaru/ssd1306xled
 *
 * Modified by Steve Sanbeg
 *
 */
#include <stdint.h>
#include <Arduino.h>
// #include <avr/pgmspace.h>
// #include <avr/interrupt.h>
#include <util/delay.h>


#ifndef SSD1306XLED_H
#define SSD1306XLED_H

// ----------------------------------------------------------------------------

// -----(+)--------------->	// Vcc,	Pin 1 on SSD1306 Board
// -----(-)--------------->	// GND,	Pin 2 on SSD1306 Board

#ifndef SSD1306_SA
#  define SSD1306_SA		0X3C	// Slave address
#endif



// ----------------------------------------------------------------------------

class SSD1306Device //: private TinyI2CMaster
{
    public:
		SSD1306Device(void);
		void ssd1306_init(void);

		void ssd1306_send_byte(uint8_t byte);

		void ssd1306_send_command(uint8_t command);
		void ssd1306_send_data_start(void);
		void ssd1306_send_data_stop(void);

		void ssd1306_setpos(uint8_t x, uint8_t y);
		void ssd1306_fillscreen(uint8_t fill);
		#if 0
		void ssd1306_char_font6x8(char ch);
		void ssd1306_string_font6x8(char *s);
		void ssd1306_char_f8x16(uint8_t x, uint8_t y, const char ch[]);
		#endif
		void ssd1306_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);

	private:
		void begin();

    void ssd1306_send_command_start(void);
    void ssd1306_send_command_stop(void);
};

extern SSD1306Device SSD1306;

// ----------------------------------------------------------------------------

#endif
