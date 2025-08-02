
#include <Wire.h>
#include <WiFi.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <TinyGPSPlus.h>
#include <LiquidCrystal_I2C.h>

#define NEO_6M_RX 16
#define NEO_6M_TX 17
#define GPSBaud 9600
#define ERROR_VALUE 0.694
#define VBAT_REF_PIN 34
#define VCHG_REF_PIN 35

#define INTERVAL 60000

#define MIN_VBAT 7.0
#define MIN_CHG_VOLT MIN_VBAT + 2
#define PWR_SW_PIN 25
#define BACKLIGHT_BTN_PIN 26

#define G_LED 27
#define R_LED 14

#define LCD_DELAY 3000
// #define BLANK_LCD "                "     //1602 display
#define BLANK_LCD "                    " // 2004 display
#define LCD_ROW 4
#define LCD_COLUMN 20
unsigned long old_millis = 0;

LiquidCrystal_I2C lcd(0x27, LCD_COLUMN, LCD_ROW); // set the LCD address to 0x27 for a 16 chars and 2 line display
TinyGPSPlus gps;                                  // The TinyGPSPlus object

const int sendInterval = 2000;
boolean internet_connectivity = false;
boolean new_GPS_data = false;
boolean pwr_switch = false;

// ENTER_GOOGLE_DEPLOYMENT_ID
const char *ssid = "IoT_Tracker";
const char *password = "iot@tracker_system";
String GOOGLE_SHEET_ID = "AKfycby4JLitl0UbhXOeK2FQfLWIyryXOVRTjRIJIA0eE4UIcDwMwh2qFIXnFsUn2qzdqR18";

// String time_value = "", date_value = "", latitude_value = "", longitude_value = "";
// String gs_url = "https://script.google.com/macros/s/" + GOOGLE_SHEET_ID + "/exec?time=" + time_value + "&date=" + date_value + "&latitude=" + latitude_value + "&longitude=" + longitude_value;

void displayInfo();
String get_time(void);
String get_date(void);
float read_dc_volt(int pin);
void print_location_details(void);
void write_to_google_sheet(String params);
void LCD_WRITE(String msg);
void welcome_display(String message);
void TITLE_DISPLAY(void);
void LCD_PRINT(String msg, int current_row);
void WiFi_connection(void);
void setup()
{
  Serial.begin(9600);
  // Serial2.begin(GPSBaud, SERIAL_8N1, NEO_6M_RX, NEO_6M_TX);
  Serial2.begin(9600);

  pinMode(PWR_SW_PIN, INPUT);
  pinMode(BACKLIGHT_BTN_PIN, INPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(R_LED, OUTPUT);
  digitalWrite(G_LED, LOW);
  digitalWrite(R_LED, LOW);

  while (digitalRead(PWR_SW_PIN))
    ;
  lcd.init(); // initialize the lcd
  lcd.backlight();
  TITLE_DISPLAY();
  //--------------------------------------------
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi_connection();
  //--------------------------------------------
}

void loop()
{
  if (read_dc_volt(VCHG_REF_PIN) > MIN_CHG_VOLT)
  {
    // CHARGING SOURCE AVAILABLE
    digitalWrite(R_LED, HIGH);
    digitalWrite(G_LED, LOW);
  }

  static unsigned long WiFi_old_millis = 0;
  long wifi_wait_time = 5000;
  if ((WiFi.status() != WL_CONNECTED) && millis() >= (WiFi_old_millis + wifi_wait_time))
  {
    WiFi_connection();
    WiFi_old_millis = millis();
  }

  (digitalRead(PWR_SW_PIN)) ? pwr_switch = false : pwr_switch = true;
  if (millis() >= (old_millis + INTERVAL) && new_GPS_data && internet_connectivity && pwr_switch)
  {
    old_millis = millis();
    new_GPS_data = false;
    print_location_details();
  }
  for (unsigned long start = millis(); millis() - start < 2000;)
  {
    while (Serial2.available())
    {
      // Serial.print(".");
      if (gps.encode(Serial2.read()))
      {
        new_GPS_data = true;
        displayInfo();
      }
    }
  }
}

String get_date(void)
{
  String result = "";

  if (gps.date.isValid())
  {
    if (gps.date.month() < 10)
      result += String("0");
    result += String(gps.date.month());
    result += String("/");
    if (gps.date.day() < 10)
      result += String("0");
    result += String(gps.date.day());
    result += String("/");
    result += String(gps.date.year());
  }
  return result;
}

String get_time(void)
{
  String result = "";
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10)
      result += String("0");
    result += String(gps.time.hour() + 1);
    result += String(":");
    if (gps.time.minute() < 10)
      result += String("0");
    result += String(gps.time.minute());
    result += String(":");
    if (gps.time.second() < 10)
      result += String("0");
    result += String(gps.time.second());
  }
  return result;
}

