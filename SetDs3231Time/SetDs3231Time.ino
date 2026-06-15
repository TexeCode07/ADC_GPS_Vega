#include "Arduino.h"
#include <Wire.h>
#include <RTClib.h>


// Note: Define the Current date and time only needed once if you set already then 
// just comment out the function rtc.adjust()
const int currentYear    = 2024;
const int currentMonth   = 10;
const int currentDay     = 21;
const int currentHour    = 22;
const int currentMinute  = 55;
const int currentSecond  = 0;

RTC_DS1307 rtc; // Create an RTC object

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to open

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Comment this line if you set already no need to set again and again
  // Set the date and time here
  // rtc.adjust(DateTime(currentYear, currentMonth, currentDay, currentHour, currentMinute, currentSecond)); // Format: (year, month, day, hour, minute, second)

  // Print current time to confirm it has been set
  printTime();
}

void loop() {
  DateTime now = rtc.now();
  printTime();
  delay(1000); // Delay to prevent checking too frequently
}

void printTime() {
  DateTime now = rtc.now();

  Serial.print("Current time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
  delay(3000);
}