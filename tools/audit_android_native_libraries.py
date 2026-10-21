#!/usr/bin/env python3
"""Static API-floor audit for packaged Android shared libraries.

This checks ELF dynamic dependencies and strong undefined imports against the
Android NDK sysroot stubs. It is a screening tool, not a runtime compatibility
proof. Unknown providers are reported as uncertain instead of being treated as
compatible.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path


def readelf(readelf_path: str, *args: str) -> str:
    return subprocess.check_output(
        [readelf_path, *args], text=True, stderr=subprocess.STDOUT
    )


def dynamic_info(
    path: Path, readelf_path: str
) -> tuple[str | None, list[str]]:
    text = readelf(readelf_path, "--dynamic", "--wide", str(path))
    soname = None
    needed = []
    for line in text.splitlines():
        match = re.search(
            r"\(SONAME\).*soname: \[([^\]]+)\]", line, re.IGNORECASE
        )
        if match:
            soname = match.group(1)
        match = re.search(r"\(NEEDED\).*Shared library: \[([^\]]+)\]", line)
        if match:
            needed.append(match.group(1))
    return soname, needed


class SymbolTableError(ValueError):
    """The dynamic symbol table could not be parsed completely."""


def symbols(
    path: Path, readelf_path: str
) -> tuple[set[str], list[tuple[str, str]]]:
    defined: set[str] = set()
    undefined: list[tuple[str, str]] = []
    text = readelf(readelf_path, "--dyn-syms", "--wide", str(path))
    header = re.search(
        r"^Symbol table .+ contains (\d+) entries?:$",
        text,
        re.MULTILINE,
    )
    if not header:
        raise SymbolTableError(
            "readelf output has no dynamic symbol table header"
        )
    rows = [line for line in text.splitlines() if re.match(r"^\s*\d+:", line)]
    expected_rows = int(header.group(1))
    if len(rows) != expected_rows:
        raise SymbolTableError(
            f"readelf reported {expected_rows} dynamic symbols but returned {len(rows)} rows"  # noqa: E501
        )
    for line in rows:
        fields = line.split()
        if len(fields) < 7 or not fields[0].rstrip(":").isdigit():
            raise SymbolTableError(f"malformed dynamic symbol row: {line}")
        bind, visibility, section = fields[4], fields[5], fields[6]
        if len(fields) < 8:
            if bind in {"GLOBAL", "WEAK"}:
                raise SymbolTableError(
                    f"named dynamic symbol has no name field: {line}"
                )
            continue
        name = fields[7]
        symbol = name.split("@", 1)[0]
        if not symbol:
            raise SymbolTableError(f"dynamic symbol has an empty name: {line}")
        if section == "UND" and bind in {"GLOBAL", "WEAK"}:
            undefined.append((symbol, bind))
        elif (
            section != "UND"
            and bind in {"GLOBAL", "WEAK"}
            and visibility in {"DEFAULT", "PROTECTED"}
        ):
            defined.add(symbol)
    return defined, undefined


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apk", required=True)
    parser.add_argument("--aapt", required=True)
    parser.add_argument("--readelf", required=True)
    parser.add_argument("--sysroot", required=True)
    parser.add_argument("--api", type=int, default=34)
    parser.add_argument("--report")
    args = parser.parse_args()

    apk = Path(args.apk)
    root = Path(args.sysroot) / "usr/lib/aarch64-linux-android"
    if not apk.is_file():
        raise SystemExit(f"APK not found: {apk}")
    if not root.is_dir():
        raise SystemExit(f"NDK Android system-library stubs not found: {root}")

    badging = subprocess.check_output(
        [args.aapt, "dump", "badging", str(apk)], text=True
    )
    minimum = re.search(r"^sdkVersion:'(\d+)'", badging, re.MULTILINE)
    target = re.search(r"^targetSdkVersion:'(\d+)'", badging, re.MULTILINE)
    if not minimum or not target:
        raise SystemExit(
            "APK manifest is missing a numeric minSdkVersion or targetSdkVersion"  # noqa: E501
        )
    min_sdk, target_sdk = int(minimum.group(1)), int(target.group(1))
    if min_sdk != args.api:
        raise SystemExit(
            f"Expected APK minSdkVersion {args.api}, found {min_sdk}"
        )
    if target_sdk < min_sdk:
        raise SystemExit(
            f"APK targetSdkVersion {target_sdk} is below minSdkVersion {min_sdk}"  # noqa: E501
        )

    api_dirs = sorted(
        (int(p.name), p)
        for p in root.iterdir()
        if p.is_dir() and p.name.isdigit()
    )
    api_dir = next((p for n, p in api_dirs if n == args.api), None)
    if api_dir is None:
        raise SystemExit(f"NDK API {args.api} stubs not found under {root}")
    later_dirs = [(n, p) for n, p in api_dirs if n > args.api]

    with zipfile.ZipFile(apk) as archive:
        members = sorted(
            n
            for n in archive.namelist()
            if n.startswith("lib/arm64-v8a/") and n.endswith(".so")
        )
        if not members:
            raise SystemExit("APK contains no lib/arm64-v8a/*.so files")
        with tempfile.TemporaryDirectory(prefix="android14-audit-") as tmp:
            extracted: dict[str, Path] = {}
            for index, member in enumerate(members):
                out = Path(tmp) / f"{index}-{Path(member).name}"
                out.write_bytes(archive.read(member))
                extracted[member] = out

            info = {}
            packaged_sonames: dict[str, str] = {}
            exported: dict[str, set[str]] = {}
            undefined_counts = {}
            audit_errors = []
            for member, path in extracted.items():
                soname, needed = dynamic_info(path, args.readelf)
                try:
                    defs, undefs = symbols(path, args.readelf)
                except (
                    SymbolTableError,
                    subprocess.CalledProcessError,
                ) as error:
                    detail = str(error)
                    output = getattr(error, "output", None)
                    if output:
                        detail += f": {str(output)[:2000]}"
                    audit_errors.append({"library": member, "error": detail})
                    defs, undefs = set(), []
                key = soname or Path(member).name
                packaged_sonames[key] = member
                exported[member] = defs
                info[member] = {
                    "soname": key,
                    "needed": needed,
                    "undefined": undefs,
                }
                undefined_counts[member] = len(undefs)

            packaged_names = set(packaged_sonames)
            system_needed = sorted(
                {
                    dep
                    for item in info.values()
                    for dep in item["needed"]
                    if dep not in packaged_names
                }
            )
            stub_paths: dict[tuple[int, str], Path] = {}
            for api, directory in [(args.api, api_dir), *later_dirs]:
                for dep in system_needed:
                    candidate = directory / dep
                    if candidate.is_file():
                        stub_paths[(api, dep)] = candidate

            stub_symbols: dict[tuple[int, str], set[str]] = {}
            for key, path in stub_paths.items():
                defs, _ = symbols(path, args.readelf)
                stub_symbols[key] = defs

            newer_api_imports = []
            uncertain = []
            missing_dependencies = []
            later_api_dependencies = []
            checked_edges = []
            import_counts = {
                "provided_by_bundled_library": 0,
                "provided_by_api34_system_stub": 0,
                "found_only_in_later_system_stub": 0,
                "uncertain_or_unresolved": 0,
            }
            for member, item in info.items():
                needed = item["needed"]
                for dep in needed:
                    if dep in packaged_names:
                        checked_edges.append(
                            {
                                "library": member,
                                "dependency": dep,
                                "provider": "packaged shared library",
                            }
                        )
                    elif (args.api, dep) in stub_paths:
                        checked_edges.append(
                            {
                                "library": member,
                                "dependency": dep,
                                "provider": f"Android system API {args.api} stub",  # noqa: E501
                            }
                        )
                    else:
                        later_apis = [
                            api
                            for api, _ in later_dirs
                            if (api, dep) in stub_paths
                        ]
                        if later_apis:
                            available_from = min(later_apis)
                            later_api_dependencies.append(
                                {
                                    "library": member,
                                    "dependency": dep,
                                    "available_from_api": available_from,
                                }
                            )
                            checked_edges.append(
                                {
                                    "library": member,
                                    "dependency": dep,
                                    "provider": f"Android system library available only from API {available_from}",  # noqa: E501
                                }
                            )
                        else:
                            missing_dependencies.append(
                                {"library": member, "dependency": dep}
                            )
                            checked_edges.append(
                                {
                                    "library": member,
                                    "dependency": dep,
                                    "provider": "unknown",
                                }
                            )

                system_candidates = [
                    dep
                    for dep in needed
                    if dep not in packaged_names
                    and (args.api, dep) in stub_paths
                ]
                for symbol, binding in item["undefined"]:
                    bundled_provider = any(
                        symbol in exported[packaged_sonames[dep]]
                        for dep in needed
                        if dep in packaged_names
                    )
                    if bundled_provider:
                        import_counts["provided_by_bundled_library"] += 1
                        continue
                    api34_provider = any(
                        symbol in stub_symbols.get((args.api, dep), set())
                        for dep in system_candidates
                    )
                    if api34_provider:
                        import_counts["provided_by_api34_system_stub"] += 1
                        continue
                    later_provider = [
                        api
                        for api, _ in later_dirs
                        if any(
                            symbol in stub_symbols.get((api, dep), set())
                            for dep in system_candidates
                        )
                    ]
                    if later_provider and binding == "GLOBAL":
                        newer_api_imports.append(
                            {
                                "library": member,
                                "symbol": symbol,
                                "binding": binding,
                                "available_from_api": min(later_provider),
                            }
                        )
                        import_counts["found_only_in_later_system_stub"] += 1
                    else:
                        uncertain.append(
                            {
                                "library": member,
                                "symbol": symbol,
                                "binding": binding,
                            }
                        )
                        import_counts["uncertain_or_unresolved"] += 1

    report_data = {
        "apk": str(apk),
        "abi": "arm64-v8a",
        "minSdkVersion": min_sdk,
        "targetSdkVersion": target_sdk,
        "api_floor_audited": args.api,
        "packaged_libraries": members,
        "undefined_dynamic_symbol_counts_by_library": undefined_counts,
        "undefined_import_classification_counts": import_counts,
        "dependencies": checked_edges,
        "missing_or_unknown_dependencies": missing_dependencies,
        "system_dependencies_found_only_in_later_api_stubs": later_api_dependencies,  # noqa: E501
        "strong_imports_found_only_in_later_api_stubs": newer_api_imports,
        "symbol_audit_errors": audit_errors,
        "uncertain_or_unresolved_imports": uncertain,
        "status": (
            "fail"
            if newer_api_imports or later_api_dependencies
            else (
                "incomplete"
                if audit_errors
                else (
                    "review_required"
                    if uncertain or missing_dependencies
                    else "screened"
                )
            )
        ),
        "limitations": "NDK stub comparison is a static screen, not runtime compatibility proof.",  # noqa: E501
    }
    print(json.dumps(report_data, indent=2))
    if args.report:
        Path(args.report).write_text(
            json.dumps(report_data, indent=2) + "\n", encoding="utf-8"
        )
    if newer_api_imports or later_api_dependencies:
        print(
            "FAIL: strong imports or system dependencies require Android API newer than the configured floor.",  # noqa: E501
            file=sys.stderr,
        )
        return 1
    if audit_errors:
        print(
            "ERROR: dynamic symbol information was missing or malformed; compatibility could not be screened.",  # noqa: E501
            file=sys.stderr,
        )
        return 2
    if uncertain or missing_dependencies:
        print(
            "::warning::REVIEW REQUIRED: unresolved imports or dependencies need human review."  # noqa: E501
        )
    else:
        print(
            f"PASS: screened {len(members)} ARM64 libraries against API {args.api} NDK stubs."  # noqa: E501
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
