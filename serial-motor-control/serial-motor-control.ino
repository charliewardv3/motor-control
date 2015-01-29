#include <QueueList.h>
#include "structures.h"
/* PIN CONSTANTS */
#define MS1 8
#define MS2 9
#define STEP 12
#define DIR 13

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

void setup() {             
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(STEP, OUTPUT); 
  pinMode(DIR, OUTPUT);
  
  digitalWrite(STEP, LOW);
  
  setResolution(8);
  setDirection(CLOCKWISE);
  
  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  checkSerial();
  run();
}

void checkSerial(){
  if(Serial.available() > 0){
    
    int res = Serial.parseInt();
    int dir = Serial.parseInt();
    int steps = Serial.parseInt();
    unsigned long stepDelay = Serial.parseFloat();
    int ref = Serial.parseInt();
    
    int nonNumeric = Serial.read();
    
    if(nonNumeric == '\n'){
      queueInstruction(res, dir, steps, stepDelay, ref);
    }
    else if(nonNumeric = 'X'){
      stopMotor();
    }
  }
}

void stopMotor(){
  current.state = COMPLETE;
  Serial.print(current.ref);
  Serial.println(":canceled");
  
  while(!queue.isEmpty()){
    instruction canceled = queue.pop();
    Serial.print(canceled.ref);
    Serial.println(":canceled");
  }
}

void queueInstruction(int res, int dir, int steps, unsigned long int stepDelay, int ref){
  if(stepDelay / 2 < MINDELAY / res){
    Serial.print(ref);
    Serial.println(":delay too short");
  }
  else if(freeRam() < MINRAM){
    Serial.print(ref);
    Serial.println(":queue full");
  }
  else {
    instruction incoming;
    incoming.res = res;
    incoming.dir = dir;
    incoming.steps = steps * 2; //account for half step behavior
    incoming.stepDelay = stepDelay / 2; //Split delay between High and LOW half steps
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

void setResolution(int resolution){
  switch(resolution){
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

void setDirection(int dir){
  digitalWrite(DIR, (dir) ? HIGH : LOW);
}

void halfStep(){
  digitalWrite(STEP, (current.stepState) ? HIGH : LOW);
  current.stepState = !current.stepState;
}

void run(){
  if(current.state == NEW && current.steps > 0){
    setResolution(current.res);
    setDirection(current.dir);
    
    halfStep();
    
    current.state = PROCESSING;
    current.previousMicros = micros();
    current.steps--;
  } 
  
  else if(current.state == PROCESSING && current.steps > 0){
    unsigned long currentMicros = micros();
    
    if((unsigned long)(currentMicros - current.previousMicros) >= current.stepDelay){
      halfStep();
      current.previousMicros = currentMicros;
      current.steps--;
    }
  }
  
  else if(current.state != COMPLETE && current.steps == 0){
    Serial.print(current.ref);
    Serial.println(":success");
    current.state = COMPLETE;
  }
  
  else if(!queue.isEmpty()){
    current = queue.pop();
  }
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
