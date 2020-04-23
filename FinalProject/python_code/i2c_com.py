
# jreamy17@georgefox.edu
# Embedded Systems
# Spring 2020

"""
Defines I2C communication class, command characters, and
  generic I2C device setup.

"""

import ast
import os
import csv

TX_S = b'1'
TX_C = b'2'
TX_E = b'3'
RX_S = b'4'
RX_C = b'5'
RX_E = b'6'

# Define the I2C command characters
I2C_commands = [
    TX_S,
    TX_E,
    RX_S,
    RX_C,
    RX_E
]

# Ensure the memory directory exists
mem_dir = "mem"
if not os.path.isdir(mem_dir):
    os.makedirs(mem_dir)

class I2C():
    """
    Class that simulates I2C communication line.
    """

    def __init__(self, com):
        """
        Creates a new I2C line listening to the given com line.
        """

        # Default things off
        self.devices = {}
        self.active = None
        self.state = None
        self.show = False

        # Connect the com lines
        self.com = com.ser
        com.connect_i2c(self)

    def __setitem__(self, addr, device):
        """
        Sets the i2c address to the given device.
        """

        self.devices[addr] = device

    def __getitem__(self, addr):
        """
        Returns the devices at the given i2c address.
        """

        return self.devices[addr]

    def write(self, data):
        """
        Begins i2c communication.  Data is the first i2c flag,
          either a start read command or a start write command,
          restart is not currently supported so writing must nack
          for instance before reading.
        """

        # If a start read command sent
        if data == RX_S:
            # Get the active device
            self.active = self.com.read(1)
            device = self.devices[self.active]

            # Optionally display device name
            if self.show:
                print("Addr", device.addr, "(" + device.name + ")")
                print("SR")

            # Read bytes from the hardware until NACK sent
            while data != RX_E:
                # Read the data byte from the hardware
                data = device.read()

                # Optionally display the data byte
                if self.show:
                    print("    ", data)

                # Write the data byte to the i2c line
                self.com.write(data)

                # Read the next i2c command
                data = self.com.read(1)

            # Optionally display the NACK
            if self.show:
                print("NACK\n")

        # If a start write command sent
        elif data == TX_S:
            # Get the active device
            self.active = self.com.read(1)
            device = self.devices[self.active]

            # Optionally display the device name
            if self.show:
                print("Addr", device.addr, "(" + device.name + ")")
                print("SW")

            # Prep hardware for a write
            device.start_write()

            # Write data until a STOP received
            flag = None
            while flag != TX_E:
                # Get next flag (either continue or stop)
                flag = self.com.read(1)
                if flag == TX_C:
                    # Read the next byte from the line
                    data = self.com.read(1)

                    # Optionally display the data byte
                    if self.show:
                        print(data)

                    # Write the byte to the hardware
                    device.write(data)

            # Optionally display the NACK
            if self.show:
                print("NACK\n")
        else:
            print("ERR", data)

class i2c_device():
    """
    Generic class defining i2c hardware.
    """

    def __init__(self, addr, name=None, i2c=None):
        """
        Creates a new i2c device with the given address.

        Optionally connects to the given i2c line.
        Optionally opens the reg file with the given name.
        """

        # Save the address
        self.addr = addr

        # Optionally connect to i2c line
        self.i2c=i2c
        if self.i2c:
            self.i2c[self.addr] = self

        # Optionally load regs file
        self.name = name
        self.file = os.path.join("mem", name+".csv") if name else None
        self.load_regs()

        # Make active reg
        self.reg = None

    def start_write(self):
        """
        Starts an i2c write.
        """

        # Clear the active reg
        self.reg = None

    def write(self, data):
        """
        Writes a data byte to the device.

        Auto increments the active register.
        """

        # Get the new active reg if none active
        if self.reg is None:
            self.reg = int.from_bytes(data, 'big')

        # Write and auto increment the given reg
        else:
            reg_idx = self.reg.to_bytes(1, 'big')
            self.regs[reg_idx] = data
            self.reg += 1

    def read(self):
        """
        Reads a data byte from the device.

        Auto increments the active register.
        """

        # Get the byte from the active register
        reg_idx = self.reg.to_bytes(1, 'big')
        data = self.regs[reg_idx]

        # Auto increment and return byte
        self.reg += 1
        return data

    def load_regs(self):
        """
        Loads the register values from the appropriate file if
          name specified on initialization.  If no file exists
          or no name specified, then calls default_regs() to
          get the default reg values specified by the class.
        """

        # Get the reg file if specified
        if self.file is not None and os.path.isfile(self.file):
            self.regs = {}

            # Open the file
            with open(self.file, "r", newline='\n') as f:
                skip = True
                for row in csv.reader(f):
                    k, v = row[0], row[1]
                    # Skip header row
                    if skip:
                        skip = False
                    else:
                        # Read reg, value into device memory
                        self.regs[bytes.fromhex(k)] = bytes.fromhex(v)

        # Get the default regs if csv not available
        else:
            self.regs = self.default_regs()

    def save_regs(self):
        """
        Saves the register values to the appropriate file if
          name specified on initialization.
        """

        # Open the appropriate file
        if self.file is not None:
            with open(self.file, "w") as f:
                # Write the header line
                w = csv.writer(f)
                w.writerow(["mem", "val"])

                # Write the register values to the csv
                for k, v in sorted(self.regs.items(), key=lambda x: x[0]):
                    w.writerow([k.hex(), v.hex()])
