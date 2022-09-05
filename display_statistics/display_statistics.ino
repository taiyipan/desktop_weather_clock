#include "SevSeg.h"
#include "SerialTransfer.h"

//instantiate objects
SevSeg sevseg, sevseg2, sevseg3;
SerialTransfer myTransfer;

//declare global constants
const int BLUE = 53;
const int YELLOW = 51;
const int RED = 49;
const int GREEN = 47;
const byte INTERRUPT = 19;

//declare variables
long package; //variable to store serial communication data package
volatile byte state = LOW;
void(*resetFunc)(void) = 0;

/*
Setup phase
*/
void setup() {
  //initiate serial transfer protocol
  Serial.begin(115200);
  myTransfer.begin(Serial);

  //define digital pins
  pinMode(BLUE, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  // pinMode(BUTTON, INPUT);

  //define display 1
  byte numDigits = 1;
  byte digitPins[] = {};
  byte segmentPins[] = {30, 32, 38, 36, 34, 28, 26, 40};
  bool resistorsOnSegments = true;
  byte hardwareConfig = COMMON_CATHODE;
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);

  //define display 2
  byte segmentPins2[] = {31, 33, 39, 37, 35, 29, 27, 41};
  sevseg2.begin(hardwareConfig, numDigits, digitPins, segmentPins2, resistorsOnSegments);

  //define display 3
  byte numDigits3 = 4;
  byte digitPins3[] = {5, 6, 7, 8};
  byte segmentPins3[] = {4, 2, 46, 50, 52, 3, 44, 48};
  bool leadingZeros = true;
  sevseg3.begin(hardwareConfig, numDigits3, digitPins3, segmentPins3, resistorsOnSegments, leadingZeros);

  //define interrupt service routine
  attachInterrupt(digitalPinToInterrupt(INTERRUPT), stateToggle, RISING);

  //initialize variables
  package = 0;
}

/*
Main control loop
*/
void loop() {
  //listen for serial communication
  if (myTransfer.available()) {
    myTransfer.rxObj(package);
  }

  //reset trigger
  if (package == -1) resetFunc();

  //2 states: normal and irregular
  // if (state == LOW) {
  //   normalRoutine();
  // } else {
  //   irregularRoutine();
  //   delay(3000);
  // }
  normalRoutine();
}

/*
Interrupt service routine: toggle state
*/
void stateToggle() {
  state = !state;
}

/*
Execution protocol during normal routine
*/
void normalRoutine() {
  //blue below zero temp indicator: + means above zero, - means below zero
  if (package < 0) {
    digitalWrite(BLUE, HIGH);
    package *= -1;
  } else {
    digitalWrite(BLUE, LOW);
  }
  //red/green weather indicator: 0 means clear weather, 1 means hazardous weather
  if (package % 10) {
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
  } else {
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);
  }
  //yellow sunlight indicator: 1 means sun is up, 0 means sun has set
  if (package / 10 % 10) {
    digitalWrite(YELLOW, HIGH);
  } else {
    digitalWrite(YELLOW, LOW);
  }
  //extract time
  long one = package / (long)pow(10, 2) % 10;
  long ten = package / (long)pow(10, 3) % 10;
  long hundred = package / (long)pow(10, 4) % 10;
  long thousand = package / (long)pow(10, 5) % 10;
  sevseg3.setNumber(thousand * 1000 + hundred * 100 + ten * 10 + one, 2);
  sevseg3.refreshDisplay();
  //extract temperature (normalized to positive)
  sevseg2.setNumber(package / (long)pow(10, 6) % 10);
  sevseg2.refreshDisplay();
  sevseg.setNumber(package / (long)pow(10, 7) % 10);
  sevseg.refreshDisplay();
}

/*
Execution protocol during irregular routine
*/
// void irregularRoutine() {
//   //blank out 7Seg displays
//   sevseg.blank();
//   sevseg2.blank();
//   sevseg3.blank();
//   //turn off LEDs
//   digitalWrite(BLUE, LOW);
//   digitalWrite(YELLOW, LOW);
//   digitalWrite(RED, LOW);
//   digitalWrite(GREEN, LOW);
// }
