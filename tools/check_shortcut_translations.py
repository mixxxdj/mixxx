"""Check and fix invalid keyboard shortcut translations in Mixxx .ts files.

See https://github.com/mixxxdj/mixxx/issues/16216

Usage:
    # Report all invalid shortcut translations (dry run):
    python3 tools/check_shortcut_translations.py res/translations/

    # Fix invalid translations by marking them unfinished:
    python3 tools/check_shortcut_translations.py --fix res/translations/

    # Fix and recompile .qm files (requires lrelease in PATH):
    python3 tools/check_shortcut_translations.py --fix --compile res/translations/

    # Recompile all stale .qm files without fixing:
    python3 tools/check_shortcut_translations.py --compile res/translations/
"""

import argparse
import pathlib
import re
import shutil
import subprocess
import sys


_SHORTCUT_PATTERN = re.compile(
    r"^(Ctrl|Alt|Shift|Meta)(\+(Ctrl|Alt|Shift|Meta|[^\s+]+))+$",
    re.IGNORECASE,
)


def _normalize(s):
    """Strip whitespace around '+' and lowercase for comparison."""
    return re.sub(r"\s*\+\s*", "+", s.strip()).lower()


def is_pure_shortcut(source):
    """Return True if source is a keyboard shortcut string."""
    return bool(_SHORTCUT_PATTERN.match(source.strip()))


def find_invalid_shortcuts(ts_text):
    """Yield (source, translation) pairs where the shortcut is incorrectly translated."""
    message_re = re.compile(r"<message\b[^>]*>(.*?)</message>", re.DOTALL)
    source_re = re.compile(r"<source>([^<]*)</source>")
    translation_re = re.compile(r"<translation(\s[^>]*)?>([^<]*)</translation>")

    for msg_match in message_re.finditer(ts_text):
        msg = msg_match.group(1)

        src = source_re.search(msg)
        if not src:
            continue
        source = src.group(1)
        if not is_pure_shortcut(source):
            continue

        trans = translation_re.search(msg)
        if not trans:
            continue
        attrs = trans.group(1) or ""
        if "unfinished" in attrs or "obsolete" in attrs:
            continue
        translation = trans.group(2)
        if not translation.strip():
            continue

        if _normalize(translation) != _normalize(source):
            yield source, translation


def fix_ts_file(ts_path):
    """Mark invalid shortcut translations as unfinished. Returns the fix count."""
    text = ts_path.read_text(encoding="utf-8")
    original = text
    count = 0

    message_re = re.compile(
        r"(<source>)([^<]*)(</source>)((?:(?!</message>).)*?)"
        r"(<translation(?:\s[^>]*)?>)([^<]*)(</translation>)",
        re.DOTALL,
    )

    def replace_if_invalid(m):
        nonlocal count
        source = m.group(2)
        translation = m.group(6)
        if (
            translation.strip()
            and is_pure_shortcut(source)
            and _normalize(translation) != _normalize(source)
        ):
            count += 1
            return (
                m.group(1) + m.group(2) + m.group(3)
                + m.group(4)
                + '<translation type="unfinished"></translation>'
            )
        return m.group(0)

    text = message_re.sub(replace_if_invalid, text)

    if text != original:
        ts_path.write_text(text, encoding="utf-8")

    return count


def compile_qm(ts_path):
    """Compile a .ts file to .qm using lrelease."""
    qm_path = ts_path.with_suffix(".qm")
    try:
        subprocess.run(
            ["lrelease", str(ts_path), "-qm", str(qm_path)],
            check=True,
            capture_output=True,
        )
        return True
    except subprocess.CalledProcessError as e:
        print(f"warning: lrelease failed for {ts_path}: {e}", file=sys.stderr)
        return False


def main(argv=None):
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "translations_dir",
        type=pathlib.Path,
        help="Path to the translations directory (e.g. res/translations/)",
    )
    parser.add_argument(
        "--fix",
        action="store_true",
        help="Mark invalid shortcut translations as unfinished",
    )
    parser.add_argument(
        "--compile",
        action="store_true",
        help="Recompile .qm files after fixing",
    )
    args = parser.parse_args(argv)

    ts_files = sorted(args.translations_dir.glob("mixxx_*.ts"))
    if not ts_files:
        print(f"No .ts files found in {args.translations_dir}", file=sys.stderr)
        sys.exit(1)

    total_issues = 0

    for ts_path in ts_files:
        lang = ts_path.stem.replace("mixxx_", "")
        text = ts_path.read_text(encoding="utf-8")
        invalid = list(find_invalid_shortcuts(text))

        if not invalid:
            continue

        total_issues += len(invalid)
        print(f"{lang} ({len(invalid)} invalid shortcut translation(s)):")
        for source, translation in invalid:
            print(f"  {source!r:25} -> {translation!r}")

        if args.fix:
            count = fix_ts_file(ts_path)
            print(f"  -> fixed {count} entry/entries in {ts_path.name}")

        print()

    if total_issues == 0:
        print("No invalid shortcut translations found.")
    elif not args.fix:
        print(
            f"Found {total_issues} invalid shortcut translation(s). "
            "Run with --fix to correct them."
        )
        sys.exit(1)

    if args.compile:
        if not shutil.which("lrelease"):
            print("warning: lrelease not found in PATH, skipping .qm compilation.", file=sys.stderr)
        else:
            compiled = 0
            for ts in ts_files:
                qm = ts.with_suffix(".qm")
                if not qm.exists() or ts.stat().st_mtime > qm.stat().st_mtime:
                    if compile_qm(ts):
                        compiled += 1
            if compiled:
                print(f"Compiled {compiled} .qm file(s).")


if __name__ == "__main__":
    main()
