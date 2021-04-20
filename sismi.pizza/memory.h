//------ VAR & CONST ------

const char * outputFileNames[] = {"/out1.txt", "/out2.txt", "/out3.txt"};
const byte outputCount = sizeof outputFileNames / sizeof outputFileNames[0];
byte outputIndex = 0;

File sourceFile;
File destinationFile;
String aLine;

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

void WritePacks() {

  aLine.reserve(20);
  
  sourceFile = LittleFS.open("/file.txt", "r");
  if (!sourceFile) {
    Serial.println(F("Error: file.txt open failed"));
  } else {
    Serial.println("File open w/ success");
    for (byte idx = 0; idx < outputCount; idx++) {
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

  aLine.reserve(20);

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
