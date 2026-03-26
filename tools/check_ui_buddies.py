#!/usr/bin/env python3
import sys
import xml.etree.ElementTree as ET


def check_file(filepath):
    try:
        tree = ET.parse(filepath)
    except ET.ParseError:
        # Let check-xml pre-commit hook handle malformed XML
        return False

    root = tree.getroot()
    has_error = False

    # Collect all widget names in the UI file
    all_widgets = set()
    for widget in root.iter("widget"):
        if "name" in widget.attrib:
            all_widgets.add(widget.attrib["name"])

    # Check QLabel buddies
    for widget in root.iter("widget"):
        if widget.attrib.get("class") == "QLabel":
            label_name = widget.attrib.get("name", "<unnamed>")

            for prop in widget.findall("property"):
                if prop.attrib.get("name") == "buddy":
                    cstring = prop.find("cstring")
                    if cstring is not None and cstring.text:
                        buddy_name = cstring.text

                        if buddy_name == label_name:
                            msg = (
                                f"{filepath}: Error: "
                                f"QLabel '{label_name}' is its own buddy!"
                            )
                            print(msg, file=sys.stderr)
                            has_error = True
                        elif buddy_name not in all_widgets:
                            msg = (
                                f"{filepath}: Error: Buddy '{buddy_name}' "
                                f"for QLabel '{label_name}' does not exist!"
                            )
                            print(msg, file=sys.stderr)
                            has_error = True

    return has_error


if __name__ == "__main__":
    sys.exit(
        int(any(check_file(sys.argv[i]) for i in range(1, len(sys.argv))))
    )
