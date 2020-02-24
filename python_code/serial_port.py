

from Data_Reader import Data_Reader

dr = Data_Reader()
dr.send("ttext\n\r")
while not dr.available():
    pass

print(dr.recieve())
