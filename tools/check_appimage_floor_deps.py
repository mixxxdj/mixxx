#!/usr/bin/env python3
"""Verify the AppImage only uses symbols the Ubuntu 22.04 floor provides.

The AppImage delegates fontconfig, freetype, harfbuzz, alsa and the rest of
the base system stack to the host, so at runtime those libraries are whatever
the target system ships.  Compiling against newer vcpkg headers can introduce
references that the floor libraries do not provide, which would crash at
startup (missing symbol) or at first call (wrong symbol version).  This check
walks every ELF the AppImage ships, collects the undefined symbols with their
version tags, resolves the dynamic-linking closure (the AppImage's own
libraries plus the system libraries of the build host, which is Ubuntu 22.04
in CI), and fails on any symbol or symbol version that nothing present can
satisfy.

Behaviour beyond the symbol level is covered separately by running the
AppImage on the same 22.04 runner (the existing smoke test) and by the test
suite, which executes against the built binary on that floor.

Invoked as a CI step in the AppImage jobs (like the smoke test) against the
packaged AppImage; the build host in CI is Ubuntu 22.04, which is the floor
the AppImage targets.

Usage:
  check_appimage_floor_deps.py <AppImage>  # CI: check the packaged AppImage
  check_appimage_floor_deps.py <mixxx-binary> <lib-dir>
      # testing: walk a given closure
"""

import collections
import os
import re
import shutil
import subprocess
import sys
import tempfile


def split_version(raw):
    """Split a dynsym name into (name, version); None = unversioned.

    readelf renders versioned symbols as name@VERSION and the default
    version as name@@VERSION, so strip any leading '@' from the version.
    """
    if "@" not in raw:
        return raw, None
    name, _, version = raw.rpartition("@")
    name = name.rstrip("@")
    return name, version


def dynsyms(path):
    """Return (undefined, defined) as {name: set(versions)}; None if
    unversioned."""
    out = subprocess.run(
        ["readelf", "--dyn-syms", "-W", path],
        capture_output=True,
        text=True,
        check=False,
    ).stdout
    undefined = collections.defaultdict(set)
    defined = collections.defaultdict(set)
    for line in out.splitlines():
        parts = line.split()
        if len(parts) < 8:
            continue
        raw = parts[7]
        if not raw:
            continue
        if parts[6] == "UND":
            # Weak undefined symbols are optional: the loader tolerates a
            # missing provider, so they must not fail the floor check.
            if parts[4] != "GLOBAL":
                continue
            name, version = split_version(raw)
            if name:
                undefined[name].add(version)
        else:
            if raw.startswith("."):
                continue
            name, version = split_version(raw)
            defined[name].add(version)
    return undefined, defined


def needed_sonames(path):
    out = subprocess.run(
        ["readelf", "-d", "-W", path],
        capture_output=True,
        text=True,
        check=False,
    ).stdout
    result = set()
    for line in out.splitlines():
        m = re.search(r"\(NEEDED\)\s+Shared library: \[([^\]]+)\]", line)
        if m:
            result.add(m.group(1))
    return result


def elf_bits(path):
    """ELF class of a file: 1 (32-bit), 2 (64-bit), or None if not an ELF."""
    try:
        with open(path, "rb") as f:
            data = f.read(6)
        if data[:4] != b"\x7fELF":
            return None
        return data[4]  # e_ident[EI_CLASS]
    except OSError:
        return None


def ldconfig_map():
    """soname -> path, from the host's dynamic linker cache."""
    ldconfig = shutil.which("ldconfig")
    if ldconfig is None:
        for cand in ("/sbin/ldconfig", "/usr/sbin/ldconfig"):
            if os.path.isfile(cand):
                ldconfig = cand
                break
    if ldconfig is None:
        return {}
    out = subprocess.run(
        [ldconfig, "-p"], capture_output=True, text=True, check=False
    ).stdout
    mapping = {}
    for line in out.splitlines():
        if "=>" not in line:
            continue
        parts = line.split()
        if len(parts) >= 2:
            mapping[parts[0]] = parts[-1]
    return mapping


_SYSTEM_SEARCH_ROOTS = ("/lib", "/usr/lib")


def _matches_soname(entry, soname):
    return entry == soname or entry.startswith(soname + ".")


def find_system_soname(soname, sysmap, want_bits):
    """Resolve a soname from the host system, via the cache or a file scan.

    Multilib hosts carry the same soname in 32-bit and 64-bit trees (e.g.
    /lib32 vs /lib/x86_64-linux-gnu); only accept a candidate whose ELF
    class matches the binary being checked.
    """
    if soname in sysmap:
        path = sysmap[soname]
        if want_bits is None or elf_bits(path) == want_bits:
            return path
    for base in _SYSTEM_SEARCH_ROOTS:
        if not os.path.isdir(base):
            continue
        for root, _dirs, files in os.walk(base):
            for entry in files:
                path = os.path.join(root, entry)
                if not _matches_soname(entry, soname):
                    continue
                bits = elf_bits(path)
                if bits is not None and (
                    want_bits is None or bits == want_bits
                ):
                    return path
    return None


