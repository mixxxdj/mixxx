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


def run_codespell_on_lines(rootdir, filename, lines, codespell_args):
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
        matched_fname_with_line = line.partition(": ")[0]
        matched_fname, _, linenum = matched_fname_with_line.rpartition(":")
        assert matched_fname == filename
        if int(linenum) in lines:
            result = 1
            print(line)

    return result


def make_ignore_regex(fp, ignore_regex):
    parts = []

    for line in fp:
        content, sep, comment = line.partition("#")
        content = content.strip()
        if not content:
            continue
        parts.append(content)

    word_regex = r"\W(?:{})\W".format(
        "|".join("(?:{})".format(part) for part in parts)
    )
    if not ignore_regex:
        return word_regex

    return r"(?:{words}|{pattern})".format(
        words=word_regex, pattern=ignore_regex
    )


def main(argv: typing.Optional[typing.List[str]] = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--from-ref", help="use changes changes since commit")
    parser.add_argument("--to-ref", help="use changes until commit")
    parser.add_argument(
        "--ignore-regex", help="patterns to treat as whitespace"
    )
    parser.add_argument(
        "--ignore-file",
        type=argparse.FileType("r"),
        help="ignore word regex (one per line)",
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

    if args.ignore_file:
        codespell_args.extend(
            [
                "--ignore-regex",
                make_ignore_regex(args.ignore_file, args.ignore_regex),
            ]
        )

    # Filter filenames
    rootdir = githelper.get_toplevel_path()

    files_with_added_lines = githelper.get_changed_lines(
        from_ref=args.from_ref,
        to_ref=args.to_ref,
        filter_lines=lambda line: line.added,
        include_files=args.files,
    )

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
        )

    return result


if __name__ == "__main__":
    sys.exit(main())
