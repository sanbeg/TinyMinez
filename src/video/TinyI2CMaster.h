/* Tiny I2C

   David Johnson-Davies - www.technoblogy.com - 28th May 2019

   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license:
   http://creativecommons.org/licenses/by/4.0/

   Updated by Steve Sanbeg
*/

#ifndef TinyI2CMaster_h
#define TinyI2CMaster_h

#include <stdint.h>


class TinyI2CMaster {

  public:
    TinyI2CMaster();
    void init();
    uint8_t read();
    uint8_t readLast();
    bool write(uint8_t data);
    bool start(uint8_t address, int readcount);
    bool restart(uint8_t address, int readcount);
    void stop();

  private:
    int I2Ccount;
    uint8_t transfer(uint8_t data);
};

#endif
