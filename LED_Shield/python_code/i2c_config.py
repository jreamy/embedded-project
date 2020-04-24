
# jreamy17@georgefox.edu
# Embedded Systems
# Spring 2020

"""
Configures the simulated I2C devices.

"""

from i2c_com import I2C
from i2c_hardware import LP5036
from hardware import LP5036_simulator

# Mux line configurations
LED_DRIVER = (b'\x30')

# Class for holding various i2c components
class hardware_config():
    pass

def setup_hardware(sercom):
    """
    Initializes the i2c line and all the simulated hardware components,
      and connects them to the given serial line allowing I2C simulation.
    """

    # Get the hardware config object and attach an i2c line
    config = hardware_config()
    config.i2c = I2C(sercom)

    # Attach the led driver to the i2c line
    config.leds = LP5036(LED_DRIVER, name="LED_DRIVER", i2c=config.i2c)
    LP5036_simulator(config.leds)

    # Do an initial dump of all the regs
    config.leds.save_regs()

    # Return the hardware config
    return config
