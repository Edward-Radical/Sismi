#include "library.h"
#include "functions.h"

unsigned long count = 0;

void setup() {

  //TIMESTAMP
  timeClient.begin();

  //MPU
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  //SERIAL
  Serial.begin(115200);

  //WIFI MANAGER
  // Local intialization.
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("Connected.");

  //CLIENT
  //client.setServer(mqttServer, mqttPort);
  //client.setCallback(callback);
  //client.setBufferSize(9000);
  server.begin();

  //MOUNTING FILE SYSTEM
  bool success = LittleFS.begin();
  if (success){
    Serial.println("File system mounted properly");
  } else {
    Serial.println("Error mounting file system");
    return;
  }

  //TIMESTAMP in seconds
  epochTime = getTime();
  Serial.println(epochTime);
  delay(1000);

  //TURN OFF WIFI
  sleep_mode();

}

void loop() {

  //Read data every 3 seconds and save them in file.txt
  ora = millis();    
  if (ora - start_time >= timer_save){ 
      read_data();
      saveHistory();
      start_time = ora;
      count++;
      epochTime = epochTime + 3000;
  }
     
  //Publish data in topic and clear file.txt
  if (count == 10){

    count = 0;

    active_mode();
    epochTime = getTime();
    gps();
    Serial.println(epochTime);
    
// Uncomment if you use MQTT
//    if (!client.connected()) {
//            reconnect();
//          }
//    client.loop();

    http_publish();
    
    LittleFS.remove("/file.txt");
     
    sleep_mode();
  }
}//end of void loop();

  
