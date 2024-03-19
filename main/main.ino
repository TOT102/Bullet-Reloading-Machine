#include <Arduino.h>
#include "EasyNextionLibrary.h"
#include <TinyStepper.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SdFat.h>
#include <SoftwareSerial.h> 

#include "pin_defines.hpp"
#include "constants.hpp"

EasyNex myNex(Serial);
SdFat sd;
TinyStepper stepper(HALFSTEPS, STEPPER_PIN1, STEPPER_PIN2, STEPPER_PIN3, STEPPER_PIN4);

//global variables
bool run = false;
bool is_homed = false;
bool error = false;
int pageId = 0;
int currentFileIndex = 0;
char fileList[10][10]; // Adjust the array size as needed
int numFiles;
/*
char* file_name_with_extension_char;
char* name_char;
char* caliber_char;
char* batch_char;
int distance = 0;
int time = 0;*/


namespace SD_handler { // Used for interacting with the SD card
     bool openFile(const char* filename) {
        String file_name = filename;
        File dataFile = sd.open(file_name.c_str(), O_CREAT);
        return dataFile;
    }

    void writeData(const char* filename, const char* name, const char* caliber, const char* batch, int distance, int time) {
        DynamicJsonDocument jsonDocument(200);
        JsonObject json = jsonDocument.to<JsonObject>();
  
        json["name"] = name;
        json["caliber"] = caliber;
        json["batch"] = batch;
        json["distance"] = distance;
        json["time"] = time;
        // Open a file for writing
        String fileName = filename;
        File dataFile = sd.open(fileName.c_str(), O_CREAT | O_WRITE);
        serializeJson(json, dataFile);

        // Close the file
        dataFile.close();
    }

    void readData(const char* filename, String& name, String& caliber, String& batch, int& timeValue, int& distanceValue) {
        File dataFile = sd.open(filename, FILE_READ);

        // Parse JSON
        StaticJsonDocument<200> jsonDocument;
        DeserializationError error = deserializeJson(jsonDocument, dataFile);
        
        // Access JSON data
        name = jsonDocument["name"].as<String>();
        caliber = jsonDocument["caliber"].as<String>();
        batch = jsonDocument["batch"].as<String>();
        timeValue = jsonDocument["time"];
        distanceValue = jsonDocument["distance"];

        dataFile.close();
    }

    void deleteData(const char* filename) {
        if (sd.exists(filename)) {
            sd.remove(filename);
        } else {
            // error handling, e.g., Serial.println("File not found");
        }
    }

    void listFiles(char fileList[][10], int& numFiles) {
        File file;
        File root = sd.open("/");
        numFiles = 0;
        
        while (file.openNext(&root, O_RDONLY)) {
            if (!file.isDir() && numFiles < 20) { // Limit to 100 files for example (adjust as needed)
                char fileName[256];
                file.getName(fileName, sizeof(fileName));
                strncpy(fileList[numFiles], fileName, sizeof(fileList[numFiles]) - 1);
                fileList[numFiles][sizeof(fileList[numFiles]) - 1] = '\0'; // Ensure null-terminated
            numFiles++;
            }
            file.close();
        }
        root.close();
    }
}
namespace machine {
  void home_and_move(int distanceInMillimeters) {
    // Convert the distance in millimeters to steps
    int stepsToMove = distanceInMillimeters * stepsPerMillimeter;

    // Homing sequence
    while (digitalRead(ENDSTOP_PIN)) {
      stepper.Move(-1);
    }

    // Rotation sequence
    int stepsMoved = 0;
    int direction = (distanceInMillimeters > 0) ? 1 : -1;

    while (stepsMoved < abs(stepsToMove)) {
      stepper.Move(direction);
      stepsMoved++;
    }
  }
  void controlCoil(int durationInSeconds) {
    digitalWrite(RELAY_PIN, HIGH); // Turn ON the relay
    delay(durationInSeconds * 1000); // Convert seconds to milliseconds
    digitalWrite(RELAY_PIN, LOW); // Turn OFF the relay
  }
  void displayErrorScreen() {
    // Send command to Nextion display to show the error screen
    // The command format is: pageID,componentID,pop,pop attribute
    // For example, "pageID,componentID,pop,1" will show the error screen with pageID and componentID

    // You can also add additional commands to customize the error screen
    // For example, change the background color, display the error message, etc.
  }
  void print_to_nextion(const char* name, const char* caliber, const char* batch, int time, int distance) {
    String stringValue1 = String(time);
    String stringValue2 = String(distance);
    
    myNex.writeStr("n0.val", stringValue1); // Set the value of n0 to time

    myNex.writeStr("n1.val", stringValue2.c_str()); // Set the value of n1 to distance

    myNex.writeStr("t5.txt", name); // Set the text of t5 to name
    myNex.writeStr("t6.txt", caliber); // Set the text of t6 to caliber
    myNex.writeStr("t7.txt", batch); // Set the text of t7 to batch
  }
}
void declare_pins(){
  pinMode(STEPPER_PIN1, OUTPUT);
  pinMode(STEPPER_PIN2, OUTPUT);
  pinMode(STEPPER_PIN3, OUTPUT);
  pinMode(STEPPER_PIN4, OUTPUT);
  pinMode(ENDSTOP_PIN, INPUT_PULLUP);

  pinMode(SS_BUTTON_PIN, INPUT_PULLUP);

  pinMode(RELAY_PIN, OUTPUT);
}
void setup(){
  declare_pins();
}

void loop(){
    
}
