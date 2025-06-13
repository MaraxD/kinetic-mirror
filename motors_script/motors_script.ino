/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>                    //install Servo library
#include <Adafruit_PWMServoDriver.h>  // adafruit pwm servo driver library
#include <Thread.h>
#include <ThreadController.h>

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);  // called this way, it uses the default address 0x40
Adafruit_PWMServoDriver board2 = Adafruit_PWMServoDriver(0x41);

ThreadController controller = ThreadController();

Thread animationThread = Thread();
Thread serialThread = Thread();

#define SERVOMIN 125  // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 625  // this is the 'maximum' pulse length count (out of 4096)


int colNumber, angle, time;
bool handDetected = false;
int startingColumnNo;

const int numColumns = 3;
int currentAngle[numColumns] = { 38 };
bool goingUp[numColumns] = { true };

unsigned long lastReceivedTime = 0;
const unsigned long handTimeout = 500;
unsigned long lastHandTime = 0;



void setup() {
  Serial.begin(115200);
  // Serial.println("16 channel Servo test!");

  board1.begin();
  board1.setOscillatorFrequency(27000000);  //Set the PWM oscillator frequency, used for fine calibration
  board1.setPWMFreq(50);

  board2.begin();
  board2.setOscillatorFrequency(27000000);
  board2.setPWMFreq(50);

  resetPanels();

  randomSeed(3);

  animationThread.onRun([]() {
    if (!handDetected) {
      delay(100);
      startingColumnNo = random(0, 6);
      secondIdleMode();
      resetPanels();
      idleMode();
      resetPanels();
      delay(100);
    }
  });
  animationThread.setInterval(50);

  serialThread.onRun([]() {
    if (Serial.available()) {

      colNumber = Serial.parseInt();
      angle = Serial.parseInt();

      if (Serial.read() == '\n') {
        handDetected = true;
        lastHandTime = millis();

        if (colNumber < 4) {

          board1.setPWM(4 * colNumber, 0, angleToPulse(angle));
          board1.setPWM(4 * colNumber + 1, 0, angleToPulse(angle));
          board1.setPWM(4 * colNumber + 2, 0, angleToPulse(angle));
          board1.setPWM(4 * colNumber + 3, 0, angleToPulse(angle));
        } else {

          //the rest of the logic will be on the second board
          board2.setPWM(((colNumber - 4) * 4) + 0, 0, angleToPulse(angle));
          board2.setPWM(((colNumber - 4) * 4) + 1, 0, angleToPulse(angle));
          board2.setPWM(((colNumber - 4) * 4) + 2, 0, angleToPulse(angle));
          board2.setPWM(((colNumber - 4) * 4) + 3, 0, angleToPulse(angle));
        }
      }
    }
  });
  serialThread.setInterval(10);

  controller.add(&animationThread);
  controller.add(&serialThread);
}

void loop() {

  controller.run();

  if (handDetected && millis() - lastHandTime > 3000) {
    handDetected = false;
  }




  //   if (handDetected) {
  //     handDetected = false;
  //     delay(100);
  //     startingColumnNo = random(0, 6);
  //     secondIdleMode();
  //     resetPanels();
  //     idleMode();
  //     resetPanels();
  //     delay(100);
  //   }
  // }
  // else {
  //   if (!handDetected) {
  //     handDetected = true;
  //   }
  // }
}

void resetPanels() {
  for (int i = 0; i < 16; i++) {
    board1.setPWM(i, 0, angleToPulse(45));
    board2.setPWM(i, 0, angleToPulse(45));
  }
}

int angleToPulse(int ang)  //gets angle in degree and returns the pulse width
{
  int pulse = map(ang, 0, 90, SERVOMIN, SERVOMAX);  // map angle of 0 to 90 to Servo min and Servo max
  Serial.write(pulse);
  return pulse;
}

void writeToColumn(int group, int angle) {
  int baseIndex = group * 4;
  int pulse = angleToPulse(angle);

  for (int i = 0; i < 4; i++) {
    int channel = baseIndex + i;
    if (channel < 16) {
      board1.setPWM(channel, 0, pulse);
    } else {
      board2.setPWM(channel - 16, 0, pulse);
    }
  }
}

void secondIdleMode() {
  for (int step = 0; step < 200; step++) {
    if (handDetected) return;
    for (int col = startingColumnNo; col < startingColumnNo + 3; col++) {
      int idx = col - startingColumnNo;
      if (step >= col * 10) {
        if (goingUp[idx]) {
          currentAngle[idx]++;
          if (currentAngle[idx] >= 58) goingUp[idx] = false;
        } else {
          currentAngle[idx]--;
          if (currentAngle[idx] <= 38) goingUp[idx] = true;
        }

        writeToColumn(col, currentAngle[idx]);
      }
    }
    delay(30);
  }
}

void idleMode() {
  for (int group = 0; group < 8; group++) {
    if (handDetected) return;
    int baseIndex = group * 4;
    bool isEven = group % 2 == 0;

    if (isEven) {
      for (int angle = 38; angle <= 58; angle += 5) {
        writeToColumn(group, angle);
        delay(50);
      }
    } else {
      for (int angle = 58; angle >= 38; angle -= 5) {
        writeToColumn(group, angle);
        delay(50);
      }
    }

    delay(200);
  }

  delay(500);
}
