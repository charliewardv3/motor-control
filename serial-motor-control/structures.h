typedef struct {
  int res;
  int dir;
  int steps;
  int stepDelay;
  int ref;
  int inProgress;
  unsigned long previousMicros;
} instruction;
