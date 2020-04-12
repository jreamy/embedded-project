
from Data_Reader import Data_Reader

dr = Data_Reader("COM3")
dr.wait()

while True:
    dr.wait()
    com, data = dr.recieve()

    if com == 'i':
        print(com, data)
    elif com == 'h':
        print(data['base'])
