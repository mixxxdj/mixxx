#!/usr/bin/env python3
"""
Small qmlformat wrapper that finds the Qt tool and warns if it is missing.

Qt's QML tools are usually not on PATH, so besides PATH this also probes
the Qt that the CMake build directory was configured with (``build/`` or
``$MIXXX_BUILD_DIR``) and the usual distribution locations.

All arguments are passed through to qmlformat unchanged.
"""

import os
import pathlib
import re
import shutil
import subprocess
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent

TOOL_MISSING_MESSAGE = """
{tool} is not installed or not in your PATH, please install it.
It is part of the Qt 6 "qtdeclarative" module (package names differ:
qt6-declarative-dev, qt6-qtdeclarative-devel, qt6-declarative, ...).
If you built Mixxx already, the Qt of that build is used automatically
when the build directory is `build/` or `$MIXXX_BUILD_DIR`.
"""

# Directories that distributions use for the Qt 6 host tools.
DISTRO_TOOL_DIRS = (
    "/usr/lib/qt6/bin",
    "/usr/lib64/qt6/bin",
    "/usr/lib/x86_64-linux-gnu/qt6/bin",
    "/usr/lib/aarch64-linux-gnu/qt6/bin",
    "/usr/lib/qt6/libexec",
    "/usr/lib64/qt6/libexec",
    "/opt/homebrew/opt/qt/bin",
    "/usr/local/opt/qt/bin",
)


def find_build_dir():
    """Return the configured CMake build directory, or None."""
    explicit = os.environ.get("MIXXX_BUILD_DIR")
    if explicit:
        path = pathlib.Path(explicit)
        return path if (path / "CMakeCache.txt").is_file() else None

    candidates = []
    for pattern in ("build", "build*", "cmake_build*", "cmake-build*"):
        for path in REPO_ROOT.glob(pattern):
            cache = path / "CMakeCache.txt"
            if cache.is_file():
                candidates.append((cache.stat().st_mtime, path))
    if not candidates:
        return None
    # Prefer the most recently configured build.
    return max(candidates)[1]


def qt_prefixes_from_cmake_cache(build_dir):
    """Yield Qt install prefixes recorded in a CMakeCache.txt."""
    cache = build_dir / "CMakeCache.txt"
    try:
        text = cache.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return
    for match in re.finditer(
        r"^(?:Qt6|Qt6Core|Qt6QmlTools|Qt6Qml)_DIR:PATH=(.+)$", text, re.M
    ):
        cmake_dir = pathlib.Path(match.group(1).strip())
        # <prefix>/lib/cmake/Qt6 (Qt installer, most distros) or
        # <prefix>/share/Qt6 (vcpkg, which is what the Mixxx deps bundle uses).
        yield cmake_dir.parent.parent
        yield cmake_dir.parent.parent.parent


def candidate_tool_dirs():
    for var in ("QTDIR", "QT_ROOT_DIR", "QT_HOST_PATH"):
        value = os.environ.get(var)
        if value:
            yield os.path.join(value, "bin")
            yield os.path.join(value, "libexec")

    build_dir = find_build_dir()
    if build_dir is not None:
        for prefix in qt_prefixes_from_cmake_cache(build_dir):
            yield str(prefix / "bin")
            yield str(prefix / "libexec")
            # vcpkg keeps the host tools out of bin/.
            yield str(prefix / "tools" / "Qt6" / "bin")

    if sys.platform != "win32":
        for path in DISTRO_TOOL_DIRS:
            yield path


def find_qt_tool(name):
    """Find a Qt 6 host tool such as qmlformat or qmllint.

    Returns the path to the executable, or None.
    """
    names = (name, name + "-qt6", name + "6")
    # PATH first, so an explicit choice by the user always wins.
    for candidate in names:
        found = shutil.which(candidate)
        if found:
            return found
    for directory in candidate_tool_dirs():
        for candidate in names:
            found = shutil.which(candidate, path=directory)
            if found:
                return found
    return None


def require_qt_tool(name):
    executable = find_qt_tool(name)
    if not executable:
        print(TOOL_MISSING_MESSAGE.format(tool=name).strip(), file=sys.stderr)
        sys.exit(1)
    return executable


def run_qt_tool(command):
    """Run a Qt tool and return its exit code.

    The standard streams are handed over explicitly: on Windows, Qt shows
    usage and version output in a message box instead of writing it to
    stdout when the parent did not pass its standard handles along.
    """
    streams = {}
    for name in ("stdin", "stdout", "stderr"):
        stream = getattr(sys, name)
        try:
            stream.fileno()
        except (AttributeError, OSError, ValueError):
            continue
        streams[name] = stream
    return subprocess.call(command, **streams)


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]
    executable = require_qt_tool("qmlformat")
    return run_qt_tool([executable] + list(argv))


if __name__ == "__main__":
    sys.exit(main())
