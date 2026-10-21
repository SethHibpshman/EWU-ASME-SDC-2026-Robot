#include <Servo.h>

const uint8_t servoPins[] = { 2, 3, 4, 5, 6, 7 };

const int NEUTRAL_US = 1500;
const int DEADBAND   = 100;
const int SMOOTH_W   = 8;

#define IBUS_BUFFSIZE    32
#define IBUS_MAXCHANNELS 10

uint8_t  ibus[IBUS_BUFFSIZE] = {0};
uint16_t rcValue[IBUS_MAXCHANNELS];
uint8_t  ibusIndex = 0;

Servo steering;
Servo esc;
Servo aux[4];

int lastThrottle    = NEUTRAL_US;
bool escArmed       = false;  // ESC stays silent until first valid packet

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  steering.attach(servoPins[0]);
  // NOTE: ESC is NOT attached yet — no signal sent on boot
  for (int i = 0; i < 4; i++) aux[i].attach(servoPins[i + 2]);

  Serial.println("Waiting for first valid RC packet before arming ESC...");
}

void loop() {
  if (Serial1.available()) {
    uint8_t val = Serial1.read();

    if (ibusIndex == 0 && val != 0x20) return;
    if (ibusIndex == 1 && val != 0x40) { ibusIndex = 0; return; }

    ibus[ibusIndex++] = val;

    if (ibusIndex >= IBUS_BUFFSIZE) {
      ibusIndex = 0;

      // Checksum
      uint16_t checksum = 0xFFFF;
      for (int i = 0; i < 30; i++) checksum -= ibus[i];
      uint16_t rxChecksum = ((uint16_t)ibus[31] << 8) | ibus[30];
      if (checksum != rxChecksum) return;

      // Parse channels
      int high = 3, low = 2;
      for (int i = 0; i < IBUS_MAXCHANNELS; i++) {
        rcValue[i] = ((uint16_t)ibus[high] << 8) | ibus[low];
        high += 2; low += 2;
      }

      // ===== ARM ESC on first valid packet =====
      // At this point the receiver is locked and CH2 is a real centered value
      if (!escArmed) {
        int firstRead = constrain(rcValue[1], 1000, 2000);
        Serial.print("First valid packet. CH2=");
        Serial.print(firstRead);
        Serial.println(" — arming ESC now.");

        esc.attach(servoPins[1]);         // attach AFTER we have a real signal
        esc.writeMicroseconds(firstRead); // send true center as first value
        lastThrottle = firstRead;
        escArmed = true;
        delay(500);                       // give ESC 500ms to latch neutral
        Serial.println("ESC armed.");
        return;
      }

      // ===== CH1: Steering =====
      steering.writeMicroseconds(constrain(rcValue[0], 1000, 2000));

      // ===== CH2: ESC =====
      int raw = constrain(rcValue[1], 1000, 2000);
      int throttle = (raw * SMOOTH_W + lastThrottle) / (SMOOTH_W + 1);
      lastThrottle = throttle;
      if (abs(throttle - NEUTRAL_US) < DEADBAND) throttle = NEUTRAL_US;
      esc.writeMicroseconds(throttle);

      // ===== CH3-CH6 =====
      for (int i = 0; i < 4; i++) {
        aux[i].writeMicroseconds(constrain(rcValue[i + 2], 1000, 2000));
      }
    }
  }
}