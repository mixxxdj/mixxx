#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import logging
import os
import re
import sys
import typing
import fnmatch

from lxml import etree
import githelper

import tempfile

# Define the path to the allow list XML file
ALLOW_LIST_PATH = "res/translations/source_copy_allow_list.tsv"
PROPOSED_ALLOW_LIST_PATH = (
    "res/translations/source_copy_allow_list_proposed.tsv"
)


def parse_line(line, info):
    if not line.endswith("\n"):
        print(f"Parse Error: No \\n at {info}")
        return False
    line = line[:-1]
    try:
        lang, source = line.split("\t", 1)
    except Exception as e:
        print(f"Parse Error at {info}")
        print(e)
    if lang is None or lang == "":
        print(f"Parse Error: lang is empty at {info}")
    return lang, source


def fn_match_comma_sep(language, pattern_cs):
    for lang_pat in pattern_cs.split(","):
        if fnmatch.fnmatchcase(language, lang_pat):
            return True
    return False


def is_untranstaled_allowed(source_text, language):
    source_text = source_text.encode("unicode_escape").decode("utf-8")

    if not os.path.exists(ALLOW_LIST_PATH):
        return False

    with open(ALLOW_LIST_PATH) as f:
        for i, line in enumerate(f):
            lang, source = parse_line(line, f"{ALLOW_LIST_PATH}:{i + 1}")
            if source == source_text:
                return fn_match_comma_sep(language, lang)
    return False


def add_to_allow_list(source_text, language):
    source_text = source_text.encode("unicode_escape").decode("utf-8")

    allow_list_path = PROPOSED_ALLOW_LIST_PATH
    if not os.path.exists(allow_list_path) and os.path.exists(ALLOW_LIST_PATH):
        allow_list_path = ALLOW_LIST_PATH

    tsv_lines = []
    existing_txt = False
    if os.path.exists(allow_list_path):
        with open(allow_list_path) as f:
            tsv_lines = f.readlines()

        # Check if the source_text already exists
        for i, line in enumerate(tsv_lines):
            lang, source = parse_line(line, f"{allow_list_path}:{i + 1}")
            if source == source_text:
                if fn_match_comma_sep(language, lang):
                    return
                lang = lang + "," + language
                tsv_lines[i] = f"{lang}\t{source}\n"
                existing_txt = True
                break
    else:
        tsv_lines.append("lang\tsource\n")

    if not existing_txt:
        tsv_lines.append(f"{language}\t{source_text}\n")

    with open(PROPOSED_ALLOW_LIST_PATH, "w", encoding="utf-8") as tsv_file:
        tsv_file.write("".join(tsv_lines))


def check_copied_source_on_lines(rootdir, file_to_format, stylepath=None):
    filename = os.path.join(rootdir, file_to_format.filename)

    # Extract the language code from the filename
    # (assuming format: mixxx_<lang_code>.ts)
    match = re.search(r".*/mixxx_(\w+)\.ts", file_to_format.filename)
    if not match:
        return False

    lang_code = match.group(1)

    if lang_code.startswith("en_"):
        # for Engish languages all source copies are allowed
        return False

    parser = etree.XMLParser(recover=True)

    try:
        tree = etree.parse(filename, parser)
    except Exception as e:
        print("XML parsing failed:")
        print(e)
        return False

    root = tree.getroot()

    found = False

    for context in root.findall("context"):
        for message in context.findall("message"):
            source = message.find("source").text
            translation_elem = message.find("translation")
            if translation_elem is None:
                continue
            if translation_elem.get("type") == "unfinished":
                continue
            translation = translation_elem.text
            if source != translation:
                continue
            line_number = translation_elem.sourceline
            for start, end in file_to_format.lines:
                if line_number >= start and line_number <= end:
                    if is_untranstaled_allowed(source, lang_code):
                        continue
                    print(
                        "Source copy found at: "
                        f"{file_to_format.filename}:{line_number}\n{source}",
                        file=sys.stderr,
                    )
                    add_to_allow_list(source, lang_code)
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

    if ret:
        print(
            "\n"  # For a distance to the list of findings
            "All disallowed copied source translations need to be removed"
            " at\n"
            "https://app.transifex.com/mixxx-dj-software and pulled again.\n"
            "If they are intended untranslated use:\n"
            f"{PROPOSED_ALLOW_LIST_PATH}\n"
            "to allow it.",
            file=sys.stderr,
        )

    return ret

# ... Code for writing warnings to temp file for count > 25

def check_warnings(warnings):
    warning_threshold = 25  # Set your threshold here
    if len(warnings) > warning_threshold:
        with tempfile.NamedTemporaryFile(delete=False, mode='w') as temp_file:
            for warning in warnings:
                temp_file.write(warning + '\n')
            print(f"Warning limit exceeded. Warnings written to {temp_file.name}")
    else:
        for warning in warnings:
            print(warning)


if __name__ == "__main__":
    sys.exit(main())
