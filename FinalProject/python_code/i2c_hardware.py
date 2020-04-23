
# jreamy17@georgefox.edu
# Embedded Systems
# Spring 2020

"""
Defines the various I2C hardware devices being simulated in this
  project.  Defines the I2C mux (PCA9548A), the accelerometer
  (KX003), and the servo / led driver (PCA9685).

"""

from i2c_com import i2c_device

class pca9548a(i2c_device):
    """
    PCA9548A I2C device simulation. (I2C 8 channel Mux)
    """

    def __init__(self, addr, i2c, name=None):
        """
        Creates a new I2C mux.
        """

        # Init the device
        super().__init__(addr, name=name, i2c=i2c)

        # Set the following i2c lines.
        self.active = None
        self.devices = {
            b'\x00':[], # Used for no-connect
            b'\x01':[], b'\x02':[], b'\x04':[], b'\x08':[],
            b'\x10':[], b'\x20':[], b'\x40':[], b'\x80':[]
        }

    def add_devices(self, prefix, *devices):
        """
        Adds the input devices to the given i2c line.
        """

        # Add all device to the line
        for device in devices:
            self.devices[prefix].append(device)

    def write(self, data):
        """
        Writes to the device and connects the devices on the
          appropriate line to the main I2C line.
        """

        # Write the mux's reg
        super().write(data)

        # Remove the old devices
        if self.active is not None:
            for device in self.devices[self.active]:
                self.i2c[device.addr] = None

        # Add all new devices
        if data in self.devices.keys():
            for device in self.devices[data]:
                self.i2c[device.addr] = device
            self.active = data
        else:
            self.active = None

        # 1 reg device.
        self.reg = 0

    def start_write(self):
        """
        1-register device, always write reg 0.
        """

        self.reg = 0

    def save_regs(self):
        """
        Dumps the reg of this device along with all devices
          attached to all of its outgoing I2C lines.
        """

        # Dump its own reg
        super().save_regs()

        # Dump all device regs
        for prefix in self.devices.keys():
            for device in self.devices[prefix]:
                device.save_regs()

    def default_regs(self):
        """
        Returns the default 'all off' register configuration.
        """

        # Default the active reg to all off
        return {b'\x00' : b'\x00'}

class kx003(i2c_device):
    """
    KX003 I2C device simulation. (I2C accelerometer)
    """

    def default_regs(self):
        """
        Returns the default reg configuration.  Currently set
          to a 'loose curl' position of all fingers.
        """

        regs = {
            b'\x06':b'\x00', b'\x07':b'\x00', b'\x08':b'\x00',
            b'\x09':b'\x00', b'\x0A':b'\x00', b'\x0B':b'\x00'
        }

        if self.name is None:
            pass
        elif "HND" in self.name:
            regs[b'\x0B'] = ( 32).to_bytes(1, 'big', signed=True)
        elif "BS" in self.name:
            regs[b'\x0B'] = ( 16).to_bytes(1, 'big', signed=True)
            regs[b'\x09'] = ( 16).to_bytes(1, 'big', signed=True)
        elif "TIP" in self.name:
            regs[b'\x0B'] = (-32).to_bytes(1, 'big', signed=True)
        elif "MID" in self.name:
            regs[b'\x0B'] = (-16).to_bytes(1, 'big', signed=True)
            regs[b'\x09'] = ( 16).to_bytes(1, 'big', signed=True)
        return regs

class pca9685(i2c_device):
    """
    PCA9685 I2C device simulation. (I2C LED / Servo driver)
    """

    def default_regs(self):
        """
        Returns default empty regs.
        """

        return {}
