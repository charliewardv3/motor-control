# motor-control
Arduino based USB Motor Control for [SeedStudio Motor Shield V2.0](http://www.seeedstudio.com/depot/Motor-Shield-V20-p-1377.html) and Shutter Release Control for [SeeedStudio Relay shield V2.0](http://www.seeedstudio.com/depot/Relay-shield-V20-p-1376.html).

Allows you to send serial commands to an Arduino board to control a stepper motor using a SeedStudio Motor Shield V2.0 or control a shutter release using SeeedStudio Relay shield V2.0 and a shutter release cable.  The shutter release mechanism has been tested with Canon EOS cameras utilizing the N3 connector, but other cameras with similar shutter release mechanisms should work fine.

### Usage:
Accepts one of three commands over serial connection.  Each `commandtype` takes different numbers and types of parameters, but all commands are terminated with a newline `\n` character.  The three `commandTypes` are currently:

```
 1 = Motor
 2 = Shutter
-1 = Cancel
```

A `Cancel` command will stop the current motor or shutter command immediately and cancel any queued commands.

#### Motor Command:
The `Motor` command is issued by specifying six parameters in the following string pattern:

```
"[commandType],[resolution],[direction],[steps],[speed],[reference]\n"
```

`commandType` is an integer value of 1.

`resolution` is an integer value that specifys the size of the step.

```
1 = full step
2 = half step
4 = quarter step
8 = eighth step
```

NOTE:  Only `full step` is supported by the SeedStudio Motor Shield V2.0, and any other value will be ignored and treated as a `full step` value.


`direction` is an integer value that specifies the direction the motor will rotate

```
0 = Clockwise
1 = Counter Clockwise
```

`steps` is an integer value that specifies the number of steps to move the motor

`speed` is an integer value between 1 and 100 that specifies the speed at which the motor moves.  Acceleration is 1/2 of this speed value.

`reference` is an integer specified by the calling program that can be used later to reconcile responses

The `Motor` command is a non-blocking operation.  Once the move command has completed, it will echo the following string back over the serial connection to signal completion:

```
    [reference]:[message]\r\n
```

When the command has been queued, the returned `message` will be `queued`.  When the move has successfully completed, the returned `message` will be `success`. 


#### Shutter Command:
The `Shutter` command is issued by specifying three parameters in the following string pattern:

```
[commandType],[duration],[reference]\n

```

`commandType` is an integer value of 2.

`duration` is an integer value that specifies the number of milliseconds to keep the shutter open.

*NOTE: Minimum reliable value is somewhere around 50ms using the SeeedStudio Relay shield V2.0.  Any value less than 50ms will be set to 50ms (1/20th of a second).  The duration value should only be used for Bulb exposures where an exposure of longer than 30 seconds is necessary, and millisecond precision isn't important.*

`reference` is an integer specified by the calling program that can be used later to reconcile responses

The `Shutter` command is a non-blocking operation.  Once the move command has completed, it will echo the following string back over the serial connection to signal completion:

```
    [reference]:[message]\r\n
```

When the command has been queued, the returned `message` will be `queued`.  When the shutter has successfully closed, the returned `message` will be `success`. 


#### Cancel Command:
The `Cancel` command is issued by specifying two parameters in the following string pattern:

```
[commandType],[reference]\n

```

`commandType` is an integer value of -1.

`reference` is an integer specified by the calling program that can be used later to reconcile responses

`Cancel` stops the currently processing command *immediately* and cancels any pending commands.  This will only shorten a shutter operation if the camera is in Bulb mode, otherwise the camera's exposure will complete normally.  `Motor` commands will abruptly terminate and will not decelerate to a stop as they do in a normal movement.


### Testing:

1. Load serial-motor-control sketch onto your arduino board

2. install pySerial on your computer (sudo pip install pyserial)

3. Run the `motor.py` script in the `examples` folder to test motor movement, changing the serial address to the address of your Arduino board.

4. Run the `shutter.py` script in the `examples` folder to test shutter triggering, changing the serial address to the address of your Arduino board.

5. Optionally run the `motor_and_shutter.py` script in the `examples` folder to test the shutter triggering interleaved with motor movement, changing the serial address to the address of your Arduino board.

