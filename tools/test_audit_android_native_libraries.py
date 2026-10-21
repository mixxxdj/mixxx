#!/usr/bin/env python3
"""Focused mocked tests for the Android native-library API-floor audit."""
from __future__ import annotations

import contextlib
import io
import json
import sys
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import audit_android_native_libraries as audit


BADGING = "package: name='example' versionCode='1'\nsdkVersion:'34'\ntargetSdkVersion:'34'\n"


def dynsym_output(definitions=(), imports=(), malformed=None):
    entries = ["   0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND"]
    for name in definitions:
        entries.append(
            f"   {len(entries)}: 0000000000000000     0 FUNC    GLOBAL DEFAULT  1 {name}"
        )
    for name, binding in imports:
        entries.append(
            f"   {len(entries)}: 0000000000000000     0 FUNC    {binding:<6} DEFAULT  UND {name}"
        )
    if malformed == "no-table":
        return "readelf: no dynamic symbol information"
    count = len(entries) + (1 if malformed == "truncated" else 0)
    return (
        f"Symbol table '.dynsym' contains {count} entries:\n"
        "   Num:    Value          Size Type    Bind   Vis      Ndx Name\n"
        + "\n".join(entries)
        + "\n"
    )


class FakeArchive:
    members = ["lib/arm64-v8a/libconsumer.so"]

    def __init__(self, *_args, **_kwargs):
        pass

    def __enter__(self):
        return self

    def __exit__(self, *_args):
        return False

    def namelist(self):
        return list(self.members)

    def read(self, _member):
        return b"mock ELF bytes"


class FakeTemporaryDirectory:
    def __init__(self, *_args, **_kwargs):
        self.path = Path("C:/virtual/android14-audit-tmp")

    def __enter__(self):
        return str(self.path)

    def __exit__(self, *_args):
        return False


