"""
 Implement here effects that are to be tested with this framework.
 Initialize the effect with desired default parameters.
 Implement set_params for setting effect parametrs and process for processing blocks of audio data
 in numpy arrays of size [number of samples per buffer, number of channels].
 Implement a help function for explaining purpose of each effect and its parameters.
"""
import numpy as np

class FxGain():
    """
    Effect description and its parameters goes here.

    """
    
    def __init__(self):
	    self.gain = 1.0
	    self.mute = 0
	    print("Initializing FxGain with default Values.")
	    print("gain: ",self.gain)
	    print("gain: ",self.mute)
    
    def set_params(self, p_list = []):
	    # self.gain = p_gain
	    # self.mute = p_mute
	    self.gain = float(p_list[0])
	    self.mute = float(p_list[1])
	    print("FxGain parameters set as:")
	    print("gain: ",self.gain)
	    print("gain: ",self.mute)
    
    def process(self, input_array):
        if self.mute:
    	    output_array = np.zeros_like(input_array)
        else:
    	    output_array = input_array*self.gain

        return output_array

    def help(self):
	    print("-------------")
	    print("FxGain effect")
	    print("-------------")
	    print("Description:")
	    print("Apply specified gain to input and mute function.")
	    print("Parameters:")
	    print("gain: (float) Apply this amount of gain to the input signal")
	    print("mute: (int) When set, the input signal is muted. Set to zero to stop muting function")

