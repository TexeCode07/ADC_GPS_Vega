#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;  // Use this for the 16-bit version
#include <Wire.h>
#include <RTClib.h>
RTC_DS1307 rtc;        // Create an RTC object

#include <SD.h>
#include <SPI.h>
const int chipSelect = 5; // SD card chip select pin
File dataFile;

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
// Define GPS TX and RX pins
SoftwareSerial ss(27, 26); // RX, TX (connect Neo6M TX to pin 4, RX to pin 3)
TinyGPSPlus gps;       // Create GPS object

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <OneButton.h>
#include "index.h"
// Define the button pin and LED for webserver status (optional)
const int buttonPin = 0;  // GPIO for the button
const int ledPin = 2;     // Optional, LED to indicate server status
// Create the button object
OneButton button(buttonPin, true); // true for active-low button
// Replace with your desired SoftAP credentials
const char* ssid = "ESP32_AP";
const char* password = "12345678";  // Set your SoftAP password (minimum 8 characters)
// Create an AsyncWebServer object
AsyncWebServer server(80);
// Variables to track the SoftAP status
bool apActive = false;  // Tracks whether the SoftAP is active or not

// Function to start the SoftAP (hotspot)
void startSoftAP() {
  if (!apActive) {
    WiFi.softAP(ssid, password);  // Start SoftAP
    delay(1000);  // Give time for the AP to initialize
    digitalWrite(ledPin, HIGH);  // Turn on LED to indicate SoftAP is active
    apActive = true;
    Serial.println("SoftAP started");
    Serial.print("SoftAP IP Address: ");
    Serial.println(WiFi.softAPIP());
    server.begin();  // Start web server
  }
}

// Function to stop the SoftAP (hotspot)
void stopSoftAP() {
  if (apActive) {
    WiFi.softAPdisconnect(true);  // Stop SoftAP
    digitalWrite(ledPin, LOW);  // Turn off LED to indicate SoftAP is inactive
    apActive = false;
    Serial.println("SoftAP stopped");
    server.end();  // Stop web server
  }
}

// Button press handler to toggle the SoftAP
void handleButtonPress() {
  if (apActive) {
    stopSoftAP();  // If AP is active, stop it
  } else {
    startSoftAP();  // If AP is inactive, start it
  }
}

// Function to list files in /track folder
// Function to list files in the /track folder with Download and Delete buttons
String listFilesInTrackFolder() {
  String fileList = "<table border='1' style='width:100%; text-align:center;'><tr><th>File Name</th><th>Size (KB)</th><th>Download</th><th>Delete</th></tr>";
  File root = SD.open("/track");
  File file = root.openNextFile();
  while (file) {
    String fileName = String(file.name());
    float fileSizeKB = file.size() / 1024.0;  // Convert file size from bytes to KB (1 KB = 1024 bytes)
    fileList += "<tr><td>" + fileName + "</td>";
    fileList += "<td>" + String(fileSizeKB, 2) + " KB</td>";  // Display file size in KB with 2 decimal places
    fileList += "<td><a href='/download?file=" + fileName + "'><button>Download</button></a></td>";
    fileList += "<td><a href='/delete?file=" + fileName + "'><button>Delete</button></a></td></tr>";
    file = root.openNextFile();
  }
  fileList += "</table>";
  return fileList;
}


// Function to get the filename based on the current date
String getFilename(DateTime now) {
  return "/track/" + String(now.year()) + "-" + String(now.month()) + "-" + String(now.day()) + ".csv";
}

void setup() {
  Serial.begin(115200);
  ss.begin(9600);  // Initialize GPS baud rate

  // Set up the button pin and LED pin
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  // Define button functions
  button.attachClick(handleButtonPress);
  
  // Set up web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      String htmlContent = FPSTR(index_html);
      htmlContent.replace("%FILE_LIST%", listFilesInTrackFolder());  // Replace placeholder with file list
      request->send(200, "text/html", htmlContent);
  });

  // Handle file download
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
      String fileName = request->getParam("file")->value();
      String filePath = "/track/" + fileName;
      if (SD.exists(filePath.c_str())) {
        request->send(SD, filePath.c_str(), "application/octet-stream");
      } else {
        request->send(404, "text/plain", "File not found");
      }
    } else {
      request->send(400, "text/plain", "File parameter missing");
    }
  });

  // Handle file deletion
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
      String fileName = request->getParam("file")->value();
      String filePath = "/track/" + fileName;
      if (SD.exists(filePath.c_str())) {
        SD.remove(filePath.c_str());
        request->send(200, "text/plain", "File deleted: " + fileName);
      } else {
        request->send(404, "text/plain", "File not found");
      }
    } else {
      request->send(400, "text/plain", "File parameter missing");
    }
  });
  
  // Initialize ADC
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Initialize SD card
  if (!SD.begin()) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // Ensure the "track" folder exists
  if (!SD.exists("/track")) {
    Serial.println("Track folder not found, creating...");
    SD.mkdir("/track");
  }

  // Set the date format for the filename
  DateTime now = rtc.now();
  String filename = getFilename(now);

  // Open the file to append data; create it if it doesn't exist
  dataFile = SD.open(filename, FILE_APPEND);
  if (!dataFile) {
    dataFile = SD.open(filename, FILE_WRITE);
    if (!dataFile) {
      Serial.println("Error creating file for writing");
      return;
    }
    dataFile.println("Time,Latitude,Longitude,AIN0,VIN0");
  } else {
    Serial.println("File opened for appending");
  }
  dataFile.close(); // Close the file after setting it up
}


// Define a variable to hold the interval for data logging
unsigned long previousMillis = 0; // Store the last time data was logged
const long interval = 3000;        // Interval at which to log data (milliseconds)
void loop() {
  button.tick();
  
  // Get current time from RTC
  DateTime now = rtc.now();
  String timeString = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day()) + " "
                    + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

  // Read ADC values
  int16_t adc0 = ads.readADC_SingleEnded(0);
  int16_t adc1 = ads.readADC_SingleEnded(1);
  // int16_t adc2 = ads.readADC_SingleEnded(2);
  // int16_t adc3 = ads.readADC_SingleEnded(3);

  // Read GPS data
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  // Check if valid GPS data is available
  double latitude = 0.0;
  double longitude = 0.0;
  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
  }

  // // // Prepare data to store
  // // String dataString = timeString + "," + String(latitude, 6) + "," + String(longitude, 6) + ","
  // //                   + String(adc0) + "," + String(adc1) + "," + String(adc2) + "," + String(adc3);
  String dataString = timeString + "," + String(latitude, 6) + "," + String(longitude, 6) + ","
                    + String(adc0) + "," + String(adc1);

    // Check if the time interval has passed
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Save the last time data was logged

    if(!apActive){
      // Open the file to append data
      String filename = getFilename(now);
      dataFile = SD.open(filename, FILE_APPEND);
      if (dataFile) {
        dataFile.println(dataString);
        dataFile.close();
        Serial.println(dataString); // Print to Serial Monitor for debugging
      } else {
        Serial.println("Error opening file for writing");
      }
    } else {
      Serial.print("No Store-"); Serial.println(dataString);
    }
  }
}
