
#ifndef _BMP085_H_
#define _BMP085_H_

#include <stdint.h>

class BMP085 {

public:
   enum MODE {ULTRALOWPOWER = 0, STANDARD = 1, HIGHRES = 2, ULTRAHIGHRES = 3};

   BMP085(uint8_t addr = 0x77, MODE mode = ULTRAHIGHRES);
   virtual ~BMP085();

   float readTemperature();
   int32_t readPressure(void);
   // Average sea-level pressure is 101.325 kPa (1013.25 mbar, or hPa)
   float readAltitude(float sealevelPressure = 101325);

private:
   int _i2cfile;
   uint8_t _i2caddr;

   int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
   uint16_t ac4, ac5, ac6;

   MODE oversampling;

   uint16_t readRawTemperature();
   uint32_t readRawPressure();

   void delay() {
      //nanosleep((struct timespec){0, 4500000}, NULL); // sleep 4.5ms
      struct timespec ts;
      ts.tv_sec = 0;
      ts.tv_nsec = 4500000;
      nanosleep(&ts, NULL);
   };

   void delay_ms(unsigned ms) {
      //nanosleep((struct timespec){0, 4500000}, NULL); // sleep 4.5ms
      struct timespec ts;
      ts.tv_sec = 0;
      ts.tv_nsec = ms * 1000000;
      nanosleep(&ts, NULL);
   };

   uint8_t read8(uint8_t);
   uint16_t read16(uint8_t);
   void write8(uint8_t, uint8_t);
   void write16(uint8_t, uint16_t);
};

#endif
