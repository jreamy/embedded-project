
# jreamy17@georgefox.edu
# Embedded Systems
# Spring 2020

"""
Defines the LED driver chip (LP5036) used in this
  project.

"""

from i2c_com import i2c_device

class LP5036(i2c_device):
    """
    LP5036 I2C device simulation. (12-LED driver chip)
    """

    def __init__(self, addr, name=None, i2c=None):
        """
        Creates a new LP5036 simulator connected to the
          given 'sim' output.

        """

        # Initialize the device and save the simulator
        super().__init__(addr, name=name, i2c=i2c)
        self.sim = None

    def default_regs(self):
        """
        Returns default empty regs.
        """

        return {}
