#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import logging
import os
import subprocess
import sys
import typing
import json

import githelper


# Function to transform FileLines to desired JSON format
def file_lines_to_json(
    file_lines_list: typing.List[githelper.FileLines],
) -> str:
    json_list = []
    for file in file_lines_list:
        # Transform each file to the desired format
        file_obj = {"name": file.filename}
        # Only include "lines" key if lines exist
        if file.lines:
            file_obj["lines"] = [list(line) for line in file.lines]
        json_list.append(file_obj)

    return json.dumps(json_list, separators=(",", ":"))


def find_compile_commands(rootdir):
    for root, dirs, files in os.walk(rootdir):
        if "compile_commands.json" in files:
            return os.path.join(root, "compile_commands.json")
    return None


def run_clang_tidy_on_lines(rootdir, lines, compile_commands):
    logger = logging.getLogger(__name__)

    line_filter = file_lines_to_json(lines)
    assert line_filter

    filenames = [os.path.join(rootdir, file.filename) for file in lines]

    cmd = [
        "clang-tidy",
        "--format-style=file",
        "--fix-notes",
        "--fix-errors",
        "--warnings-as-errors=*",
        "-p=" + compile_commands,
        "--line-filter=" + line_filter,
        *filenames,
    ]

    logger.info(f"running on {line_filter}")
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        logger.error(proc.stdout)
        sys.exit(1)


def main(argv: typing.Optional[typing.List[str]] = None) -> int:
    logging.basicConfig(
        format="[%(levelname)s] %(message)s", level=logging.INFO
    )

    logger = logging.getLogger(__name__)

    parser = argparse.ArgumentParser()
    parser.add_argument("--from-ref", help="use changes changes since commit")
    parser.add_argument("--to-ref", help="use changes until commit")
    parser.add_argument("files", nargs="*", help="only check these files")
    args = parser.parse_args(argv)

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

    logger.debug("Looking for compile_commands.json...")
    compile_commands = find_compile_commands(rootdir)

    logger.debug("getting changed lines")
    changed_lines = githelper.get_changed_lines_grouped(
        from_ref=args.from_ref,
        to_ref=args.to_ref,
        filter_lines=lambda line: line.added,
        include_files=args.files,
    )
    logger.debug("running clang-tidy")
    run_clang_tidy_on_lines(rootdir, list(changed_lines), compile_commands)


if __name__ == "__main__":
    sys.exit(main())