def find_in_dir(soname, libdir, want_bits):
    """Find the ELF matching a soname in libdir."""
    for entry in os.listdir(libdir):
        if not _matches_soname(entry, soname):
            continue
        path = os.path.join(libdir, entry)
        bits = elf_bits(path)
        if bits is not None and (want_bits is None or bits == want_bits):
            return path
    return None


def extract_appimage(appimage):
    """Self-extract an AppImage into a temp dir, return (binary, libdir)."""
    # The runner passes a workspace-relative path, which would not resolve
    # in the extraction cwd; resolve it against the script's cwd first.
    appimage = os.path.abspath(appimage)
    tmp = tempfile.mkdtemp(prefix="floor-deps-")
    try:
        subprocess.run(
            [appimage, "--appimage-extract"],
            cwd=tmp,
            capture_output=True,
            text=True,
            check=True,
        )
        root = os.path.join(tmp, "squashfs-root")
        binary = os.path.join(root, "bin", "mixxx")
        libdir = os.path.join(root, "lib")
        if not os.path.isfile(binary) or not os.path.isdir(libdir):
            raise RuntimeError(f"unexpected AppImage layout in {root}")
        return binary, libdir, tmp
    except Exception as e:
        shutil.rmtree(tmp, ignore_errors=True)
        raise SystemExit(f"cannot extract AppImage {appimage}: {e}")


def main():
    if len(sys.argv) not in (2, 3):
        print(__doc__, file=sys.stderr)
        return 2
    if len(sys.argv) == 2:
        appimage = sys.argv[1]
        if not os.path.isfile(appimage):
            print(f"ERROR: AppImage not found: {appimage}", file=sys.stderr)
            return 2
        binary, libdir, tmp = extract_appimage(appimage)
        try:
            return _run_check(binary, libdir)
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
    binary = sys.argv[1]
    libdir = sys.argv[2]
    if not os.path.isfile(binary):
        print(f"ERROR: binary not found: {binary}", file=sys.stderr)
        return 2
    if not os.path.isdir(libdir):
        print(f"ERROR: buildenv lib dir not found: {libdir}", file=sys.stderr)
        return 2
    return _run_check(binary, libdir)


def _run_check(binary, libdir):
    sysmap = ldconfig_map()
    want_bits = elf_bits(binary)

    # Dynamic-linking closure: start from the binary, follow DT_NEEDED.
    worklist = [binary]
    processed = set()
    undef = collections.defaultdict(set)  # name -> set(versions)
    resolvable = collections.defaultdict(set)  # name -> set(versions)
    missing_libs = []

    while worklist:
        elf = worklist.pop()
        if elf in processed:
            continue
        processed.add(elf)

        u, d = dynsyms(elf)
        for name, versions in u.items():
            undef[name].update(versions)
        for name, versions in d.items():
            resolvable[name].update(versions)

        for soname in needed_sonames(elf):
            # bundled/AppImage lib dir first, then the host system
            path = find_in_dir(soname, libdir, want_bits)
            if path is None:
                path = find_system_soname(soname, sysmap, want_bits)
            if path is None or not os.path.isfile(path):
                missing_libs.append(soname)
                continue
            worklist.append(path)

    # Symbol-version aware resolution.
    unresolved = []
    for name in sorted(undef):
        wanted = undef[name]
        # defaultdict.get() returns None for a missing key, which would crash
        # the versioned check below; index instead so a missing name yields
        # the empty set.
        have = resolvable[name]
        # An unversioned reference needs the name at all; a versioned one
        # needs the exact version.  Check every reference independently so a
        # same-name unversioned reference cannot mask a versioned mismatch.
        for ver in wanted:
            if ver is None:
                if not have:
                    unresolved.append((name, "any"))
            elif ver not in have:
                unresolved.append((name, ver))

    print(f"ELFs walked: {len(processed)}")
    print(f"Undefined symbols: {len(undef)}")
    print(f"Missing system libs: {len(missing_libs)}")
    for lib in sorted(set(missing_libs)):
        print(f"  MISSING LIB: {lib}")

    if unresolved:
        print("\nUNRESOLVED on the Ubuntu 22.04 floor:")
        for name, ver in unresolved[:50]:
            print(f"  {name}@{ver}")
        print(
            f"\nFAIL: {len(unresolved)} symbol(s) not satisfiable by the "
            "floor."
        )
        return 1

    if missing_libs:
        print(
            "\nWARNING: needed libraries absent from the build host (would "
            "be delegated at runtime)."
        )
    print("OK: every referenced symbol is satisfiable by the floor.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
