
import ast
import os
import csv

TX_S = b'1'
TX_C = b'2'
TX_E = b'3'
RX_S = b'4'
RX_C = b'5'
RX_E = b'6'

I2C_commands = [
    TX_S,
    TX_E,
    RX_S,
    RX_C,
    RX_E
]

class I2C():
    def __init__(self, com):
        self.devices = {}
        self.active = None
        self.state = None
        self.com = com.ser
        self.show = False
        com.connect_i2c(self)

    def __setitem__(self, addr, device):
        self.devices[addr] = device

    def __getitem__(self, addr):
        return self.devices[addr]

    def write(self, data):
        if data == RX_S:
            self.active = self.com.read(1)
            device = self.devices[self.active]
            if self.show:
                print("Addr", device.addr, "(" + device.name + ")")
                print("SR")
            while data != RX_E:
                data = device.read()
                if self.show:
                    print("    ", data)
                self.com.write(data)
                data = self.com.read(1)
            if self.show:
                print("NACK\n")
        elif data == TX_S:
            self.active = self.com.read(1)
            device = self.devices[self.active]
            if self.show:
                print("Addr", device.addr, "(" + device.name + ")")
                print("SW")
            device.start_write()
            flag = None
            while flag != TX_E:
                flag = self.com.read(1)
                if flag == TX_C:
                    data = self.com.read(1)
                    if self.show:
                        print(data)
                    device.write(data)

            if self.show:
                print("NACK\n")
        else:
            print("ERR", data)

class i2c_device():
    def __init__(self, addr, name=None, i2c=None):
        self.addr = addr
        self.i2c=i2c
        if self.i2c:
            self.i2c[self.addr] = self
        self.name = name
        self.file = os.path.join("mem", name+".csv") if name else None
        self.load_regs()
        self.reg = 0

    def start_write(self):
        self.reg = None

    def write(self, data):
        if self.reg is None:
            self.reg = int.from_bytes(data, 'big')
        else:
            reg_idx = self.reg.to_bytes(1, 'big')
            self.regs[reg_idx] = data
            self.reg += 1

    def read(self):
        reg_idx = self.reg.to_bytes(1, 'big')
        data = self.regs[reg_idx]
        self.reg += 1
        return data

    def load_regs(self):
        if self.file is not None and os.path.isfile(self.file):
            self.regs = {}
            with open(self.file, "r", newline='\n') as f:
                skip = True
                for row in csv.reader(f):
                    k, v = row[0], row[1]
                    if skip:
                        skip = False
                    else:
                        self.regs[bytes.fromhex(k)] = bytes.fromhex(v)
        else:
            self.regs = self.default_regs()

    def save_regs(self):
        if self.file is not None:
            with open(self.file, "w") as f:
                w = csv.writer(f)
                w.writerow(["mem", "val"])
                for k, v in sorted(self.regs.items(), key=lambda x: x[0]):
                    w.writerow([k.hex(), v.hex()])


class i2c_mux(i2c_device):
    def __init__(self, addr, i2c, name=None):
        super().__init__(addr, name=name, i2c=i2c)
        self.active = None
        self.devices = {
            b'\x01':[], b'\x02':[], b'\x04':[], b'\x08':[],
            b'\x10':[], b'\x20':[], b'\x40':[], b'\x80':[]
        }

    def add_devices(self, prefix, *devices):
        for device in devices:
            self.devices[prefix].append(device)

    def write(self, data):
        super().write(data)
        # Remove the old devices
        if self.active is not None:
            for device in self.devices[self.active]:
                self.i2c[device.addr] = None

        # Add all new devices
        if data in self.devices.keys():
            for device in self.devices[data]:
                self.i2c[device.addr] = device
            self.active = data
        else:
            self.active = None

    def start_write(self):
        self.reg = 0

    def save_regs(self):
        super().save_regs()
        for prefix in self.devices.keys():
            for device in self.devices[prefix]:
                device.save_regs()

    def default_regs(self):
        return {b'\x00' : b'\x00'}

class kx003(i2c_device):
    def __init__(self, addr, name=None, i2c=None):
        super().__init__(addr, name=name, i2c=i2c)

    def default_regs(self):
        regs = {
            b'\x06':b'\x00', b'\x07':b'\x00', b'\x08':b'\x00',
            b'\x09':b'\x00', b'\x0A':b'\x00', b'\x0B':b'\x00'
        }

        if self.name is None:
            pass
        elif "HND" in self.name:
            regs[b'\x0B'] = ( 32).to_bytes(1, 'big', signed=True)
        elif "BS" in self.name:
            regs[b'\x0B'] = ( 16).to_bytes(1, 'big', signed=True)
            regs[b'\x09'] = ( 16).to_bytes(1, 'big', signed=True)
        elif "TIP" in self.name:
            regs[b'\x0B'] = (-32).to_bytes(1, 'big', signed=True)
        elif "MID" in self.name:
            regs[b'\x0B'] = (-16).to_bytes(1, 'big', signed=True)
            regs[b'\x09'] = ( 16).to_bytes(1, 'big', signed=True)
        return regs

class servos(i2c_device):
    def __init__(self, addr, name=None, i2c=None):
        super().__init__(addr, name=name, i2c=i2c)

    def default_regs(self):
        return {}
