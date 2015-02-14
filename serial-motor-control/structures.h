typedef struct {
  int commandType;  // 1: motor, 2: shutter
  int res;
  int dir;
  int steps;
  unsigned long stepDelay;
  unsigned long shutterDuration;
  int ref;
  int state;
  int stepState;
  unsigned long previousMicros;
} instruction;
