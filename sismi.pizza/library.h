//WIFI
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//MPU
#include<Wire.h>

//ESP MEMORY
#include "LittleFS.h" // LittleFS is declared
#include "FS.h" // SPIFFS is declared

//TIMESTAMP
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

//GPS
#include <ESP8266HTTPClient.h>
#include <Arduino.h>

//JSON
#include <ArduinoJson.h> // version 6.17.2
