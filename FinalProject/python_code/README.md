
# Python script used to interface with the Arduino

The main script used to interface with the Arduino is `run.py`.  This script utilizes the I2C -> Serial forwarding protocol allowing I2C hardware to be simulated.  The registers of the simulated hardware devices get saved in the 'mem' directory, displaying register address (in hex) and register value (in hex) in csv format.

The hardware addresses and setup are configured in `i2c_config.py`.  For the project, the finger position monitoring KX003 units were given default values approximating curled fingers: 45 degrees for the first joint, 90 for the second, and 45 for the final joint in each finger.  The i2c hardware simulating code can be found in `i2c_hardware.py`.

The code simulating I2C communication can be found in `i2c_com.py` which receives forwarded I2C data from `serial_com.py`.

## Dependencies
 - numpy
 - pySerial
