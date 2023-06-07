# PurpleTheopolis v2.0.0 June 2023
Copyright 2023 Jeff Luszcz
https://github.com/jeff-luszcz/PurpleTheopolis

An Arduino sketch used to create PurpleAir air quality display using a Jewel 7 LED module and a Feather M0 Wifi board

This sketch uses NeoPixel LEDS or Rings to display the current air quality color (AQI) based on an internet available sensor 
from the PurpleAir air sensor network. Any public sensor can be selected and the air quality value (PM 2.5) will be used
to change the color of the attached NeoPixels every 10 minutes using Wifi, access to the internet and a Purple Air API Key.

To use, download project locally, add required libraries to your Arduino IDE ( ), update required variables like wifi sssid, password, 
sensor id, number of NeoPixels, brightness, etc, compile and then update your Feather M0 Wifi device with the updated sketch.

# Required Hardware
Feather M0 Wifi, NeoPixel LED or NeoPixel Ring

# Required Arduino Libraries:
Wifi101, SPI, ArduinoJson and Adafruit Neopixel

# Required API Keys
PurpleAir REST Read API Key
See https://develop.purpleair.com/keys

This is the source code for the Arduino project written up on Instrucables.com and elsewhere.

This code has been updated in June 2023 to reflect changes in the PurpleAir API, api keys and JSON data format

See https://www.instructables.com/id/PurpleAir-Air-Quality-Status-LED-Display/

License: MIT
