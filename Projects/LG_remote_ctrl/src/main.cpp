#include <Arduino.h>

#define IR_SEND_PIN 3
#include <IRremote.h>

IRsend irsend;

uint32_t remoteCodes[] = { 0x20DF10EF, 0x20DF0FF0, 0x20DF10EF, 0x20DF0FF0, 0x20DFD02F, 0x20DFC23D, 0x20DFF50A, 0x20DF9C63, 0x20DF8877, 0x20DF48B7,
                           0x20DFC837, 0x20DF28D7, 0x20DFA857, 0x20DF6897, 0x20DFE817, 0x20DF18E7, 0x0DF9867, 0x20DF08F7, 0x20DFCA35, 0x20DFD52A,
                           0x20DF40BF, 0x20DFC03F, 0x20DF55AA, 0x20DF1EE1, 0x20DF906F, 0x20DF00FF, 0x20DF807F, 0x20DFAD52, 0x20DF3EC1, 0x20DF7986,
                           0x20DF14EB, 0x20DFDA25, 0x20DF02FD, 0x20DF827D, 0x20DF609F, 0x20DFE01F, 0x20DF22DD, 0x20DF04FB, 0x20DF847B, 0x20DF8976,
                           0x20DFBD42, 0x20DF8D72, 0x20DF0DF2, 0x20DF5DA2, 0x20DFF10E, 0x20DF718E, 0x20DF4EB1, 0x20DF8E71, 0x20DFC639, 0x20DF8679 
};
void setup() {
  Serial.begin(9600);
  Serial.println("Initialization");
}

void loop() {
  for (int i = 0; i < (sizeof(remoteCodes) / sizeof(remoteCodes[0])); i++) {
    irsend.sendNEC(remoteCodes[i], 32);  //Power Code
    Serial.print(i);
    Serial.println(".\t\tDone.");
    delay(5000);
  }
}
