#!/usr/bin/env python3
import argparse
import sys
import xml.dom.minidom
import xml.parsers.expat


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "files",
        type=argparse.FileType("r+"),
        nargs="+",
        help="files to reformat",
    )
    args = parser.parse_args(argv)

    exitcode = 0
    for f in args.files:
        content = f.read()
        try:
            dom = xml.dom.minidom.parseString(content)
        except xml.parsers.expat.ExpatError as e:
            print(
                "{filename}:{lineno}:{offset}: {message}".format(
                    filename=f.name,
                    lineno=e.lineno,
                    offset=e.offset,
                    message=xml.parsers.expat.errors.messages[e.code],
                ),
                file=sys.stderr,
            )
            exitcode = 1
            continue
        output = "\n".join(
            line.rstrip()
            for line in dom.toprettyxml(indent=" " * 4).splitlines()
            if line.strip()
        )
        f.seek(0)
        f.truncate()
        f.write(output)
        f.write("\n")

    return exitcode


if __name__ == "__main__":
    sys.exit(main())
