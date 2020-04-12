
from Data_Reader import Data_Reader

dr = Data_Reader("COM3", 1000000)
dr.wait()

while True:
    dr.wait()
    com, data = dr.recieve()

    if com == 'i':
        print(com, data)
    elif com == 'h':
        print(com, data)
    elif com == 'c':
        print(com, data)
