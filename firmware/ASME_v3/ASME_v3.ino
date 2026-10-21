#include <string.h>
#include <Servo.h>

#define IBUS_BUFFSIZE 32    
#define IBUS_MAXCHANNELS 10

static uint8_t ibusIndex = 0;
static uint8_t ibus[IBUS_BUFFSIZE] = {0};
static uint16_t rcValue[IBUS_MAXCHANNELS];

static boolean rxFrameDone;

// ===== SERVO OBJECTS =====
Servo ch1;
Servo ch2;
Servo ch3;
Servo ch4;
Servo ch5;
Servo ch6;
Servo ch7;
Servo ch8;
Servo ch9;
Servo ch10;

Servo esc;   // <<< ESC SERVO

void setup()
{
  Serial.begin(115200);

  ch1.attach(2, 500, 2500);
  ch2.attach(3, 500, 2500);
  ch3.attach(4, 500, 2500);
  ch4.attach(5, 500, 2500);
  ch5.attach(6, 500, 2500);
  ch6.attach(7, 500, 2500);
  ch7.attach(8, 500, 2500);
  ch8.attach(9, 500, 2500);
  ch9.attach(10, 500, 2500);
  ch10.attach(11, 500, 2500);

  // ===== ESC SETUP =====
  esc.attach(12, 1000, 2000);     // ESC signal pin

  // ARM ESC (throttle low)
  esc.writeMicroseconds(1000);
  delay(3000);   // wait for ESC beeps
}

void loop()
{
  readRx();
}

void readRx()
{
  if (Serial.available())
  {
    uint8_t val = Serial.read();

    // Look for IBUS header 0x2040
    if (ibusIndex == 0 && val != 0x20) { ibusIndex = 0; return; }
    if (ibusIndex == 1 && val != 0x40) { ibusIndex = 0; return; }

    if (ibusIndex == IBUS_BUFFSIZE)
    {
      ibusIndex = 0;

      int high = 3;
      int low  = 2;

      for (int i = 0; i < IBUS_MAXCHANNELS; i++)
      {
        rcValue[i] = (ibus[high] << 8) + ibus[low];
        high += 2;
        low  += 2;
      }

      // ===== NORMAL SERVOS =====
      ch1.writeMicroseconds(map(rcValue[0], 988, 2012, 500, 2500));
      ch2.writeMicroseconds(map(rcValue[1], 988, 2012, 500, 2500));
      ch3.writeMicroseconds(map(rcValue[2], 988, 2012, 500, 2500));
      ch4.writeMicroseconds(map(rcValue[3], 988, 2012, 500, 2500));
      ch5.writeMicroseconds(map(rcValue[4], 988, 2012, 500, 2500));
      ch6.writeMicroseconds(map(rcValue[5], 988, 2012, 500, 2500));
      ch7.writeMicroseconds(map(rcValue[6], 988, 2012, 500, 2500));
      ch8.writeMicroseconds(map(rcValue[7], 988, 2012, 500, 2500));
      ch9.writeMicroseconds(map(rcValue[8], 988, 2012, 500, 2500));
      ch10.writeMicroseconds(map(rcValue[9], 988, 2012, 500, 2500));

      // ===== ESC CONTROL (CHANNEL 3) =====
      int throttle = constrain(rcValue[2], 1000, 2000);
      esc.writeMicroseconds(throttle);

      // ===== DEBUG =====
      Serial.print("Throttle: ");
      Serial.println(throttle);
    }
    else
    {
      ibus[ibusIndex++] = val;
    }
  }
}
