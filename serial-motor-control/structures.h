typedef struct {
  int res;
  int dir;
  int steps;
  unsigned long stepDelay;
  int ref;
  int inProgress;
  unsigned long previousMicros;
} instruction;
