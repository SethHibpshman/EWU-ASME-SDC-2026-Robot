#include <Servo.h>
const uint8_t servoPins[] = { 2, 3, 4, 5, 6, 7 };
const int NEUTRAL_US = 1500;
const int DEADBAND   = 100;
const int SMOOTH_W   = 8;
// DRV8833 brushed motor pins (CH2 → forward/backward PWM)
const uint8_t MOTOR_FWD = 9;
const uint8_t MOTOR_REV = 10;
#define IBUS_BUFFSIZE    32
#define IBUS_MAXCHANNELS 10
uint8_t  ibus[IBUS_BUFFSIZE] = {0};
uint16_t rcValue[IBUS_MAXCHANNELS];
uint8_t  ibusIndex = 0;
Servo steering;
Servo aux[4];
int lastThrottle = NEUTRAL_US;
// Map a 1000–2000µs RC value to 0–255 PWM, split around neutral+deadband
void driveMotor(int us) {
  int centered = us - NEUTRAL_US;
  if (abs(centered) < DEADBAND) {
    // Deadband — coast
    analogWrite(MOTOR_FWD, 0);
    analogWrite(MOTOR_REV, 0);
    return;
  }
  if (centered > 0) {
    // Forward
    int pwm = map(centered, DEADBAND, 500, 0, 255);
    analogWrite(MOTOR_FWD, constrain(pwm, 0, 255));
    analogWrite(MOTOR_REV, 0);
  } else {
    // Reverse
    int pwm = map(-centered, DEADBAND, 500, 0, 255);
    analogWrite(MOTOR_FWD, 0);
    analogWrite(MOTOR_REV, constrain(pwm, 0, 255));
  }
}
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(MOTOR_FWD, OUTPUT);
  pinMode(MOTOR_REV, OUTPUT);
  analogWrite(MOTOR_FWD, 0);   // motor off at boot
  analogWrite(MOTOR_REV, 0);
  steering.attach(servoPins[0]);
  for (int i = 0; i < 4; i++) aux[i].attach(servoPins[i + 2]);
  Serial.println("Brushed motor ready. Waiting for RC...");
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
      // ===== CH1: Steering (MG90S, ±35° from neutral) =====
      steering.writeMicroseconds(constrain(rcValue[0], 1080, 1820));
      // ===== CH2: Brushed motor via DRV8833 =====
      int raw = constrain(rcValue[1], 1000, 2000);
      int throttle = (raw * SMOOTH_W + lastThrottle) / (SMOOTH_W + 1);
      lastThrottle = throttle;
      driveMotor(throttle);
      // ===== CH3–CH6 =====
      for (int i = 0; i < 4; i++) {
        aux[i].writeMicroseconds(constrain(rcValue[i + 2], 1000, 2000));
      }
    }
  }
}