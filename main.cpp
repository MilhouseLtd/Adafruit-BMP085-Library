

#include <iostream>
#include "BMP085.h"
#include "log.h"


int main(int argc, char* argv[])
{
   FILELog::ReportingLevel() = FILELog::FromString("INFO");

   BMP085 *bmp085 = new BMP085();

   float temp = bmp085->readTemperature();
   std::cout << "Temperature: " << temp << " ˚C\n";

   int32_t press = bmp085->readPressure();
   std::cout << "Pressure: " << press << " Pa\n";

   // Calculate altitude assuming 'standard' barometric
   // pressure of 1013.25 millibar = 101325 Pascal
   float alt = bmp085->readAltitude();
   std::cout << "Altitude: " << alt << " m\n";

   // you can get a more precise measurement of altitude
   // if you know the current sea level pressure which will
   // vary with weather and such. If it is 1015 millibars
   // that is equal to 101500 Pascals.
   float altr = bmp085->readAltitude(101300);
   std::cout << "Real altitude: " << altr << " m\n";

   return 0;
}
