/*
  Coded by
  Ricardo Carretero
  & servo functions by Tim Enbom
*/
//from front of sensor
//220 ohm resistor is signal, ground, 5v
//3/24/2016 EDIT
//Servo notes: closer toward input power is the line sensor on the right
//looking at the sensors from the front of the robot.
//Make sure the ground all goes to the same place
//Red dot on motor goes to white wire on to Motorshield A port
#include "DualVNH5019MotorShield.h"
#include <Servo.h>
DualVNH5019MotorShield md;
Servo myServo;
Servo myServo2;
//Pins for the sensors========================================================================================
#define Pin2 22 //left side
#define Pin3 24
#define Pin4 26
#define Pin5 28
#define Pin6 30 // right side
//============================================================================================================
uint64_t currentTime = 0;
uint64_t startTime = 0;
unsigned long runTime = 0;
int rotate360Speed = 40; // 1.29s per rotation; 12.91s for 10 rotations
int rotate360Time = 1376; // off by a fraction of a degree.
int rotateStopSpeed = 92;
int Pin32 = 32;
int Pin34 = 34;
float ratio = 0.166;
int lastTurn = 0;

int pathState = 0;
int pathArray[16] = {1, 0, 1, 1, 2, 2, 1, 1, 1, 2, 1, 1, 1, 0, 1, 1}; //1 = left, 0 straight, 2 right
//See Path.png for navigation path
void setup() {
  Serial.begin(9600);
  md.init();
  myServo.attach(Pin32);
  myServo2.attach(Pin34);
  setUp();
}
//MAIN LOOP===================================================================================================
void loop() {
  //currentTime = micros();    // Refresh Timer
  //Serial.print("Time: ");
  //long time = currentTime; //If using currentTime print won't work by reference
  //Serial.print(time );
  //Serial.print("    ");
  drive(sensorState());
  inputValues();           //Debug values for sensors
  //virtualTrack();          //Virtual Track for debugging
  //rotate360();
}
//Servo Functions=============================================================================================================
void setUp() {
  myServo.write(96);
  delay(10);
  myServo2.write(92);
  delay(10);
}
void rotate360() {
  myServo.write(rotate360Speed);
  myServo2.write(rotate360Speed);
  delay(rotate360Time);
  setUp();
}

//Sensor Functions============================================================================================================
int isOnLine(int sensorIn) {
  pinMode(sensorIn, OUTPUT);     // Make pin OUTPUT
  if (sensorIn == 22)
    PORTA |= _BV(PA0);
  if (sensorIn == 24)
    PORTA |= _BV(PA2);
  if (sensorIn == 26)
    PORTA |= _BV(PA4);
  if (sensorIn == 28)
    PORTA |= _BV(PA6);
  if (sensorIn == 30)
    PORTC |= _BV(PC7);
  pinMode(sensorIn, INPUT);      // Make pin INPUT
  if (sensorIn == 22)
    PORTA &= ~_BV(PA0);
  if (sensorIn == 24)
    PORTA &= ~_BV(PA2);
  if (sensorIn == 26)
    PORTA &= ~_BV(PA4);
  if (sensorIn == 28)
    PORTA &= ~_BV(PA6);
  if (sensorIn == 30)
    PORTC &= ~_BV(PC7);
  if (!digitalRead(sensorIn)) {  // Pin goes HIGH which means its on the black line
    return 0;

  }
  if (digitalRead(sensorIn)) {   // Pin goes LOW which means its not on the black line
    return 1;
  }
}
/*
  Taking the position of each of the 5 sensors into account, I arranged the
  function to produce a binary digit that is being created as base 2 but is
  stored as its base 10 value. This allows us to produce a single variable
  state for our entire array of line sensors as opposed to having to create
  a for loop that would ruin the continuity of multi tasking for the robot.
*/
int sensorState() {
  int currentState = 0;
  currentState = ((isOnLine(Pin6) * 16) + (isOnLine(Pin5) * 8) + (isOnLine(Pin4) * 4) + (isOnLine(Pin3) * 2) + isOnLine(Pin2));
  //Serial.println(currentState);
  return currentState;
}

