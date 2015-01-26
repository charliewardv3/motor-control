# motor-control
Arduino based USB Motor Control for Easy Stepper v4.4

Allows you to send serial commands to an Arduino board to control a stepper motor using an Easy Stepper v4.4

### Usage:
Accepts a single command over serial connection specifying four parameters in the following string pattern:

```
"[resolution],[direction],[steps],[delay],[reference]\n"
```

`resolution` is an integer value that specifys the size of the step

```
1 = full step
2 = half step
4 = quarter step
8 = eigth step
```

`direction` is an integer value that specifies the direction the motor will rotate

```
0 = Clockwise
1 = Counter Clockwise
```

`steps` is an integer value that specifies the number of steps to move the motor

`delay` is an integer value that specifies the delay between each step

`reference` is an integer specified by the calling program that can be used later to reconcile responses

The move command is a non blocking operation.  Once the move command has completed, it will echo the following string back over the serial connection to signal completion:

```
[reference]:[message]\r\n
```
When the move was successful, the returned message will be "success".

### Testing:  

1. Load serial-motor-control sketch onto your arduino board

2. Configure pin constants to match your Arduino / Easy Stepper v4.4 pin mappings for MS1, MS2, STEP and DIR

3. install pySerial on your computer (sudo pip install pyserial)

4. Run this short script to test motor movement, changing the serial address to the address of your Arduino board


```
import serial
import time

ser = serial.Serial('/dev/cu.usbmodem14211', 115200)

reference = 0

while True:
    line = ser.readline()
    parsed = line.strip().split(":")

    if line.strip() == "Ready":
        # wait for the Ready message before starting.
        ser.write('8,1,1600,100,' + str(reference) + '\n')
        reference += 1
    elif parsed[1] == 'success':
        # move has completed...wait 1s and issue another move command.
        print line.replace('\r\n', '')
        time.sleep(1)
        ser.write('8,1,1600,100,' + str(reference) + '\n')
        reference += 1
    else:
        # unknown message
        print '*' * 80
        print line.replace('\r\n', '')
        print '*' * 80

```