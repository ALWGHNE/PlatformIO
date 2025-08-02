#include <Arduino.h>

#define MAX_DUTY 100
#define DT 2
#define VBAT  A2
#define VSOL  A7

void set_duty_cycle(int dutyCycle);
void read_voltage(void);
void setup()
{
  Serial.begin(9600);
  Serial.println("PWM TEST.");

  DDRB |= _BV(DDB1) | _BV(DDB2);            // pwm pins configured as output pins.
  PORTB &= ~(_BV(PORTB1)) & ~(_BV(PORTB2)); // pwm pins deactivated.

  TCCR1A |= _BV(COM1A1) | _BV(COM1B0) | _BV(COM1B1);
  TCCR1A &= ~(_BV(WGM11)) & ~(_BV(WGM10)) & ~(_BV(COM1A0));
  TCCR1B |= _BV(WGM13) | _BV(CS10);
  TCCR1B &= ~(_BV(WGM12)) & ~(_BV(CS12)) & ~(_BV(CS11));

  ICR1 = MAX_DUTY;
}

void loop()
{
    for(int i = 10; i<= 90; i+=1) {
    set_duty_cycle(i);
    delay(10);
  }
  delay(100);
    for(int i = 90; i>= 10; i-=1) {
    set_duty_cycle(i);
    delay(10);
  }
}
void set_duty_cycle(int dutyCycle)
{
  if (dutyCycle <= ((MAX_DUTY / 2) - DT))
  {
    // Serial.println("If condition true.");
    OCR1A = dutyCycle - DT;
    OCR1B = dutyCycle + DT;
  }
  else
  {
    // Serial.println("Else statement executed.");
    OCR1A = dutyCycle - DT;
    // OCR1B = MAX_DUTY - (dutyCycle + DT);
    OCR1B = dutyCycle + DT;
  }
}

void read_voltage(void)
{
  //read voltage.
}