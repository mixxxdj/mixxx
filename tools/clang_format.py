#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import logging
import os
import subprocess
import typing


def main(argv: typing.Optional[typing.List[str]] = None) -> int:
    logging.basicConfig()
    logger = logging.getLogger(__name__)

    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="only check these files")
    args = parser.parse_args(argv)

    cmd = ["git-clang-format"]

    source_commit = os.environ.get("PRE_COMMIT_SOURCE")
    if source_commit:
        cmd += [source_commit]

    # Filter filenames
    rootdir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
    if args.files:
        files = [
            os.path.abspath(os.path.join(rootdir, filename))
            for filename in args.files
        ]

        cmd += ["--"]
        cmd += files

    proc = subprocess.run(cmd, capture_output=True, text=True)
    try:
        proc.check_returncode()
    except subprocess.CalledProcessError:
        logger.error(
            "Error while executing command %s: %s", cmd, proc.stderr,
        )
        return 1
    if proc.stderr:
        print(proc.stderr)
    return 0


if __name__ == "__main__":
    exit(main())
