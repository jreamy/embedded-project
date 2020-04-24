
# jreamy17@georgefox.edu
# Embedded Systems
# Spring 2020

from tkinter import *
import itertools
import threading

"""
Defines the LED driver chip (LP5036) hardware simulation.

"""

class LP5036_simulator(threading.Thread):

    def __init__(self, device):
        super().__init__()
        self.device = device
        self.device.sim = self
        self.active = False
        self.start()

    def callback(self):
        self.tk.quit()

    def run(self):
        self.tk = Tk()
        self.tk.title("LP5036_simulator")
        w = 50
        self.tk.protocol("WM_DELETE_WINDOW", self.callback)

        self.cv = Canvas(self.tk, bg="black", width=5*w, height=5*w)
        self.cv.pack()
        self.grid = [
            self.cv.create_rectangle(w*col, w*row, w*col+w, w*row+w)
                for row, col in itertools.product(range(5), range(5))
        ]

        self.active = True
        self.tk.mainloop()

    def update(self):
        if not self.active:
            return None

        for led_idx in range(12):
            br_reg = (led_idx + 8).to_bytes(1, 'little')
            r_reg = (20 + led_idx*3).to_bytes(1, 'little')
            g_reg = (21 + led_idx*3).to_bytes(1, 'little')
            b_reg = (22 + led_idx*3).to_bytes(1, 'little')
            r_val = int.from_bytes(self.device.regs[r_reg], 'little')
            g_val = int.from_bytes(self.device.regs[g_reg], 'little')
            b_val = int.from_bytes(self.device.regs[b_reg], 'little')
            br_val = int.from_bytes(self.device.regs[br_reg], 'little')

            # Update the color of the led
            color = self.get_color(r_val, g_val, b_val, br_val)
            self.cv.itemconfig(self.get_led(led_idx), fill=color)

    def get_led(self, idx):
        loc = [1,2,3,9,14,19,23,22,21,15,10,5][idx]
        return self.grid[loc]

    def get_color(self, r, g, b, br):
        r *= br / 255.0
        g *= br / 255.0
        b *= br / 255.0
        return "#{:02x}{:02x}{:02x}".format(int(r), int(g), int(b))
