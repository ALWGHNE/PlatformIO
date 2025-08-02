#include "Handlers.h"
#include "FingerprintManager.h"
#include <SD.h>
#include <RTClib.h>
#include <WiFiClientSecure.h>
#include "LCDManager.h"
extern RTC_DS3231 rtc;
String currentMode = "idle";

String urlencode(String str) {
  String encoded = "";
  char c;
  char code0, code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      encoded += '%';
      code0 = (c >> 4) & 0xF;
      code1 = c & 0xF;
      encoded += char(code0 > 9 ? code0 - 10 + 'A' : code0 + '0');
      encoded += char(code1 > 9 ? code1 - 10 + 'A' : code1 + '0');
    }
  }
  return encoded;
}

// https://script.google.com/macros/s/AKfycbw4vYmO0V2GaW_mqw4CpWz6Ie-OyrlPqBaIOGZuG8f-KDdvJlk7NyJAkbYb0qfU-KXh/exec
bool uploadToGoogleSheet(String id, String name, String className, String gender, String matric, String course, String date, String time) {
  WiFiClientSecure client;
  client.setInsecure(); // Skip SSL certificate check (dev only)

  const char* host = "script.google.com";
  const int httpsPort = 443;

  // Replace with your actual script ID (from Apps Script)
  String baseUrl = "/macros/s/AKfycbw4vYmO0V2GaW_mqw4CpWz6Ie-OyrlPqBaIOGZuG8f-KDdvJlk7NyJAkbYb0qfU-KXh/exec";

  // Build query string
  String query = "?fingerprintId=" + id +
                 "&name=" + urlencode(name) +
                 "&class=" + urlencode(className) +
                 "&gender=" + urlencode(gender) +
                 "&matric=" + urlencode(matric) +
                 "&course=" + urlencode(course) +
                 "&date=" + urlencode(date) +
                 "&time=" + urlencode(time);

  String fullUrl = baseUrl + query;

  Serial.println("Connecting to: " + String(host) + fullUrl);

  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return false;
  }

  // Send GET request
  client.println("GET " + fullUrl + " HTTP/1.1");
  client.println("Host: " + String(host));
  client.println("Connection: close");
  client.println();

  // Wait for response
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break; // headers done
  }

  String response = client.readString();
  Serial.println("Google Sheet Response:");
  Serial.println(response);

  return response.indexOf("OK") >= 0;
}


String extractValue(String body, String key) {
  int start = body.indexOf(key + "=");
  if (start == -1) return "";
  start += key.length() + 1;
  int end = body.indexOf("&", start);
  if (end == -1) end = body.length();
  return body.substring(start, end);
}

void handleLogAttendance() {
  if (!server.hasArg("course")) {
    server.send(400, "text/plain", "Missing course name");
    return;
  }

  currentCourse = server.arg("course");
  isLoggingAttendance = true;
  server.send(200, "text/plain", "Attendance logging started");
}



void handleSetMode() {
  if (server.hasArg("mode")) {
    String modeParam = server.arg("mode");

    if (modeParam == "enroll") currentMode = "enroll";
    else if (modeParam == "record") currentMode = "record";
    else currentMode = "idle";

    showMode(currentMode); // Update display or UI
    server.send(200, "text/plain", "Mode set to: " + currentMode);
    Serial.println("Mode: " + currentMode);
  } else {
    server.send(400, "text/plain", "Missing mode parameter");
    
  }
}

void handleAddCourse() {
  if (!server.hasArg("course")) {
    server.send(400, "text/plain", "Course name missing");
    return;
  }

  String courseName = server.arg("course");

  File file = SD.open("/courses.csv", FILE_APPEND);
  if (file) {
    file.println(courseName);
    file.close();
    server.send(200, "text/plain", "Course added: " + courseName);
  } else {
    server.send(500, "text/plain", "Failed to write to SD");
  }
}

void handleEnrollStudent() {
  // Ensure required fields are present
  if (!server.hasArg("name") || !server.hasArg("matric")) {
    server.send(400, "text/plain", "Missing required fields");
    return;
  }

  String name   = server.arg("name");
  String clasz  = server.hasArg("class")  ? server.arg("class")  : "";
  String gender = server.hasArg("gender") ? server.arg("gender") : "";
  String matric = server.arg("matric");

  // Determine next fingerprint ID
  int lastID = 0;
  File file = SD.open("/students.csv", FILE_READ);
  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
        int commaIdx = line.indexOf(',');
        if (commaIdx > 0) {
          int id = line.substring(0, commaIdx).toInt();
          if (id > lastID) lastID = id;
        }
      }
    }
    file.close();
  }

  int newID = lastID + 1;

  // Enroll fingerprint
  int result = enrollFingerprint(newID);
  if (result == FINGERPRINT_OK) {
    File file = SD.open("/students.csv", FILE_APPEND);
    if (file) {
      file.printf("%d,%s,%s,%s,%s\n", newID, name.c_str(), clasz.c_str(), gender.c_str(), matric.c_str());
      file.close();
      server.send(200, "text/plain", "Student enrolled with ID: " + String(newID));
    } else {
      server.send(500, "text/plain", "Failed to write to SD");
    }
  } else {
    server.send(500, "text/plain", "Fingerprint enrollment failed");
  }
}


void handleGetCourses() {
  File file = SD.open("/courses.csv", FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "Courses not found");
    return;
  }

  String content = "";
  while (file.available()) {
    content += file.readStringUntil('\n') + "\n";
  }
  file.close();
  server.send(200, "text/plain", content);
}

void handleEndAttendance() {
  isLoggingAttendance = false;
  currentCourse = "";
  server.send(200, "text/plain", "Attendance logging ended");
}
