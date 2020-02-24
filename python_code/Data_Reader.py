
import serial

class Data_Reader():

    def __init__(self, port='/dev/ttyACM0', baudrate=115200):
        self.ser = serial.Serial(port=port, baudrate=baudrate)

    def send(self, text):
        if isinstance(text, str):
            self.ser.write(text.encode())

    def recieve(self, bytes):
        return self.ser.read(size=bytes)

    def available(self):
        return self.ser.in_waiting
