#include <AlashUltrasonic.h>

const uint8_t TRIGGER_PIN = A5;
const uint8_t ECHO_PIN = A4;
AlashUltrasonic sensor(TRIGGER_PIN, ECHO_PIN);

void setup() {
  Serial.begin(9600);
  sensor.begin();
}

void loop() {
  float distance = sensor.getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(1000);
}
