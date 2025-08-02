#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define ACTIVATE(PIN) digitalWrite(PIN, HIGH)
#define DEACTIVATE(PIN) digitalWrite(PIN, LOW)
#define CONFIGURE_AS_OUTPUT(PIN) pinMode(PIN, OUTPUT)

#define RELAY1 4
#define RELAY2 16
#define RELAY3 23
#define RELAY4 19

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

String commands[] = {
    "RL1_OFF", "RL1_ON",
    "RL2_OFF", "RL2_ON",
    "RL3_OFF", "RL3_ON",
    "RL4_OFF", "RL4_ON",
    "ALL_OFF", "ALL_ON",
    "STATUS"};
String response[] = {
    "RL1_OFF_OK",
    "RL1_ON_OK",
    "RL2_OFF_OK",
    "RL2_ON_OK",
    "RL3_OFF_OK",
    "RL3_ON_OK",
    "RL4_OFF_OK",
    "RL4_ON_OK",
    "ALL_ON_OK",
    "ALL_OFF_OK",
    ""};
int pins[] = {RELAY1, RELAY2, RELAY3, RELAY4};
void setup()
{
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.print("Testing....");
  delay(2000);
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Testing....");
  CONFIGURE_AS_OUTPUT(RELAY1);
  CONFIGURE_AS_OUTPUT(RELAY2);
  CONFIGURE_AS_OUTPUT(RELAY3);
  CONFIGURE_AS_OUTPUT(RELAY4);

  DEACTIVATE(RELAY1);
  DEACTIVATE(RELAY2);
  DEACTIVATE(RELAY3);
  DEACTIVATE(RELAY4);
}

void loop()
{
  static String input = "";
  // lcd.clear();
  while (Serial.available())
  {
    char ch = Serial.read();
    lcd.print(ch);
    // End of command (newline)
    if (ch == '\n')
    {
      input.trim(); // Remove any trailing CR or space
      lcd.print(input);
      for (int i = 0; i < (sizeof(commands) / sizeof(commands[0])); i++)
      {
        if (input == commands[i])
        {

          switch (i)
          {
          case 0:
          case 1:
            digitalWrite(pins[0], i % 2);
            break;
          case 2:
          case 3:
            digitalWrite(pins[1], i % 2);
            break;
          case 4:
          case 5:
            digitalWrite(pins[2], i % 2);
            break;
          case 6:
          case 7:
            digitalWrite(pins[3], i % 2);
            break;
          case 8:
          case 9:
            for (int pin = 0; pin < (sizeof(pins) / sizeof(pins[0])); pin++)
              digitalWrite(pins[pin], i % 2);
            break;
          case 10:
            // status.
            String result = "RL1=";
            result += digitalRead(pins[0]) ? "ON" : "OFF";
            result += ",RL2=";
            result += digitalRead(pins[1]) ? "ON" : "OFF";
            result += ",RL3=";
            result += digitalRead(pins[2]) ? "ON" : "OFF";
            result += ",RL4=";
            result += digitalRead(pins[3]) ? "ON" : "OFF";
            Serial.println(result);
            break;
          }
          Serial.println(response[i]);
        }
      }
      input = "";
    }
    else
    {
      input += ch;
    }
  }
  // // put your main code here, to run repeatedly:
  // static String incomingString = "";

  // while (Serial.available())
  // {
  //   lcd.write(Serial.read());
  //   incomingString = Serial.readString();

  // }
}
