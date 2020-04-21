
class joint():
    def __init__(self, finger, start_idx):
        self.data = finger
        self.idx = start_idx

    def __getitem__(self, index):
        if isinstance(index, int):
            return self.data[self.idx + index]
        elif index == "x":
            return self.data[self.idx+0]
        elif index == "y":
            return self.data[self.idx+1]
        elif index == "z":
            return self.data[self.idx+2]
        else:
            raise ValueError("{} not a valid finger index".format(index))

    def __setitem__(self, index, value):
        print("joint set")
        if isinstance(index, int):
            self.data[self.idx + index] = value
        elif index == "x":
            self.data[self.idx] = value
        elif index == "y":
            self.data[self.idx+1] = value
        elif index == "z":
            self.data[self.idx+2] = value
        else:
            raise ValueError("{} not a valid joint index".format(index))

    def __setattr__(self, attr, value):
        if attr in ["x", "y", "z"]:
            idx = ["x", "y", "z"].index(attr)
            self[idx] = value
        else:
            self.__dict__[attr] = value

class finger():
    def __init__(self, hand, start_idx):
        self.data = hand
        self.idx = start_idx
        self.base = joint(self.data, self.idx)
        self.middle = joint(self.data, self.idx+3)
        self.tip = joint(self.data, self.idx+6)

    def __getitem__(self, index):
        if isinstance(index, int):
            return self.data[self.idx + index]
        elif index in self.__dict__:
            return self.__dict__[index]
        else:
            raise ValueError("{} not a valid finger index".format(index))

    def __setitem__(self, index, value):
        print("finger set")
        if isinstance(index, int):
            self.data[self.idx + index] = value
        else:
            raise ValueError("{} not a valid finger index".format(index))

    def __str__(self):
        return str(self.data[self.idx:self.idx+9])

class Hand():

    def __init__(self, data):
        self.data = data
        self.thumb = finger(self.data, 9*0)
        self.pointer = finger(self.data, 9*1)
        self.middle = finger(self.data, 9*2)
        self.ring = finger(self.data, 9*3)
        self.pinky = finger(self.data, 9*4)
        self.base = joint(self.data, 9*5)

    def __getitem__(self, index):
        if isinstance(index, int):
            return self.data[index]
        elif index in self.__dict__:
            return self.__dict__[index]
        else:
            raise ValueError("{} not a valid hand index".format(index))

    def __setitem__(self, index, value):
        if isinstance(index, int):
            self.data[index] = value
        else:
            raise ValueError("{} not a valid hand index".format(index))

    def __str__(self):
        return str(self.data)

    def encode(self):
        return b''.join([x.to_bytes(2, 'little') for x in self.data])
