#!/usr/bin/env python
import matplotlib.pyplot as plt
import numpy as np
import argparse


def createSlice(columns):
    """convert an input string into a numpy slice object"""
    if columns == 'all':
        return slice(None)
    else:
        return [int(c) for c in columns.split(',')]


def AudioPlot(fname, slice):
    data = np.genfromtxt(fname, delimiter=',')
    plt.plot(data[:, slice])
    plt.show()


def parseArguments():
    p = argparse.ArgumentParser(prog='AudioPlot',
                                formatter_class=argparse.RawDescriptionHelpFormatter,
                                description="""
Audioplot is a simple script for drawing graphs of values. It reads a simple
csv text file where each line needs to contain the same number of
comma-separated values, like so:

0, 0, 0
1, 2, 1
2, 4, 2

Each column will be plotted as a separate curve on the same time series.
Matplotlib is used to display the result."""
                                )
    p.add_argument('file', type=str,
                   help='file to plot from')
    p.add_argument('-c', '--columns', type=str, default='all',
                   help='lines to plot seperated by a comma, default "all"')
    return p.parse_args()


if __name__ == "__main__":
    args = parseArguments()
    AudioPlot(args.file, createSlice(args.columns))
