#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import logging
import os
import re
import subprocess
import sys
import tempfile
import typing

import githelper


# We recommend a maximum line length of 80, but do allow up to 100 characters
# if deemed necessary by the developer. Lines that exceed that limit will
# be wrapped after 80 characters automatically.
LINE_LENGTH_THRESHOLD = 100
BREAK_BEFORE = 80


def get_clang_format_config_with_columnlimit(rootdir, limit):
    cpp_file = os.path.join(rootdir, "src/mixxx.cpp")
    proc = subprocess.run(
        ["clang-format", "--dump-config", cpp_file],
        capture_output=True,
        text=True,
    )
    proc.check_returncode()
    return re.sub(r"(ColumnLimit:\s*)\d+", r"\g<1>{}".format(80), proc.stdout,)


def run_clang_format_on_lines(rootdir, changed_file, assume_filename=None):
    logger = logging.getLogger(__name__)

    line_arguments = [
        "--lines={}:{}".format(start, end) for start, end in changed_file.lines
    ]
    assert line_arguments

    logger.info("Reformatting %s...", changed_file.filename)
    filename = os.path.join(rootdir, changed_file.filename)
    cmd = [
        "clang-format",
        "--style=file",
        "--assume-filename={}".format(
            assume_filename if assume_filename else filename
        ),
        *line_arguments,
    ]

    with open(filename) as fp:
        logger.debug("Executing: %r", cmd)
        proc = subprocess.run(cmd, stdin=fp, capture_output=True, text=True)
    try:
        proc.check_returncode()
    except subprocess.CalledProcessError:
        logger.error(
            "Error while executing command %s: %s", cmd, proc.stderr,
        )
        raise

    if proc.stderr:
        logger.error(proc.stderr)
    with open(filename, mode="w+") as fp:
        fp.write(proc.stdout)


def main(argv: typing.Optional[typing.List[str]] = None) -> int:
    logging.basicConfig(
        format="[%(levelname)s] %(message)s", level=logging.INFO
    )

    logger = logging.getLogger(__name__)

    parser = argparse.ArgumentParser()
    parser.add_argument("--from-ref", help="compare against git reference")
    parser.add_argument("files", nargs="*", help="only check these files")
    args = parser.parse_args(argv)

    if not args.from_ref:
        args.from_ref = os.getenv("PRE_COMMIT_FROM_REF") or os.getenv(
            "PRE_COMMIT_SOURCE"
        )

    # Filter filenames
    rootdir = githelper.get_toplevel_path()

    # First pass: Format added lines using clang-format
    logger.info("First pass: Reformatting added/changed lines...")
    files_with_added_lines = githelper.get_changed_lines_grouped(
        from_ref=args.from_ref,
        filter_lines=lambda line: line.added,
        include_files=args.files,
    )
    for changed_file in files_with_added_lines:
        run_clang_format_on_lines(rootdir, changed_file)

    # Second pass: Wrap long added lines using clang-format
    logger.info("Second pass: Breaking long added/changed lines...")
    files_with_long_added_lines = githelper.get_changed_lines_grouped(
        from_ref=args.from_ref,
        filter_lines=lambda line: line.added
        and LINE_LENGTH_THRESHOLD < (len(line.text) - 1),
        include_files=args.files,
    )
    config = get_clang_format_config_with_columnlimit(rootdir, BREAK_BEFORE)
    with tempfile.TemporaryDirectory(prefix="clang-format") as tempdir:
        # Create temporary config with ColumnLimit enabled
        configfile = os.path.join(tempdir, ".clang-format")
        with open(configfile, mode="w") as configfp:
            configfp.write(config)

        for changed_file in files_with_long_added_lines:
            run_clang_format_on_lines(
                rootdir,
                changed_file,
                assume_filename=os.path.join(tempdir, changed_file.filename),
            )
    return 0


if __name__ == "__main__":
    sys.exit(main())
