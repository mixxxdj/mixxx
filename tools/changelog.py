#!/usr/bin/env python3
import argparse
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
    parser.add_argument("file", type=argparse.FileType("r+"))
    args = parser.parse_args(argv)

    # Fetch changelog and convert to RST
    changelog = args.file.read()
    fixed_changelog = add_missing_links(changelog)
    if changelog != fixed_changelog:
        args.file.seek(0)
        args.file.write(fixed_changelog)
        args.file.truncate()
    args.file.close()


if __name__ == "__main__":
    main()
