
class Hand():

    def __init__(self, data):
        self.data = data

    def __getitem__(self, index):
        if isinstance(index, int):
            return self.data[index]
        elif index == 'thumb':
            return self.data[0:3]
        elif index == 'pointer':
            return self.data[3:6]
        elif index == "middle":
            return self.data[6:9]
        elif index == "ring":
            return self.data[9:12]
        elif index == "pinky":
            return self.data[12:15]
        elif index == "base":
            return self.data[15]
        else:
            raise ValueError("{} not a valid index".format(index))

    def __setitem__(self, index, value):
        if isinstance(index, int):
            self.data[index] = value
        else:
            raise ValueError("{} not a valid index".format(index))

    def __str__(self):
        return str(self.data)

    def encode(self):
        return b''.join([x.to_bytes(2, 'little') for x in self.data])
