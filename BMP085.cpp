
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <math.h>             // pow()
#include <time.h>             // nanosleep()
#if (DEBUG == 1)
  #include <iostream>
#endif

#include "log.h"
#include "BMP085.h"


const uint8_t BMP085_ULTRALOWPOWER   = 0;
const uint8_t BMP085_STANDARD        = 1;
const uint8_t BMP085_HIGHRES         = 2;
const uint8_t BMP085_ULTRAHIGHRES    = 3;

const uint8_t BMP085_CAL_AC1         = 0xAA;  // Calibration data (16 bits)
const uint8_t BMP085_CAL_AC2         = 0xAC;  //
const uint8_t BMP085_CAL_AC3         = 0xAE;  //
const uint8_t BMP085_CAL_AC4         = 0xB0;  //
const uint8_t BMP085_CAL_AC5         = 0xB2;  //
const uint8_t BMP085_CAL_AC6         = 0xB4;  //
const uint8_t BMP085_CAL_B1          = 0xB6;  //
const uint8_t BMP085_CAL_B2          = 0xB8;  //
const uint8_t BMP085_CAL_MB          = 0xBA;  //
const uint8_t BMP085_CAL_MC          = 0xBC;  //
const uint8_t BMP085_CAL_MD          = 0xBE;  //

const uint8_t BMP085_CONTROL         = 0xF4;
const uint8_t BMP085_TEMPDATA        = 0xF6;
const uint8_t BMP085_PRESSUREDATA    = 0xF6;
const uint8_t BMP085_READTEMPCMD     = 0x2E;
const uint8_t BMP085_READPRESSURECMD = 0x34;


BMP085::BMP085(uint8_t addr, BMP085::MODE mode)
   : _i2caddr(addr),
     oversampling(mode)
{
   //FILELog::ReportingLevel() = FILELog::FromString("DEBUG");

   // Todo: make device selectable; 0 or 1
   FILE_LOG(logDEBUG) << "Opening I2C device at: 0x" << std::hex << (int)_i2caddr;

   _i2cfile = open("/dev/i2c-0", O_RDWR);
   if (_i2cfile < 0) {
      // Todo: throw exception
      perror("i2cOpen");
      exit(1);
   }

   // set the I2C slave address for all subsequent I2C device transfers
   if (ioctl(_i2cfile, I2C_SLAVE, _i2caddr) < 0) {
      // Todo: throw exception
      perror("i2cSetAddress");
      exit(1);
   }

   // read calibration data
   ac1 = read16(BMP085_CAL_AC1);
   ac2 = read16(BMP085_CAL_AC2);
   ac3 = read16(BMP085_CAL_AC3);
   ac4 = read16(BMP085_CAL_AC4);
   ac5 = read16(BMP085_CAL_AC5);
   ac6 = read16(BMP085_CAL_AC6);

   b1 = read16(BMP085_CAL_B1);
   b2 = read16(BMP085_CAL_B2);

   mb = read16(BMP085_CAL_MB);
   mc = read16(BMP085_CAL_MC);
   md = read16(BMP085_CAL_MD);

   FILE_LOG(logDEBUG1) << "ac1 = " << std::dec << ac1;
   FILE_LOG(logDEBUG1) << "ac2 = " << std::dec << ac2;
   FILE_LOG(logDEBUG1) << "ac3 = " << std::dec << ac3;
   FILE_LOG(logDEBUG1) << "ac4 = " << std::dec << ac4;
   FILE_LOG(logDEBUG1) << "ac5 = " << std::dec << ac5;
   FILE_LOG(logDEBUG1) << "ac6 = " << std::dec << ac6;

   FILE_LOG(logDEBUG1) << "b1 = " << std::dec << b1;
   FILE_LOG(logDEBUG1) << "b2 = " << std::dec << b2;

   FILE_LOG(logDEBUG1) << "mb = " << std::dec << mb;
   FILE_LOG(logDEBUG1) << "mc = " << std::dec << mc;
   FILE_LOG(logDEBUG1) << "md = " << std::dec << md;
}

BMP085::~BMP085()
{
   if (_i2cfile) {
      close(_i2cfile);
   }
}

float BMP085::readTemperature()
{
   int32_t UT, X1, X2, B5;       // following datasheet convention
   float temp;

   UT = readRawTemperature();

#ifdef DEBUG
   // use datasheet numbers!
   UT = 27898;
   ac6 = 23153;
   ac5 = 32757;
   mc = -8711;
   md = 2868;
#endif

   // See datasheet for details
   X1 = ((UT - (int32_t)ac6) * (int32_t)ac5) >> 15;
   X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md);
   B5 = X1 + X2;
   temp = (B5 + 8) >> 4;
   temp /= 10;

   return temp;
}

