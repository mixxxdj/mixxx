#!/usr/bin/python

# audioplot is a simple script for drawing graphs of values.  It reads a simple
# csv text file where each line needs to contain the same number of comma-separated values,
# like so:
# 0, 0, 0
# 1, 2, 1
# 2, 4, 2
#
# Each column will be plotted as a separate curve on the same time series.
# Matplotlib is used to display the result.

from matplotlib import pyplot as plt
from matplotlib.lines import Line2D
import time, string, glob, sys

import getopt

def AudioPlot(f, columns):
    colors = ['g','c','b','m','r']
    plt.figure()

    #set up dictionary of lists
    if columns != "all":
        data = {}
        for c in columns:
            data[c] = []
    else:
        data = None

    #build the individual columns
    for line in f.readlines():
        splitted = line.split(",")
        if columns == "all" and data is None:
            #make list of columns based on number of entries in first line
            columns = [i for i in range(0,len(splitted))]
            data = {}
            for c in columns:
                data[c] = []

        for c in columns:
            try:
                data[c].append(splitted[c].strip())
            except IndexError:
                pass

    i=0

    for c in columns:
        normalized = []
        for d in data[c]:
            try:
                normalized.append(float(d))
            except ValueError:
                print "skipping ", d
        #rotate through my five favorite colors
        plt.plot(normalized, colors[i % len(colors)])
        i+=1

    plt.show()

    return

def usage():
    print """
AudioPlot.py:
-c              pick column to plot (more than one is OK, 1-based indexes, or "all")
-f              file to plot from
"""

if __name__ == "__main__":
    columns = []
    fname = None
    try:
        opts, args = getopt.getopt(sys.argv[1:], "c:f:",[])
        for o, a in opts:
            if o == "-c":
                if columns != "all":
                    if a.upper() == "ALL":
                        columns = "all"
                    else:
                        columns.append(int(a)-1)
            elif o == "-f":
                fname = a
            else:
                usage()
                sys.exit(1)
    except Exception, e:
        print str(e)
        usage()
        sys.exit(1)

    if fname is None:
        usage()
        sys.exit(1)

    try:
        f = open(fname, "r")
    except Exception, e:
        print "Error opening file %s: %s" % (fname, str(e))
        sys.exit(1)

    AudioPlot(f, columns)
