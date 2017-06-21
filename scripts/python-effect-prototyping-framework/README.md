Intro
-----

Purpose of this framework and usage.


Contents:
 - audio-testbench-fileIO.py
 - audio-testbench-realtime.py  
 - effect_prototypes.py 
 - utilities.py
 - audio-data-lib

effect_prototypes.py 
-----------------

 Implements effects that are to be tested with this framework.
 Initialize the effect with desired default parameters.
 Implement set_params for setting effect parametrs and process for processing blocks of audio data
 in numpy arrays of size [number of samples per buffer, number of channels].
 Implement a help function for explaining purpose of each effect and its parameters.

audio-testbench-fileIO.py
-------------------------
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

usage: audio-testbench-fileIO.py [-h] [-e EFFECT_ID] [-f FXPARAMETERS]
                                 [-i INPUTFILE] [-o OUTPUTFILE]
                                 [-b BUFFERLENGTH] [-p PLOTFLAG]


optional arguments:
  -h, --help            show this help message and exit
  -e EFFECT_ID, --effect_id EFFECT_ID
                        effect to be applied to inupt
  -f FXPARAMETERS, --fxparameters FXPARAMETERS
                        effect parameters comma seperated
  -i INPUTFILE, --inputfile INPUTFILE
                        Input file directory
  -o OUTPUTFILE, --outputfile OUTPUTFILE
                        Output file directory
  -b BUFFERLENGTH, --bufferlength BUFFERLENGTH
                        Define length of buffer to be used for block
                        processing
  -p PLOTFLAG, --plotflag PLOTFLAG
                        Set to get a debug plot at the end of proceesing



audio-testbench-realtime.py  
----------------------------

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


usage: audio-testbench-realtime.py [-h] [-e EFFECT_ID] [-f FXPARAMETERS]
                                   [-i INPUT_DEVICE] [-o OUTPUT_DEVICE]
                                   [-c CHANNELS] [-t DTYPE] [-s SAMPLERATE]
                                   [-b BLOCKSIZE] [-l LATENCY]

optional arguments:
  -h, --help            show this help message and exit
  -e EFFECT_ID, --effect_id EFFECT_ID
                        effect to be applied to inupt
  -f FXPARAMETERS, --fxparameters FXPARAMETERS
                        effect parameters comma seperated
  -i INPUT_DEVICE, --input-device INPUT_DEVICE
                        input device ID or substring
  -o OUTPUT_DEVICE, --output-device OUTPUT_DEVICE
                        output device ID or substring
  -c CHANNELS, --channels CHANNELS
                        number of channels
  -t DTYPE, --dtype DTYPE
                        audio data type
  -s SAMPLERATE, --samplerate SAMPLERATE
                        sampling rate
  -b BLOCKSIZE, --blocksize BLOCKSIZE
                        block size
  -l LATENCY, --latency LATENCY
                        latency in seconds



utilities.py
------------
Audio framework helper functions.

audio-data-lib
--------------
Directory used in the examples/test as audio files root directory. 
