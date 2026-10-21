#include <Servo.h>

const int NEUTRAL_US = 1500;
const int DEADBAND   = 100;
const int SMOOTH_W   = 8;

// Motor pins
const uint8_t MOTOR_FWD = 9;
const uint8_t MOTOR_REV = 13;

#define IBUS_BUFFSIZE    32
#define IBUS_MAXCHANNELS 10
uint8_t  ibus[IBUS_BUFFSIZE] = {0};
uint16_t rcValue[IBUS_MAXCHANNELS];
uint8_t  ibusIndex = 0;

Servo steering;    // D2  — CH1, MG90S ±35°
Servo aux;         // D4  — CH3
Servo gripperServo; // D3 — CH5, MG90S 0–90°
Servo shoulderServo;// D5 — CH6, MG995 slowed

int lastThrottle = NEUTRAL_US;
int shoulderPos  = 90;

void driveMotor(int us) {
  int centered = us - NEUTRAL_US;
  if (abs(centered) < DEADBAND) {
    analogWrite(MOTOR_FWD, 0);
    analogWrite(MOTOR_REV, 0);
    return;
  }
  if (centered > 0) {
    int pwm = map(centered, DEADBAND, 500, 0, 255);
    analogWrite(MOTOR_FWD, constrain(pwm, 0, 255));
    analogWrite(MOTOR_REV, 0);
  } else {
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
  analogWrite(MOTOR_FWD, 0);
  analogWrite(MOTOR_REV, 0);
  steering.attach(2);
  aux.attach(4);
  gripperServo.attach(3);
  shoulderServo.attach(5);
  shoulderServo.write(shoulderPos);
  Serial.println("Ready. Waiting for RC...");
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
      // ===== CH1: Steering (MG90S, ±35°) =====
      steering.writeMicroseconds(constrain(rcValue[0], 1080, 1820));
      // ===== CH2: Brushed motor =====
      int raw = constrain(rcValue[1], 1000, 2000);
      int throttle = (raw * SMOOTH_W + lastThrottle) / (SMOOTH_W + 1);
      lastThrottle = throttle;
      driveMotor(throttle);
      // ===== CH3: Aux =====
      aux.writeMicroseconds(constrain(rcValue[2], 1000, 2000));
      // ===== CH5: Gripper (MG90S, 0–90°) =====
      int gripAngle = map(constrain(rcValue[4], 1000, 2000), 1000, 2000, 0, 90);
      gripperServo.write(gripAngle);
      // ===== CH6: Shoulder (MG995, slowed) =====
      int targetPos = map(constrain(rcValue[5], 1000, 2000), 1000, 2000, 0, 180);
      if (shoulderPos < targetPos) shoulderPos++;
      else if (shoulderPos > targetPos) shoulderPos--;
      shoulderServo.write(shoulderPos);
    }
  }
}