#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <RTClib.h>
#include <SD.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

Adafruit_ADS1115 ads;  // Use this for the 16-bit version
RTC_DS1307 rtc;        // Create an RTC object
TinyGPSPlus gps;       // Create GPS object

// Define GPS TX and RX pins
SoftwareSerial ss(27, 26); // RX, TX (connect Neo6M TX to pin 4, RX to pin 3)

const int chipSelect = 5; // SD card chip select pin
File dataFile;

// Function to get the filename based on the current date
String getFilename(DateTime now) {
  return "/track/" + String(now.year()) + "-" + String(now.month()) + "-" + String(now.day()) + ".csv";
}

void setup() {
  Serial.begin(115200);
  ss.begin(9600);  // Initialize GPS baud rate

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
    dataFile.println("Time,Latitude,Longitude,AIN0,AIN1,AIN2,AIN3");
  } else {
    Serial.println("File opened for appending");
  }
  dataFile.close(); // Close the file after setting it up
}

void loop() {
  // Get current time from RTC
  DateTime now = rtc.now();
  String timeString = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day()) + " "
                    + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

  // Read ADC values
  int16_t adc0 = ads.readADC_SingleEnded(0);
  int16_t adc1 = ads.readADC_SingleEnded(1);
  int16_t adc2 = ads.readADC_SingleEnded(2);
  int16_t adc3 = ads.readADC_SingleEnded(3);

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

  // Prepare data to store
  String dataString = timeString + "," + String(latitude, 6) + "," + String(longitude, 6) + ","
                    + String(adc0) + "," + String(adc1) + "," + String(adc2) + "," + String(adc3);
  // String dataString = timeString + "," + String(latitude, 6) + "," + String(longitude, 6) + ","
  //                 + String(adc0) ;

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

  // Wait for 3 seconds before the next reading
  delay(3000);
}
