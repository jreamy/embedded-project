
from Data_Reader import Data_Reader

dr = Data_Reader()
dr.wait()

while True:
    dr.wait()
    print(dr.recieve())
