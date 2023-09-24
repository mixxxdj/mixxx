#!/usr/bin/env python3
import argparse
import pathlib
import re


def add_missing_links(changelog):
    """Ensure that all the changelog conforms to our common format."""
    # Ensure that all launchpad issues are hyperlinks
    changelog = re.sub(
        r"(?!\[)\blp:?(?P<lpid>\d+)\b(?!\])",
        r"[lp:\g<lpid>](https://bugs.launchpad.net/mixxx/+bug/\g<lpid>)",
        changelog,
    )

    # Ensure that all GitHub PRs are hyperlinks
    changelog = re.sub(
        r"(?!\[)#\b(?P<ghid>\d+)\b(?!\])",
        r"[#\g<ghid>](https://github.com/mixxxdj/mixxx/pull/\g<ghid>)",
        changelog,
    )

    return changelog


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=pathlib.Path)
    args = parser.parse_args(argv)

    # Fetch changelog and convert to RST
    with args.file.open("r") as fp:
        changelog = fp.read()

    fixed_changelog = add_missing_links(changelog)
    if changelog != fixed_changelog:
        with args.file.open("w") as fp:
            fp.write(fixed_changelog)


if __name__ == "__main__":
    main()
