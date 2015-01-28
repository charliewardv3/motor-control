#include <QueueArray.h>
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

QueueArray <instruction> queue;
instruction current;

unsigned long minDelay = 800; //minimum delay for a full step

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
  if(Serial.available() > 0){
    
    int res = Serial.parseInt();
    int dir = Serial.parseInt();
    int steps = Serial.parseInt();
    unsigned long stepDelay = Serial.parseFloat();
    int ref = Serial.parseInt();
    
    if(Serial.read() == '\n'){
      processIncoming(res, dir, steps, stepDelay, ref);
    }
  }
  
  run();
}

void processIncoming(int res, int dir, int steps, unsigned long int stepDelay, int ref){
    if(stepDelay < minDelay / res){
      Serial.print(ref);
      Serial.println(":delay too short");
    }
    else if(queue.isFull()){
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
};

void run(){
  if(current.steps > 0 && current.state != COMPLETE){
    setResolution(current.res);
    setDirection(current.dir);

    unsigned long currentMicros = micros();
    
    if(current.state == NEW){
      halfStep();
      current.state = PROCESSING;
      current.previousMicros = currentMicros;
      current.steps--;
    }
    else if((unsigned long)(currentMicros - current.previousMicros) >= current.stepDelay){
      halfStep();
      current.previousMicros = currentMicros;
      current.steps--;
    }
  }
  else if(current.steps == 0 && current.state == PROCESSING){
    Serial.print(current.ref);
    Serial.println(":success");
    current.state = COMPLETE;
  }
  else if(!queue.isEmpty()){
    current = queue.pop();
  }
}

