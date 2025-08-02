#include "FingerprintManager.h"
#include <HardwareSerial.h>

HardwareSerial fpSerial(2); // UART2 for fingerprint
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpSerial);

void initFingerprintSensor()
{
  fpSerial.begin(57600, SERIAL_8N1, 16, 17); // TX = 17, RX = 16
  finger.begin(57600);

  if (finger.verifyPassword())
  {
    Serial.println("Fingerprint sensor found");
  }
  else
  {
    Serial.println("Fingerprint sensor NOT found");
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);
}

int getFingerprintMatch()
{
  int p = -1;
  Serial.println("Waiting for finger placement....");
  while((p = finger.getImage()) != FINGERPRINT_OK);
  // p = finger.getImage();
  // if (p != FINGERPRINT_OK)
  //   return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1;
  Serial.println("Remove finger");
  delay(2000);
  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK)
    return -1;

  return finger.fingerID; // Matched ID
}

int enrollFingerprint(uint8_t id)
{
  int p = -1;
  Serial.println("Waiting for finger placement...");

  // Step 1: Get image
  while ((p = finger.getImage()) != FINGERPRINT_OK)
    ;
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK)
    return p;

  Serial.println("Remove finger");
  delay(2000);

  while ((p = finger.getImage()) != FINGERPRINT_NOFINGER)
    ;
  Serial.println("Place same finger again");

  while ((p = finger.getImage()) != FINGERPRINT_OK)
    ;
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK)
    return p;

  p = finger.createModel();
  if (p != FINGERPRINT_OK)
    return p;

  p = finger.storeModel(id);
  return p;
}
