
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
from models import Predictive_Model
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

def arg_parse():
    """
    Function to get arguments from the command line input.
    """

    ap = argparse.ArgumentParser()
    ap.add_argument("-m", "--mode", default="Train",
        help="the mode to run in")
    ap.add_argument("-d", "--data", default="data",
        help="path for data to be saved to")
    ap.add_argument("-r", "--reset", action="store_true",
        help="whether to reset the data csvs")
    ap.add_argument("-p", "--print", nargs="+",
        help="whenther to print data to console")

    return vars(ap.parse_args())

def change_mode(new_mode):
    """
    Function that sends the mode change command and waits
      for Arduino to respond with the mode change message.
    """

    # Send the mode change command
    sercom.send(new_mode)
    sercom.wait()

    # Receive data from serial until mode change takes effect
    com, data = sercom.receive()
    while com != 't':
        com, data = sercom.receive()
    return com, data

def train(data_path="data", reset=False):
    """
    Train mode.  Collects data from the EMG sensor and hand
      position monitoring accelerometers.  Stores data in
      csvs.

    Keyword Arguments:
    data_path : String or Path obeject, default "data"
        Path used to store csvs in
    reset : boolean, default False
        Whether to delete pre-existing data csv or append.
    """

    # Reset the idx for csv logging
    data_idx = 0

    # Ensure the directory exists
    if not os.path.isdir(data_path):
        os.makedirs(data_path)

    # Ensure the EMG data file exists
    emg_file = os.path.join(data_path, "emg.csv")

    if not os.path.isfile(emg_file) or reset:
        with open(emg_file, "w+") as f:
            writer(f).writerow(["idx", "ch1", "ch2", "ch3"])

    # Open the EMG file writer
    emg_csv = open(emg_file, "a+")
    emg_writer = writer(emg_csv)

    # Ensure the Hand data file exists
    hand_file = os.path.join(data_path, "hand.csv")

    if not os.path.isfile(hand_file) or reset:
        with open(hand_file, "w+") as f:
            writer(f).writerow(["idx", *list(range(48))])

    # Open the Hand file writer
    hand_csv = open(hand_file, "a+")
    hand_writer = writer(hand_csv)

    # Wait for mode change message from Arduino
    print(change_mode(MODE_TRAIN))

    com = None
    while com != EXIT:
        # receive serial / i2c data
        sercom.wait()
        com, data = sercom.receive()

        # Receive EMG data
        if com == 'e':
            emg_writer.writerow([data_idx, *data])
            data_idx += 1

        # Receive hand position data
        elif com == 'h':
            hand_writer.writerow([data_idx, *data])

    # Close the CSVs if still open
    emg_csv.close()
    hand_csv.close()

def control():
    """
    Comtrol mode.  Compute controls the prosthetic based on EMG
      data sent by the Arduino.  This function provides I2C
      simulation.
    """

    # Get the predictive model that would be used
    model = Predictive_Model()

    # Wait for mode change message from Arduino
    print(change_mode(MODE_CONTROL))

    # Store the time
    prev = time.time()

    # Run until stop command received
    com = None
    while com != EXIT:
        # receive serial / i2c data
        sercom.wait()
        com, data = sercom.receive()

        # Receive EMG data
        if com == 'e':
            model += data

        # Receive Compute command
        elif com == 'c':
            prediction = model.predict()
            for val in prediction:
                sercom.send(val)

        # Periodically store the i2c register values
        if time.time() - prev > 1:
            prev = time.time()
            i2ccom.mux.save_regs()
            i2ccom.servos.save_regs()

def mirror():
    """
    Mirror mode.  Arduino mirrors the measured hand position
      in the prosthetic.  Demonstrates working servos.
    """

    # Wait for mode change message from Arduino
    print(change_mode(MODE_MIRROR))

    # Store the time
    prev = time.time()

    # Run until stop command received
    com = None
    while com != EXIT:
        # receive serial / i2c data
        sercom.wait()
        com, data = sercom.receive()

        # Debug any non-i2c commands received
        if not com in I2C_commands:
            print(com, data)

        # Periodically store the i2c register values
        if time.time() - prev > 1:
            prev = time.time()
            i2ccom.mux.save_regs()
            i2ccom.servos.save_regs()

def solo():
    """
    Solo mode.  Arduino does on board computation of EMG data
      and controls the prosthetic accordingly.  This function
      provides I2C simulation.
    """

    # Wait for mode change message from Arduino
    print(change_mode(MODE_SOLO))

    # Store the time
    prev = time.time()

    # Run until stop command received
    com = None
    while com != EXIT:
        # receive serial / i2c data
        sercom.wait()
        com, data = sercom.receive()

        # Debug any non-i2c commands received
        if not com in I2C_commands:
            print(com, data)

        # Periodically store the i2c register values
        if time.time() - prev > 1:
            prev = time.time()
            i2ccom.mux.save_regs()
            i2ccom.servos.save_regs()

        print(time.time())

def run_i2c():
    """
    I2C mode.  Used for developing and testing I2C protocol.
    """

    # Run until stop command received
    com = None
    while com != EXIT:
        # receive serial / i2c data
        sercom.wait()
        com, data = sercom.receive()

def main(args):
    """
    Parses args and begins the corrosponding operation mode.
    """

    # Set the i2c or serial to display what's happening
    i2ccom.i2c.show = ("i2c" in args["print"])
    sercom.show = ("serial" in args["print"])

    # Run the correct mode
    if args["mode"].lower() == "train":
        train(data_path=args["data"], reset=args["reset"])
    elif args["mode"].lower() == "control":
        control()
    elif args["mode"].lower() == "mirror":
        mirror()
    elif args["mode"].lower() == "solo":
        solo()
    elif args["mode"].lower() == "i2c":
        run_i2c()

    # Final dump of all simulated hardware regs
    i2ccom.mux.save_regs()
    i2ccom.servos.save_regs()

# When executed, runs the main with the command line args
if __name__ == "__main__":
    args = arg_parse()
    main(args)
