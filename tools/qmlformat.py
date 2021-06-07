#!/usr/bin/env python3
"""
Small qmlformat wrapper that warns the user if the tool is not installed.
"""
import argparse
import shutil
import subprocess
import pathlib
import sys

QMLFORMAT_MISSING_MESSAGE = """
qmlformat is not installed or not in your $PATH. It is included in Qt 5.15
and later. If that Qt version is not available on your system, please
use the SKIP environment variable when committing:

    $ SKIP=qmlformat git commit
"""


def main(argv=None):
    qmlformat_executable = shutil.which("qmlformat")
    if not qmlformat_executable:
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
