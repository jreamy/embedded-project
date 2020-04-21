
from Data_Reader import Data_Reader
from csv import writer
import numpy as np
import os
from i2c import *
import time

import argparse

MODE_TRAIN   = 'T'
MODE_CONTROL = 'C'
MODE_MIRROR	 = 'M'
MODE_SOLO    = 'S'

EXIT = b'X'

MUX_PNT_TOP = (1<<7).to_bytes(1, 'big')
MUX_MID_TOP = (1<<6).to_bytes(1, 'big')
MUX_PNT_MID = (1<<5).to_bytes(1, 'big')
MUX_RNG_TOP = (1<<4).to_bytes(1, 'big')
MUX_PNK_TOP = (1<<3).to_bytes(1, 'big')
MUX_RNG_PNK = (1<<2).to_bytes(1, 'big')
MUX_THM_TOP = (1<<1).to_bytes(1, 'big')
MUX_THM_HND = (1<<0).to_bytes(1, 'big')

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

dr = Data_Reader("COM3", 1000000)

i2c_line = I2C(dr)

mux = i2c_mux(b'\xE0', i2c_line, name="mux")
mux.add_devices(MUX_PNT_TOP,
    kx003(PNT_TIP_ADDR, "PNT_TIP"), kx003(PNT_MID_ADDR, "PNT_MID"))
mux.add_devices(MUX_MID_TOP,
    kx003(MID_TIP_ADDR, "MID_TIP"), kx003(MID_MID_ADDR, "MID_MID"))
mux.add_devices(MUX_RNG_TOP,
    kx003(RNG_TIP_ADDR, "RNG_TIP"), kx003(RNG_MID_ADDR, "RNG_MID"))
mux.add_devices(MUX_PNK_TOP,
    kx003(PNK_TIP_ADDR, "PNK_TIP"), kx003(PNK_MID_ADDR, "PNK_MID"))
mux.add_devices(MUX_THM_TOP,
    kx003(THM_TIP_ADDR, "THM_TIP"), kx003(THM_MID_ADDR, "THM_MID"))
mux.add_devices(MUX_PNT_MID,
    kx003(PNT_BS_ADDR, "PNT_BS"), kx003(MID_BS_ADDR, "MID_BS"))
mux.add_devices(MUX_RNG_PNK,
    kx003(RNG_BS_ADDR, "RNG_BS"), kx003(PNK_BS_ADDR, "PNK_BS"))
mux.add_devices(MUX_THM_HND,
    kx003(THM_BS_ADDR, "THM_BS"), kx003(HND_BS_ADDR, "HND_BS"))

servo_ctrl = servos(b'\x80', "servos", i2c=i2c_line)

servo_ctrl.save_regs()
mux.save_regs()

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
    dr.send(new_mode)
    dr.wait()
    com, data = dr.receive()
    while (com != 't'):
        dr.wait()
        com, data = dr.receive()
    return com, data

def train(data_path="data", reset=False, show=False):
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

    print(change_mode(MODE_TRAIN))

    com = None
    while com != EXIT:
        dr.wait()
        com, data = dr.receive()

        if show:
            print(com, data)

        if com == 'e':
            emg_writer.writerow([data_idx, *data])
            data_idx += 1

        elif com == 'h':
            hand_writer.writerow([data_idx, *data])

    # Close the CSVs
    emg_csv.close()
    hand_csv.close()

def control():
    print(change_mode(MODE_CONTROL))

def mirror():
    print(change_mode(MODE_MIRROR))

    prev = time.time()

    com = None
    while com != EXIT:
        dr.wait()
        com, data = dr.receive()

        if not com in I2C_commands:
            print(com, data)

        if time.time() - prev > 1:
            prev = time.time()
            mux.save_regs()
            servo_ctrl.save_regs()

def run_i2c():
    com = None
    while com != EXIT:
        dr.wait()
        com, data = dr.receive()

def solo():
    print(change_mode(MODE_SOLO))

def main(args):
    i2c_line.show = ("i2c" in args["print"])
    dr.show = ("serial" in args["print"])
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

    mux.save_regs()
    servo_ctrl.save_regs()

if __name__ == "__main__":
    args = arg_parse()
    main(args)
