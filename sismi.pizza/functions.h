//------ HEADER ------
//------ HEADER ------

// Battery voltage resistance
#define BAT_RES_VALUE_VCC 12.0
#define BAT_RES_VALUE_GND 20.0

float batVolt;
String batteria;

//GPS
String GPS;
String latitudine;
String longitudine;
float lati;
float longi;
String payload;

//TIMESTAMP
//uncomment utcOffsetInSeconds if you need
//to set the time in your local zone
const long utcOffsetInSeconds = 7200;

//define ntp client to get time
WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org");
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
unsigned long epochTime;


//BATTERY
int threshold_1 = 7;
int threshold_2 = 16;
int threshold_3 = 21;

int threshold_c;

int battery;

//JSON CONF
//#include <ArduinoJson.h> // version 6.17.2
#define BUFFER 1024
const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(10) + 10*JSON_OBJECT_SIZE(4) + 10;

unsigned long samples = 100; 
unsigned long count = 0;

//MPU
const int MPU = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ;
int id = 1;


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//Client
WiFiClient espClient;

//TIMER
unsigned long start_time;
unsigned long ora;

//------ FUNCTIONS ------


//------ WIFI ------
void active_mode(){
  if(WiFi.status() != WL_CONNECTED){
    WiFi.forceSleepWake();
    delay(1);
    
    // WiFiManager
    WiFiManager wifiManager;
    wifiManager.autoConnect("AutoConnectAP");
    Serial.println("Connected.");
    delay(1);
  }
}//end active_mode

void sleep_mode(){
  //WiFi.disconnect();
  WiFi.forceSleepBegin();
  if(WiFi.status() != WL_CONNECTED){
    Serial.print(WiFi.status());
    Serial.println(" :WiFi Off");
  }
  //sleep time in ms
  //delay(Minutes + 1);
  delay(1);
}//end sleep_mode


