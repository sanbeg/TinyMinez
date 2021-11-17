
#include <stdint.h>

#include "TinyI2CMaster.h"

#ifdef CORE_WIRE
//#include <Wire.h>
#endif

namespace wirerap {

  class Tiny {
  private:
    static TinyI2CMaster i2c;

  public:
    static void init() {
      i2c.init();
    }

    static bool start(uint8_t addr, int count=0) {
      return i2c.start(addr, count);
    }

    static void stop() {
      i2c.stop();
    }

    static void write(uint8_t data) {
      i2c.write(data);
    }

  };

  #ifdef CORE_WIRE

class Core {
private:
  static uint8_t address;
  static uint8_t cnt;
  static uint8_t reg;
public:
  static void init() {
    Wire.begin();
  }
  static bool start(uint8_t addr) {
    address = addr;
    cnt = 1;
    reg = 0;
    Wire.beginTransmission(addr);
    return true;
  }
  static void stop() {
    //address = 0;
    //cnt = 0;
    //reg = 0;
    Wire.endTransmission();
  }
  // this probably buffers, needs to occationally flush
  static void write(uint8_t data) {
    if (cnt == 1) {
      reg = data;
    }
    if (++cnt >= 16) {
      Wire.endTransmission();
      cnt = 2;
      Wire.beginTransmission(address);
      Wire.write(reg);
    }
    Wire.write(data);
  }
};
#endif



// bitbang

// Defines for OLED output

class BitBang {
private:
  static void xfer_start(void);
  static void xfer_stop(void);

public:
  static void init();
  static void send_byte(uint8_t byte);
  static void write(uint8_t data);
  static bool start(uint8_t addr);
  static void stop();
};

class WireWrap : public BitBang {};


}
