#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>
#include "LCDManager.h"
#include "FingerprintManager.h"
#include "Handlers.h"
#include <RTClib.h>
#include <Wire.h>
#include <WiFi.h>
#include <Arduino.h>
#include <HTTPClient.h>

RTC_DS3231 rtc;

#define SD_CS 5
WebServer server(80);

const char *ssid = "DESKTOP-4153";
const char *password = "@__alwGhne";
bool isLoggingAttendance = false;
String currentCourse = "";

void setup()
{
  Serial.begin(9600);
  lcdInit();

  if (!rtc.begin())
    Serial.println("RTC not found!");

  WiFi.begin(ssid, password);
  Serial.println("WiFi Started");
  // DISPLAY IP ADDRESS
  while (WiFi.status() != WL_CONNECTED)
  {
    static int temp = 1;
    if (temp)
    {
      Serial.print("WiFi.status(): ");
      Serial.println(WiFi.status());
      // temp = 0;
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  show_message("Wifi connected!     ", 0);
  show_message("IP address:         ", 1);
  show_message(String(WiFi.localIP()) + "    ", 2);
  
  // Serial.println(WiFi.softAPIP());

  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS))
  {
    Serial.println("SD init failed!");
    return;
  }
  Serial.println("initialization done.");

  initFingerprintSensor();

  server.on("/set_mode", HTTP_GET, handleSetMode);
  server.on("/add_course", HTTP_GET, handleAddCourse);
  server.on("/enroll_student", HTTP_GET, handleEnrollStudent);
  server.on("/get_courses", HTTP_GET, handleGetCourses);
  server.on("/log_attendance", HTTP_GET, handleLogAttendance);
  server.on("/end_attendance", HTTP_GET, handleEndAttendance);

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  server.handleClient();

  if (isLoggingAttendance)
  {
    int id = getFingerprintMatch(); // blocking or non-blocking depending on your fingerprint library
    if (id < 0)
      return; // No match or error, skip

    // Check for duplicate
    char dateBuf[16], timeBuf[16];
    DateTime now = rtc.now();
    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", now.year(), now.month(), now.day());
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    char filename[64];
    snprintf(filename, sizeof(filename), "/attendance_%s_%s.csv", currentCourse.c_str(), dateBuf);

    File attFile = SD.open(filename, FILE_READ);
    bool isDuplicate = false;
    while (attFile && attFile.available())
    {
      String line = attFile.readStringUntil('\n');
      if (line.startsWith(String(id) + ","))
      {
        isDuplicate = true;
        break;
      }
    }
    if (attFile)
      attFile.close();

    if (isDuplicate)
    {
      Serial.printf("Duplicate entry for ID %d\n", id);
      delay(1000);
      return;
    }

    // Lookup student info
    File file = SD.open("/students.csv");
    String studentLine = "";
    while (file.available())
    {
      String line = file.readStringUntil('\n');
      if (line.startsWith(String(id) + ","))
      {
        studentLine = line;
        break;
      }
    }
    file.close();

    if (studentLine == "")
    {
      Serial.println("Student not found for ID: " + String(id));
      delay(1000);
      return;
    }

    // Save attendance
    attFile = SD.open(filename, FILE_APPEND);
    if (attFile)
    {
      attFile.print(studentLine);
      attFile.print(",");
      attFile.println(timeBuf);
      attFile.close();
    }

    // Extract parts
    int idx1 = studentLine.indexOf(',');
    int idx2 = studentLine.indexOf(',', idx1 + 1);
    int idx3 = studentLine.indexOf(',', idx2 + 1);
    int idx4 = studentLine.indexOf(',', idx3 + 1);

    String name = studentLine.substring(idx1 + 1, idx2);
    String classN = studentLine.substring(idx2 + 1, idx3);
    String gender = studentLine.substring(idx3 + 1, idx4);
    String matric = studentLine.substring(idx4 + 1);

    if (!uploadToGoogleSheet(String(id), name, classN, gender, matric, currentCourse, dateBuf, timeBuf))
    {
      Serial.println("Upload to Google Sheet failed");
    }
    else
    {
      Serial.println("Attendance logged and uploaded");
    }

    delay(2000); // Optional: give time between scans
  }
}
