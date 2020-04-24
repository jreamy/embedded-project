
# jreamy17@georgefox.edu
# Embedded Systems
# Spring 2020

"""
Serial Communication protocol.  Uses the pySerial serial object
  to communicate with Arduino via serial port.  Performs I2C ->
  serial forwarding for I2C device simulation.

"""

import serial
from time import sleep
from i2c_com import I2C_commands

def uint16_list(uint8_list):
    """
    Converts the list of uint8 bytes to uint16 data.
    """

    return [(uint8_list[x*2+1]<<8) | uint8_list[x*2]
        for x in range(int(len(uint8_list)/2))]

def int16_list(uint8_list):
    """
    Converts the list of uint8 bytes to signed int16 data.
    """

    print(uint8_list[2:2+2])
    return [(int.from_bytes(uint8_list[x*2:x*2+2], 'little', signed=True))
        for x in range(int(len(uint8_list)/2))]

class SERCOM():
    """
    Serial port listener class.
    """

    def __init__(self, port='/dev/ttyACM0', baudrate=115200):
        """
        Initializes the given port at the given baud rate.
        """

        self.ser = serial.Serial(port=port, baudrate=baudrate)
        self.show = False

    def __del__(self):
        """
        forward the delete command to the serial port object.
        """

        self.ser.__del__()

    def send(self, data):
        """
        Encodes and sends the data to the connected Arduino.
        """

        # Make sure the data can be encoded
        if isinstance(data, (str)):
            for char in data:
                while self.ser.out_waiting:
                    pass
                print(char)
                self.ser.write(char.encode())
        elif isinstance(data, (int)):
            self.ser.write(data.to_bytes(1, 'little'))

        else:
            raise Exception("Non-encodable data sent!")

    def receive(self):
        """
        Receives and processes data from the serial port.
        """

        # Read the command char
        command_char = self.ser.read(1)

        # Forwards to I2C commands
        if command_char in I2C_commands:
            self.i2c.write(command_char)
            com, data = command_char, None

        # Not impletented commands or erroneous commands return
        else:
            data = command_char
            com = command_char

        # Optionally print the serial data
        if self.show:
            print(com, data)

        # Return the command and data
        return com, data

    def wait(self):
        """
        Waits for serial data to arrive.
        """

        while not self.available():
            pass

    def available(self):
        """
        Returns whether serial data is present.
        """

        return self.ser.in_waiting

    def connect_i2c(self, i2c):
        """
        Connects an I2C parasite :) for I2C -> Serial forward
        """

        self.i2c = i2c
