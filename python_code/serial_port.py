

from Data_Reader import Data_Reader

dr = Data_Reader()
dr.send("text\n\r")
while not dr.available():
    pass
while dr.available():
    print(dr.recieve(6))
