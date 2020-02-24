
import serial

class Data_Reader():

    def __init__(self, port='/dev/ttyACM0', baudrate=115200):
        self.ser = serial.Serial(port=port, baudrate=baudrate)

    def send(self, text):
        if isinstance(text, str):
            self.ser.write(text.encode())

    def recieve(self):
        command_char = self.ser.read(1)
        if command_char == b"t":
            return self.ser.read(5)
        else:
            return command_char

    def available(self):
        return self.ser.in_waiting
