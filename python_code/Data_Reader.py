
import serial
from hand import Hand

class Data_Reader():

    def __init__(self, port='/dev/ttyACM0', baudrate=115200):
        self.ser = serial.Serial(port=port, baudrate=baudrate)

    def __del__(self):
        self.ser.__del__()

    def send(self, text):
        if isinstance(text, (str, Hand)):
            self.ser.write(text.encode())

    def recieve(self):
        command_char = self.ser.read(1)
        if command_char == b't':
            data = self.ser.readline(10)
            return ('t', ''.join([chr(int(x)) for x in data[:-1]]))
        elif command_char == b'i':
            data = self.ser.read(3)
            return ('i', [int(x) for x in data])
        elif command_char == b'h':
            data = self.ser.read(32)
            return ('h', Hand([(data[x*2+1]<<8) | data[x*2]
                for x in range(16)]))
        elif command_char == b'c':
            data = self.ser.read(32)
            return ('c', Hand([(data[x*2+1]<<8) | data[x*2]
                for x in range(16)]))
        else:
            return (command_char, command_char)

    def wait(self):
        while not self.available():
            pass

    def available(self):
        return self.ser.in_waiting
