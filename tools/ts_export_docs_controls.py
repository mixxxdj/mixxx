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
    name: str
    groups: set[str]
    description: str
    range: str | None
    is_read_only: bool
    feedback: str | None
    deprecated_since: str | None
    is_pot_meter: bool

    @staticmethod
    def from_node(node: document) -> "Control":
        signatures: list[document] = list(node.findall(addnodes.desc_signature))
        content: document = next(node.findall(addnodes.desc_content))
        description: str = "\n".join(
            [d.astext() for d in filter_children(content, "paragraph")]
        )
        fl = next(content.findall(nodes.field_list), None)
        fb_range = field_content(fl, "Range")
        is_read_only = fb_range is not None and "read-only" in fb_range
        feedback, deprecated_since = feeback_and_deprecated(fl)

        return Control(
            signatures[0]["control"],
            set(map(lambda s: s["group"], signatures)),
            description,
            fb_range,
            is_read_only,
            feedback,
            deprecated_since,
            is_pot_meter(content),
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


def read_table(node: nodes.table) -> list[list[str]]:
    table_data = []

    for row_elem in node.findall(nodes.row):
        row_content = []

        for entry_elem in row_elem.findall(nodes.entry):
            all_text_parts = entry_elem.astext()
            cell_text = " ".join("".join(all_text_parts).split())
            row_content.append(cell_text)

        if row_content:
            table_data.append(tuple(row_content))

    return table_data


@dataclass
class PotMeterSuffix:
    suffix: str
    description: str


def read_pot_meter_suffixes(doc: document) -> list[PotMeterSuffix]:
    cpms_table: nodes.table = next(
        doc.findall(
            lambda n: isinstance(n, nodes.table) and "Control Suffix" in n.astext()
        )
    )  # type: ignore
    return [PotMeterSuffix(cpms[0], cpms[1]) for cpms in read_table(cpms_table)[1:]]


def is_pot_meter(content: document) -> bool:
    return (
        next(
            content.findall(
                lambda n: isinstance(n, addnodes.pending_xref)
                and "controlpotmeter" in n["reftarget"]  # type: ignore
            ),
            None,
        )
        is not None
    )


# ====================================
# Generate TypeScript type definitions
# ====================================

type GroupedControls = dict[str, dict[str, Control]]


def ts_doc_comment(control: Control) -> str:
    lines = []

    if control.description:
        for line in control.description.split("\n"):
            lines.append(line)
    else:
        lines.append("(No description)")
    lines.append("")
    lines.append(f"@groups {', '.join(control.groups)}")

    if control.range:
        lines.append(f"@range {control.range}")
    if control.feedback:
        lines.append(f"@feedback {control.feedback}")
    if control.is_read_only:
        lines.append("@readonly")
    if control.deprecated_since:
        lines.append(f"@deprecated since {control.deprecated_since}")

    return "\n".join(comment_lines(lines))


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


def group_var(name: str) -> str:
    return re.sub(r"[\[\]\_]", "", name) + "Control"


def control_var(name: str, is_pot_meter: bool) -> str:
    pattern = r"([A-Z])(?=\]|$|_)"
    name, n = re.subn(pattern, r"${number}", name)
    pm_str = "${PotMeterSuffix}" if is_pot_meter else ""
    return f"`{name}{pm_str}`" if n > 0 or is_pot_meter else f'"{name}"'


def create_control_types_str(
    groupControls: dict[str, dict[str, Control]], prefix=""
) -> list[str]:
    lines: list[str] = []
    ctls: list[Control] = [c for gc in groupControls.values() for c in gc.values()]
    for group, controls in groupControls.items():
        lines.append(f"type {prefix}{group_var(group)} = ")

        # controls
        for name, control in controls.items():
            lines.append(ts_doc_comment(control))
            lines.append(f"| {control_var(name, control.is_pot_meter)}")

        # parent groups
        lines.extend(
            [f"| {prefix}{group_var(p)}" for p in find_group_parents(group, ctls)]
        )
        lines.append(";\n")
    lines.extend(create_combined_types(ctls, prefix))
    return lines


def find_group_parents(group: str, controls: list[Control]) -> set[str]:
    return {
        "".join(c.groups) for c in controls if len(c.groups) > 1 and group in c.groups
    }


def create_combined_types(controls: list[Control], prefix: str) -> list[str]:
    lines = []
    groups = {el for c in controls for el in c.groups}
    for group in groups:
        # If group has it's own controls, it should already exist
        if any(len(c.groups) == 1 and group in c.groups for c in controls):
            continue

        parents = find_group_parents(group, controls)
        if len(parents) > 0:
            parents_str = " | ".join([prefix + group_var(p) for p in parents])
            lines.append(f"type {prefix}{group_var(group)} = {parents_str};\n")
    return lines


def comment_lines(lines: list[str]) -> list[str]:
    lines = [f" * {line}" for line in lines]
    return ["\n/**"] + lines + [" */"]


def create_pot_meter_suffixes(pm_suffixes: list[PotMeterSuffix]) -> list[str]:
    lines = ["type PotMeterSuffix = ", '| ""']
    for pms in pm_suffixes:
        lines.extend(comment_lines([pms.description]))
        lines.append(f'| "{pms.suffix}"')
    lines.append(";\n")
    return lines


def export_ts_types(
    output: Path, controls: list[Control], pm_suffixes: list[PotMeterSuffix]
):
    lines: list[str] = create_pot_meter_suffixes(pm_suffixes)

    grouped_controls = defaultdict(dict)
    for control in controls:
        grouped_controls["".join(control.groups)][control.name] = control
    grouped_controls: GroupedControls = dict(grouped_controls)

    # read/write controls
    rw_controls = filter_controls(
        grouped_controls,
        lambda c: c.deprecated_since is None and not c.is_read_only,
    )
    lines.extend(
        create_control_types_str(
            rw_controls,
        )
    )

    # read-only controls
    lines.append("\tnamespace ReadOnly {\n")
    r_controls = filter_controls(
        grouped_controls, lambda c: c.deprecated_since is None and c.is_read_only
    )
    lines.extend(create_control_types_str(r_controls, "ReadOnly"))
    lines.append("\n\t}\n")

    # deprecated controls
    lines.append("\tnamespace Deprecated {\n")
    l_controls = filter_controls(
        grouped_controls, lambda c: c.deprecated_since is not None
    )
    lines.extend(create_control_types_str(l_controls, "Deprecated"))
    lines.append("\n\t}\n")

    # Write file
    with open(output, "w") as f:
        f.write(f"""// Mixxx control types
// Generated file, don't change anything by hand

declare namespace MixxxControls {{ 
\t{"\n\t".join(lines)} 
}}
""")


if __name__ == "__main__":
    doc = read_doc(docname_to_parse)

    pm_suffixes = read_pot_meter_suffixes(doc)

    controls: list[Control] = []

    for node in doc.findall(addnodes.desc):
        if "mixxx" not in node["classes"] or "control" not in node["classes"]:
            continue
        control = Control.from_node(node)
        controls.append(control)

    export_ts_types(output, controls, pm_suffixes)