//------ Read Data from MPU ------
void read_sensor(){

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

  const char * outputFileNames[] = {"/out1.txt", "/out2.txt", "/out3.txt", "/out4.txt", "/out5.txt", "/out6.txt", "/out7.txt", "/out8.txt", "/out9.txt", "/out10.txt"};
  const byte outputCount = sizeof outputFileNames / sizeof outputFileNames[0];
  byte outputIndex = 0;
  
  File sourceFile;
  File destinationFile;
  
  DynamicJsonDocument doc(capacity);
  DynamicJsonDocument globalDoc(capacity);
  StaticJsonDocument <100> lineDoc;
  String aLine;
  aLine.reserve(100);


  for (byte idx = 0; idx < outputCount; idx++) {
          
      destinationFile = LittleFS.open(outputFileNames[idx], "r");
      if (!destinationFile) {
        Serial.print(F("can't open destination "));
        Serial.println(outputFileNames[idx]);
        break;
      } else {
        Serial.print("Reading: ");
        Serial.println(outputFileNames[idx]);
        doc.clear();
        globalDoc.clear();
        int lineCount = 0;
        
        while (destinationFile.available()) {
          aLine = destinationFile.readStringUntil('\n');
          DeserializationError error = deserializeJson(lineDoc, aLine);
          lineCount++;
          if (!error) globalDoc.add(lineDoc);  
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
        char buffer[capacity];
        size_t n = serializeJson(doc, buffer, capacity);
        
        http.POST(buffer);
        Serial.println(buffer);
        http.end();
        destinationFile.close();

        Serial.printf_P(PSTR("free heap memory: %d\n"), ESP.getFreeHeap());
        //doc.~BasicJsonDocument();
        Serial.printf_P(PSTR("free heap memory: %d\n"), ESP.getFreeHeap());
      }
    }// end for   
}//end httpPublish

//------ VAR & CONST ------

const char * outputFileNames[] = {"/out1.txt", "/out2.txt", "/out3.txt", "/out4.txt", "/out5.txt", "/out6.txt", "/out7.txt", "/out8.txt", "/out9.txt", "/out10.txt"};
const byte outputCount = sizeof outputFileNames / sizeof outputFileNames[0];
byte outputIndex = 0;

File sourceFile;
File destinationFile;

//------ SPIFFS ------
//void History(String Path, String Mode){
//  File main = LittleFS.open(Path, Mode);
//  if(!main){
//    Serial.println("Error opening file");
//    return;
//  }
//
//  if(Mode == "a"){
//    StaticJsonDocument <BUFFER> doc;
//    doc["AcX"] = AcX;
//    doc["AcY"] = AcY;
//    doc["AcZ"] = AcZ;
//    doc["time"] = epochTime;
//  
//    char Data[BUFFER];
//    serializeJson(doc, Data);
//  
//    file.println(Data);
//    file.close();
//  } else if (Mode=="r"){
//      for (int i=0; i<=samples; i++){
//        String s = file.readStringUntil('\n');
//        Serial.print(i);
//        Serial.print(":");
//        Serial.println(s);
//      }//for
//  }//elif
//}

//------ TIMESTAMP ------
unsigned long getTime(){
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}//end getTime

//------ THRESHOLD ------
int get_threshold_c(){
  
  epochTime = getTime();
  threshold_c = hour(epochTime);
  //int Minutes = minute(epochTime);

  //String Time = String(Hour) + String(Minutes);
  //Serial.println(Time);
  return threshold_c;
}

//------ BATTERY ------
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

//------ GPS ------
void getGps(){
  
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


void saveHistory(){
  File file = LittleFS.open("/file.txt", "a");
  if (!file) {
    Serial.println("Error opening file for writing");
    return;
  }
  
  StaticJsonDocument <BUFFER> doc;
  doc["AcX"] = AcX;
  doc["AcY"] = AcY;
  doc["AcZ"] = AcZ;
  doc["time"] = epochTime;

  char Data[BUFFER];
  serializeJson(doc, Data);

  file.println(Data);
  file.close();  
}//end saveHistory

void readHistory(){
  File file = LittleFS.open("/file.txt", "r");
  if(!file){
    Serial.println("File open failed");
  } Serial.println("------ Reading ------");

  for (int i=0; i<=samples; i++){
    String s = file.readStringUntil('\n');
    Serial.print(i);
    Serial.print(":");
    Serial.println(s);
  }
}//end readHistory

void WritePacks() {
  
  sourceFile = LittleFS.open("/file.txt", "r");
  if (!sourceFile) {
    Serial.println(F("Error: file.txt open failed"));
  } else {
    Serial.println("File open w/ success");
    for (byte idx = 0; idx < outputCount; idx++) {
      String aLine;
      aLine.reserve(100);
      if (sourceFile.available() == 0) break;
      destinationFile = LittleFS.open(outputFileNames[idx], "w");
      if (!destinationFile) {
        Serial.print(F("can't open destination "));
        Serial.println(outputFileNames[idx]);
        break;
      } else {
        int lineCount = 0;
        while (sourceFile.available() && (lineCount < 10)) {
          aLine = sourceFile.readStringUntil('\n');
          destinationFile.println(aLine); // double check if the '\n' is in the String or not (--> print or println accordingly)
          lineCount++;
        }
        outputIndex = idx;
        Serial.println(outputIndex);
        destinationFile.close();
      }
    } // end for
    sourceFile.close();
  }
}//end WritePacks

//Use this just to check if the packages were written
void ReadPacks(){

  String aLine;
  aLine.reserve(capacity);

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
          Serial.println(aLine);   
        }//while
        destinationFile.close();
      }
    }// end for 
}//end ReadPacks 

void read_every(unsigned long timer_save, int increment){

  if (ora - start_time >= timer_save){ 
        read_sensor();
        //History("a")
        saveHistory();
        start_time = ora;
        count++;
        epochTime = epochTime + increment;
        //get_threshold_c();
  }//if

  if (count == 100){
  
    count = 0;
  
    active_mode();
    epochTime = getTime();
    getGps();
    getBatteryVoltage();
    WritePacks();
    httpPublish();
    get_threshold_c();
    
    LittleFS.remove("/file.txt");
    sleep_mode();
  } 
}//read_every
