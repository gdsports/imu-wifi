/****************************************************************************
MIT License

Copyright (c) 2017 gdsports625@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************/
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "bno055_calibrate.h"

static bool FoundCalib = false;

bool calib_found()
{
  return FoundCalib;
}

void calib_setup()
{
  int eeAddress = 0;
  long bnoID;
  adafruit_bno055_offsets_t calibrationData;

#ifdef ESP8266
  EEPROM.begin(4096);
  for (int i = 0; i < sizeof(calibrationData)+sizeof(long); i++ ) {
    Serial.print(EEPROM.read(i), HEX); Serial.print(' ');
  }
  Serial.println();
#endif
  //EEPROM.put(eeAddress, 0x0F0F0F);
  EEPROM.get(eeAddress, bnoID);
  Serial.print("bnoID "); Serial.println(bnoID, HEX);

  /*
  *  Look for the sensor's unique ID at the beginning oF EEPROM.
  *  This isn't foolproof, but it's better than nothing.
  */
  if (bnoID != ESP.getChipId()) {
    Serial.println("\nNo Calibration Data for this sensor exists in EEPROM");
    delay(500);
  }
  else {
    Serial.println("\nFound Calibration for this sensor in EEPROM.");
    eeAddress += sizeof(bnoID);
    EEPROM.get(eeAddress, calibrationData);
    displaySensorOffsets(calibrationData);

    if ((calibrationData.accel_offset_x != 0xFFFF) ||
        (calibrationData.accel_offset_y != 0xFFFF) ||
        (calibrationData.accel_offset_z != 0xFFFF)) {
      Serial.println("\n\nRestoring Calibration data to the BNO055...");
      bno.setSensorOffsets(calibrationData);

      Serial.println("\n\nCalibration data loaded into BNO055");
      FoundCalib = true;
    }
  }
}

void calib_save()
{
  int eeAddress;
  uint32_t bnoID;
  adafruit_bno055_offsets_t newCalib;

  Serial.println(F("Saving calibration data..."));
  bno.getSensorOffsets(newCalib);
  displaySensorOffsets(newCalib);

  eeAddress = 0;
  bnoID = ESP.getChipId();

  EEPROM.put(eeAddress, bnoID);

  eeAddress += sizeof(bnoID);
  EEPROM.put(eeAddress, newCalib);
#ifdef ESP8266
  EEPROM.commit();
#endif
  FoundCalib = true;
}

/**************************************************************************/
/*
    Display the raw calibration offset and radius data
*/
/**************************************************************************/
void displaySensorOffsets(const adafruit_bno055_offsets_t &calibData)
{
  Serial.print("Accelerometer: ");
  Serial.print(calibData.accel_offset_x); Serial.print(" ");
  Serial.print(calibData.accel_offset_y); Serial.print(" ");
  Serial.print(calibData.accel_offset_z); Serial.print(" ");

  Serial.print("\nGyro: ");
  Serial.print(calibData.gyro_offset_x); Serial.print(" ");
  Serial.print(calibData.gyro_offset_y); Serial.print(" ");
  Serial.print(calibData.gyro_offset_z); Serial.print(" ");

  Serial.print("\nMag: ");
  Serial.print(calibData.mag_offset_x); Serial.print(" ");
  Serial.print(calibData.mag_offset_y); Serial.print(" ");
  Serial.print(calibData.mag_offset_z); Serial.print(" ");

  Serial.print("\nAccel Radius: ");
  Serial.print(calibData.accel_radius);

  Serial.print("\nMag Radius: ");
  Serial.println(calibData.mag_radius);
}