void displayInfo()
{
  Serial.print(F("Location: "));
  LCD_PRINT("IoT Tracking System ", 0);
  if (gps.time.isValid() && gps.date.isValid())
    LCD_PRINT((get_time() + "  " + get_date()), 1);
  else
    LCD_PRINT("GPS MODULE INIT.....", 1);

  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
    LCD_PRINT("LATITUDE:  " + String(gps.location.lat(), 6), 2);
    LCD_PRINT("LONGITUDE: " + String(gps.location.lng(), 6), 3);
  }
  else
  {
    Serial.print(F("INVALID"));
    LCD_PRINT("LATITUDE: INVALID   ", 2);
    LCD_PRINT("LONGITUDE: INVALID  ", 3);
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.hour() + 1);
    Serial.print(F(":"));
    if (gps.time.minute() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}

float read_dc_volt(int pin)
{
  // Serial.print("Voltage at bat pin: ");
  // // Serial.print(read_dc_volt(34));
  // Serial.print(read_dc_volt(35));
  // Serial.println("V");

  int adcValue = 0;
  float result = 0;
  const int average = 100;
  const float resistorRatio = 5.5454; // R1 = 10k, R2 = 2k2;

  for (int i = 0; i < average; i++)
    adcValue += analogRead(pin);

  adcValue = adcValue / average;

  result = adcValue / 4095.0;
  result = result * 5.0;
  result = result * resistorRatio;
  result = result * ERROR_VALUE;
  return result;
}

void print_location_details(void)
{
  if (gps.location.isValid())
  {
    // String gps_speed = String(gps.speed.kmph());
    Serial.println(gps.location.lat(), 6);
    Serial.println(gps.location.lng(), 6);

    String param;
    param = "latitude=" + String(gps.location.lat(), 6);
    param += "&longitude=" + String(gps.location.lng(), 6);
    param += "&time=" + get_time();
    param += "&date=" + get_date();

    Serial.println(param);
    write_to_google_sheet(param);
  }
  else
  {
    Serial.println("Invalid GPS data.");
  }
}

void write_to_google_sheet(String params)
{
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GOOGLE_SHEET_ID + "/exec?" + params;
  // Serial.print(url);
  Serial.println("Sending GPS data to Google Sheet");
  LCD_PRINT("Sending data 2 cloud", 1);
  //---------------------------------------------------------------------
  // starts posting data to google sheet
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  //---------------------------------------------------------------------
  // getting response from google sheet
  String payload;
  if (httpCode > 0)
  {
    payload = http.getString();
    Serial.println("Payload: " + payload);
    LCD_PRINT("Data sent!          ", 1);
    delay(2000);
    LCD_PRINT(payload, 1);
  }
  //---------------------------------------------------------------------
  http.end();
}

void LCD_WRITE(String msg)
{
  delay(200);
  static int current_row = 0;
  lcd.setCursor(0, current_row);
  lcd.print(BLANK_LCD);
  lcd.setCursor(0, current_row);
  lcd.print(msg);
  current_row++;
  if (current_row == LCD_ROW)
    current_row = 0;
}

void LCD_PRINT(String msg, int current_row)
{
  // delay(200);
  // lcd.setCursor(0, current_row);
  // lcd.print(BLANK_LCD);
  lcd.setCursor(0, current_row);
  lcd.print(msg);
}

void welcome_display(String message)
{
  int message_length = message.length();
  int row_length = message_length / LCD_COLUMN;
  // int page = row_length/LCD_ROW;

  for (int i = 0; i < row_length; i++)
  {
    LCD_WRITE(message.substring((i * LCD_COLUMN), ((i + 1) * LCD_COLUMN)));
    if ((row_length + 1) % LCD_ROW == 0)
    {
      delay(LCD_DELAY);
      lcd.clear();
    }
  }
}

void TITLE_DISPLAY(void)
{
  LCD_WRITE("DESIGN AND CONSTRUCT");
  LCD_WRITE("ION OF AN IoT BASED ");
  LCD_WRITE("VISITOR TRACKING SYS");
  LCD_WRITE("BY KADIR VICTORIA ");
  delay(LCD_DELAY);
  LCD_WRITE("MATRIC NO: 20/0812");
  LCD_WRITE("DEPARTMENT OF ELECTR");
  LCD_WRITE("ICAL AND ELECTRONICS");
  LCD_WRITE("ENGINEERING, FACULTY");
  delay(LCD_DELAY);
  lcd.clear();
  LCD_WRITE("OF ENGINEERING.");
}

void WiFi_connection(void)
{
  unsigned long setup_millis = millis();
  const int wait_time = 10000;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  LCD_PRINT("Connecting to WiFi..", 0);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
    if (millis() >= (setup_millis + wait_time))
    {
      break;
    }
  }

  (WiFi.status() != WL_CONNECTED) ? internet_connectivity = 0 : internet_connectivity = 1;

  if (internet_connectivity)
  {
    Serial.println("Connected!");
    LCD_PRINT("Connection success. ", 1);
  }

  else
  {
    Serial.println("Not connected!");
    LCD_PRINT("Connection failed!  ", 1);
    LCD_PRINT("Turn ON the router! ", 2);
  }
  delay(2000);
}
/*
 * TO BE ADDED:
 * LCD DISPLAY
 * BATTERY MANAGEMENT DURING CHARGING + BATTERY LOW ALERT
 * CHARGING INDICATION WITH LED
 * PUSHBUTTON FOR BACKLIGHT
 * ANDROID APP
 */