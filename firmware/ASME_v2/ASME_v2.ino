#include <string.h>
#include <Servo.h>

#define IBUS_BUFFSIZE 32    
#define IBUS_MAXCHANNELS 10   // FlySky i6 = 10 channels max

static uint8_t ibusIndex = 0;
static uint8_t ibus[IBUS_BUFFSIZE] = {0};
static uint16_t rcValue[IBUS_MAXCHANNELS];

static boolean rxFrameDone;

int ch_width_1;
int ch_width_2;
int ch_width_3;
int ch_width_4;
int ch_width_5;
int ch_width_6;
int ch_width_7;
int ch_width_8;
int ch_width_9;
int ch_width_10;

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

void setup()
{
  Serial.begin(115200);

  // Attach servos with full pulse range
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
}

void loop()
{
  readRx();
}

void readRx()
{
  rxFrameDone = false;

  if (Serial.available())
  {
    uint8_t val = Serial.read();

    // Look for 0x2040 as start of packet
    if (ibusIndex == 0 && val != 0x20)
    {
      ibusIndex = 0;
      return;
    }
    if (ibusIndex == 1 && val != 0x40)
    {
      ibusIndex = 0;
      return;
    }

    if (ibusIndex == IBUS_BUFFSIZE)
    {
      ibusIndex = 0;

      int high = 3;
      int low  = 2;

      for(int i = 0; i < IBUS_MAXCHANNELS; i++)
      {
        rcValue[i] = (ibus[high] << 8) + ibus[low];
        high += 2;
        low  += 2;
      }

      // ===== MAP IBUS → FULL SERVO RANGE =====
      ch_width_1  = constrain(map(rcValue[0],  988, 2012, 500, 2500), 500, 2500);
      ch_width_2  = constrain(map(rcValue[1],  988, 2012, 500, 2500), 500, 2500);
      ch_width_3  = constrain(map(rcValue[2],  988, 2012, 500, 2500), 500, 2500);
      ch_width_4  = constrain(map(rcValue[3],  988, 2012, 500, 2500), 500, 2500);
      ch_width_5  = constrain(map(rcValue[4],  988, 2012, 500, 2500), 500, 2500);
      ch_width_6  = constrain(map(rcValue[5],  988, 2012, 500, 2500), 500, 2500);
      ch_width_7  = constrain(map(rcValue[6],  988, 2012, 500, 2500), 500, 2500);
      ch_width_8  = constrain(map(rcValue[7],  988, 2012, 500, 2500), 500, 2500);
      ch_width_9  = constrain(map(rcValue[8],  988, 2012, 500, 2500), 500, 2500);
      ch_width_10 = constrain(map(rcValue[9],  988, 2012, 500, 2500), 500, 2500);

      // ===== WRITE SERVOS =====
      ch1.writeMicroseconds(ch_width_1);
      ch2.writeMicroseconds(ch_width_2);
      ch3.writeMicroseconds(ch_width_3);
      ch4.writeMicroseconds(ch_width_4);
      ch5.writeMicroseconds(ch_width_5);
      ch6.writeMicroseconds(ch_width_6);
      ch7.writeMicroseconds(ch_width_7);
      ch8.writeMicroseconds(ch_width_8);
      ch9.writeMicroseconds(ch_width_9);
      ch10.writeMicroseconds(ch_width_10);

      // ===== DEBUG PRINT =====
      Serial.print(ch_width_1);  Serial.print("  ");
      Serial.print(ch_width_2);  Serial.print("  ");
      Serial.print(ch_width_3);  Serial.print("  ");
      Serial.print(ch_width_4);  Serial.print("  ");
      Serial.print(ch_width_5);  Serial.print("  ");
      Serial.print(ch_width_6);  Serial.print("  ");
      Serial.print(ch_width_7);  Serial.print("  ");
      Serial.print(ch_width_8);  Serial.print("  ");
      Serial.print(ch_width_9);  Serial.print("  ");
      Serial.print(ch_width_10); Serial.println();

      rxFrameDone = true;
      return;
    }
    else
    {
      ibus[ibusIndex] = val;
      ibusIndex++;
    }
  }
}
