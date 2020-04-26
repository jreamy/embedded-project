
# Python script used to interface with the Arduino

The main script used to interface with the Arduino is `run.py`.  This script utilizes the I2C -> Serial forwarding protocol allowing I2C hardware to be simulated.  The registers of the simulated hardware devices get saved in the 'mem' directory, displaying register address (in hex) and register value (in hex) in csv format.

The `LP5036` is set up and configured in `i2c_config.py`.  This includes attaching the visual simulator (defined in `hardware.py`) which reads the registers as they are being written and updates the display accordingly.  The simulator slightly slows down the serial communication. The code simulating the registers of the `LP5036` device be found in `i2c_hardware.py`.

The code simulating I2C communication can be found in `i2c_com.py` which receives forwarded I2C data from `serial_com.py`.

## Dependencies
 - numpy
 - pySerial
 - tkinter