class AuditFixture(unittest.TestCase):
    def run_fixture(
        self,
        *,
        needed=("libandroid.so",),
        api34_symbols=(),
        later_symbols=(),
        packaged_provider=False,
        imports=(("target", "GLOBAL"),),
        malformed=None,
        api34_libraries=("libandroid.so",),
        api35_libraries=("libandroid.so",),
    ):
        apk = Path("C:/virtual/fixture.apk")
        sysroot = Path("C:/virtual/sysroot")
        root = sysroot / "usr/lib/aarch64-linux-android"
        api_dirs = {34: root / "34", 35: root / "35"}
        stub_paths = {
            api_dirs[api] / library
            for api, libraries in ((34, api34_libraries), (35, api35_libraries))
            for library in libraries
        }
        members = ["lib/arm64-v8a/libconsumer.so"]
        dynamic = {
            "libconsumer.so": (["libconsumer.so"], list(needed)),
        }
        symbol_tables = {
            "libconsumer.so": dynsym_output(imports=imports, malformed=malformed),
        }
        if packaged_provider:
            members.append("lib/arm64-v8a/libprovider.so")
            dynamic["libprovider.so"] = (["libprovider.so"], [])
            symbol_tables["libprovider.so"] = dynsym_output(definitions=("target",))
        for library in set(api34_libraries) | set(api35_libraries):
            api34_defs = api34_symbols if library == "libandroid.so" else ()
            later_defs = later_symbols if library == "libandroid.so" else ()
            symbol_tables[f"34/{library}"] = dynsym_output(definitions=api34_defs)
            symbol_tables[f"35/{library}"] = dynsym_output(definitions=later_defs)
        archive = FakeArchive()
        archive.members = members
        report_files = {}

        def path_is_file(path):
            return path == apk or path in stub_paths

        def path_is_dir(path):
            return path == root or path in api_dirs.values()

        def path_iterdir(path):
            if path == root:
                return iter(api_dirs.values())
            raise AssertionError(f"Unexpected directory enumeration: {path}")

        def mock_readelf(_readelf_path, *args):
            operation = args[0]
            path = Path(args[-1])
            filename = path.name
            if filename[:1].isdigit() and "-" in filename:
                filename = filename.split("-", 1)[1]
            if operation == "--dynamic":
                sonames, dependencies = dynamic[filename]
                lines = ["Dynamic section at offset 0x0 contains entries:"]
                lines.extend(f"  (SONAME) Library soname: [{name}]" for name in sonames)
                lines.extend(f"  (NEEDED) Shared library: [{name}]" for name in dependencies)
                return "\n".join(lines)
            if operation == "--dyn-syms":
                if filename.startswith("lib") and filename not in symbol_tables:
                    api = path.parent.name
                    filename = f"{api}/{filename}"
                return symbol_tables[filename]
            raise AssertionError(f"Unexpected readelf operation: {operation}")

        argv = [
            "audit_android_native_libraries.py",
            "--apk", str(apk),
            "--aapt", "fake-aapt",
            "--readelf", "fake-readelf",
            "--sysroot", str(sysroot),
            "--api", "34",
            "--report", str(Path("C:/virtual/report.json")),
        ]
        stdout, stderr = io.StringIO(), io.StringIO()
        with (
            mock.patch.object(audit.sys, "argv", argv),
            mock.patch.object(audit.subprocess, "check_output", return_value=BADGING),
            mock.patch.object(audit, "readelf", side_effect=mock_readelf),
            mock.patch.object(audit.zipfile, "ZipFile", return_value=archive),
            mock.patch.object(audit.tempfile, "TemporaryDirectory", FakeTemporaryDirectory),
            mock.patch.object(audit.Path, "is_file", new=path_is_file),
            mock.patch.object(audit.Path, "is_dir", new=path_is_dir),
            mock.patch.object(audit.Path, "iterdir", new=path_iterdir),
            mock.patch.object(audit.Path, "write_bytes", new=lambda _path, data: len(data)),
            mock.patch.object(
                audit.Path,
                "write_text",
                new=lambda path, data, encoding=None: report_files.__setitem__(path, data),
            ),
            contextlib.redirect_stdout(stdout),
            contextlib.redirect_stderr(stderr),
        ):
            code = audit.main()
        self.assertIn(Path("C:/virtual/report.json"), report_files)
        return (
            code,
            json.loads(report_files[Path("C:/virtual/report.json")]),
            stdout.getvalue(),
            stderr.getvalue(),
        )

    def test_strong_import_present_in_api34_stub_is_screened(self):
        code, report, _stdout, _stderr = self.run_fixture(api34_symbols=("target",))
        self.assertEqual(code, 0)
        self.assertEqual(report["status"], "screened")
        self.assertEqual(report["undefined_import_classification_counts"]["provided_by_api34_system_stub"], 1)

    def test_strong_import_first_present_in_api35_stub_fails(self):
        code, report, _stdout, stderr = self.run_fixture(later_symbols=("target",))
        self.assertEqual(code, 1)
        self.assertEqual(report["status"], "fail")
        self.assertEqual(report["strong_imports_found_only_in_later_api_stubs"][0]["available_from_api"], 35)
        self.assertIn("FAIL:", stderr)

    def test_symbol_exported_by_packaged_dependency_is_resolved(self):
        code, report, _stdout, _stderr = self.run_fixture(
            needed=("libprovider.so",), packaged_provider=True
        )
        self.assertEqual(code, 0)
        self.assertEqual(report["status"], "screened")
        self.assertEqual(report["undefined_import_classification_counts"]["provided_by_bundled_library"], 1)

    def test_unresolved_external_dependency_is_review_required_not_pass(self):
        code, report, stdout, _stderr = self.run_fixture(
            needed=("libprivate.so",), api34_libraries=(), api35_libraries=()
        )
        self.assertEqual(code, 0)
        self.assertEqual(report["status"], "review_required")
        self.assertEqual(report["missing_or_unknown_dependencies"][0]["dependency"], "libprivate.so")
        self.assertEqual(report["undefined_import_classification_counts"]["uncertain_or_unresolved"], 1)
        self.assertIn("::warning::REVIEW REQUIRED", stdout)

    def test_weak_import_is_uncertain_and_does_not_fail_as_strong(self):
        code, report, _stdout, _stderr = self.run_fixture(
            imports=(("optional", "WEAK"),), later_symbols=("optional",)
        )
        self.assertEqual(code, 0)
        self.assertEqual(report["status"], "review_required")
        self.assertEqual(report["strong_imports_found_only_in_later_api_stubs"], [])
        self.assertEqual(report["uncertain_or_unresolved_imports"][0]["binding"], "WEAK")

    def test_malformed_or_missing_symbol_table_fails_closed_with_report(self):
        for malformed in ("no-table", "truncated"):
            with self.subTest(malformed=malformed):
                code, report, _stdout, stderr = self.run_fixture(
                    imports=(), malformed=malformed
                )
                self.assertEqual(code, 2)
                self.assertEqual(report["status"], "incomplete")
                self.assertEqual(report["symbol_audit_errors"][0]["library"], "lib/arm64-v8a/libconsumer.so")
                self.assertIn("ERROR:", stderr)

    def test_system_library_available_only_from_api35_fails(self):
        code, report, _stdout, stderr = self.run_fixture(
            needed=("libnewapi.so",),
            api34_libraries=(),
            api35_libraries=("libnewapi.so",),
        )
        self.assertEqual(code, 1)
        self.assertEqual(report["status"], "fail")
        self.assertEqual(
            report["system_dependencies_found_only_in_later_api_stubs"][0]["available_from_api"],
            35,
        )
        self.assertIn("FAIL:", stderr)


if __name__ == "__main__":
    unittest.main()
