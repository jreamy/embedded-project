
import serial
from hand import Hand
from time import sleep
from i2c import I2C_commands

def uint16_list(uint8_list):
    return [(uint8_list[x*2+1]<<8) | uint8_list[x*2]
        for x in range(int(len(uint8_list)/2))]

def int16_list(uint8_list):
    print(uint8_list[2:2+2])
    return [(int.from_bytes(uint8_list[x*2:x*2+2], 'little', signed=True))
        for x in range(int(len(uint8_list)/2))]

class Data_Reader():

    def __init__(self, port='/dev/ttyACM0', baudrate=115200):
        self.ser = serial.Serial(port=port, baudrate=baudrate)
        self.show = False

    def __del__(self):
        self.ser.__del__()

    def send(self, text):
        if isinstance(text, (str, Hand)):
            for char in text:
                while self.ser.out_waiting:
                    pass
                print(char)
                self.ser.write(char.encode())

    def receive(self):
        command_char = self.ser.read(1)
        if command_char == b't':
            data = ['d']
            while data[-1] != '\x00':
                data.append(chr(self.ser.readline(1)[0]))
            com = 't'
            data = ''.join(data[1:-1])
        elif command_char == b'i':
            data = self.ser.read(3)
            com = 'i'
            data = [int(x) for x in data]
        elif command_char == b'e':
            data = self.ser.read(6)
            com = 'e'
            data = uint16_list(data)
        elif command_char == b'h':
            data = self.ser.read(96)
            com = 'h'
            data = Hand(int16_list(data))
        elif command_char in I2C_commands:
            self.i2c.write(command_char)
            return command_char, None
        else:
            data = command_char
            com = command_char

        if self.show:
            print(com, data)
        return com, data

    def wait(self):
        while not self.available():
            print("", end="")

    def available(self):
        return self.ser.in_waiting

    def connect_i2c(self, i2c):
        self.i2c = i2c
