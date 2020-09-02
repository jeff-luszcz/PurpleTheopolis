#include <ArduinoJson.h>
#include <WiFi101.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include "arduino_secrets.h"


/* Copyright 2020 Jeff Luszcz

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the 
Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR 
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// #0 this sketch uses the libraries referenced above, you will want to import them into your IDE
// Wifi101, SPI, ArduinoJson and Adafruit Neopixel

// NOTE: this sketch is hardcoded to use he first sensor named [0] in a purpleair device, they often have 2, and one might work better
// than the other, check the JSON data using the website via click on a sensor, then selecting "Get this Widget" in the bottom of the square dialog box 
// and then select JSON

// To use this sketch you will need to configure it
// #1 UPDATE the SSID and password in a file called arduino_secret.h in this same directory
// #2 UPDATE the URL fragment for the sensor you want to monitor below (URL of JSON feed found by clicking on a sensor and select Pin this Widget and then JSON)
//     this should be the format of "/json?key=ABCDEF&show=12345"              CUT AND PASTE that fragment into the below variable
#define PURPLE_AIR_SENSOR "/json?key=ABCDEF&show=20721"


// #3 UPDATE which pin on the Arduino is connected to the NeoPixels
#define LED_PIN    9

// #4 UPDATE How many NeoPixels are attached to the Arduino? (a Jewel 7 uses 7 natch!)
#define LED_COUNT 7

// #5 UPDATE How long to delay between each sensor reading (probably should not be more than once every 10 minutes)

#define DELAY_TIME_MILLIS 10 * 60 * 1000 // 10 min = 10*60*1000

// #6 UPDATE change color of LEDS based on your favorite breakdown of PM25 to potential hazard level
//           by updating the values seen in the routine getColorFromPM25 at bottom of this file


// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


#define JSON_BUFF_DIMENSION 2500

///////please enter your sensitive data in the file arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)


int status = WL_IDLE_STATUS;


void setup() {


  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  WiFi.setPins(8, 7, 4, 2);
  // Initialize Serial port
  Serial.begin(9600);


  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  printWifiStatus();


}

void loop() {

  Serial.println(F("Connecting..."));

  // Connect to HTTP server
  WiFiClient client;
  client.setTimeout(10000);
  if (!client.connect("www.purpleair.com", 80)) {
    Serial.println(F("Connection failed"));
    return;
  }

  Serial.println(F("Connected!"));

  String url = String("GET " + String(PURPLE_AIR_SENSOR) + " HTTP/1.0");
  client.println(url);
  client.println(F("Host: www.purpleair.com"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.0 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  // Allocate the JSON document
  // Use arduinojson.org/v6/assistant to compute the capacity.
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(30) + JSON_OBJECT_SIZE(39) + 1485;
  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Extract values
  Serial.println(F("Response:"));
  float pm = doc["results"][0]["PM2_5Value"].as<float>();

  Serial.println(pm, 6);


  // set the color of the LEDS
  uint32_t aqi = 0.0;
  aqi = getColorFromPM25(pm);
  setAllLeds(aqi);

  // Disconnect
  client.stop();

  delay(DELAY_TIME_MILLIS);  // wait here till the timer counts down (10 mins or 1 min etc...)
}




void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}



// used to set all LEDS in the Jewel to the same color and brightness
void setAllLeds(uint32_t rgb) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, rgb);

  }
  strip.show();
}



// Update ths routine if you like different colors for different PM 2.5 levels
// current mapping taken from https://smartairfilters.com/en/blog/difference-pm2-5-aqi-measurements/
uint32_t getColorFromPM25( float pm25) {
  int pm25int = int(pm25);

  if (pm25int <= 12) {
    return strip.Color(  0, 255,   0);        // green
  }


  if (pm25int <= 35) {
    return strip.Color( 255, 255,   0);      // moderate / yellow
  }

  if (pm25int <= 55) {
    return strip.Color( 255, 153,  51);     // dark yellow
  }
  if (pm25int <= 150) {
    return strip.Color( 255, 0,   0);      // red
  }

  if (pm25int <= 250) {
    return strip.Color( 128, 0,   204);   // purple
  }


  if (pm25int <= 500) {
    return strip.Color( 51, 0,   102);   // maroon
  }


  if (pm25int <= 1000) {
    return strip.Color( 0, 206,   209);   // turqoise (self selected color for over 500 < 1000
  }

  // got value over 1000, this doesn't make sense, return black
  return strip.Color( 0, 0,   0);   // black
}
