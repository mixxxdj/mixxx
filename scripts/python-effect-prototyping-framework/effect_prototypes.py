"""
 Implement here effects that are to be tested with this framework.
 Initialize the effect with desired default parameters.
 Implement set_params for setting effect parametrs and process for processing blocks of audio data
 in numpy arrays of size [number of samples per buffer, number of channels].
 Implement a help function for explaining purpose of each effect and its parameters.
"""
import numpy as np
import audio_framework_params as afp
import ipdb

class FxGain():
    """
    Effect description and its parameters goes here.

    """
    gain=1.0
    mute=0
    
    def __init__(self):
        self.gain = 1.0
        self.mute = 0
        print("Initializing FxGain with default Values.")
        print("gain: ",self.gain)
        print("mute: ",self.mute)
    
    def set_params(self, p_list = []):
        self.gain = float(p_list[0])
        self.mute = float(p_list[1])
        print("FxGain parameters set as:")
        print("gain: ",self.gain)
        print("mute: ",self.mute)
    
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


class UtilityRms():
    """
    RMS calculation, Sliding window, running sum root-mean-square.

    Given the requested sliding window length N, output is calculated as:
    x_rms = sqrt( (x_1 ^2 + x_2 ^2 + ... + x_N ^2)/N )

    """ 

    def __init__(self):
        self.sliding_window_length = 8192
        self.sliding_window_buffer = np.array([])
        print("Initializing UtilityRms with default Values.")
        print("sliding_window_length: ",self.sliding_window_length)
    
    def set_params(self, p_list = []):
        self.sliding_window_length = int(p_list[0])
        # empty array initialization so to be reset in next process call.
        self.sliding_window_buffer = np.array([])
        print("UtilityRms parameters set as:")
        print("sliding_window_length: ",self.sliding_window_length)
    
    def process(self, input_array):
        # if buffer is empty means it should be initialized with zeros.
        if self.sliding_window_buffer.size == 0:
            rows,num_of_channels = input_array.shape
            self.sliding_window_buffer = np.zeros([self.sliding_window_length, num_of_channels])
        # move input_array into sliding window while discarding older data
        input_array_length = len(input_array)
        buffer_slice = self.sliding_window_buffer[input_array_length:-1,:]
        self.sliding_window_buffer = np.vstack((buffer_slice,input_array))

        # calculate rms of data in sliding window buffer.
        rms = np.sqrt(sum(self.sliding_window_buffer**2)/self.sliding_window_length)
        return rms

    def help(self):
        print("-------------")
        print("UtilityRms")
        print("-------------")
        print("Description:")
        print("Calculates sliding window, running sum root-mean-square.")
        print("Parameters:")
        print("sliding_window_length: (int) sliding window buffer length")


class FxIIR():
    """
    First order IIR filters.

    lp,hp

    """
    filter_type ='hp'

    
    def __init__(self):
        self.filter_type = 'hp'
        self.f_c = 10.0
        d = np.exp(-2*np.pi*(self.f_c/afp.input_file_samplerate))
        self.a0= 0.5*(1.0 + d)
        self.a1= -0.5*(1.0 + d)
        self.b1= d
        self.last_input = np.array([])
        self.last_output = np.array([])

        print("Initializing FxIIR with default Values.")
        print("filter_type: ",self.filter_type)
        print("f_c: ",self.f_c)
    
    def set_params(self, p_list = []):
        self.filter_type = (p_list[0])
        self.f_c = float(p_list[1])
        if self.filter_type == 'lp':
            d = 1 - np.exp(-2*np.pi*(self.f_c/afp.input_file_samplerate))
            self.a0= d
            self.a1= 0
            self.b1= 1-d        
        elif self.filter_type == 'hp':
            d = np.exp(-2*np.pi*(self.f_c/afp.input_file_samplerate))
            self.a0= 0.5*(1.0 + d)
            self.a1= -0.5*(1.0 + d)
            self.b1= d
        else:
            print('not a valid filter type')
            exit(-1)
        print("FxIIR parameters set as:")
        print("filter_type: ",self.filter_type)
        print("f_c: ",self.f_c)
    
    def process(self, input_array):
        output_array = np.zeros_like(input_array)
        # if buffer is empty means it should be initialized with zeros.
        if self.last_input.size == 0:
            rows,num_of_channels = input_array.shape
            self.last_input = np.zeros([1, num_of_channels])
            self.last_output = np.zeros([1, num_of_channels])
        if self.filter_type == 'lp':
            output_array[0,:] = self.a0*input_array[0,:]+self.b1*self.last_output[0,:] 
            index = 1
            while index < len(input_array):
                output_array[index,:] = self.a0*input_array[index,:]+self.b1*output_array[index-1,:]
                index += 1
            self.last_input = input_array[-2:-1,:]
            self.last_output = output_array[-2:-1,:]

        elif self.filter_type == 'hp':
            output_array[0,:] = self.a0*input_array[0,:]+self.a1*self.last_input[0,:] +self.b1*self.last_output[0,:] 
            index = 1
            while index < len(input_array):
                output_array[index,:] = self.a0*input_array[index,:]+self.a1*input_array[index-1,:]+self.b1*output_array[index-1,:]
                index += 1
            self.last_input = input_array[-2:-1,:]
            self.last_output = output_array[-2:-1,:]
                
        else:
            print('not a valid filter type')
            exit(-1)

        return output_array

    def help(self):
        print("-------------")
        print("FxIIR effect")
        print("-------------")
        print("Description:")
        print("Apply specified filter .")
        print("Parameters:")
        print("filter_type: (string) hp for highpass or lp for lowpass")
        print("f_c: (float)cutoff frequency of filter.")