//PATTERN RECOGNITION=========================================================================================
void drive(int state) {
  if (state == 4) {    //00100
      forward();
      //stopMotors();
      //Serial.println("Straight");
  }
  if (state == 0) {    //00000
      //forward();
      stopMotors(); 
      backward();
      //Serial.println("Start Area");
  }
  if (state == 7) {   //00111
      stopMotors();
      rotate360();
      gotTurn();
      Serial.println("Left turn");
  }
  /*
  if (state == 3) {   //00011
      stopMotors();
      gotTurn();
      //Serial.println("Left turn");
  }
  */
  if (state == 28) {   //11100
      stopMotors();
        gotTurn();
      //rightTurn();
      Serial.println("Right turn");
      //lastTurn == state;
  }
  /*
  if (state == 24) {   //11000
      stopMotors();
      gotTurn();
      //Serial.println("Right turn");
  }
  */
  if (state == 31) {   //11111 second time we encounter this it has to be a right turn
      //forward();
      gotTurn();
      //stopMotors();
  }

  //Correction routines------------------------------------

  if (state == 12) {    //01100
    slightRight();
    //forward();
    //Serial.println("Correct right");
  }
  if (state == 8) {    //01000
    slightRight();
    //Serial.println("Correct right");
  }

  if (state == 6) {    //00110
    slightLeft();
    //forward();
    //Serial.println("Correct left");
  }

  if (state == 2) {    //00010
    slightLeft();
    //Serial.println("Correct left");
  }
}
//MOTOR FUNCTIONS=============================================================================================
void gotTurn() {
  //still ahve to add failsafes, if there is a bad readibng it doesn't throw the whole track off?
  if (pathArray[pathState] == 0) {
    //go forward
    forward();
    //delay(50);
    //if (pathArray[pathState-1]!= 0) //not sure if this works. It is almost always true
    //Serial.println("Foward Path  called");
  }
  else if (pathArray[pathState] == 1) {
    leftTurn();
    //Serial.println("Left Path called");
  }
  else {
    rightTurn();
    //Serial.println("Right Path called");
  }
  pathState++;
}
void forward() {  //Moves the robot forward
  md.setM1Speed(200); //right side                 300 or  200
  md.setM2Speed(-165);//left side //more powerful -247 or -165
  //delay(10);
}
void backward() { //Moves the robot backwards
  md.setM1Speed(-300); //right side 300
  md.setM2Speed(247);//left side    247
}
void leftTurn() { //Turns the robot left from the center|| both have to be positive
  md.setM1Speed(400); //Forward right side 400
  md.setM2Speed(20);//Reverse left side     10
  delay(630);
}
void rightTurn() { //Turns the robot right from the center|| both have to be negative
  md.setM1Speed(-20); //Forward right side                  400
  md.setM2Speed(-(motorRatio(400)));//Reverse left side     10
  delay(630);
}
void slightLeft() {
  md.setM1Speed(55); //right side //55             75
  md.setM2Speed(45);//left side  // 45             65
}
void slightRight() {
  md.setM1Speed(-40); //right side //-40           -50
  md.setM2Speed(-55);//left side  // -55           -75
}
void stopMotors() {//Stop the robot
  md.setM1Speed(-10);
  delay(10);
  md.setM2Speed(10);
  delay(10);
}
float motorRatio(int power) {
  return (power - power * ratio);
}
//END OF MOTOR FUNCTIONS===============================================================
void inputValues() {
  Serial.print("Sensor 1: ");
  Serial.print(isOnLine(Pin2));   // Connect to pin 2, display results
  Serial.print(", Sensor 2: ");
  Serial.print(isOnLine(Pin3));   // Same for these
  Serial.print(", Sensor 3: ");
  Serial.print(isOnLine(Pin4));
  Serial.print(", Sensor 4: ");
  Serial.print(isOnLine(Pin5));
  Serial.print(", Sensor 5: ");
  Serial.print(isOnLine(Pin6));
  Serial.println();
}
/*
  Robot path
  00000 Start
  11111 Out of the start area
  00100 first straight
  11100 first left turn
  |OOOOOOOO|>>>>>>>>>>>>>>>>>>Drop rings 1 and 2
  =====First Quadrant End-----------------------------

  00100 second straight
  11100 ignore and go straight
  00100 third straight
  11100 second turn left
  |OOOOOOOO|>>>>>>>>>>>>>>>>>>Drop rings 3 and 4
  =====Second Quadrant End----------------------------

  00100 go straight
  11100 third turn left
  |OOOOOOOO|>>>>>>>>>>>>>>>>>>Drop ring 5
  =====Third Quadrant End-----------------------------

  00100 go straight SHORT
  11111 turn right
  00100 go straight SHORT
  |OOOOOOOO|>>>>>>>>>>>>>>>>>>Drop ring 6
  00111 turn right
  00100 go straight SHORT
  =====Fourth Quadrant End----------------------------

  11100 turn left
  |OOOOOOOO|>>>>>>>>>>>>>>>>>>Drop ring 7
  00100 go straight
  11100 turn left
  |OOOOOOOO|>>>>>>>>>>>>>>>>>>Drop rings 8 and 9
  =====Fifth Quadrant END-----------------------------

  00100 go straight
  11100 turn left
  =====Sixth Quadrant END-----------------------------

  00100 go straight
  11111 turn right
  00100 go straight
  11100 turn left
  00100 go straight
  |OOOOOOOO|>>>>>>>>>>>>>>>>>>Drop ring 11
  11100 turn left
  00100 go straight
  11111 turn left
  00100 go straight
  11111 ignore and go straight
  00100 straight very short
  11111 turn left
  =====Inside track END-------------------------------

  00100 go straight
  11100 left turn
  |OOOOOOOO|>>>>>>>>>>>>>>>>>>Drop ring 12
  00100 go straight
  11111 END!
  //END=======================================================================================================


*/

