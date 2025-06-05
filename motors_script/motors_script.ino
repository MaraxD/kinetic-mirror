/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h> //install Servo library
#include <Adafruit_PWMServoDriver.h> // adafruit pwm servo driver library

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);   // called this way, it uses the default address 0x40   
Adafruit_PWMServoDriver board2 = Adafruit_PWMServoDriver(0x41);

#define SERVOMIN  125   // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  625   // this is the 'maximum' pulse length count (out of 4096)

#define OE1 7
#define OE2 8

int activeChannelsBoard1[8];
int activeChannelsBoard2[8];

char buffer[30];
int len=0, lastAngle=-1,colNumber, angle, time;
char boardNo;
int lista[3]={0,0,0};
bool justStarted=false;


void setup() {
  Serial.begin(115200);
  Serial.println("16 channel Servo test!");

  board1.begin();
  board1.setOscillatorFrequency(27000000);    //Set the PWM oscillator frequency, used for fine calibration
  board1.setPWMFreq(50); 
  
  board2.begin();
  board2.setOscillatorFrequency(27000000);
  board2.setPWMFreq(50); 


  // for(int i=0;i<16;i++){
  //   board1.setPWM(i, 0, angleToPulse(45));
  //   board2.setPWM(i, 0, angleToPulse(45));
  // }

  justStarted=true;
}

void loop() { 

  if(Serial.available()){
    justStarted=false;
  
    colNumber=Serial.parseInt();
    angle=Serial.parseInt();

    if(Serial.read()=='\n'){
      Serial.print("col: ");
      Serial.print(colNumber);
      Serial.print(" | angle: ");
      Serial.println(angle);

      if(colNumber<4){

        board1.setPWM(4*colNumber, 0, angleToPulse(angle));
        board1.setPWM(4*colNumber+1, 0, angleToPulse(angle));
        board1.setPWM(4*colNumber+2, 0, angleToPulse(angle));
        board1.setPWM(4*colNumber+3, 0, angleToPulse(angle));
      }else{

        //the rest of the logic will be on the second board
        board2.setPWM(((colNumber-4)*4)+0, 0, angleToPulse(angle));
        board2.setPWM(((colNumber-4)*4)+1, 0, angleToPulse(angle));
        board2.setPWM(((colNumber-4)*4)+2, 0, angleToPulse(angle));
        board2.setPWM(((colNumber-4)*4)+3, 0, angleToPulse(angle));
      }
    }

    time=0;
  }else{
    time+=1;
  }

  //after some elapsed time, enter idle mode

  if(time > 100 || justStarted){
    idleMode();
  }
}

int angleToPulse(int ang) //gets angle in degree and returns the pulse width
{ 
  int pulse = map(ang, 0, 90, SERVOMIN, SERVOMAX);  // map angle of 0 to 90 to Servo min and Servo max 
  return pulse;
}

void idleMode(){
  for (int group = 0; group < 8; group++) {
    int baseIndex = group * 4;
    bool isEven = group % 2 == 0;

    Adafruit_PWMServoDriver* activeBoard;
    int localIndex;

    // ----- Overlapping zone (columns 3 and 4) -----
    bool overlap = (group == 3 || group == 4);

    if (overlap) {
    }
    else if (group < 4) {  // Columns 0–3
      activeBoard = &board1;
    } else {               // Columns 4–7
      activeBoard = &board2;
    }

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

  // Disable all outputs
  delay(500);

  // for(int group=0;group<8;group+=2){
  //   int colA=group;
  //   int colB=group+1;

  //   bool overlap=(colA<=3 && colB>=4);


  //   if(overlap){
  //     digitalWrite(OE1, LOW);
  //     digitalWrite(OE2, LOW);
  //   }else if(colA<4){
  //     digitalWrite(OE1, LOW);
  //     digitalWrite(OE2, HIGH);
  //   }else{
  //     digitalWrite(OE1, HIGH);
  //     digitalWrite(OE2, LOW);
  //   }

  //   for(int angle=38;angle<=58;angle+=5){
  //     int pulseUp=angleToPulse(angle);
  //     int pulseDown=angleToPulse(96-angle);

  //     moveColumn(colA, pulseUp);
  //     moveColumn(colB, pulseDown);

  //     delay(60);
  //   }
  //   delay(200);
  // }

  // digitalWrite(OE1, HIGH);
  // digitalWrite(OE2, HIGH);
  // delay(500);
}

void writeToColumn(int group, int angle){
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

void moveColumn(int col, int angle){
  int baseIndex=col*4;

  for(int i=0;i<4;i++){
    int channel=baseIndex+i;
    if(channel<16){
      board1.setPWM(channel, 0, angle);
    }else{
      board2.setPWM(channel-16, 0, angle);
    }
  }
}




void testMotors(){
  for (int angle = 38; angle <= 58; angle += 5) {
    for (int i = 8; i < 16; i++) {
      board1.setPWM(i, 0, angleToPulse(angle));
      board2.setPWM(i, 0, angleToPulse(angle));
    }
    delay(300);  // Let the servo move
  }

  // delay(500);  // Pause at the top

  for (int angle = 58; angle >= 38; angle -= 5) {
    for (int i = 8; i < 16; i++) {
      board1.setPWM(i, 0, angleToPulse(angle));
      board2.setPWM(i, 0, angleToPulse(angle));
    }
    delay(300);
  }

  // delay(1000);  // Pause before starting again
}


