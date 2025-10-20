import os
import re

from pathlib import Path
from docutils import nodes
from sphinx import addnodes
from dataclasses import dataclass
from collections import defaultdict
from sphinx.application import Sphinx
from collections.abc import Iterator, Callable
from docutils.nodes import document, Node, field_list

mixxx_dir = Path(os.path.dirname(os.path.realpath(__file__))) / ".."
manual_dir = (mixxx_dir / ".." / "manual").resolve()
docname_to_parse = "chapters/appendix/mixxx_controls"
output = mixxx_dir / "res" / "controllers" / "_mixxx-controls.ts"

# ======================================
# Read Mixxx controls from documentation
# ======================================


@dataclass
class Control:
    group: str
    name: str
    description: str
    range: str | None
    is_read_only: bool
    feedback: str | None
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
        is_read_only = fb_range is not None and "read-only" in fb_range
        feedback, deprecated_since = feeback_and_deprecated(fl)

        return Control(
            signature["group"],
            signature["control"],
            description,
            fb_range,
            is_read_only,
            feedback,
            deprecated_since,
        )


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


def field_content(fl: field_list | None, name: str) -> str | None:
    if fl is None:
        return None
    field = field_list_find(fl, name)
    if field is None:
        return None
    body = next(filter_children(field, "field_body"), None)
    if body:
        return body.astext()
    return None


def feeback_and_deprecated(fl: field_list | None) -> tuple[str | None, str | None]:
    if fl is None:
        return None, None
    fb = field_list_find(fl, "Feedback")
    if fb is None:
        return None, None
    deprecated = next(fb.findall(addnodes.versionmodified), None)
    if deprecated:
        feedback = next(filter_children(fb, "paragraph"), None)
        return feedback.astext() if feedback else None, deprecated.astext().replace(
            "Deprecated since", ""
        )
    return None, None


# ====================================
# Generate TypeScript type definitions
# ====================================

type GroupedControls = dict[str, dict[str, Control]]


def ts_doc_comment(control: Control) -> str:
    lines = ["\n/**"]

    if control.description:
        for line in control.description.split("\n"):
            lines.append(f" * {line}")
    else:
        lines.append(" * (No description)")

    lines.append(" *")
    lines.append(f" * @name {control.name}")
    lines.append(f" * @group {control.group}")

    if control.range:
        lines.append(f" * @range {control.range}")
    if control.feedback:
        lines.append(f" * @feedback {control.feedback}")
    if control.is_read_only:
        lines.append(" * @readonly")
    if control.deprecated_since:
        lines.append(f" * @deprecated since {control.deprecated_since}")

    lines.append(" */")

    return "\n".join(lines)


def filter_controls(
    groupControls: GroupedControls, filter: Callable[[Control], bool]
) -> GroupedControls:
    filtered_controls = {}
    for group, c in groupControls.items():
        f_c = {k: v for k, v in c.items() if filter(v)}
        if len(f_c) > 0:
            filtered_controls[group] = f_c
    return filtered_controls


def tabs(count: int) -> str:
    return "".join(["   " for _ in range(count)])


def create_control_types_str(
    groupControls: dict[str, dict[str, Control]], indent: int
) -> str:
    lines: list[str] = []
    for group, controls in groupControls.items():
        lines.append(f"{tabs(indent)}type {re.sub(r'[\[\]]', '', group)}Controls = ")
        for name, control in controls.items():
            lines.append(ts_doc_comment(control))
            lines.append(f'{tabs(indent + 1)}| "{name}"')
        lines.append(f"{tabs(indent)};\n")
    return "\n".join(lines)


def export_ts_types(output: Path, controls: list[Control]):
    grouped_controls = defaultdict(dict)
    for control in controls:
        grouped_controls[control.group][control.name] = control
    grouped_controls: GroupedControls = dict(grouped_controls)

    ts_str: str = ""

    # read/write controls
    rw_controls = filter_controls(
        grouped_controls,
        lambda c: c.deprecated_since is None and not c.is_read_only,
    )
    ts_str += create_control_types_str(rw_controls, 1)

    # read-only controls
    ts_str += "   namespace ReadOnly {\n\n"
    r_controls = filter_controls(
        grouped_controls, lambda c: c.deprecated_since is None and c.is_read_only
    )
    ts_str += create_control_types_str(r_controls, 2) + "\n   }\n\n"

    # deprecated controls
    ts_str += "   namespace Deprecated {\n\n"
    l_controls = filter_controls(
        grouped_controls, lambda c: c.deprecated_since is not None
    )
    ts_str += create_control_types_str(l_controls, 2) + "\n   }\n\n"

    # Write file
    with open(output, "w") as f:
        f.write(f"""// Mixxx control types
// Generated file, don't change anything by hand

declare namespace MixxxControls {{ 
    {ts_str} 
}}
""")


if __name__ == "__main__":
    doc = read_doc(docname_to_parse)
    controls: list[Control] = []

    for node in doc.findall(addnodes.desc):
        if "mixxx" not in node["classes"] or "control" not in node["classes"]:
            continue
        control = Control.from_node(node)
        controls.append(control)

    export_ts_types(output, controls)
