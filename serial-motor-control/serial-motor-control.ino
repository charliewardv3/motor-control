#include "structures.h"
/* PIN CONSTANTS */
#define MS1 8
#define MS2 9
#define STEP 12
#define DIR 13

/* DIRECTION CONSTANTS */
#define CLOCKWISE 0
#define COUNTER_CLOCKWISE 1

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
      current.res = res;
      current.dir = dir;
      current.steps = steps * 2;
      current.stepDelay = stepDelay / 2;
      current.ref = ref;
      current.inProgress = 0;
      current.previousMicros = 0;
      current.stepState = 0;
      
      if(current.stepDelay < minDelay / res){
        current.stepDelay = minDelay / res;
      }
    }
  }
  
  run();
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

void setDirection(int motor_direction){
  digitalWrite(DIR, (motor_direction) ? HIGH : LOW);
}

void halfStep(){
  digitalWrite(STEP, (current.stepState) ? HIGH : LOW);
  current.stepState = !current.stepState;
};

void run(){
  if(current.steps > 0){
    setResolution(current.res);
    setDirection(current.dir);

    unsigned long currentMicros = micros();
    
    if(current.inProgress == 0){
      halfStep();
      current.inProgress = 1;
      current.previousMicros = currentMicros;
      current.steps--;
    }
    else if((unsigned long)(currentMicros - current.previousMicros) >= current.stepDelay){
      halfStep();
      current.previousMicros = currentMicros;
      current.steps--;
    }
  }
  else if(current.steps == 0 && current.inProgress == 1){
    Serial.print(current.ref);
    Serial.println(":success");
    current.inProgress = 0;
  }
}

