#include "LCDManager.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

void lcdInit() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  showStartup();
}

void showStartup() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ESP32 Attendance");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void showMode(const String& mode) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode: " + mode);
  lcd.setCursor(0, 1);
  if (mode == "ENROLL") {
    lcd.print("Use app to enroll");
  } else if (mode == "RECORD") {
    lcd.print("Scan fingerprint...");
  } else {
    lcd.print("Waiting...");
  }
}

void showEnrollStatus(const String& id, const String& result) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enrolling ID: " + id);
  lcd.setCursor(0, 1);
  lcd.print(result);
}

void showAttendanceStatus(const String& name, const String& course, const String& result) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Course: " + course);
  lcd.setCursor(0, 1);
  lcd.print("Student: " + name);
  lcd.setCursor(0, 2);
  lcd.print("Status: " + result);
}

void showError(const String& message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROR:");
  lcd.setCursor(0, 1);
  lcd.print(message.substring(0, 20));
  if (message.length() > 20) {
    lcd.setCursor(0, 2);
    lcd.print(message.substring(20, 40));
  }
}

void showMessage(const String& line0, const String& line1, const String& line2, const String& line3) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(line0);
  lcd.setCursor(0, 1); lcd.print(line1);
  lcd.setCursor(0, 2); lcd.print(line2);
  lcd.setCursor(0, 3); lcd.print(line3);
}

void show_message(String message, int row) {
  lcd.setCursor(0, row);
  lcd.print(message);
}
