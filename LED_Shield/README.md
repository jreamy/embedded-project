
# LED Shield Project

### Notes on software development

Due to Coronavirus the hardware for the LED Shield could not be completed.  I had originally intended to commute to school once a week in order to continue using the lab and complete the hardware, but as the lab closed I was no longer able to do so.  I developed an **I2C simulator** that forwards all I2C commands to Serial, with a python script simulating the various I2C hardware that needed to be communicated with.  The **LED Shield** utilizes this driver, allowing the intended functionality of the hardware to be demonstrated, even though none of the I2C hardware is physically connected to the device.
