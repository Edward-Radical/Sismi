//------ HEADER ------

// Battery voltage resistance
#define BAT_RES_VALUE_VCC 12.0
#define BAT_RES_VALUE_GND 20.0

float batVolt;
String batteria;

unsigned long samples = 10; 

//MPU
const int MPU = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ;

//JSON CONF
#include <ArduinoJson.h> // version 6.17.2
#define BUFFER 1024

int battery;
int id = 1;

String GPS;

String latitudine;
String longitudine;

float lati;
float longi;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//Client
WiFiClient espClient;

//TIMER
unsigned long timer_save = 3000;
unsigned long start_time;
unsigned long ora;
const unsigned long Minutes = 2 * 60 * 1000UL;

//TIMESTAMP
//uncomment utcOffsetInSeconds if you need
// to set the time in your local zone
//const long utcOffsetInSeconds = 3600;

//define ntp client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
//NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
unsigned long epochTime;

//GPS
String payload;


//------ FUNCTIONS ------


//------ WIFI ------
void active_mode(){
  if(WiFi.status() != WL_CONNECTED){
    WiFi.forceSleepWake();
    
    // WiFiManager
    WiFiManager wifiManager;
    wifiManager.autoConnect("AutoConnectAP");
    Serial.println("Connected.");
    delay(1);
  }
}//end active_mode

void sleep_mode(){
  WiFi.forceSleepBegin();
  if(WiFi.status() != WL_CONNECTED){
    Serial.print(WiFi.status());
    Serial.println(" :WiFi Off");
  }
  //sleep time in ms
  delay(Minutes + 1);
}//end sleep_mode


//------ Read Data from MPU ------
void read_data(){

  //MPU reading
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
   
}//end read_data

//------ HTTP Publish ------
void httpPublish(){

  const char * outputFileNames[] = {"/out1.txt", "/out2.txt", "/out3.txt"};
  const byte outputCount = sizeof outputFileNames / sizeof outputFileNames[0];
  byte outputIndex = 0;
  
  File sourceFile;
  File destinationFile;
  
  
  //const size_t capacity = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(30) + 30*JSON_OBJECT_SIZE(4);
  DynamicJsonDocument doc(4096);
  DynamicJsonDocument globalDoc(4096);
  StaticJsonDocument <1000> localDoc;

  String aLine;
  aLine.reserve(1024);

  for (byte idx = 0; idx < outputCount; idx++) {
      destinationFile = LittleFS.open(outputFileNames[idx], "r");
      if (!destinationFile) {
        Serial.print(F("can't open destination "));
        Serial.println(outputFileNames[idx]);
        break;
      } else {
        Serial.print("Reading: ");
        Serial.println(outputFileNames[idx]);
        //int lineCount = 0;
        while (destinationFile.available()) {
          aLine = destinationFile.readStringUntil('\n');
          DeserializationError error = deserializeJson(localDoc, aLine);
          if (!error) globalDoc.add(localDoc);  
          else{ Serial.println("Error Writing All files");}
        }//while

        JsonObject Info = doc.createNestedObject("Info");
        Info["Battery"] = battery;
        Info["ID"] = id;
        Info["Latitudine"] = latitudine;
        Info["Longitudine"] = longitudine;
    
        
        JsonArray Data = doc.createNestedArray("Data"); 
        Data.add(globalDoc);
    
        HTTPClient http;
        //Send request
        http.begin("http://raspi-hyperink:1880/postjdoc");
        char buffer[4096];
        size_t n = serializeJson(doc, buffer);
        
        http.POST(buffer);
        Serial.println(buffer);
        http.end();
        destinationFile.close();
        globalDoc.clear();
        localDoc.clear();
      }
    }// end for   
}//end httpPublish

//------ TIMESTAMP ------
unsigned long getTime(){
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}//end getTime

//------ GPS ------
void gps(){
  
    //Declare an object of class HTTPClient
    HTTPClient http;  

    //Specify request destination and fields
    http.begin("http://api.ipstack.com/check?access_key=233bf289dff160082dd3e5d915ce3135&fields=latitude,longitude");  
    //Send the request
    int httpCode = http.GET();                                  

    //Check the returning code
    if (httpCode > 0) { 
      //Get the request response payload
      payload = http.getString(); 
      //Print the response payload
      Serial.println(payload); 

      DynamicJsonDocument doc(512);
      DeserializationError error = deserializeJson(doc, payload);
      
      lati  = doc["latitude"];
      longi = doc["longitude"];
     
      latitudine = String(lati, 2);
      longitudine = String(longi, 2); 
    }
    //Close connection
    http.end();  
 
}//end of GPS

float getBatteryVoltage(){
    //************ Measuring Battery Voltage ***********
    double sample1 = 0;
    // Get 100 analog read to prevent unusefully read
    for (int i = 0; i < 100; i++) {
        sample1 = sample1 + analogRead(A0); //read the voltage from the divider circuit
        delay(2);
    }
    sample1 = sample1 / 100;
    // REFERENCE_VCC is reference voltage of microcontroller 3.3v for esp8266 5v Arduino
    // BAT_RES_VALUE_VCC is the kohm value of R1 resistor
    // BAT_RES_VALUE_GND is the kohm value of R2 resistor
    // 1023 is the max digital value of analog read (1024 == Reference voltage)
    batVolt = (sample1 * 3.3  * (BAT_RES_VALUE_VCC + BAT_RES_VALUE_GND) / BAT_RES_VALUE_GND) / 1023;
    //float batVolt = sample1 * 5/ 1023;
    batteria = String(batVolt, 2);
    Serial.println(batteria);
}
