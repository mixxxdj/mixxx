#!/bin/bash

# Takes multiple audioplot-compatible files and shows them on the same graph.
# If the files differ in length, the shortest file will be used.  This utility
# is useful for comparing signalpathtest golden data with .actual results.
# See also, audioplot.py.

fname=$(tempfile)
script_dir=$(dirname $(realpath $0))

echo paste $@
paste $@ | sed -e 's/\t/,/g;' > $fname
exec $script_dir/audioplot.py -c all -f $fname
