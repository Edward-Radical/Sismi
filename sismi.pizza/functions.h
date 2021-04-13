//------ HEADER ------

unsigned long samples = 10; 

//MPU
const int MPU = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ;

//JSON CONF
#include <ArduinoJson.h> // version 6.17.2
#define BUFFER 1024


//IP adress Raspberry Pi and MQTT Conf.
//const char* mqttServer = "raspi-hyperink";
//const int mqttPort = 1883;

//const char* mqttServer = "broker.mqtt-dashboard.com";
//const int mqttPort = 1883;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//Client
WiFiClient espClient;
//PubSubClient client(espClient);

//TIMER
unsigned long timer_save = 3000;
unsigned long start_time;
unsigned long ora;
const unsigned long Minutes = 1 * 60 * 1000UL;

//TIMESTAMP
//uncomment utcOffsetInSeconds if you need
// to set the time in your local zone
//const long utcOffsetInSeconds = 3600;

//define ntp client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
//NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
unsigned long epochTime;


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


//------ SPIFFS ------
void saveHistory(){
  File file = LittleFS.open("/file.txt", "a");
  if (!file) {
    Serial.println("Error opening file for writing");
    return;
  }
  
  StaticJsonDocument<BUFFER> doc;
  doc["AcX"] = AcX;
  doc["AcY"] = AcY;
  doc["AcZ"] = AcZ;
  doc["time"] = epochTime;

  //Uncomment these if you want to store values as array
  //StaticJsonDocument<BUFFER> doc;
  //doc.add(AcX);
  //doc.add(AcY);
  //doc.add(AcZ);
  //doc.add(start_time);

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

  for (int i=0; i<samples; i++){
    String s = file.readStringUntil('\n');
    Serial.print(i);
    Serial.print(":");
    Serial.println(s);
  }
}//end readHistory


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
void http_publish(){
  
  File file = LittleFS.open("/file.txt", "r");
  if(!file){
    Serial.println("file open failed");
    while (true) yield();
  }else {

    const size_t capacity = JSON_ARRAY_SIZE(10) + 10*JSON_OBJECT_SIZE(4);
    DynamicJsonDocument doc(capacity);
    DynamicJsonDocument globalDoc(capacity);
    String jsonLine;
    jsonLine.reserve(capacity);
    StaticJsonDocument <capacity> localDoc;
    while (file.available()) {
        jsonLine = file.readStringUntil('\n');
        DeserializationError error = deserializeJson(localDoc, jsonLine);
        if (!error) globalDoc.add(localDoc);
    }

    JsonArray data = doc.createNestedArray("Data"); 
    data.add(globalDoc);

    HTTPClient http;
    //Send request
    http.begin("http://raspi-hyperink:1880/postjdoc");
    char buffer[capacity];
    size_t n = serializeJson(doc, buffer);
    
    http.POST(buffer);
    Serial.println(buffer);
    http.end();
    file.close();
     
  }  
}//end http_publish


//------  MQTT Publish -------
//void mqtt_publish(){
//  File file = LittleFS.open("/file.txt", "r");
//  if(!file){
//    Serial.println("file open failed");
//    while (true) yield();
//  }else {
//
//    const size_t capacity = JSON_ARRAY_SIZE(10) + 10*JSON_OBJECT_SIZE(4);
//    DynamicJsonDocument doc(capacity);
//    DynamicJsonDocument globalDoc(capacity);
//    String jsonLine;
//    jsonLine.reserve(capacity);
//    StaticJsonDocument <capacity> localDoc;
//    while (file.available()) {
//        jsonLine = file.readStringUntil('\n');
//        DeserializationError error = deserializeJson(localDoc, jsonLine);
//        if (!error) globalDoc.add(localDoc);
//    }
//
//    JsonArray data = doc.createNestedArray("Data"); 
//    data.add(globalDoc);
//          
//    char buffer[capacity];
//    size_t n = serializeJson(doc, buffer);
//    int ret = client.publish("esp8266/JSON", buffer, n); 
//    Serial.println(buffer);
//    //globalDoc.clear();
//    file.close();
//  }
//}//end mqtt_publish

//Uncomment if you use MQTT
//void callback(char* topic, byte* payload, unsigned int length) {
//
//  Serial.print("Message arrived in topic: ");
//  Serial.println(topic);
//  Serial.print("Message:");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
//  }
//  Serial.println();
//  Serial.println("-----------------------"); 
//}//end callback

//------ RECONNECT FUNCTION ------
//void reconnect() {
//  // Loop until we're reconnected
//  while (!client.connected()) {
//    Serial.println("Attempting MQTT connection...");
//    // Attempt to connect
//    if (client.connect("esp8266/JSON")) {
//        Serial.print("Connected to topic: ");
//        Serial.println("esp8266/JSON");
//        
//        // Once connected, publish an announcement...
//        //client.publish("outTopic", "hello world");
//        // ... and resubscribe
//        //client.subscribe("inTopic");
//        client.subscribe("esp8266/JSON");
//    } else {
//        Serial.print("failed, rc=");
//        Serial.print(client.state());
//        Serial.println(" try again in 5 seconds");
//        // Wait 5 seconds before retrying
//        delay(5000);
//      }
//  }
//}//end reconnect

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
      String payload = http.getString(); 
      //Print the response payload
      Serial.println(payload);  
    }
    //Close connection
    http.end();  
 
}//end of GPS
