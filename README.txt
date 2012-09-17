This is Raspberry Pi port of the Adafruit BMP085 Barometric Pressure and
Temperature sensor library.

Designed specifically to work with the Adafruit BMP085 Breakout
  ----> https://www.adafruit.com/products/391

Original version written by Limor Fried/Ladyada for Adafruit Industries.
BSD license, all text above must be included in any redistribution


Raspberry Pi BMP085 Barometric Pressure and Temperature sensor
--------------------------------------------------------------

This is a C++ class to interface the Raspberry Pi to an Adafruit BMP085
Barometric Pressure and Temperature sensor. The BMP085 sensor is manufactured by
Bosch and can provide pressure, temperature and altitude over an I2C interface.

The actual Bosch device is in a surface mount package so Adafruit have designed a
breakout board to make it very easy for prototyping and project use. More
information can be found at http://learn.adafruit.com/bmp085.

The code here hasn't changed much from the original Adafruit code apart from
changes to the I2C interface to suit the Raspberry Pi rather than the Arduino
and the addition of a simple logging library.
