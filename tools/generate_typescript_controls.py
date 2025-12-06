import os
import re
import glob

from pathlib import Path
from dataclasses import dataclass
from collections.abc import Callable


# ======================================
# # Scan the C++ code for Mixxx controls
# ======================================

type GroupedControls = dict[str, dict[str, ControlInfo]]


def scan_codebase(root: Path) -> GroupedControls:
    """Scans codebase for Mixxx controls

    Returns:
        dict[group]:
            dict[controlname]:
                ControlInfo
    """
    all_controls = []
    for path in find_all_cpp_files(root / "src"):
        controls, aliases = extract_controls(path)
        all_controls.extend(controls + aliases)

    grouped: dict[str, dict[str, ControlInfo]] = {}
    for item in all_controls:
        grp, ctl, legacy = item["group"], item["control"], item["legacy"]
        grouped.setdefault(grp, {})[ctl] = ControlInfo(legacy, False)

    return grouped


def contains_str(filepath: Path, search_string: str) -> bool:
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            for line in f:
                if search_string in line:
                    return True
    except Exception as e:
        print(f"Error: {e}")
    return False


def find_all_cpp_files(dir_path: Path) -> list[Path]:
    paths = glob.glob(f"{dir_path}/**/*.cpp", recursive=True)
    search_str = "std::make_unique<ControlObject>"
    return list(
        filter(
            lambda f: contains_str(f, search_str),
            map(lambda p: Path(p), paths),
        )
    )


# --- Regex-Patterns ---
# Group definition: const QString kAppGroup = QStringLiteral("[App]");
GROUP_PATTERN = re.compile(
    r'const\s+QString\s+(?P<varname>\w+)\s*=\s*QStringLiteral\("(?P<group>\[.*?\])"\);'
)

# Control creation: ConfigKey(kAppGroup, QStringLiteral("gui_tick_full_period_s"))
CONTROL_PATTERN = re.compile(
    r'ConfigKey\((?P<group>\w+),\s*QStringLiteral\("(?P<control>[^"]+)"\)\)'
)

# Alias creation (legacy links)
ALIAS_PATTERN = re.compile(
    r'(?:addAlias|insertAlias)\s*\(\s*ConfigKey\((?P<group>\w+),\s*"?(?:QStringLiteral\(")?(?P<control>[^")]+)"?\)?'
)


def extract_controls(filepath: Path):
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        code = f.read()

    groups = dict(GROUP_PATTERN.findall(code))  # varname -> group
    controls = []
    aliases = []

    for match in CONTROL_PATTERN.finditer(code):
        group_var = match.group("group")
        control_name = match.group("control")
        group_value = groups.get(group_var, group_var)
        controls.append(
            {"group": group_value, "control": control_name, "legacy": False}
        )

    for match in ALIAS_PATTERN.finditer(code):
        group_var = match.group("group")
        control_name = match.group("control")
        group_value = groups.get(group_var, group_var)
        aliases.append({"group": group_value, "control": control_name, "legacy": True})

    return controls, aliases


@dataclass
class ControlInfo:
    is_legacy: bool
    is_read_only: bool


# =========================================
# Export Mixxx controls as TypeScript types
# =========================================


def filter_controls(
    groupControls: GroupedControls, filter: Callable[[ControlInfo], bool]
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
    groupControls: dict[str, dict[str, ControlInfo]], indent: int
) -> str:
    ts_str: str = ""
    for group, controls in groupControls.items():
        ts_str += f"{tabs(indent)}type {re.sub(r'[\[\]]', '', group)}Controls = \n"
        for ctl in controls:
            ts_str += f'{tabs(indent + 1)}| "{ctl}"\n'
        ts_str += f"{tabs(indent)};\n\n"
    return ts_str


def export_ts_types(output: Path, groupControls: dict[str, dict[str, ControlInfo]]):
    with open(output, "w") as f:
        f.write("""// Mixxx control types
// Generated file, don't change anything by hand

declare namespace MixxxControls {
                
""")
        # read/write controls
        rw_controls = filter_controls(
            groupControls, lambda c: not c.is_legacy and not c.is_read_only
        )
        f.write(create_control_types_str(rw_controls, 1))

        # read-only controls
        f.write("   namespace ReadOnly {\n\n")
        r_controls = filter_controls(
            groupControls, lambda c: not c.is_legacy and c.is_read_only
        )
        f.write(create_control_types_str(r_controls, 2))
        f.write("   }\n\n")

        # deprecated controls
        f.write("   namespace Deprecated {\n\n")
        l_controls = filter_controls(groupControls, lambda c: c.is_legacy)
        f.write(create_control_types_str(l_controls, 2))
        f.write("   }\n\n")

        f.write("}")


if __name__ == "__main__":
    root_dir = (Path(os.path.dirname(os.path.realpath(__file__))) / "..").absolute()
    controls = scan_codebase(root_dir)
    output = root_dir / "res" / "controllers" / "_mixxx-controls.ts"
    export_ts_types(output, controls)
