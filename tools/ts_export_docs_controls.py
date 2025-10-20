import os

from pathlib import Path
from docutils import nodes
from sphinx import addnodes
from dataclasses import dataclass
from collections.abc import Iterator
from sphinx.application import Sphinx
from docutils.nodes import document, Node, field_list

manual_dir = (
    Path(os.path.dirname(os.path.realpath(__file__))) / ".." / "manual"
).resolve()
docname_to_parse = "chapters/appendix/mixxx_controls"


# ======================================
# Read Mixxx controls from documentation
# ======================================


def read_doc(doc_rel_path: str) -> document:
    build_dir = manual_dir / "build"
    app = Sphinx(
        srcdir=str(manual_dir / "source"),
        confdir=str(manual_dir / "source"),
        outdir=str(build_dir / "dummy"),
        doctreedir=str(build_dir / ".doctrees"),
        buildername="dummy",
    )

    app.build()

    if doc_rel_path not in app.env.all_docs:
        raise Exception(f"Document '{doc_rel_path}' not found in spinx env")

    return app.env.get_doctree(doc_rel_path)


def filter_children(n: Node, tagname: str) -> Iterator[document]:
    return filter(lambda n: hasattr(n, "tagname") and n.tagname == tagname, n.children)  # type: ignore


def field_list_find(fl: field_list, name: str) -> document | None:
    for f in filter_children(fl, "field"):
        fn = next(filter_children(f, "field_name"), None)
        if fn is not None and fn.astext() == name:
            return f
    return None


def field_content(fl: field_list | None, name: str) -> str:
    if fl is None:
        return ""
    field = field_list_find(fl, name)
    if field is None:
        return ""
    body = next(filter_children(field, "field_body"), None)
    if body:
        return body.astext()
    return ""


def deprecated(fl: field_list | None) -> str | None:
    if fl is None:
        return None
    fb = field_list_find(fl, "Feedback")
    if fb is None:
        return None
    deprecated = next(fb.findall(addnodes.versionmodified), None)
    if deprecated:
        return deprecated.astext()
    return None


@dataclass
class Control:
    group: str
    name: str
    description: str
    range: str
    feedback: str
    is_read_only: bool
    deprecated_since: str | None

    @staticmethod
    def from_node(node: document) -> "Control":
        signature: document = next(node.findall(addnodes.desc_signature))
        content: document = next(node.findall(addnodes.desc_content))
        description: str = "\n".join(
            [d.astext() for d in filter_children(content, "paragraph")]
        )
        fl = next(content.findall(nodes.field_list), None)
        fb_range = field_content(fl, "Range")
        feedback = field_content(fl, "Feedback")
        is_read_only = "read-only" in fb_range
        deprecated_since = deprecated(fl)

        return Control(
            signature["group"],
            signature["control"],
            description,
            fb_range,
            feedback,
            is_read_only,
            deprecated_since,
        )


# ====================================
# Generate TypeScript type definitions
# ====================================

# TODO


if __name__ == "__main__":
    doc = read_doc(docname_to_parse)
    controls: list[Control] = []

    for node in doc.findall(addnodes.desc):
        if "mixxx" not in node["classes"] or "control" not in node["classes"]:
            continue
        control = Control.from_node(node)
        controls.append(control)

    print(len(controls))
