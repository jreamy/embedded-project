
# jreamy17@georgefox.edu
# Embedded Systems
# Spring 2020

"""
Configures the simulated I2C devices.

"""

from i2c_com import I2C
from i2c_hardware import pca9548a, kx003, pca9685

# Mux line configurations
MUX_PNT_TOP = (1<<7).to_bytes(1, 'big')
MUX_MID_TOP = (1<<6).to_bytes(1, 'big')
MUX_PNT_MID = (1<<5).to_bytes(1, 'big')
MUX_RNG_TOP = (1<<4).to_bytes(1, 'big')
MUX_PNK_TOP = (1<<3).to_bytes(1, 'big')
MUX_RNG_PNK = (1<<2).to_bytes(1, 'big')
MUX_THM_TOP = (1<<1).to_bytes(1, 'big')
MUX_THM_HND = (1<<0).to_bytes(1, 'big')

# KX003 addresses
PNT_TIP_ADDR = (b'\x1E')
MID_TIP_ADDR = (b'\x1E')
RNG_TIP_ADDR = (b'\x1E')
PNK_TIP_ADDR = (b'\x1E')
THM_TIP_ADDR = (b'\x1E')

PNT_MID_ADDR = (b'\x1C')
MID_MID_ADDR = (b'\x1C')
RNG_MID_ADDR = (b'\x1C')
PNK_MID_ADDR = (b'\x1C')
THM_MID_ADDR = (b'\x1C')

PNT_BS_ADDR  = (b'\x1E')
MID_BS_ADDR  = (b'\x1C')
RNG_BS_ADDR  = (b'\x1E')
PNK_BS_ADDR  = (b'\x1C')
THM_BS_ADDR  = (b'\x1E')
HND_BS_ADDR  = (b'\x1C')

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

    # Attach the mux to the i2c line
    config.mux = pca9548a(b'\xE0', config.i2c, name="i2c_mux")

    # Attach the kx003 units to the mux lines
    config.mux.add_devices(MUX_PNT_TOP,
        kx003(PNT_TIP_ADDR, "PNT_TIP"), kx003(PNT_MID_ADDR, "PNT_MID"))
    config.mux.add_devices(MUX_MID_TOP,
        kx003(MID_TIP_ADDR, "MID_TIP"), kx003(MID_MID_ADDR, "MID_MID"))
    config.mux.add_devices(MUX_RNG_TOP,
        kx003(RNG_TIP_ADDR, "RNG_TIP"), kx003(RNG_MID_ADDR, "RNG_MID"))
    config.mux.add_devices(MUX_PNK_TOP,
        kx003(PNK_TIP_ADDR, "PNK_TIP"), kx003(PNK_MID_ADDR, "PNK_MID"))
    config.mux.add_devices(MUX_THM_TOP,
        kx003(THM_TIP_ADDR, "THM_TIP"), kx003(THM_MID_ADDR, "THM_MID"))
    config.mux.add_devices(MUX_PNT_MID,
        kx003(PNT_BS_ADDR, "PNT_BS"), kx003(MID_BS_ADDR, "MID_BS"))
    config.mux.add_devices(MUX_RNG_PNK,
        kx003(RNG_BS_ADDR, "RNG_BS"), kx003(PNK_BS_ADDR, "PNK_BS"))
    config.mux.add_devices(MUX_THM_HND,
        kx003(THM_BS_ADDR, "THM_BS"), kx003(HND_BS_ADDR, "HND_BS"))

    # Attach the servo controller to the i2c line
    config.servos = pca9685(b'\x80', "servos", i2c=config.i2c)

    # Do an initial dump of all the regs
    config.servos.save_regs()
    config.mux.save_regs()

    # Return the hardware config
    return config
