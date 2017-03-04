# Common Object Temperature Controller
Arduino project for temperature sense and control using a K-type thermocouple, and a serial interface.

Unit of measure used is farenheit.

The default setpoint is 1050*f.

# Controls

This kiln controller accepts serial commands, which can be issued individually or in a chained string (let n represent a number from 0-9):

> Pn.nn - P of PID, followed by a float

> In.nn - I of PID, followed by a float

> Dn.nn - D of PID, followed by a float

> Tnnnn - Temperature, followed by an int

Example Commands: 

> P2.5 - sets P value to 2.5

> P0.01D0.3T940 - sets P value to 0.01, D value to 0.3, and Temperature to 940*f

> T1050 - sets the temperature setpoint to 1050*f

# Hardware
- Max6675 with breakout 
- K-Type thermocouple
- Arduino Nano
- HC-06 Bluetooth TX/RX
- 1k ohm resistor
- 2k ohm resistor
- Relay

*Notes:
There is a 500ms delay within the loop to allow for the Max6675 chip to settle. Without this delay, the values are not useable. *
