#ifndef HANDLERS_H
#define HANDLERS_H

#include <WebServer.h>

extern WebServer server;
extern bool isLoggingAttendance;
extern String currentCourse;
void handleSetMode();
void handleAddCourse();
void handleEnrollStudent();
void handleGetCourses();
void handleLogAttendance();
bool uploadToGoogleSheet(String id, String name, String className, String gender, String matric, String course, String date, String time);
void handleEndAttendance();

#endif
