#include <AFMotor.h>
#include <LinkedList.h>

AF_DCMotor motor1(1, MOTOR12_1KHZ);
AF_DCMotor motor2(2, MOTOR12_1KHZ);
AF_DCMotor motor3(3, MOTOR34_1KHZ);
AF_DCMotor motor4(4, MOTOR34_1KHZ);

char command;

struct Coordinate {
  int x;
  int y;
  char direction;
  unsigned long duration;
};

LinkedList<Coordinate> path = LinkedList<Coordinate>();
int posX = 0;
int posY = 0;

void controlMotors(char command) {
  motor1.setSpeed(0); 
  motor1.run(RELEASE); 
  motor2.setSpeed(0); 
  motor2.run(RELEASE); 
  motor3.setSpeed(0); 
  motor3.run(RELEASE); 
  motor4.setSpeed(0); 
  motor4.run(RELEASE);

  switch(command) {
    case 'F':
      motor1.setSpeed(255); 
      motor1.run(FORWARD); 
      motor2.setSpeed(255); 
      motor2.run(FORWARD); 
      motor3.setSpeed(255);
      motor3.run(FORWARD); 
      motor4.setSpeed(255);
      motor4.run(FORWARD); 
      break;
    case 'B':
      motor1.setSpeed(255); 
      motor1.run(BACKWARD); 
      motor2.setSpeed(255); 
      motor2.run(BACKWARD); 
      motor3.setSpeed(255); 
      motor3.run(BACKWARD); 
      motor4.setSpeed(255); 
      motor4.run(BACKWARD); 
      break;
    case 'L':
      motor1.setSpeed(255); 
      motor1.run(BACKWARD); 
      motor2.setSpeed(255); 
      motor2.run(BACKWARD); 
      motor3.setSpeed(255); 
      motor3.run(FORWARD);  
      motor4.setSpeed(255);
      motor4.run(FORWARD);  
      break;
    case 'R':
      motor1.setSpeed(255); 
      motor1.run(FORWARD);
      motor2.setSpeed(255);
      motor2.run(FORWARD);
      motor3.setSpeed(255);
      motor3.run(BACKWARD); 
      motor4.setSpeed(255); 
      motor4.run(BACKWARD); 
      break;
    case '0':
      break;
  }
}

void updatePosition(char direction) {
  switch (direction) {
    case 'F':
      posY++;
      break;
    case 'B':
      posY--;
      break;
    case 'L':
      posX--;
      break;
    case 'R':
      posX++;
      break;
  }
  Coordinate coord = {posX, posY, direction,};
  path.add(coord);
}

void clearPathAndResetCoordinates() {
  path.clear();
  posX = 0;
  posY = 0;
}

float leftRotationAdjustmentFactor = 0.91;
float rightRotationAdjustmentFactor = 1.11;

bool stopFlag = false;

void returnHome() {
  for (int i = path.size() - 1; i >= 0; i--) {
    if (stopFlag) {
      stopFlag = false;
      break;
    }
    Coordinate coord = path.get(i);
    char reverseDirection;

    switch (coord.direction) {
      case 'F':
        reverseDirection = 'B';
        break;
      case 'B':
        reverseDirection = 'F';
        break;
      case 'L':
        reverseDirection = 'R';
        break;
      case 'R':
        reverseDirection = 'L';
        break;
    }
    controlMotors(reverseDirection);
    if (reverseDirection == 'L') {
      if (!waitForStop(coord.duration * leftRotationAdjustmentFactor)) {
        break;
      }
    } else if (reverseDirection == 'R') {
      if (!waitForStop(coord.duration * rightRotationAdjustmentFactor)) {
        break;
      }
    } else {
      if (!waitForStop(coord.duration)) {
        break;
      }
    }
    controlMotors('0');
    delay(250);
  }
  clearPathAndResetCoordinates();
}

bool waitForStop(unsigned long duration) {
  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    if (Serial.available() > 0) {
      char newCommand = Serial.read();
      if (newCommand == 'S') {
        clearPathAndResetCoordinates();
        controlMotors('0');
        stopFlag = true;
        return false;
      }
    }
  }
  return true;
}

void setup() {
  Serial.begin(9600);
}

unsigned long commandStartTime = 0;

void loop() {
  if (Serial.available() > 0) {
    char prevCommand = command;
    command = Serial.read();
    unsigned long commandEndTime = millis();

    if (prevCommand == 'F' || prevCommand == 'B' || prevCommand == 'L' || prevCommand == 'R') {
      unsigned long duration = commandEndTime - commandStartTime;
      Coordinate coord = path.get(path.size() - 1);
      coord.duration = duration;
      path.set(path.size() - 1, coord);
    }

    switch (command) {
      case 'F':
      case 'B':
      case 'L':
      case 'R':
        controlMotors(command);
        updatePosition(command);
        commandStartTime = millis();
        break;
      case '0': 
        controlMotors(command);
        break;
      case 'H': 
        returnHome();
        controlMotors('0'); 
        break;
      case 'C':
        clearPathAndResetCoordinates();
        break;
      case 'S':
        controlMotors('0');
        clearPathAndResetCoordinates();
        break;
    }
  }
}