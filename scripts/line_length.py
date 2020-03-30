#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import re
import os
import subprocess
import tempfile
from typing import Optional
from typing import Sequence

# We recommend a maximum line length of 80, but do allow up to 100 characters
# if deemed necessary by the developer. Lines that exceed that limit will
# be wrapped after 80 characters automatically.
LINE_LENGTH_THRESHOLD = 100
BREAK_BEFORE = 80


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("filenames", nargs="*", help="Filenames to check")
    args = parser.parse_args(argv)

    proc = subprocess.run(
        ["clang-format", "--dump-config"], capture_output=True, text=True
    )
    proc.check_returncode()

    with tempfile.TemporaryDirectory(prefix="clang-format") as tempdir:
        # Create temporary config with ColumnLimit enabled
        configfile = os.path.join(tempdir, ".clang-format")
        with open(configfile, mode="w") as configfp:
            configfp.write(
                re.sub(
                    r"(ColumnLimit:\s*)\d+",
                    r"\g<1>{}".format(BREAK_BEFORE),
                    proc.stdout,
                )
            )

        for filename in args.filenames:
            lineArguments = []
            with open(filename) as fd:
                for lineno, line in enumerate(fd):
                    if len(line) <= LINE_LENGTH_THRESHOLD:
                        continue
                    humanlineno = lineno + 1
                    print(f"{filename}:{humanlineno} Line is too long.")
                    lineArguments += [f"-lines={humanlineno}:{humanlineno}"]

                if not lineArguments:
                    continue

                fd.seek(0)
                cmd = [
                    "clang-format",
                    "--style=file",
                    "--assume-filename={}".format(
                        os.path.join(tempdir, "file.cpp")
                    ),
                    *lineArguments,
                ]
                proc = subprocess.run(
                    cmd, stdin=fd, capture_output=True, text=True
                )
                proc.check_returncode()
                print(proc.stderr)

            with open(filename, mode="w+") as fp:
                fp.write(proc.stdout)
    return 0


if __name__ == "__main__":
    exit(main())
