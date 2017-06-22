#!/usr/bin/env python3
"""
 This script main purpose is to serve as a framework for prototyping new dsp 
 algorithm implementation in python. 

 Accepts audio input from the selected device and process audio in real time in specified
 buffer length.
 
 A minimal cli interface can issue python statements and alter the parameters of the effect real time.

 arguments:

 -effect: (mandatory) 
           Select effect to be used from effects module.
 -fxparameters: (mandatory) 
                 comma seperated values of the effect parameters.
 -input_device: (optional)
                 Device to input audio from, if not specified default audio device will be selected.
 -buffer_length: (optional/defaults to 1024)
                  Length of buffer that each block will be processed at.
 -output_device: (optional)
                Device to consume audio to, if not specified default audio device will be selected.               
 -samplerate: (optional)
        Select sampling rate if not specified, default samplerate of selected device will be set. 

 example call:
    python3 audio-testbench-realtime.py -e FxGain -f "0.25, 0"

 author: 
     sqrwvzblw - father.aris@gmail.com
 
 TODO: add realtime plot of input vs processed frames using queue and maplotlib in a seperate thread.
 TODO: tidy up keyboard input functionality.
      
 hint: try stty sane in case enter/return does not return a new line but ^M instead.        

 author: sqrwvzblw - father.aris@gmail.com
"""

import sys
import sounddevice as sd
import utilities
import effect_prototypes
import threading
import time

kbdInput = ''
playingID = ''
finished_kbd_flag = True


def kbdListener():
    global kbdInput, finished_kbd_flag
    kbdInput = input("> ")
    finished_kbd_flag = True

[effect_id, fx_parameters_list,input_device,output_device,samplerate,blocksize,dtype,latency,channels] = utilities.parse_input_arguments_rt()

# Match effect id to imported fx modules and initialize effect object.
try:
    exec("effect=dsp_prototypes."+effect_id+"()")
except:
    print("something went wrong - effect id either mispelled or not properly imported")
    sys.exit(-1)

# # Setup requested effect parameters
effect.set_params(fx_parameters_list)

try:
    
    def audio_callback(indata, outdata, frames, time, status):
        if status:
            print(status)
        outdata[:] = effect.process(indata)

    with sd.Stream(device=(input_device, output_device),
                   samplerate=samplerate, blocksize=blocksize,
                   dtype=dtype, latency=latency,
                   channels=channels, callback=audio_callback):

        # Handle input commands from keyboard input thread
        print("Type python expression to evaluate - type quit to exit.")
        while True:
            if playingID != kbdInput:
                if kbdInput == "quit" or kbdInput == "q" :
                    print("Thank you very much")
                    sys.exit(0)
                try:
                    if kbdInput == "help" or kbdInput == "h" :
                        print("type a python statement to evaluate")
                        print("i.e if gain is an effect parameter, it can be set by typing:")
                        print("effect.gain = 0.7")
                    else:
                        exec(kbdInput)
                except:
                    print("Unexpected error - not a valid statement")
        
                playingID = kbdInput
            if finished_kbd_flag:
                finished_kbd_flag = False
                listener = threading.Thread(target=kbdListener)
                listener.start()
            time.sleep(2)

except KeyboardInterrupt:
    print('\nProcess terminated by user')
    sys.exit(0)
except Exception as e:
    print(type(e).__name__ + ': ' + str(e))
    sys.exit(-1)
