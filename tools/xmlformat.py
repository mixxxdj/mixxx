#!/usr/bin/env python3
import argparse
import sys
import textwrap
import xml.dom.minidom
import xml.parsers.expat

import cssbeautifier

INDENT = " " * 2


def get_node_indent(node):
    if (
        node.previousSibling
        and node.previousSibling.nodeType == xml.dom.Node.TEXT_NODE
    ):
        text = node.previousSibling.data
        return text.rpartition("\n")[2]
    return ""


def format_css(text):
    css_opts = cssbeautifier.default_options()
    css_opts.indent_size = 2
    css_opts.indent_char = " "
    css = cssbeautifier.beautify(text, opts=css_opts)
    return textwrap.indent(css, INDENT)


def walk_dom(node):
    for child in node.childNodes:
        yield child
        yield from walk_dom(child)


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "files",
        type=argparse.FileType("rb+"),
        nargs="+",
        help="files to reformat",
    )
    args = parser.parse_args(argv)

    exitcode = 0
    for f in args.files:
        try:
            dom = xml.dom.minidom.parse(f)
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
            for line in dom.toprettyxml(indent=INDENT).splitlines()
            if line.strip()
        )

        # Fix some issues caused pretty printing skin files
        dom = xml.dom.minidom.parseString(output)
        for node in walk_dom(dom):
            if node.nodeType == xml.dom.Node.ELEMENT_NODE:
                # Fix indentation for Style Elements (Inline QSS)
                if (
                    node.tagName == "Style"
                    and len(node.childNodes) == 1
                    and node.childNodes[0].nodeType == xml.dom.Node.TEXT_NODE
                ):
                    indent = get_node_indent(node)
                    node.childNodes[0].data = (
                        "\n"
                        + textwrap.indent(
                            format_css(node.childNodes[0].data.strip()), indent
                        )
                        + "\n"
                        + indent
                    )
                elif node.tagName == "Variable":
                    # Remove superfluous whitespace around Variable elements
                    for sibling in node.parentNode.childNodes:
                        if sibling.nodeType == xml.dom.Node.TEXT_NODE:
                            sibling.data = sibling.data.strip()
            elif node.nodeType == xml.dom.Node.COMMENT_NODE:
                # Fix indentation for comments
                indent = get_node_indent(node)
                lines = node.data.strip().splitlines()
                if len(lines) > 1:
                    text = (
                        lines[0].strip()
                        + "\n"
                        + textwrap.dedent("\n".join(lines[1:]))
                    )
                    node.data = (
                        "\n" + textwrap.indent(text, indent) + "\n" + indent
                    )

        output = dom.toxml(encoding="utf-8")

        # Replace escaped quotes (e.g. in Style elements) with raw char and
        # insert newline between XML declaration and root element
        output = output.replace(b"&quot;", b'"').replace(b"?><", b"?>\n<", 1)

        f.seek(0)
        f.truncate()
        f.write(output)
        f.write(b"\n")

    return exitcode


if __name__ == "__main__":
    sys.exit(main())
