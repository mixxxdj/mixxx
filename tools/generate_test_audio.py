#!/usr/bin/env python3

import argparse
import sys
import struct
import math
import pathlib


def main(argv=None):
    """
    Generate a modulating audio sine wave signal using linear interpolation
    """
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-s",
        "--sample-rate",
        dest="sample_rate",
        default=44100,
        help="Audio sampling rate",
    )
    parser.add_argument(
        "-d",
        "--duration",
        dest="duration",
        type=float,
        default=5,
        help="Duration in second",
    )
    parser.add_argument(
        "-min",
        "--min-freq",
        dest="min_frequency",
        type=int,
        default=400,
        help="Minimum frequency",
    )
    parser.add_argument(
        "-max",
        "--max-freq",
        dest="max_frequency",
        type=int,
        default=800,
        help="Maximum frequency",
    )
    parser.add_argument(
        "-ch",
        "--channels",
        dest="channels",
        type=int,
        default=2,
        help="Channel count",
    )
    parser.add_argument(
        "-e",
        "--endianness",
        dest="endianness",
        type=str,
        default="little",
        help="Endianness. Either 'big' or 'little' (default)",
    )
    parser.add_argument(
        "-b",
        "--bits",
        dest="bits",
        type=int,
        default=16,
        help="Bits. SUpported value are 16, 24, 32",
    )

    parser.add_argument("file", type=pathlib.Path, help="Output file")

    args = parser.parse_args(argv)

    endianness = ">"
    if args.endianness == "little":
        endianness = "<"
    elif args.endianness != "big":
        print("Invalid endianness value", file=sys.stderr)
        sys.exit(1)

    bits = "h"
    if args.bits == 32:
        bits = "i"
    elif args.bits != 16:
        print("Invalid bits value", file=sys.stderr)
        sys.exit(1)

    with open(args.file, "wb") as f:
        total = 0
        delta_freq = args.max_frequency - args.min_frequency
        sample_duration = 1 / args.sample_rate
        sample_count = 0
        while args.duration > total:
            value = int(
                math.sin(
                    (
                        args.min_frequency
                        + (delta_freq * (total / args.duration))
                    )
                    * total
                )
                * 0x7FFF
            )
            f.write(
                struct.pack(
                    endianness + bits * args.channels,
                    *[value for c in range(args.channels)]
                )
            )
            total += sample_duration
            sample_count += 1


if __name__ == "__main__":
    sys.exit(main())
