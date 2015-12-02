#include <SoftwareSerial.h>
SoftwareSerial XBee(2, 3); // RX, TX

void setup() {
  // put your setup code here, to run once:
  XBee.begin(9600);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  XBee.write("Hello, World!\n");
  Serial.write("Sent a message...\n");
  delay(1000);
}
