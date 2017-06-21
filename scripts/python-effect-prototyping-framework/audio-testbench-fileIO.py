#!/usr/bin/env python3
"""
 This script main purpose is to serve as a framework for prototyping new dsp 
 algorithm implementation in python. 

 The input audio file is processed by the specified effect from effects 
 module, in a frame by frame way, as it would be processed in real time.

 Effect parameters are passed as arguments, and the processd file is
 saved to the described output file. 

 arguments:

 -effect: (mandatory) 
           Select effect to be used from effects module.
 -fxparameters: (mandatory) 
                 comma seperated values of the effect parameters.
 -input_file: (mandatory)
               Input file directory.
 -buffer_length: (optional/defaults to 1024)
                  Length of buffer that each block will be processed at.
 -output_file: (mandatory)
                Output file directory.               
 -plot: (optional/default 0)
        if this flag is set then a debug plot is performed a the end of audio processing the whole file.

 example call:
    python3 audio-testbench-fileIO.py -e FxGain -i 'audio-data-lib/test_input_mono.wav' -o  'audio-data-lib/test_output_mono.wav' -f "0.25, 0"

 author: 
     sqrwvzblw - father.aris@gmail.com
 """

import sys
import utilities
import matplotlib.pyplot as plt
import numpy as np
import soundfile as sf
import effect_prototypes

if __name__ == "__main__":

    # Parse User Input Arguments
    [effect_id, fx_parameters_list, input_file_dir, buffer_length, output_file_dir, plot_id] = utilities.parse_input_arguments_file()

    # Match effect id to imported fx modules and initialize effect object.
    try:
        exec("effect=dsp_prototypes."+effect_id+"()")
    except:
    	print("something went wrong - effect id either mispelled or not properly imported")
    	sys.exit(-1)

    # Collect info for audio input file.
    audio_file_info = sf.info(input_file_dir, verbose=False)
    num_of_channels = audio_file_info.channels
    input_length = audio_file_info.frames

    # Read input file into a numpy array object and reshape so as to be able to work with multichannel arrays.
    input_array_data, samplerate = sf.read(input_file_dir)
    input_array = np.reshape(input_array_data,(input_length, num_of_channels))
    # Initialize output array
    output_array = np.zeros_like(input_array)

    # Setup requested effect parameters
    effect.set_params(fx_parameters_list)

    # Process each block of buffer_length samples with effect.process
    sample_index = 0
    while sample_index<input_length:
    	block = input_array[sample_index:sample_index+buffer_length,:]
    	output_block = effect.process(block)
    	output_array[sample_index:sample_index+buffer_length,:] = output_block
    	sample_index+=buffer_length

    	#TODO: mayybe add here a scope update with block and output block.

    # write result to output file
    sf.write(output_file_dir, output_array, audio_file_info.samplerate)
    print("Processed data written to ",output_file_dir)

    # plot final waveforms
    if plot_id:
        plt.figure(1)
        plt.title('input vs output waveform')
        plt.plot(input_array)
        plt.plot(output_array)
        plt.show()

    # Terminate session
    print("Processing complete\n")
    sys.exit(0)