#!/usr/bin/env python3
import argparse
import sys
import xml.dom.minidom


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "files",
        type=argparse.FileType("r+"),
        nargs="+",
        help="files to reformat",
    )
    args = parser.parse_args(argv)

    for f in args.files:
        content = f.read()
        dom = xml.dom.minidom.parseString(content)
        output = "\n".join(
            line.rstrip()
            for line in dom.toprettyxml(indent=" " * 4).splitlines()
            if line.strip()
        )
        f.seek(0)
        f.truncate()
        f.write(output)
        f.write("\n")

    return 0


if __name__ == "__main__":
    sys.exit(main())
