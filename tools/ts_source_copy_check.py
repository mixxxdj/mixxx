#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import logging
import os
import re
import sys
import typing

from lxml import etree
import githelper

# Define the path to the allow list XML file
ALLOW_LIST_PATH = "res/translations/source_copy_allow_list.xml"
PROPOSED_ALLOW_LIST_PATH = (
    "res/translations/source_copy_allow_list_proposed.xml"
)


def is_untranstaled_allowed(source_text, language):
    if os.path.exists(ALLOW_LIST_PATH):
        parser = etree.XMLParser(recover=True)
        tree = etree.parse(ALLOW_LIST_PATH, parser)
        root = tree.getroot()
    else:
        return False

    for message in root.findall("message"):
        if message.find("source").text == source_text:
            if message.find("allow_all_languages").text == "true":
                return True
            languages_elem = message.find("allowed_languages")
            if languages_elem is None:
                return False
            allowed_languages = {
                lang.text for lang in languages_elem.findall("language")
            }
            if language in allowed_languages:
                return True
    return False


def add_to_allow_list(source_text, language):
    if os.path.exists(PROPOSED_ALLOW_LIST_PATH):
        parser = etree.XMLParser(recover=True)
        tree = etree.parse(PROPOSED_ALLOW_LIST_PATH, parser)
        root = tree.getroot()
    else:
        if os.path.exists(ALLOW_LIST_PATH):
            parser = etree.XMLParser(recover=True)
            tree = etree.parse(ALLOW_LIST_PATH, parser)
            root = tree.getroot()
        else:
            root = etree.Element("allow_list")
            tree = etree.ElementTree(root)

    # Check if the message already exists
    existing_message = None
    for message in root.findall("message"):
        if message.find("source").text == source_text:
            existing_message = message
            break

    if existing_message is not None:
        allow_all_languages = (
            existing_message.find("allow_all_languages").text == "true"
        )
        if not allow_all_languages:
            languages_elem = existing_message.find("allowed_languages")
            if languages_elem is None:
                languages_elem = etree.SubElement(
                    existing_message, "allowed_languages"
                )
            allowed_languages = {
                lang.text for lang in languages_elem.findall("language")
            }
            if language not in allowed_languages:
                etree.SubElement(languages_elem, "language").text = language
    else:
        message = etree.SubElement(root, "message")
        etree.SubElement(message, "source").text = source_text
        etree.SubElement(message, "allow_all_languages").text = "false"
        languages_elem = etree.SubElement(message, "allowed_languages")
        etree.SubElement(languages_elem, "language").text = language

    # Write the updated tree to the file with pretty printing
    etree.indent(root, space="  ")
    with open(PROPOSED_ALLOW_LIST_PATH, "wb") as f:
        f.write(
            etree.tostring(
                tree, pretty_print=True, xml_declaration=True, encoding="UTF-8"
            )
        )


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
    tree = etree.parse(filename, parser)
    root = tree.getroot()

    found = False

    for context in root.findall("context"):
        for message in context.findall("message"):
            source = message.find("source").text
            translation_elem = message.find("translation")
            if translation_elem is None:
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
                        f"{file_to_format.filename}:{line_number}",
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
            "\n"
            "All not allowed copied source translations need to be removed"
            " at\n"
            "https://app.transifex.com/mixxx-dj-software and pulled again.\n"
            "If they are intended untranslated use:\n"
            f"{PROPOSED_ALLOW_LIST_PATH}\n"
            "to allow it.",
            file=sys.stderr,
        )

    return ret


if __name__ == "__main__":
    sys.exit(main())
