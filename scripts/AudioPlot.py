#!/usr/bin/python

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
            data[c].append(splitted[c].strip())

    i=0

    for c in columns:
        print "another column"
        normalized = []
        for d in data[c]:
            try:
                normalized.append(float(d))
            except:
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
                print "usage??"
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
