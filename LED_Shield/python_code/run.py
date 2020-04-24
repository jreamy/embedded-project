
# jreamy17@georgefox.edu
# Embedded Systems
# Spring 2020

"""
Main script to interface with the Arduino in this project.  Uses
  serial interface for primary communication and performs I2C ->
  serial forwarding for I2C device simulation.

"""

from serial_com import SERCOM
from i2c_config import setup_hardware
from csv import writer
import numpy as np
import os
from i2c_com import *
import time

import argparse

# Define the mode change commands (same as for the Arduino code)
MODE_TRAIN   = 'T'
MODE_CONTROL = 'C'
MODE_MIRROR	 = 'M'
MODE_SOLO    = 'S'

EXIT = b'X' # exit command useful for debug

# Set up the serial and i2c lines
sercom = SERCOM("COM3", 1000000)
i2ccom = setup_hardware(sercom)

def run_i2c():
    """
    I2C mode.  Displays the I2C communication between
      the Arduino and simulated I2C hardware devices
    """

    # Store the time
    prev = time.time()

    # Run until stop command received
    com = None
    while com != EXIT:

        # receive serial / i2c data
        if (sercom.available()):
            com, data = sercom.receive()
            i2ccom.leds.sim.update()

        # Periodically store the i2c register values
        if time.time() - prev > 1:
            prev = time.time()
            i2ccom.leds.save_regs()

def main():
    """
    Parses args and begins the corrosponding operation mode.
    """

    # Set the i2c to display what's happening
    i2ccom.i2c.show = False
    sercom.show = False

    # Run the i2c_communication
    run_i2c()

    # Final dump of all simulated hardware regs
    i2ccom.leds.save_regs()

# When executed, runs the main
if __name__ == "__main__":
    main()
