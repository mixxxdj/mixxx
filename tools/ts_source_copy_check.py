#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import logging
import os
import sys
import typing

from lxml import etree

import githelper


def check_copied_source_on_lines(rootdir, file_to_format, stylepath=None):
    filename = os.path.join(rootdir, file_to_format.filename)

    parser = etree.XMLParser(recover=True)
    tree = etree.parse(filename, parser)
    root = tree.getroot()

    found = False

    for context in root.findall("context"):
        for message in context.findall("message"):
            source = message.find("source").text
            translation_elem = message.find("translation")
            if translation_elem is not None:
                translation = translation_elem.text
                if source == translation:
                    line_number = translation_elem.sourceline
                    for start, end in file_to_format.lines:
                        if line_number >= start and line_number <= end:
                            print(
                                "Copied source translation found at line "
                                f"{file_to_format.filename}:{line_number}",
                                file=sys.stderr,
                            )
                            found = True
    return found


def main(argv: typing.Optional[typing.List[str]] = None) -> int:
    logging.basicConfig(
        format="[%(levelname)s] %(message)s", level=logging.INFO
    )

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

    files_with_added_lines = githelper.get_changed_lines_grouped(
        from_ref=args.from_ref,
        to_ref=args.to_ref,
        filter_lines=lambda line: line.added,
        include_files=args.files,
    )

    ret = 0
    for changed_file in files_with_added_lines:
        if check_copied_source_on_lines(rootdir, changed_file):
            ret = 1

    return ret


if __name__ == "__main__":
    sys.exit(main())
