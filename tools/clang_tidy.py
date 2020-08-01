#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import logging
import json
import os
import subprocess
import sys
import typing

import githelper


def run_clang_tidy_on_lines(rootdir, changed_files):
    logger = logging.getLogger(__name__)

    files = []
    lines = []
    for changed_file in changed_files:
        logger.debug("Tidying up %s...", changed_file.filename)
        filename = os.path.join(rootdir, changed_file.filename)
        files.append(filename)
        lines.append({"name": filename, "lines": changed_file.lines})

    cmd = [
        "clang-tidy",
        "--fix",
        "--line-filter={}".format(json.dumps(lines)),
        *files,
    ]

    proc = subprocess.run(cmd, capture_output=True, text=True)
    try:
        proc.check_returncode()
    except subprocess.CalledProcessError:
        logger.error(
            "Error while executing command %s: %s", cmd, proc.stderr,
        )
        raise


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

    rootdir = githelper.get_toplevel_path()

    # clang-tidy requires a compile_commands.json file to work correctly
    # If it doesn't exist, this hook should be skipped without failing.
    if not os.path.exists(os.path.join(rootdir, "compile_commands.json")):
        logger.warning("compile-commands.json not found, skipping hook...")
        return 0

    # Detect added/changed lines and run clang-tidy on them
    files_with_added_lines = githelper.get_changed_lines_grouped(
        from_ref=args.from_ref,
        filter_lines=lambda line: line.added,
        include_files=args.files,
    )
    run_clang_tidy_on_lines(rootdir, files_with_added_lines)

    return 0


if __name__ == "__main__":
    sys.exit(main())
