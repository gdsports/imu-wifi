/****************************************************************************
MIT License

Copyright (c) 2017-2018 gdsports625@gmail.com

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

/*
 * Read quaternions from the BNO055, convert to heading, pitch, roll.
 * Run HTTP server on this device which uses p5.js (http://p5js.org) to
 * present a 3D model that matches the orientation of the BNO055.

   Connections BNO055 breakout to Huzzah ESP8266
   ===========
   Connect SCL to GPIO#5
   Connect SDA to GPIO#4
   Connect Vin to 3V
   Connect GND to GND
   Connect RST to RST

*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#include <elapsedMillis.h>

#include <DNSServer.h>
#include <WiFiClient.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer Webserver(80);
#include <ESP8266mDNS.h>
#else
#include <WiFi.h>
#include <WebServer.h>
WebServer Webserver(80);
#include <ESPmDNS.h>
#endif
#include <WebSocketsServer.h>
#include <Hash.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include "bno055_calibrate.h"
#include "index_html.h"
#include "sketch_js.h"

#define DEBUG_IMU (0)

WebSocketsServer webSocket = WebSocketsServer(81);

char DEVICE_NAME[32] = "bno055-a";

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (10)

Adafruit_BNO055 bno = Adafruit_BNO055();

bool Connected = false;

elapsedMillis imuElapsed;

const char IMU_JSON[] PROGMEM = R"=====({"heading":%f,"pitch":%f,"roll":%f})=====";

void handleRoot() {
  char html[1024];

  snprintf_P(html, sizeof(html), INDEX_HTML,
      DEVICE_NAME, 1000/BNO055_SAMPLERATE_DELAY_MS);
  Webserver.send(200, "text/html", html);
}

void handleSketch() {
  Webserver.send_P(200, "text/javascript", SKETCH_JS);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += Webserver.uri();
  message += "\nMethod: ";
  message += (Webserver.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += Webserver.args();
  message += "\n";
  for (uint8_t i=0; i<Webserver.args(); i++){
    message += " " + Webserver.argName(i) + ": " + Webserver.arg(i) + "\n";
  }
  Webserver.send(404, "text/plain", message);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  static uint32_t lastMillis = 0;

  //Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // Send the current orientation
      }
      break;
    case WStype_TEXT:
      //Serial.printf("[%u] [%u ms] get Text: %s\r", num, millis()-lastMillis, payload);
      lastMillis = millis();
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void webserver_setup()
{
  Webserver.on("/", handleRoot);
  Webserver.on("/sketch.js", handleSketch);

  Webserver.onNotFound(handleNotFound);

  Webserver.begin();
  Serial.println("HTTP server started");

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started");
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println(F("\nOrientation Sensor Raw Data Test")); Serial.println();

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect(DEVICE_NAME);

  Serial.print(F("WiFi connected! IP address: "));
  Serial.println(WiFi.localIP());
  Connected = true;

  /* Initialise the sensor */
  if (!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print(F("BNO055 not found, Check your wiring or I2C ADDR!"));
    while (1) delay(1);
  }

  calib_setup();

  if (MDNS.begin(DEVICE_NAME)) {
    Serial.print("MDNS responder started. Connect to ");
    Serial.print(DEVICE_NAME);
    Serial.println(".local");
  }

  webserver_setup();

  bno.setExtCrystalUse(true);
  /* Display the current temperature */
  delay(1000);
  int8_t temp = bno.getTemp();
  Serial.print(F("Current Temperature: "));
  Serial.print(temp);
  Serial.println(F(" C"));
  Serial.println();
}

void imu_loop()
{
  static uint8_t last_cal = 0xC0;

  // Thanks to gammaburst @ forums.adafruit.com for this conversion.
  imu::Quaternion q = bno.getQuat();
  // flip BNO/Adafruit quaternion axes to aerospace: x forward, y right, z down
  float temp = q.x();  q.x() = q.y();  q.y() = temp;  q.z() = -q.z();
  q.normalize();

  // convert aerospace quaternion to aerospace Euler, because BNO055 Euler data is broken
  float heading = 180/M_PI * atan2(q.x()*q.y() + q.w()*q.z(), 0.5 - q.y()*q.y() - q.z()*q.z());
  float pitch   = 180/M_PI * asin(-2.0 * (q.x()*q.z() - q.w()*q.y()));
  float roll    = 180/M_PI * atan2(q.w()*q.x() + q.y()*q.z(), 0.5 - q.x()*q.x() - q.y()*q.y());
  heading = heading < 0 ? heading+360 : heading;

#if DEBUG_IMU
  Serial.print(F("HeadingPitchRoll: "));
  Serial.print(heading);  // heading, nose-right is positive
  Serial.print(F(" "));
  Serial.print(pitch);    // pitch, nose-up is positive
  Serial.print(F(" "));
  Serial.print(roll);     // roll, leftwing-up is positive
  Serial.println(F(""));
#endif

  // Send JSON over websocket
  char payload[80];
  snprintf_P(payload, sizeof(payload), IMU_JSON, heading, pitch, roll);
  webSocket.broadcastTXT(payload, strlen(payload));

  /* Display calibration status for each sensor. */
  uint8_t system, gyro, accel, mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);
  uint8_t now_cal = (system << 6) | (gyro << 4) | (accel << 2) | mag;
  // Show calibration statuses only when they change
  if (now_cal != last_cal) {
    Serial.print(F("CAL: Sys="));
    Serial.print(system, DEC);
    Serial.print(F(" G="));
    Serial.print(gyro, DEC);
    Serial.print(F(" A="));
    Serial.print(accel, DEC);
    Serial.print(F(" M="));
    Serial.println(mag, DEC);
    last_cal = now_cal;

    if (!calib_found()) {
      if (system == 3 && gyro == 3 && accel == 3 && mag == 3) {
        calib_save();
      }
    }
  }
}

void loop(void)
{
  if (WiFi.status() == WL_CONNECTED) {
    if (!Connected) {
      Serial.print(F("WiFi connected! IP address: "));
      Serial.println(WiFi.localIP());
      Connected = true;
    }
  }
  else {
    if (Connected) {
      Serial.println(F("WiFi not connected!"));
      Connected = false;
    }

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect(DEVICE_NAME);

    Serial.print(F("WiFi connected! IP address: "));
    Serial.println(WiFi.localIP());
    Connected = true;
  }

  webSocket.loop();
  Webserver.handleClient();

  if (imuElapsed > BNO055_SAMPLERATE_DELAY_MS) {
    imuElapsed = 0;
    imu_loop();
  }
}
