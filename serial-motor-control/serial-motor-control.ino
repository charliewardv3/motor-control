#include <AccelStepper.h>

#include <QueueList.h>
#include "structures.h"
/* PIN CONSTANTS */
#define MS1 8
#define MS2 9
#define STEP 12
#define DIR 13
#define SLP 7

/* DIRECTION CONSTANTS */
#define CLOCKWISE 0
#define COUNTER_CLOCKWISE 1

/* Instruction States */
#define NEW 0
#define PROCESSING 1
#define COMPLETE 2

/* Define Safety Values */
#define MINDELAY 800 //Minimum Delay for a full step
#define MINRAM 500 //Minumum RAM threshold for adding a new instruction to the queue

QueueList <instruction> queue;
instruction current;

AccelStepper stepper(AccelStepper::DRIVER, STEP, DIR);

void setup() {             
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(STEP, OUTPUT); 
  pinMode(DIR, OUTPUT);
  pinMode(SLP, OUTPUT);

  digitalWrite(STEP, LOW);
//  digitalWrite(SLP, LOW); //Initialize Sleeping

  setResolution(8);

  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  checkSerial();
  run();
}

void checkSerial() {
  if (Serial.available() > 0) {
    wake(); //wake up and get ready

    int res = Serial.parseInt();
    int dir = Serial.parseInt();
    int steps = Serial.parseInt();
    unsigned long stepDelay = Serial.parseFloat();
    int ref = Serial.parseInt();

    int nonNumeric = Serial.read();

    if (nonNumeric == '\n') {
      if (res == 0 && dir == 0 && steps == 0 && stepDelay == 0) {
        stopMotor();
      }
      else {
        queueInstruction(res, dir, steps, stepDelay, ref);
      }
    }
  }
}

void stopMotor() {
  current.state = COMPLETE;
  stepper.stop();
  Serial.print(current.ref);
  Serial.println(":canceled");

  while (!queue.isEmpty()) {
    instruction canceled = queue.pop();
    Serial.print(canceled.ref);
    Serial.println(":canceled");
  }
}

void queueInstruction(int res, int dir, int steps, unsigned long int stepDelay, int ref) {
  if(freeRam() < MINRAM) {
    Serial.print(ref);
    Serial.println(":queue full");
  }
  else {
    instruction incoming;
    incoming.res = res;
    incoming.dir = dir;
    incoming.steps = steps;
    incoming.stepDelay = stepDelay; // This is SPEED now.
    incoming.ref = ref;
    incoming.state = NEW;
    incoming.previousMicros = 0;
    incoming.stepState = 0;
    queue.push(incoming);
    Serial.print(ref);
    Serial.println(":queued");
    Serial.print("ram:");
    Serial.println(freeRam());
  }
}

void setResolution(int resolution) {
  switch(resolution) {
  case 1:
    digitalWrite(MS1, LOW);
    digitalWrite(MS2, LOW);
    break;
  case 2:
    digitalWrite(MS1, HIGH); 
    digitalWrite(MS2, LOW);
    break;
  case 4:
    digitalWrite(MS1, LOW);
    digitalWrite(MS2, HIGH);
    break; 
  case 8:
    digitalWrite(MS1, HIGH); 
    digitalWrite(MS2, HIGH);
    break;
  }
}


void sleep() {
//  digitalWrite(SLP, LOW); //Sleep
}

void wake() {
//  digitalWrite(SLP, HIGH); //WakeUp!
}

void run() {
  if(current.state == NEW && current.steps != 0){
    Serial.print(current.ref);
    Serial.print(":NEW:res:");
    Serial.print(current.res);
    Serial.print(":steps:");
    Serial.print(current.steps);
    Serial.print(":dir:");
    Serial.print(current.dir);
    Serial.print(":speed:");
    Serial.println(current.stepDelay);

    setResolution(current.res);

    current.state = PROCESSING;
    if (current.dir == 0) {  // Forward
      stepper.moveTo(current.steps);
      stepper.setMaxSpeed(current.stepDelay);
      stepper.setAcceleration(current.stepDelay / 2);
//      stepper.setSpeed(current.stepDelay);
    }
    else {  // Backward...need to provide negative position.
      stepper.moveTo(0 - current.steps);
      stepper.setMaxSpeed(current.stepDelay);
      stepper.setAcceleration(current.stepDelay / 2);
//      stepper.setSpeed(current.stepDelay);
    }
  }

  else if (current.state == PROCESSING && stepper.distanceToGo() != 0) {
//    Serial.print(current.ref);
//    Serial.print(":dtg:");
//    Serial.println(stepper.distanceToGo());

    stepper.run();
//    stepper.runSpeedToPosition();
  }

  else if (current.state == PROCESSING && stepper.distanceToGo() == 0) {
    Serial.print(current.ref);
    Serial.println(":success");
    current.state = COMPLETE;
    stepper.setCurrentPosition(0);  // Reset 'current' position so that we don't have to maintain it.
  }

  else if (!queue.isEmpty()) {
    current = queue.pop();
  }

  else if (current.state == COMPLETE && queue.isEmpty()) {
    // TODO:  Does this maintain the stepper position, or does it de-energize the coils, 
    // allowing the stepper to move freely?
    sleep();
  }
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

