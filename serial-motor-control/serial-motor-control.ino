#include <AccelStepper.h>

#include <QueueList.h>
#include "structures.h"
/* PIN CONSTANTS */
#define A1 8
#define A2 11
#define B1 12
#define B2 13
#define ENABLE_A 9
#define ENABLE_B 10
#define SHUTTER_RELAY_PIN 5

/* DIRECTION CONSTANTS */
#define CLOCKWISE 0
#define COUNTER_CLOCKWISE 1

/* COMMAND TYPE CONSTANTS */
#define MOTOR_COMMAND 1
#define SHUTTER_COMMAND 2
#define CANCEL_COMMAND -1

/* Instruction States */
#define NEW 0
#define PROCESSING 1
#define COMPLETE 2

/* Define Safety Values */
#define MINDELAY 800 //Minimum Delay for a full step
#define MINRAM 500 //Minumum RAM threshold for adding a new instruction to the queue

QueueList <instruction> queue;
instruction current;

AccelStepper stepper(AccelStepper::FULL4WIRE, A1, A2, B1, B2);

void setup() {
  // Enable Motor A and Motor B.
  pinMode(ENABLE_A, OUTPUT);
  pinMode(ENABLE_B, OUTPUT);
  digitalWrite(ENABLE_A, HIGH);
  digitalWrite(ENABLE_B, HIGH);
  
  pinMode(SHUTTER_RELAY_PIN, OUTPUT);

  setResolution(1);

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

    int commandType = Serial.parseInt();
    
    if (commandType == MOTOR_COMMAND) {
      // move motor
      // 1,[resolution],[direction],[steps],[delay],[reference]\n
      int res = Serial.parseInt();
      int dir = Serial.parseInt();
      int steps = Serial.parseInt();
      unsigned long stepDelay = Serial.parseFloat();
      int ref = Serial.parseInt();
  
      int nonNumeric = Serial.read();
  
      if (nonNumeric == '\n') {
//        if (res == 0 && dir == 0 && steps == 0 && stepDelay == 0) {
//          stopProcessing();
//        }
//        else {
          queueMotorInstruction(res, dir, steps, stepDelay, ref);
//        }
      }
    }
    
    if (commandType == SHUTTER_COMMAND) {
      // trip shutter
      // 2,[duration],[reference]\n
      unsigned long shutterDuration = Serial.parseFloat();
      int ref = Serial.parseInt();      

      int nonNumeric = Serial.read();
  
      if (nonNumeric == '\n') {
          queueShutterInstruction(shutterDuration, ref);
      }
    }
    if (commandType == CANCEL_COMMAND) {
      // Cancel.
      // 3,[reference]\n
      stopProcessing();
    }
  }
}

void stopProcessing() {
  current.state = COMPLETE;
  stepper.stop();
  closeShutter();
  Serial.print(current.ref);
  Serial.println(":canceled");

  while (!queue.isEmpty()) {
    instruction canceled = queue.pop();
    Serial.print(canceled.ref);
    Serial.println(":canceled");
  }
}

void queueShutterInstruction(unsigned long int shutterDuration, int ref) {
  if(freeRam() < MINRAM) {
    Serial.print(ref);
    Serial.println(":queue full");
  }
  else {
    instruction incoming;
    incoming.commandType = SHUTTER_COMMAND;
    if (shutterDuration < 50) {
      // 50µs is the minimum to reliably trip the shutter.
      shutterDuration = 50;
    }
    shutterDuration = shutterDuration * 1000;  // Convert ms to µs.
    incoming.shutterDuration = shutterDuration; 
    incoming.ref = ref;
    incoming.state = NEW;
    incoming.previousMicros = 0;
    queue.push(incoming);
    Serial.print(ref);
    Serial.println(":queued");
    Serial.print("ram:");
    Serial.println(freeRam());
  }
}

void queueMotorInstruction(int res, int dir, int steps, unsigned long int stepDelay, int ref) {
  if(freeRam() < MINRAM) {
    Serial.print(ref);
    Serial.println(":queue full");
  }
  else {
    instruction incoming;
    incoming.commandType = MOTOR_COMMAND;
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
//  switch(resolution) {
//  case 1:
//    digitalWrite(MS1, LOW);
//    digitalWrite(MS2, LOW);
//    break;
//  case 2:
//    digitalWrite(MS1, HIGH); 
//    digitalWrite(MS2, LOW);
//    break;
//  case 4:
//    digitalWrite(MS1, LOW);
//    digitalWrite(MS2, HIGH);
//    break; 
//  case 8:
//    digitalWrite(MS1, HIGH); 
//    digitalWrite(MS2, HIGH);
//    break;
//  }
}


void sleep() {
}

void wake() {
}

void openShutter() {
  digitalWrite(SHUTTER_RELAY_PIN, HIGH);
}

void closeShutter() {
  digitalWrite(SHUTTER_RELAY_PIN, LOW);  
}

void run() {
  if (current.commandType == 0 and !queue.isEmpty()) {
    current = queue.pop();
  }
  
  if (current.commandType == SHUTTER_COMMAND) {
    if (current.state == NEW){
      current.state = PROCESSING;
      current.previousMicros = micros();
      openShutter();
    }
    else if (current.state == PROCESSING) {
      unsigned long currentMicros = micros();
      
      if ((unsigned long)(currentMicros - current.previousMicros) >= current.shutterDuration){
        closeShutter();
        current.state = COMPLETE;
        Serial.print(current.ref);
        Serial.println(":success");
      }
    }
    else if(!queue.isEmpty()){
      current = queue.pop();
    }
  }
  
  if (current.commandType == MOTOR_COMMAND) {  
    if(current.state == NEW && current.steps != 0){
      current.state = PROCESSING;
      setResolution(current.res);
  
      if (current.dir == 0) {  // Forward
        stepper.moveTo(current.steps);
        
        stepper.setMaxSpeed(current.stepDelay);
        stepper.setAcceleration(current.stepDelay / 2);
        //stepper.setSpeed(current.stepDelay);
      }
      else {  // Backward...need to provide negative position.
        stepper.moveTo(0 - current.steps);
  
        stepper.setMaxSpeed(current.stepDelay);
        stepper.setAcceleration(current.stepDelay / 2);
        //stepper.setSpeed(current.stepDelay);
      }
    }
  
    else if (current.state == PROCESSING && stepper.distanceToGo() != 0) {
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
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

