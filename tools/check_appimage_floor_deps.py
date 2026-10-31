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
satisfy, on any DT_NEEDED library the host cannot provide, and on any
definition that the loader could not actually bind to (non-exported symbols,
non-default versions).

Behaviour beyond the symbol level is covered separately by running the
AppImage on the same 22.04 runner (the existing smoke test) and by the test
suite, which executes against the built binary on that floor.

Known approximations: mandatory version nodes are validated against the
specific provider DT_VERNEED attributes them to, but a versioned symbol
reference is otherwise resolved against the union of symbols and versions
exported across the closure; the closure's libraries own disjoint symbol
version namespaces, so both models agree on its data.  Loader search order,
symbol interposition and dlopen-delegated libraries are not modelled;
failures in that territory surface in the smoke test, which runs the real
loader on the same floor.

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


def elf_info(path):
    """One readelf pass over an ELF, returning its dynamic-linking facts.

    Returns (undefined, defined, default_defs, needed, version_needs,
    version_defs):

    - undefined, defined: {name: set(versions)} from .dynsym (None =
      unversioned); defined holds only externally visible definitions
      (GLOBAL/WEAK/UNIQUE, DEFAULT/PROTECTED).
    - default_defs: names with an unversioned or default-version ('@@')
      definition, which is what an unversioned reference may bind to.
    - needed: DT_NEEDED sonames.
    - version_needs: {soname: set(nodes)} from .gnu.version_r, keeping only
      mandatory records (without VER_FLG_WEAK).  A version requirement is
      mandatory at load time even when the referencing symbol is weak.
    - version_defs: set of nodes this ELF defines in .gnu.version_d.
    """
    out = subprocess.run(
        ["readelf", "--dyn-syms", "-d", "-V", "-W", path],
        capture_output=True,
        text=True,
        check=True,
        env={"LC_ALL": "C"},
    ).stdout
    undefined = collections.defaultdict(set)
    defined = collections.defaultdict(set)
    default_defs = set()
    needed = set()
    version_needs = collections.defaultdict(set)
    version_defs = set()
    section = None
    need_file = None
    for line in out.splitlines():
        if line.startswith("Dynamic section"):
            section = "dynamic"
        elif line.startswith("Version symbols section"):
            section = "versym"  # version indexes, irrelevant here
        elif line.startswith("Version definition section"):
            section = "verdef"
        elif line.startswith("Version needs section"):
            section = "verneed"
        elif line.startswith("Symbol table"):
            section = "dynsym"
        elif section == "dynamic":
            m = re.search(r"\(NEEDED\)\s+Shared library: \[([^\]]+)\]", line)
            if m:
                needed.add(m.group(1))
        elif section == "dynsym":
            parts = line.split()
            if len(parts) < 8:
                continue
            # readelf may render STB_GNU_UNIQUE as one token ("UNIQUE") or
            # two ("GNU UNIQUE"); normalize so the columns stay stable.
            if parts[4] == "GNU" and parts[5] == "UNIQUE":
                parts = parts[:4] + ["UNIQUE"] + parts[6:]
                if len(parts) < 8:
                    continue
            raw = parts[7]
            if not raw:
                continue
            if parts[6] == "UND":
                # Weak undefined symbols are optional at the symbol level:
                # the loader tolerates a missing provider.  Their version
                # requirement is still mandatory and checked via
                # .gnu.version_r.
                if parts[4] != "GLOBAL":
                    continue
                name, version = split_version(raw)
                if name:
                    undefined[name].add(version)
            else:
                if raw.startswith("."):
                    continue
                # Only externally visible definitions can satisfy another
                # ELF's undefined reference; LOCAL / HIDDEN / INTERNAL ones
                # cannot.
                if parts[4] not in ("GLOBAL", "WEAK", "UNIQUE"):
                    continue
                if parts[5] in ("HIDDEN", "INTERNAL"):
                    continue
                name, version = split_version(raw)
                defined[name].add(version)
                # An unversioned reference binds to the default version or
                # to an unversioned definition; a non-default versioned
                # definition alone cannot satisfy it.
                if "@@" in raw or version is None:
                    default_defs.add(name)
        elif section == "verdef":
            m = re.search(
                r"Flags: (\S+)\s+Index: \d+\s+Cnt: \d+\s+Name: (\S+)", line
            )
            if m and m.group(1).lower() != "base":
                version_defs.add(m.group(2))
        elif section == "verneed":
            m = re.search(r"File: (\S+)\s+Cnt:", line)
            if m:
                need_file = m.group(1)
                continue
            m = re.search(r"Name: (\S+)\s+Flags: (\S+)", line)
            if (
                m
                and need_file is not None
                and "weak" not in m.group(2).lower()
            ):
                version_needs[need_file].add(m.group(1))
    return (
        undefined,
        defined,
        default_defs,
        needed,
        version_needs,
        version_defs,
    )


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
        [ldconfig, "-p"],
        capture_output=True,
        text=True,
        check=False,
        env={"LC_ALL": "C"},
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
    """DT_NEEDED is resolved against the file name / SONAME exactly.

    The loader never accepts a versioned suffix for an unversioned needed
    name, so a prefix heuristic would wrongly treat libjack.so.0 as
    satisfying a DT_NEEDED of libjack.so and let an AppImage that cannot
    even start pass the check.
    """
    return entry == soname


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
    defaultable = set()  # names an unversioned reference may bind to
    missing_libs = []
    resolved = {}  # soname -> provider path
    provider_defs = {}  # path -> set of version nodes it defines
    version_needs = []  # (soname, node) mandatory version requirements

    while worklist:
        elf = worklist.pop()
        if elf in processed:
            continue
        processed.add(elf)

        u, d, default_defs, needed, vneeds, vdefs = elf_info(elf)
        for name, versions in u.items():
            undef[name].update(versions)
        for name, versions in d.items():
            resolvable[name].update(versions)
        defaultable.update(default_defs)
        provider_defs[elf] = vdefs
        for soname, nodes in vneeds.items():
            for node in nodes:
                version_needs.append((soname, node))

        for soname in needed:
            # bundled/AppImage lib dir first, then the host system
            path = find_in_dir(soname, libdir, want_bits)
            if path is None:
                path = find_system_soname(soname, sysmap, want_bits)
            if path is None or not os.path.isfile(path):
                missing_libs.append(soname)
                continue
            resolved[soname] = path
            worklist.append(path)

    # Symbol-version aware resolution.
    unresolved = []
    for name in sorted(undef):
        wanted = undef[name]
        # defaultdict.get() returns None for a missing key, which would crash
        # the versioned check below; index instead so a missing name yields
        # the empty set.
        have = resolvable[name]
        # An unversioned reference binds to the default version or an
        # unversioned definition; a versioned one needs the exact version.
        # Check every reference independently so a same-name unversioned
        # reference cannot mask a versioned mismatch.
        for ver in wanted:
            if ver is None:
                if name not in defaultable:
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
        # A DT_NEEDED library absent from both the AppImage and the host
        # cannot load on the floor: the packaged binary would fail at
        # startup.  The CI host is Ubuntu 22.04, the modeled floor.
        print("\nFAIL: required library missing on the Ubuntu 22.04 floor:")
        for lib in sorted(set(missing_libs)):
            print(f"  {lib}")
        return 1

    # Mandatory version requirements (.gnu.version_r without VER_FLG_WEAK):
    # each required node must be defined by the library the requirement is
    # attributed to (DT_VERNEED).  The symbol level above already checks
    # versioned references, but a weak symbol's version requirement is
    # mandatory at load time even though the symbol itself is optional.
    bad_nodes = []
    for soname, node in version_needs:
        provider = resolved.get(soname)
        if provider is None:
            # The missing library is reported separately above.
            continue
        if node not in provider_defs.get(provider, set()):
            bad_nodes.append((soname, node))
    if bad_nodes:
        print(
            "\nFAIL: required version node missing from its provider on "
            "the Ubuntu 22.04 floor:"
        )
        for soname, node in sorted(bad_nodes):
            print(f"  {node} (from {soname})")
        return 1

    print(
        "OK: every referenced symbol, version node and library is "
        "satisfiable."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
