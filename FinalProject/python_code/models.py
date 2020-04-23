
# jreamy17@georgefox.edu
# Embedded Systems
# Spring 2020

"""
Defines the predictive model that would be used
  if valid data were available.

"""

import numpy as np
from collections import deque

EXTERNAL_COMP_SAMPLE = 128
NUM_JOINTS = 15

class Predictive_Model():
    """
    The predictive model that would do all
      external computation for the Arduino
      when in Control mode.
    """

    def __init__(self):
        """
        Creates a new Predictive Model.
        """

        self.data = deque((0,0,0) for x in range(EXTERNAL_COMP_SAMPLE))

    def __add__(self, emg_sample):
        """
        Adds a data sample to the model
        """

        self.data.append(emg_sample)
        self.data.popleft()
        return self

    def predict(self):
        """
        Performs the model computation.
        """

        # Prep data for computation (numpy array)
        comp_data = np.array(self.data)

        # Return dummy computation
        return [240 - 10*x for x in range(NUM_JOINTS)]
