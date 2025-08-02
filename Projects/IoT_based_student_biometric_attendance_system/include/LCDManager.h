#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <Arduino.h>

void lcdInit();
void showStartup();
void show_message(String message, int row);
void showMode(const String& mode);
void showEnrollStatus(const String& id, const String& result);
void showAttendanceStatus(const String& name, const String& course, const String& result);
void showError(const String& message);
void showMessage(const String& line0, const String& line1 = "", const String& line2 = "", const String& line3 = "");

#endif