int32_t BMP085::readPressure()
{
   int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
   uint32_t B4, B7;

   UT = readRawTemperature();
   UP = readRawPressure();

#ifdef DEBUG
   // use datasheet numbers!
   UT  = 27898;
   UP  = 23843;
   ac6 = 23153;
   ac5 = 32757;
   mc  = -8711;
   md  = 2868;
   b1  = 6190;
   b2  = 4;
   ac3 = -14383;
   ac2 = -72;
   ac1 = 408;
   ac4 = 32741;
   oversampling = ULTRALOWPOWER;
#endif

   // do temperature calculations
   X1 = ((UT - (int32_t)ac6) * (int32_t)ac5) >> 15;
   X2 = ((int32_t)mc << 11) - (X1 + md)/2;     // round up
   X2 /= (X1 + md);
   B5 = X1 + X2;

   FILE_LOG(logDEBUG1) << "X1 = " << std::dec << X1;
   FILE_LOG(logDEBUG1) << "X2 = " << std::dec << X2;
   FILE_LOG(logDEBUG1) << "B5 = " << std::dec << B5;

   // do pressure calcs
   B6 = B5 - 4000;
   X1 = ((int32_t)b2 * ( (B6 * B6)>>12 )) >> 11;
   X2 = ((int32_t)ac2 * B6) >> 11;
   X3 = X1 + X2;
   B3 = ((((int32_t)ac1*4 + X3) << oversampling) + 2) / 4;

   FILE_LOG(logDEBUG1) << "B6 = " << std::dec << B6;
   FILE_LOG(logDEBUG1) << "X1 = " << std::dec << X1;
   FILE_LOG(logDEBUG1) << "X2 = " << std::dec << X2;
   FILE_LOG(logDEBUG1) << "B3 = " << std::dec << B3;

   X1 = ((int32_t)ac3 * B6) >> 13;
   X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
   X3 = ((X1 + X2) + 2) >> 2;
   B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
   B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oversampling );

   FILE_LOG(logDEBUG1) << "X1 = " << std::dec << X1;
   FILE_LOG(logDEBUG1) << "X2 = " << std::dec << X2;
   FILE_LOG(logDEBUG1) << "B4 = " << std::dec << B4;
   FILE_LOG(logDEBUG1) << "B7 = " << std::dec << B7;

   if (B7 < 0x80000000) {
      p = (B7 * 2) / B4;
   } else {
      p = (B7 / B4) * 2;
   }

   X1 = (p >> 8) * (p >> 8);
   X1 = (X1 * 3038) >> 16;
   X2 = (-7357 * p) >> 16;

   FILE_LOG(logDEBUG1) << "p = " << std::dec << p;
   FILE_LOG(logDEBUG1) << "X1 = " << std::dec << X1;
   FILE_LOG(logDEBUG1) << "X2 = " << std::dec << X2;

   p = p + ((X1 + X2 + (int32_t)3791)>>4);
   FILE_LOG(logDEBUG1) << "p = " << std::dec << p;

   return p;
}

float BMP085::readAltitude(float sealevelPressure)
{
   float altitude;

   float pressure = readPressure();

   altitude = 44330 * (1.0 - pow(pressure / sealevelPressure, 0.1903));

   return altitude;
}

uint16_t BMP085::readRawTemperature()
{
   write8(BMP085_CONTROL, BMP085_READTEMPCMD);
   delay();

   FILE_LOG(logDEBUG) << "Raw Temperature: " << std::dec << read16(BMP085_TEMPDATA);

   return read16(BMP085_TEMPDATA);
}

uint32_t BMP085::readRawPressure()
{
   uint32_t raw;

   write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));

   if (oversampling == ULTRALOWPOWER)
      delay_ms(5);
   else if (oversampling == STANDARD)
      delay_ms(8);
   else if (oversampling == HIGHRES)
      delay_ms(14);
   else
      delay_ms(26);

   raw = read16(BMP085_PRESSUREDATA);

   raw <<= 8;
   raw |= read8(BMP085_PRESSUREDATA + 2);
   raw >>= (8 - oversampling);

   FILE_LOG(logDEBUG) << "Raw Pressure: " << std::dec << raw;

   return raw;
}

/////////////////////////////////////////////////////////////////////////////////////////
// I2C Primitives

uint8_t BMP085::read8(uint8_t reg)
{
   ssize_t nbytes = 1;
   uint8_t data[nbytes];
   data[0] = reg;

   if (write(_i2cfile, data, nbytes) != nbytes) {
      perror("BMP085 Read register set error");
   }

   if (read(_i2cfile, data, nbytes) != nbytes) {
      perror("BMP085 Read register read value");
   }

   return data[0];
}


uint16_t BMP085::read16(uint8_t reg)
{
	uint8_t data[3];
	data[0] = reg;

	if (write(_i2cfile, data, 1) != 1) {
		perror("Error: set register");
	}

	if (read(_i2cfile, data, 2) != 2) {
		perror("Error: read value");
	}

	return data[1] | (data[0] << 8);
}


void BMP085::write8(uint8_t reg, uint8_t val)
{
   ssize_t nbytes = 2;
   uint8_t data[nbytes];
   data[0] = reg;
   data[1] = val;

   if (write(_i2cfile, data, nbytes) != nbytes) {
      perror("BMP085 Set register error");
   }
}


void BMP085::write16(uint8_t reg, uint16_t val)
{
   ssize_t nbytes = 3;
   uint8_t data[nbytes];
   data[0] = reg;
   data[1] = val & 0xff;
   data[2] = (val >> 8) & 0xff;

   if (write(_i2cfile, data, nbytes) != nbytes) {
      perror("BMP085 Set register error");
   }
}
