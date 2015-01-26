/* PIN CONSTANTS */
#define MS1 8
#define MS2 9
#define STEP 12
#define DIR 13

/* DIRECTION CONSTANTS */
#define CLOCKWISE 0
#define COUNTER_CLOCKWISE 1

int minDelay = 800; //minimum delay for a full step

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
    int stepDelay = Serial.parseInt();
    
    if(Serial.read() == '\n'){
      move(res, dir, steps, stepDelay);
    }
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

void setDirection(int motor_direction){
  digitalWrite(DIR, (motor_direction) ? HIGH : LOW);
}

void move(int resolution, int motor_direction, int steps, int stepDelay){
  setResolution(resolution);
  setDirection(motor_direction);
  
  if(stepDelay < minDelay / resolution){
    stepDelay = minDelay / resolution;
  }
  
  for(int i=0; i < steps; i++){
    digitalWrite(STEP, HIGH);
    delayMicroseconds(minDelay/resolution);
    digitalWrite(STEP, LOW);
    delayMicroseconds(stepDelay);
  }
  Serial.print("moved(");
  Serial.print(resolution);
  Serial.print(",");
  Serial.print(motor_direction);
  Serial.print(",");
  Serial.print(steps);
  Serial.print(",");
  Serial.print(stepDelay);
  Serial.println(")");
}

