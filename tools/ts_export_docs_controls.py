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
    groups: list[str]
    description: list[str]
    val_range: list[str]
    version_added: str | None
    is_read_only: bool
    feedback: str | None
    deprecated_since: str | None
    is_pot_meter: bool

    @staticmethod
    def from_node(node: addnodes.desc) -> "Control":
        signatures: list[addnodes.desc_signature] = list(
            node.findall(addnodes.desc_signature)
        )
        content = next(node.findall(addnodes.desc_content))
        fl = next(content.findall(nodes.field_list), None)
        val_range, is_read_only = range_and_readonly(fl)

        return Control(
            signatures[0]["control"],
            list(sorted(map(lambda s: s["group"], signatures))),
            text_content(content),
            val_range,
            get_version_added(content),
            is_read_only,
            get_feedback(fl),
            get_deprecated(content),
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


def markdown_row(row: list[str], fillchar: str = " ") -> str:
    row_str = "|".join([row[i].ljust(3, fillchar) for i in range(len(row))])
    return "|" + row_str + "|"


def markdown_table(table: list[list[str]]) -> list[str]:
    lines = []
    header = table[0]

    lines.append(markdown_row(header))
    lines.append(markdown_row(["---" for _ in range(len(header))], "-"))
    lines.extend([markdown_row(row) for row in table[1:]])

    return lines


def text_content(node: nodes.Element) -> list[str]:
    lines: list[str] = []
    for c in node.children:
        if isinstance(c, nodes.table):
            table = read_table(c)
            lines.extend(markdown_table(table))
        elif isinstance(c, nodes.paragraph):
            lines.extend(c.astext().split("\n"))

    return lines


def filter_children(n: Node, tagname: str) -> Iterator[document]:
    return filter(lambda n: hasattr(n, "tagname") and n.tagname == tagname, n.children)  # type: ignore


def field_list_find(fl: field_list, name: str) -> nodes.field | None:
    for f in filter_children(fl, "field"):
        fn = next(filter_children(f, "field_name"), None)
        if fn is not None and fn.astext() == name:
            return f  # type: ignore
    return None


def field_content(field: nodes.field | None) -> nodes.field_body | None:
    if field is None:
        return None
    return next(filter_children(field, "field_body"), None)  # type: ignore


def field_text(fl: field_list | None, name: str) -> list[str]:
    if fl is None:
        return []
    field_body = field_content(field_list_find(fl, name))

    if field_body:
        return text_content(field_body)
    return []


def range_and_readonly(fl: nodes.field_list | None) -> tuple[list[str], bool]:
    if fl is None:
        return [], False

    val_range = field_text(fl, "Range")
    is_read_only = False

    for i in range(len(val_range)):
        Line_cleaned, count = re.subn(", read-only", "", val_range[i])
        if count > 0:
            is_read_only = True
            val_range[i] = Line_cleaned

    return val_range, is_read_only


def get_feedback(fl: field_list | None) -> str | None:
    if fl is None:
        return None

    field = field_list_find(fl, "Feedback")
    if field is None:
        return None

    feedback = field_content(field)

    return "".join(text_content(feedback)) if feedback else None


def get_version_modified_tag(
    content: nodes.Element, attr: str, attr_val: str
) -> addnodes.versionmodified | None:
    tags = content.findall(addnodes.versionmodified)
    return next(filter(lambda t: t[attr] == attr_val, tags), None)


def get_deprecated(content: nodes.Element) -> str | None:
    dep_tag = get_version_modified_tag(content, "type", "deprecated")
    if dep_tag:
        return dep_tag.astext().replace("Deprecated since", "")
    return None


def get_version_added(content: nodes.Element) -> str | None:
    dep_tag = get_version_modified_tag(content, "type", "versionadded")
    if dep_tag:
        return dep_tag.astext()
    return None


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


def is_pot_meter(content: addnodes.desc_content) -> bool:
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

    if len(control.description) > 0:
        lines.extend(control.description)
    else:
        lines.append("(No description)")
    lines.append("")
    lines.append(f"@groups {', '.join(control.groups)}")

    if len(control.val_range) > 0:
        if len(control.val_range) == 1:
            lines.append(f"@range {control.val_range[0]}")
        else:
            lines.append("@range")
            lines.extend(control.val_range)
    if control.feedback:
        lines.append(f"@feedback {control.feedback}")
    if control.version_added:
        lines.append(f"@since {control.version_added}")
    if control.is_read_only:
        lines.append("@readonly")
    if control.is_pot_meter:
        lines.append("@kind pot meter control")
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


def replace_dynamics(name: str) -> tuple[str, int]:
    # Search for every big letter at the end or before ] or _, exception DJ
    return re.subn(r"([A-IK-Z]|(?<!D)J)(?=\]|$|_)", r"${number}", name)


def control_var(name: str, is_pot_meter: bool) -> str:
    name, n = replace_dynamics(name)
    pm_str = "${PotMeterSuffix}" if is_pot_meter else ""
    return f"`{name}{pm_str}`" if n > 0 or is_pot_meter else f'"{name}"'


def create_control_types_str(
    groupControls: dict[str, dict[str, Control]], prefix=""
) -> list[str]:
    lines: list[str] = []
    ctls: list[Control] = sorted(
        [c for gc in groupControls.values() for c in gc.values()], key=lambda c: c.name
    )
    for group, controls in sorted(groupControls.items()):
        lines.append(f"type {prefix}{group_var(group)} = ")

        # controls
        for name, control in sorted(controls.items()):
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


def create_group_control_linking(groups: list[str], type_prefix: str = "") -> list[str]:
    lines = [f"type {type_prefix}Controls = {{"]
    groups_and_count = [replace_dynamics(group) for group in groups]
    prefix = "" if type_prefix == "" else f"{type_prefix}.{type_prefix}"

    # static groups
    lines.extend(
        [
            f'"{group}": {prefix}{group_var(group)};'
            for group, count in groups_and_count
            if count == 0
        ]
    )

    # dynamic groups
    dynamics = [
        f"[key: `{groups_and_count[i][0]}`]: {prefix}{group_var(groups[i])};"
        for i in range(len(groups_and_count))
        if groups_and_count[i][1] > 0
    ]
    if dynamics:
        lines.append("} & {")
        lines.extend(dynamics)

    lines.append("};")
    return lines


def extract_groups(grouped_controls: GroupedControls) -> list[str]:
    return sorted(
        {
            group
            for controls in grouped_controls.values()
            for c in controls.values()
            for group in c.groups
        }
    )


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
    lines.extend(create_control_types_str(rw_controls))

    # read-only controls
    lines.append("\tnamespace ReadOnly {\n")
    lines.append("\t\t// Read-only controls")
    ro_controls = filter_controls(
        grouped_controls, lambda c: c.deprecated_since is None and c.is_read_only
    )
    lines.extend(create_group_control_linking(extract_groups(ro_controls), "ReadOnly"))
    lines.extend(create_control_types_str(ro_controls, "ReadOnly"))
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
    /*
     * Public
     */
    export type MixxxGroup = keyof Controls | (string & {{}});

    // All controls
    export type MixxxControl<TGroup> =
        | (string & {{}})
        | (0 extends 1 & TGroup // is any check
              ? string
              : TGroup extends keyof Controls | keyof ReadOnly.ReadOnlyControls
              ?
                    | (TGroup extends keyof Controls ? Controls[TGroup] : never)
                    | (TGroup extends keyof ReadOnly.ReadOnlyControls
                          ? ReadOnly.ReadOnlyControls[TGroup]
                          : never)
              : string);

    // Controls that are read & write at the same time
    export type MixxxControlReadAndWrite<TGroup> =
        | (string & {{}})
        | (0 extends 1 & TGroup // is any check
              ? string
              : TGroup extends keyof Controls
              ? Controls[TGroup]
              : string);

    /*
     * Group <-> control linking
     */

    // Read/Write controls
    {"\n".join(create_group_control_linking(extract_groups(rw_controls)))} 

    /*
     * Values
     */
\t{"\n\t".join(lines)} 
}}
""")


if __name__ == "__main__":
    doc = read_doc(docname_to_parse)

    pm_suffixes = read_pot_meter_suffixes(doc)

    controls: list[Control] = []

    for section in doc.findall(nodes.section):
        if "removed-controls" in section["ids"] or "mixxx-controls" in section["ids"]:
            continue
        for node in section.findall(addnodes.desc):
            if "mixxx" not in node["classes"] or "control" not in node["classes"]:
                continue
            control = Control.from_node(node)
            controls.append(control)

    export_ts_types(output, controls, pm_suffixes)
