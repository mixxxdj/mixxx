#/usr/bin/python
#
# GNU General Public License Usage
# This file may be used under the terms of the GNU 
# General Public License version 3 as published by the Free Software
# Foundation. Please review the following information to
# ensure the GNU General Public License version 3 requirements
# will be met: https://www.gnu.org/licenses/gpl-3.0.html.

__author__      = "Michele Albano"
__copyright__   = "Copyright 2023"
__credits__ = ["Michele Albano"]
__license__ = "GPL"
__version__ = "0.9"
__maintainer__ = "Michele Albano"
__email__ = "michele.albano@gmail.com"
__status__ = "Prototype"


"""
This program enters recursively a folder containing a Mixxx skin, and creates an "inverted color" skin.
In particular, it copies the directory structure and the .png / .psd files from the src directory to the current directory, and it creates svg / xml / ... files where the codes for the colors (e.g.: #001122) are inverted (#ffffff - #001122).

RUN THIS FILE FROM THE FOLDER YOU WANT TO POPULATE WITH THE NEW SKIN!

Example of usage:
md c:\Program Files\Mixxx\skins\LateNight\palesun
cd c:\Program Files\Mixxx\skins\LateNight\palesun
python3 invertcolor.py "c:\Program Files\Mixxx\skins\LateNight\palemoon"
"""

import sys
import re
import pathlib
import os
import shutil


def from_hex(hexdigits):
    return int(hexdigits, 16)

def invert_color(len, line):
    regge = '^[0-9A-Fa-f]{'+str(len)+'}[^0-9a-fA-F]'
    p = re.compile(regge)
    tokens = line.split("#")
    ret = tokens.pop(0)
    for tok in tokens:
        m = p.search(tok)
        if m:
            last_char = m.group()[-1]
            val = from_hex("f"*len) - from_hex("0x" + m.group()[:-1])
            ret += "#" + f"{val:#0{len+2}x}"[2:]  + tok[len:]
        else:
            ret += "#" + tok
    return ret

def process_binary(src, dst):
    print("copying from " + src + " to " + dst)
    if not os.path.exists(dst):
        shutil.copyfile(src, dst)

def process_file(filename):
    fp1 = open(filename, "r")
    lines = fp1.readlines()
    output = ""
    for line in lines:
        output += invert_color(3, invert_color(6, line))
    fp1.close()
    return output

def process_dir(rootdir, where):
    desktop = pathlib.Path(rootdir+where)
    for item in desktop.iterdir():
        filename = os.path.basename(str(item))
        newwhere = where + os.path.sep + filename
        if item.is_file():
            something, file_extension = os.path.splitext(newwhere)
            if file_extension == ".png" or file_extension == ".psd": # currently, I consider every non-PNG file as text to be processed
                process_binary(str(item), newwhere)
            else:
                processed_file = process_file(str(item))
                fp2 = open(newwhere, "w")
                fp2.write(processed_file)
                fp2.close()
        else:
            print("create dir " + newwhere)
            if not os.path.exists(newwhere):
                os.makedirs(newwhere)
            process_dir(rootdir, newwhere)

if len(sys.argv) < 2:
    print("tell me where to find the source skin")
    print("""for example:
md c:\Program Files\Mixxx\skins\LateNight\palesun
cd c:\Program Files\Mixxx\skins\LateNight\palesun
python3 invertcolor.py "c:\Program Files\Mixxx\skins\LateNight\palemoon"
""")
    sys.exit('missing argument')

start_string = sys.argv[1]
while start_string.endswith('\\'):
    start_string = start_string[:-1]

print(start_string)
target = pathlib.Path(start_string)



if not target.is_dir():
    output = process_file(start_string)
    print(output)
else:
    process_dir(start_string + "\\", ".")

