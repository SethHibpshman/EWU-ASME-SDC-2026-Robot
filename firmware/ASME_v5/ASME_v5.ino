#include <Servo.h>

// ===== CONFIG =====
const uint8_t iBusRXPin = 0;    // Serial1 RX pin on Nano Every (connect iBUS signal here)
const uint8_t servoPins[10] = {2,3,4,5,6,7,8,9,10,11}; // Pins for servos 1–10
const uint8_t escPin = 12;      // ESC pin

// ===== DATA =====
#define IBUS_BUFFSIZE 32
#define IBUS_MAXCHANNELS 10

uint8_t ibus[IBUS_BUFFSIZE] = {0};
uint16_t rcValue[IBUS_MAXCHANNELS];
uint8_t ibusIndex = 0;

// ===== SERVOS =====
Servo ch[10];
Servo esc;

void setup() {
  Serial.begin(115200);      // debug output
  Serial1.begin(115200);     // iBUS receiver baud

  // Attach servos
  for (int i = 0; i < 10; i++) ch[i].attach(servoPins[i]);

  // Attach ESC
  esc.attach(escPin);
  esc.writeMicroseconds(1500);  // neutral to arm bidirectional ESC
  delay(3000);
  Serial.println("Setup complete, reading iBUS...");
}

void loop() {
  while (Serial1.available()) {
    uint8_t val = Serial1.read();

    // Start of packet check
    if (ibusIndex == 0 && val != 0x20) continue;
    if (ibusIndex == 1 && val != 0x40) { ibusIndex = 0; continue; }

    ibus[ibusIndex++] = val;

    if (ibusIndex >= IBUS_BUFFSIZE) {
      ibusIndex = 0;

      // Parse channels
      int high = 3;
      int low = 2;
      for (int i = 0; i < IBUS_MAXCHANNELS; i++) {
        rcValue[i] = (ibus[high] << 8) | ibus[low];
        high += 2; low += 2;
      }

      // Write to servos
      for (int i = 0; i < 10; i++) {
        ch[i].writeMicroseconds(rcValue[i]);
      }

      // Control ESC with channel 1 (index 0)
      int throttle = constrain(rcValue[0], 1000, 2000);

      // Deadband around neutral to prevent motor creep
      if (abs(throttle - 1500) < 25) throttle = 1500;

      esc.writeMicroseconds(throttle);

      // Debug output
      Serial.print("Ch1/Throttle: ");
      Serial.print(throttle);
      Serial.print("  |  Raw Ch1: ");
      Serial.print(rcValue[0]);
      if (throttle < 1475)       Serial.println("  [REVERSE]");
      else if (throttle > 1525)  Serial.println("  [FORWARD]");
      else                       Serial.println("  [STOP]");
    }
  }
}