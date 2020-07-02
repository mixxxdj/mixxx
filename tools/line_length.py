#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import re
import logging
import os
import itertools
import subprocess
import tempfile
import typing

# We recommend a maximum line length of 80, but do allow up to 100 characters
# if deemed necessary by the developer. Lines that exceed that limit will
# be wrapped after 80 characters automatically.
LINE_LENGTH_THRESHOLD = 100
BREAK_BEFORE = 80

Line = typing.NamedTuple(
    "Line", [("sourcefile", str), ("number", int), ("text", str)]
)
LineGenerator = typing.Generator[Line, None, None]
FileLines = typing.NamedTuple(
    "FileLines",
    [("filename", str), ("lines", typing.Sequence[typing.Tuple[int, int]])],
)


def get_git_added_lines() -> LineGenerator:
    proc = subprocess.run(
        ["git", "diff", "--cached", "--unified=0"], capture_output=True
    )
    proc.check_returncode()
    current_file = None
    current_lineno = None
    lines_left = 0
    for line in proc.stdout.decode(errors="replace").splitlines():
        match_file = re.match(r"^\+\+\+ b/(.*)$", line)
        if match_file:
            current_file = match_file.group(1)
            lines_left = 0
            continue

        match_lineno = re.match(
            r"^@@ -\d+(?:,\d+)? \+([0-9]+(?:,[0-9]+)?) @@", line
        )
        if match_lineno:
            start, _, length = match_lineno.group(1).partition(",")
            current_lineno = int(start)
            lines_left = 1
            if length:
                lines_left += int(length)
            continue

        if lines_left and line.startswith("+"):
            yield Line(
                sourcefile=current_file, number=current_lineno, text=line[1:]
            )
            lines_left -= 1
            current_lineno += 1


def group_lines(
    lines: LineGenerator,
) -> typing.Generator[FileLines, None, None]:
    for filename, file_lines in itertools.groupby(
        lines, key=lambda line: line.sourcefile
    ):
        grouped_linenumbers = []
        start_linenumber = None
        last_linenumber = None
        for line in file_lines:
            if None not in (start_linenumber, last_linenumber):
                if line.number != last_linenumber + 1:
                    grouped_linenumbers.append(
                        (start_linenumber, last_linenumber)
                    )

            if start_linenumber is None:
                start_linenumber = line.number
            last_linenumber = line.number

        if None not in (start_linenumber, last_linenumber):
            grouped_linenumbers.append((start_linenumber, last_linenumber))

        if grouped_linenumbers:
            yield FileLines(filename, grouped_linenumbers)


def main(argv: typing.Optional[typing.List[str]] = None) -> int:
    logging.basicConfig()
    logger = logging.getLogger(__name__)

    import sys

    print(sys.argv)

    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="only check these files")
    args = parser.parse_args(argv)
    print(args)

    all_lines = get_git_added_lines()

    # Filter filenames
    rootdir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
    if args.files:
        files = [
            os.path.abspath(os.path.join(rootdir, filename))
            for filename in args.files
        ]
        all_lines = (
            line
            for line in all_lines
            if os.path.abspath(os.path.join(line.sourcefile)) in files
        )

    # Only keep long lines
    long_lines = (
        line
        for line in all_lines
        if (len(line.text) - 1) >= LINE_LENGTH_THRESHOLD
    )
    changed_files = group_lines(long_lines)

    cpp_file = os.path.join(rootdir, "src/mixxx.cpp")
    proc = subprocess.run(
        ["clang-format", "--dump-config", cpp_file],
        capture_output=True,
        text=True,
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

        for changed_file in changed_files:
            line_arguments = [
                "--lines={}:{}".format(start, end)
                for start, end in changed_file.lines
            ]

            if not line_arguments:
                continue

            cmd = [
                "clang-format",
                "--style=file",
                "--assume-filename={}".format(
                    os.path.join(tempdir, changed_file.filename)
                ),
                *line_arguments,
            ]

            filename = os.path.join(rootdir, changed_file.filename)
            with open(filename) as fp:
                proc = subprocess.run(
                    cmd, stdin=fp, capture_output=True, text=True
                )
            try:
                proc.check_returncode()
            except subprocess.CalledProcessError:
                logger.error(
                    "Error while executing command %s: %s", cmd, proc.stderr,
                )
                return 1

            if proc.stderr:
                print(proc.stderr)
            with open(filename, mode="w+") as fp:
                fp.write(proc.stdout)
    return 0


if __name__ == "__main__":
    exit(main())
