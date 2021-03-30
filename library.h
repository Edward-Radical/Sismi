//WIFI
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//MPU
#include<Wire.h>

//Client
#include <PubSubClient.h>
#define MQTT_KEEPALIVE 300

//ESP MEMORY
#include "LittleFS.h" // LittleFS is declared
#include "FS.h" // SPIFFS is declared

//TIMESTAMP
#include <NTPClient.h>
#include <WiFiUdp.h>
