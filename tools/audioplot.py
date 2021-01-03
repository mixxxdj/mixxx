#!/usr/bin/env python
import matplotlib.pyplot as plt
import numpy as np
import argparse
import sys


def createSlice(columns):
    """convert an input string into a slice object"""
    if columns == "all":
        return slice(None)
    # creates a numpy array
    return np.int64(columns.split(","))


def combine_files(files):
    """
    reads a bunch of files and stacks their content together into one numpy
    array. The number of data points in the columns will be set to the
    shortest file.
    """
    raw = []
    min_len = sys.maxsize  # max integer
    for fname in files:
        raw.append(np.genfromtxt(fname, delimiter=","))
        min_len = min(len(raw[-1]), min_len)
    data = raw[0][:min_len]

    for d in raw[1:]:
        data = np.hstack((data, d[:min_len]))
    # in case only one column is given
    if data.ndim == 1:
        data = data.reshape(len(data), 1)
    return data


def AudioPlot(files, slice):
    data = combine_files(files)
    plt.plot(data[:, slice])
    plt.show()


def parseArguments():
    p = argparse.ArgumentParser(
        prog="AudioPlot",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="""
Audioplot is a simple script for drawing graphs of values. It reads a simple
csv text file where each line needs to contain the same number of
comma-separated values, like so:

0, 0, 0
1, 2, 1
2, 4, 2

Each column will be plotted as a separate curve on the same time series.
Matplotlib is used to display the result.

This script is useful to compare the sample files produced by the engine test
of mixxx-test with the golden sample files.

""",
    )
    p.add_argument("files", type=str, nargs="+", help="file to plot from")
    p.add_argument(
        "-c",
        "--columns",
        type=str,
        default="all",
        help='lines to plot separated by a comma, default "all"',
    )
    return p.parse_args()


if __name__ == "__main__":
    args = parseArguments()
    AudioPlot(args.files, createSlice(args.columns))
