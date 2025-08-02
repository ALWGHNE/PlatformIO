#ifndef FINGERPRINT_MANAGER_H
#define FINGERPRINT_MANAGER_H

#include <Adafruit_Fingerprint.h>

void initFingerprintSensor();
int enrollFingerprint(uint8_t id);
int getFingerprintMatch(); // Returns ID or -1

#endif
