#include "library.h"
#include "functions.h"


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
  server.begin();

  //MOUNTING FILE SYSTEM
  bool success = LittleFS.begin();
  if (success){
    Serial.println("File system mounted properly");
  } else {
    Serial.println("Error mounting file system");
    return;
  }

  //Checking Info
  // To format all space in LittleFS
    // LittleFS.format()

    LittleFS.remove("/file.txt");
  
    // Get all information of your LittleFS
    FSInfo fs_info;
    LittleFS.info(fs_info);
  
    Serial.println("File sistem info.");
  
    Serial.print("Total space:      ");
    Serial.print(fs_info.totalBytes);
    Serial.println("byte");
  
    Serial.print("Total space used: ");
    Serial.print(fs_info.usedBytes);
    Serial.println("byte");
  
    Serial.print("Block size:       ");
    Serial.print(fs_info.blockSize);
    Serial.println("byte");
  
    Serial.print("Page size:        ");
    Serial.print(fs_info.totalBytes);
    Serial.println("byte");
  
    Serial.print("Max open files:   ");
    Serial.println(fs_info.maxOpenFiles);
  
    Serial.print("Max path lenght:  ");
    Serial.println(fs_info.maxPathLength);
  
    Serial.println();
  
    // Open dir folder
    Dir dir = LittleFS.openDir("/");
    // Cycle all the content
    while (dir.next()) {
        // get filename
        Serial.print(dir.fileName());
        Serial.print(" - ");
        // If element have a size display It else write 0
        if(dir.fileSize()) {
            File f = dir.openFile("r");
            Serial.println(f.size());
            f.close();
        }else{
            Serial.println("0");
        }
    }  

  //TIMESTAMP in seconds
  epochTime = getTime();
  //Get Time in hour
  get_threshold_c();
  
  //TURN OFF WIFI
  sleep_mode();

}

void loop() {

  //Get Timer
  ora = millis(); 

  if(threshold_c >= threshold_1 && threshold_c < threshold_2 ){
    //Send data every 3 seconds (tot. 100)
    read_every(3000, 3);
  }  

  else if(threshold_c >= threshold_2 && threshold_c < threshold_3){
    //Send data every 6 seconds (tot. 100)
    read_every(6000, 6);
  } 
  
  else if(threshold_c >= threshold_3 && threshold_c < threshold_1){
  //Send data every 12 seconds (tot. 100)
  read_every(12000, 12);
  }
  
}//end of void loop();

  
