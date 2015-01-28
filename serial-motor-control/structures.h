typedef struct {
  int res;
  int dir;
  int steps;
  unsigned long stepDelay;
  int ref;
  int state;
  int stepState;
  unsigned long previousMicros;
} instruction;
