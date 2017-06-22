"""
 Helper functions for effect prototyping framework,
"""
import sys
import os 
import argparse


def int_or_str(text):
    """Helper function for argument parsing."""
    try:
        return int(text)
    except ValueError:
        return text


def parse_input_arguments_rt():
    """  Parse input arguments and return a list with the user defined arguments. Used for real time version.
    """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-e', '--effect_id', type=str,
                        help='effect to be applied to inupt')
    parser.add_argument('-f', '--fxparameters', type=str,
                        help='effect parameters comma seperated')    
    parser.add_argument('-i', '--input-device', type=int_or_str,
                        help='input device ID or substring')
    parser.add_argument('-o', '--output-device', type=int_or_str,
                        help='output device ID or substring')
    parser.add_argument('-c', '--channels', type=int, default=2,
                        help='number of channels')
    parser.add_argument('-t', '--dtype', help='audio data type')
    parser.add_argument('-s', '--samplerate', type=float, help='sampling rate')
    parser.add_argument('-b', '--blocksize', type=int, help='block size')
    parser.add_argument('-l', '--latency', type=float, help='latency in seconds')
    args = parser.parse_args()

    fx_parameters_list = args.fxparameters.split(",")

    return [args.effect_id, fx_parameters_list, args.input_device,args.output_device,args.samplerate,args.blocksize,args.dtype,args.latency,args.channels] 


def parse_input_arguments_file():
    """  Parse input arguments and return a list with the user defined arguments. Used for fileIO version.
    """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-e', '--effect_id', type=str,
                        help='effect to be applied to inupt')
    parser.add_argument('-f', '--fxparameters', type=str,
                        help='effect parameters comma seperated')    
    parser.add_argument('-i', '--inputfile', type=str,
                        help='Input file directory')
    parser.add_argument('-o', '--outputfile', type=str,
                        help='Output file directory')
    parser.add_argument('-b', '--bufferlength', type=int_or_str, help='Define length of buffer to be used for block processing', default="1024")
    parser.add_argument('-p', '--plotflag', type=int_or_str, help='Set to get a debug plot at the end of proceesing', default="0")
    args = parser.parse_args()

    # TODO: Move here check that effect exist and return effect object instead of effect id.
    # Check that input file exists otherwise exit with error message.    
    if not os.path.isfile(args.inputfile):
        print("Input file does not exist or is not a file.")
        sys.exit(-1)
    # Form a list with parameters from input argument to be parsed by the effect when setting parameters.
    fx_parameters_list = args.fxparameters.split(",")

    return [args.effect_id, fx_parameters_list, args.inputfile, args.bufferlength, args.outputfile, args.plotflag]
