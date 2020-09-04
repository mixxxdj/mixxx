#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import itertools
import logging
import os
import subprocess
import sys
import typing

import githelper


def run_codespell_on_lines(
    rootdir, filename, lines, codespell_args, ignore_matches
):
    """
    Run codespell on the requested lines.

    Returns 1 if any changes were made, else 0.
    """
    logger = logging.getLogger(__name__)
    cmd = ["codespell", *codespell_args, "--", filename]
    logger.debug("Running command: %r", cmd)

    try:
        subprocess.check_output(cmd)
    except subprocess.CalledProcessError as e:
        output = e.output.decode().strip()
    else:
        return 0

    result = 0
    for line in output.splitlines():
        matched_fname_with_line, _, content = line.partition(": ")
        matched_fname, _, linenum = matched_fname_with_line.rpartition(":")
        assert matched_fname == filename
        if int(linenum) not in lines:
            continue

        match, _, suggestions = content.partition("==>")
        if match.strip() in ignore_matches:
            continue

        result = 1
        print(line)

    return result


def get_ignore_matches(fp):
    for line in fp:
        content, sep, comment = line.partition("#")
        content = content.strip()
        if not content:
            continue
        yield content


def main(argv: typing.Optional[typing.List[str]] = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--from-ref", help="use changes changes since commit")
    parser.add_argument("--to-ref", help="use changes until commit")
    parser.add_argument(
        "--ignore-file",
        type=argparse.FileType("r"),
        help="ignore matches (one per line)",
    )
    parser.add_argument("--files", nargs="*", help="only check these files")
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Be verbose"
    )
    args, codespell_args = parser.parse_known_args(argv)

    logging.basicConfig(
        format="[%(levelname)s] %(message)s",
        level=logging.DEBUG if args.verbose else logging.INFO,
    )

    if not args.from_ref:
        args.from_ref = os.getenv("PRE_COMMIT_FROM_REF") or os.getenv(
            "PRE_COMMIT_SOURCE"
        )

    if not args.to_ref:
        args.to_ref = os.getenv("PRE_COMMIT_TO_REF") or os.getenv(
            "PRE_COMMIT_ORIGIN"
        )

    # Filter filenames
    rootdir = githelper.get_toplevel_path()

    files_with_added_lines = githelper.get_changed_lines(
        from_ref=args.from_ref,
        to_ref=args.to_ref,
        filter_lines=lambda line: line.added,
        include_files=args.files,
    )

    ignore_matches = set()
    if args.ignore_file:
        ignore_matches = set(get_ignore_matches(args.ignore_file))

    result = 0
    for filename, file_lines in itertools.groupby(
        files_with_added_lines, key=lambda line: line.sourcefile
    ):
        lines = set(line.number for line in file_lines)
        result |= run_codespell_on_lines(
            rootdir,
            filename,
            lines,
            [arg for arg in codespell_args if arg != "--"],
            ignore_matches,
        )

    return result


if __name__ == "__main__":
    sys.exit(main())
