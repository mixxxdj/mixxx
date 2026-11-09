#!/usr/bin/env python3
import sys
import xml.etree.ElementTree as ET


def check_file(filepath):
    try:
        tree = ET.parse(filepath)
    except ET.ParseError:
        return False

    root = tree.getroot()
    has_error = False

    has_width = "width" in root.attrib
    has_height = "height" in root.attrib

    if not has_width or not has_height:
        missing = []
        if not has_width:
            missing.append("'width'")
        if not has_height:
            missing.append("'height'")

        msg = (
            f"{filepath}: Error: SVG root tag is missing "
            f"{', '.join(missing)} attribute(s)."
        )
        print(msg, file=sys.stderr)
        has_error = True

    return has_error


if __name__ == "__main__":
    sys.exit(int(any(check_file(arg) for arg in sys.argv[1:])))
