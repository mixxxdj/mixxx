#!/usr/bin/env python3
"""
Small qmlformat wrapper that warns the user if the tool is not installed.
"""
import argparse
import shutil
import subprocess
import pathlib
import sys
import re

QMLFORMAT_MISSING_MESSAGE = """
qmlformat not found, please install
"""


def main(argv=None):
    qmlformat_executable = shutil.which("qmlformat")
    if not qmlformat_executable:
        # verify if qmlformat is available on this machine
        moc_executable = shutil.which("moc")
        if moc_executable:
            moc_version = subprocess.check_output(
                (moc_executable, "-v")
            ).strip()
            v = re.search("moc ([0-9]*)\\.([0-9]*)\\.[0-9]*", str(moc_version))
            if v:
                if int(v.group(1)) < 5:
                    return 0
                if int(v.group(1)) == 5 and int(v.group(2)) < 15:
                    return 0
        print(QMLFORMAT_MISSING_MESSAGE.strip(), file=sys.stderr)
        return 1

    parser = argparse.ArgumentParser()
    parser.add_argument("file", nargs="+", type=pathlib.Path)
    args = parser.parse_args(argv)

    for filename in args.file:
        subprocess.call((qmlformat_executable, "-i", filename))

    return 0


if __name__ == "__main__":
    sys.exit(main())
