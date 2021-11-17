
#include "WireWrap.h"
#include <avr/io.h>

namespace wirerap {

  #ifdef CORE_WIRE
  // hack, should put core into its own cpp
  uint8_t Core::address;
  uint8_t Core::cnt;
  uint8_t Core::reg;
#endif

#define I2C_PORT  PORTA
#define I2C_DDR DDRA
#define SSD1306_SCL   PORTA4 //PORTB4  // SCL, Pin 4 on SSD1306 Board - for webbogles board
#define SSD1306_SDA   PORTA6 //PORTB3  // SDA, Pin 3 on SSD1306 Board - for webbogles board

#define OPEN_DRAIN

#ifdef OPEN_DRAIN
#  define DIGITAL_WRITE_HIGH(PORT) I2C_DDR &= ~(1 << PORT)
#  define DIGITAL_WRITE_LOW(PORT) I2C_DDR |= (1 << PORT)
#else
#  define DIGITAL_WRITE_HIGH(PORT) I2C_PORT |= (1 << PORT)
#  define DIGITAL_WRITE_LOW(PORT) I2C_PORT &= ~(1 << PORT)
#endif


  void BitBang::xfer_start(void){
    DIGITAL_WRITE_HIGH(SSD1306_SCL);  // Set to HIGH
    DIGITAL_WRITE_HIGH(SSD1306_SDA);  // Set to HIGH
    DIGITAL_WRITE_LOW(SSD1306_SDA);   // Set to LOW
    DIGITAL_WRITE_LOW(SSD1306_SCL);   // Set to LOW
  }

  void BitBang::xfer_stop(void){
    DIGITAL_WRITE_LOW(SSD1306_SCL);   // Set to LOW
    DIGITAL_WRITE_LOW(SSD1306_SDA);   // Set to LOW
    DIGITAL_WRITE_HIGH(SSD1306_SCL);  // Set to HIGH
    DIGITAL_WRITE_HIGH(SSD1306_SDA);  // Set to HIGH
  }

  void BitBang::init() {
    //I2C_DDR |= (1 << SSD1306_SDA); // Set port as output
    //I2C_DDR |= (1 << SSD1306_SCL); // Set port as output

    // Set port as output
    I2C_DDR |= (1 << SSD1306_SDA) | (1 << SSD1306_SCL);
    I2C_PORT &= ~((1 << SSD1306_SDA) | (1 << SSD1306_SCL));
  }

  void BitBang::send_byte(uint8_t byte) {
    uint8_t i;
    for(i=0; i<8; i++) {
      if((byte << i) & 0x80)
        DIGITAL_WRITE_HIGH(SSD1306_SDA);
      else
        DIGITAL_WRITE_LOW(SSD1306_SDA);

      DIGITAL_WRITE_HIGH(SSD1306_SCL);
      DIGITAL_WRITE_LOW(SSD1306_SCL);
    }
    DIGITAL_WRITE_HIGH(SSD1306_SDA);
    DIGITAL_WRITE_HIGH(SSD1306_SCL);
    DIGITAL_WRITE_LOW(SSD1306_SCL);
  }

  void BitBang::write(uint8_t data) {
    send_byte(data);
  }

  bool BitBang::start(uint8_t addr) {
    xfer_start();
    send_byte(addr << 1);
    return true;
  }

  void BitBang::stop() {
    xfer_stop();
  }

}
