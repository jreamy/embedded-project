
# Hardware design

## Accelerometer Chip Design

The accelerometer chip is a 4-wire to 4-wire board designed to hold a single KX003-1077 accelerometer, allowing multiple accelerometers to be connected to the I2C communication lines while preserving flexibility between components.  The project will contain 16 such accelerometer chips, connecting to the 8-channel I2C MUX chip.  The chip has a no-place 0 Ohm resistor which optionally configures the address of the KX003-1077.  Current chip design is .5 x .8 inches.

Chip needs mounting holes!

![alt text](Layouts/AccelerometerChipLayout.png "Accelerometer Chip Layout" =400x)
