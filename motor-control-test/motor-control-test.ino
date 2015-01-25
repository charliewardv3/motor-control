#define MS1 8
#define MS2 9
#define STEP 12
#define DIR 13

int state = 0;
int dirState = 0;
int steps = 200;
int res = 8;

int minDelay = 100;

void setup() {             
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(STEP, OUTPUT); 
  pinMode(DIR, OUTPUT);
  digitalWrite(DIR, HIGH);
  
  setResolution(8);
}

void loop() {
  for(int i=0; i< 3200; i++){
    digitalWrite(STEP, (state) ? HIGH : LOW);
    state = !state;
    delayMicroseconds(minDelay);
  }
  delay(1000);
  digitalWrite(DIR, (dirState) ? HIGH : LOW);
    dirState = !dirState;
}

void setResolution(int resolution){
  switch(resolution){
    case 1:
      digitalWrite(MS1, LOW);
      digitalWrite(MS2, LOW);
      minDelay = 800;
      break;
    case 2:
      digitalWrite(MS1, HIGH); 
      digitalWrite(MS2, LOW);
      minDelay = 400;
      break;
    case 4:
      digitalWrite(MS1, LOW);
      digitalWrite(MS2, HIGH);
      minDelay = 200;
      break; 
    case 8:
      digitalWrite(MS1, HIGH); 
      digitalWrite(MS2, HIGH);
      minDelay = 100;
      break;
  }
  res = resolution;
}

