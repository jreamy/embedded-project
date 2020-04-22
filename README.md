
# ENGE 420: Embedded Systems

## Coronavirus
Due to Coronavirus the hardware for the Final Project and LED Shield could not be completed.  I had originally intended to commute to school once a week in order to continue using the lab and complete the hardware, but as the lab closed I was no longer able to do so.  I developed an **I2C simulator** that forwards all I2C commands to Serial, with a python script simulating the various I2C hardware that needed to be communicated with.  The **Final Project** and **LED Shield** both utilize this driver, allowing the intended functionality of the hardware to be demonstrated, even though none of the I2C hardware is physically connected to the device.

## Final Project
The Final Project folder contains all the code and hardware design intended for the Mind Controlled Prosthetic Controller project.  It utilizes the above mentioned I2C simulator.

## LED Shield
The LED Shield folder contains all the code and hardware design intended for the LED Shield project.  It too utilizes the above mentioned I2C simulator.

## tests
This folder contains various fragments of code written to test various aspects of the software written in this class.
