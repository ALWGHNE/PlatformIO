#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#define MAX_DUTY 100
#define DT 2
#define VBAT A2
#define VSOL A7
#define ERR_VALUE 0.945  // 1.022857

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define R_RATIO 11  //(R1+R2)/R2
#define MIN_VSOL 15.0
#define MAX_VBAT 12.6

float vbat = 0;
float vsol = 0;
int active = 0;
void set_duty_cycle(int dutyCycle);
void read_voltage(void);
void lcd_print(void);
void shutdown_PWM(void);
void configure_PWM(void);

void setup() {
  Serial.begin(9600);
  Serial.println("PWM TEST.");

  DDRB |= _BV(DDB1) | _BV(DDB2);             // pwm pins configured as output pins.
  PORTB &= ~(_BV(PORTB1)) & ~(_BV(PORTB2));  // pwm pins deactivated.

  lcd.init();
  lcd.backlight();

  configure_PWM();
}

void loop() {
  // Solar voltage is above minimum voltage required to charge the battery.
  // Duty cycle = vout/vin * 100
  static int previous_mode = 0;
  lcd_print();
  read_voltage();
  if (vsol < MIN_VSOL) {
    active = 0;
    previous_mode = active;
    shutdown_PWM();
    Serial.println("Solar voltage is below minimum voltage required to charge the battery.");
    PORTB &= ~(_BV(PORTB1)) & ~(_BV(PORTB2));  // pwm pins deactivated.
  } else {
    active = 1;
    if(active && previous_mode == 0)
    {
      //active was = 0 previously.
      configure_PWM();
      previous_mode = active;
    }
    set_duty_cycle((int)(MAX_VBAT / vsol * MAX_DUTY));
    Serial.print("Duty Cycle: ");
    Serial.println((int)(MAX_VBAT / vsol * MAX_DUTY));
  }
}
void set_duty_cycle(int dutyCycle) {
  if (dutyCycle <= ((MAX_DUTY / 2) - DT)) {
    // Serial.println("If condition true.");
    OCR1A = dutyCycle - DT;
    OCR1B = dutyCycle + DT;
  } else {
    // Serial.println("Else statement executed.");
    OCR1A = dutyCycle - DT;
    // OCR1B = MAX_DUTY - (dutyCycle + DT);
    OCR1B = dutyCycle + DT;
  }
}

void read_voltage(void) {
  // read voltage.
  float iteration = 50.0;
  vbat = 0;
  vsol = 0;
  for (int i = 0; i < iteration; i++) {
    vbat += analogRead(VBAT);
    vsol += analogRead(VSOL);
  }
  vbat = vbat / iteration;
  vsol = vsol / iteration;
  vbat = (vbat * 5.37) / 1023.0;  // the peak voltage on the arduino nano is not 5.0, its 5.37 instead.
  vsol = (vsol * 5.32) / 1023.0;
  vbat = vbat * R_RATIO * ERR_VALUE;  // the error value is used to compensate for the tolerance of the resistors used.
  vsol = vsol * R_RATIO * ERR_VALUE;

  Serial.print("VBAT: ");
  Serial.print(vbat);
  Serial.println("V");
  Serial.print("VSOL: ");
  Serial.print(vsol);
  Serial.println("V");
}

void configure_PWM(void) {
  TCCR1A |= _BV(COM1A1) | _BV(COM1B0) | _BV(COM1B1);
  TCCR1A &= ~(_BV(WGM11)) & ~(_BV(WGM10)) & ~(_BV(COM1A0));
  TCCR1B |= _BV(WGM13) | _BV(CS10);
  TCCR1B &= ~(_BV(WGM12)) & ~(_BV(CS12)) & ~(_BV(CS11));
  ICR1 = MAX_DUTY;
}

void shutdown_PWM(void)
{
  TCCR1A = 0;
}
void lcd_print(void) {
  static int page = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();
    switch (page) {
      case 0:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("VBAT: ");
        lcd.print(vbat);
        lcd.print("V");
        lcd.setCursor(0, 1);
        lcd.print("VSOL: ");
        lcd.print(vsol);
        lcd.print("V");
        break;
      case 1:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  CURRENT MODE  ");
        if (active) {
          lcd.setCursor(0, 1);
          lcd.print("CHARGING @ " + String((int)(MAX_VBAT / vsol * MAX_DUTY)) + "DC");
        } else {
          lcd.setCursor(0, 1);
          lcd.print("  IDLE MODE!!!!");
        }
        break;
      default:
        break;
    }
    page++;
    if (page == 2) {
      page = 0;
    }
  }
}