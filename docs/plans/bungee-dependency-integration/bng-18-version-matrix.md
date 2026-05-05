# BNG-18 — Bungee Version / CMake Compatibility Matrix

## Key finding: No older-CMake-compatible Bungee release exists

Every Bungee release, from **v2.1.8 through v2.4.24** (the current latest, May 2026), declares:

```cmake
cmake_minimum_required(VERSION 3.30...3.31)
```

There is **no Bungee version with a lower CMake requirement**. The original plan assumption — "use an older Bungee for builders with CMake < 3.30" — is not viable.

---

## Version inventory

| Tag | CMake req | Mixxx API compat | vcpkg port |
| --- | --- | --- | --- |
| v2.4.24 (latest, 2026-05-03) | 3.30...3.31 | ✓ identical API | No — port not yet updated |
| v2.4.23 | 3.30...3.31 | ✓ | No |
| v2.4.15 (our current vendor) | 3.30...3.31 | ✓ (our baseline) | ✓ microsoft/vcpkg (merged 2026-03-19) |
| v2.3.x through v2.1.8 | 3.30...3.31 | likely ✓ | No |

### API stability: v2.4.15 → v2.4.24

`bungee/Bungee.h` and `bungee/Modes.h` are **byte-for-byte identical** between v2.4.15 and v2.4.24.
The public structs we use (`Request`, `InputChunk`, `Functions`, `Edition`) are unchanged.
Updating from v2.4.15 to v2.4.24 is safe for the Mixxx scaler integration.

---

## Upstream CMakeLists.txt is not install-capable by itself

The upstream Bungee CMakeLists.txt does **not** produce a `BungeeConfig.cmake` or proper
exported targets. The vcpkg port's `cmake-use-vcpkg-deps-and-install-layout.patch` adds:

- `find_package(Eigen3 CONFIG REQUIRED)`
- `find_package(pffft CONFIG REQUIRED)` (replaces vendored submodule)
- `install(TARGETS bungee_library EXPORT unofficial-bungee-targets ...)`
- Removal of the CLI executable and vendored pffft from the build
- Proper `GNUInstallDirs` paths

This means any non-vcpkg `ExternalProject_Add` fallback must apply the same or equivalent patches
to make Bungee installable and discoverable via `find_package`.

---

## What does CMake 3.30 actually provide that Bungee uses?

Looking at the upstream CMakeLists.txt, the **actual CMake features used** are all compatible with
CMake 3.14+:

- `cmake_minimum_required(VERSION x...y)` — policy range syntax requires CMake ≥ 3.12
- `find_package(Eigen3 CONFIG)` — CMake 3.1+
- `find_package(pffft CONFIG)` — CMake 3.1+
- `target_link_libraries(... PRIVATE ...)` — CMake 3.1+
- `GNUInstallDirs` — CMake 3.1+
- `install(EXPORT ...)` — CMake 3.1+

The `3.30...3.31` declaration is a **policy-setting choice by Bungee maintainers**, not a hard
technical requirement. CMake uses the range to enable specific policies. The code does not use any
CMake 3.30-specific commands.

---

## vcpkg buildenv status

| Component | Status |
| --- | --- |
| microsoft/vcpkg Bungee port | ✓ Merged 2026-03-19 at commit ae92331, version 2.4.15 |
| mixxxdj/vcpkg default branch | `2.6`, forked from microsoft/vcpkg, **477 commits behind** microsoft/vcpkg:master |
| mixxxdj/vcpkg Bungee port | **Not present** — Bungee port has not been backported/cherry-picked |
| Mixxx buildenv artifacts | Pre-built archives named `mixxx-deps-2.6-{triplet}-{hash}`, updated via `tools/update_vcpkg.py` |

This means Windows/macOS CI **cannot yet discover Bungee via vcpkg** without first adding the port
to `mixxxdj/vcpkg`. This is out-of-repo work (a PR against mixxxdj/vcpkg + artifact rebuild).

---

## Decision needed: CMake < 3.30 and ExternalProject fallback

Since the older-Bungee-for-older-CMake path does not exist, we need to decide what happens when
a builder has CMake < 3.30 and no system Bungee package.

---

### Decision needed: ExternalProject fallback behavior for CMake < 3.30

### Option A — Patch `cmake_minimum_required` in the ExternalProject build

Apply a trivial 1-line patch that changes `3.30...3.31` to `3.21` (or `3.14`) in Bungee's
CMakeLists.txt, in addition to the install-layout patch the vcpkg port already applies.

Pros:

- Enables source builds for any CMake ≥ 3.21 builder (the default on Ubuntu 22.04 is ~3.22)
- The actual CMake features Bungee uses are all compatible with CMake 3.21
- Low mechanical risk: one-line patch

Cons:

- Overrides Bungee maintainer's stated minimum; could silently break if Bungee ever adds a 3.30-only CMake feature
- Adds a patch to maintain alongside the install-layout patch
- Inconsistent with our recommendation to use CMake 3.30+ for Bungee builds

### Option B — Require CMake ≥ 3.30 for ExternalProject fallback; fail clearly if not met

If `BUNGEE=ON`, no package is found, and CMake < 3.30: emit an actionable configure error.
If CMake ≥ 3.30: allow the ExternalProject fallback to proceed normally.

Pros:

- Consistent with recommending CMake 3.30+ for Bungee-enabled builds
- No custom patch of Bungee's stated requirement
- Clear, actionable error message for users with older CMake
- Users with CMake 3.21–3.29 can upgrade CMake (Kitware PPA on Ubuntu, brew on macOS)

Cons:

- Ubuntu 22.04 LTS users get a configure error unless they install a newer CMake

### Option C — No ExternalProject fallback at all; require vcpkg or system package

If no package is found, configure fails regardless of CMake version.

Pros:

- Simplest CMake code; no fallback to implement or maintain
- Pushes towards proper package management

Cons:

- Linux developers without a Bungee system package must wait for distro packaging or do a manual install
- Weakens the "developer just clones and builds" experience that ExternalProject provides

### Recommendation: Option B

Option B is consistent with the user's stated direction ("we can increase the recommended CMake version for the most recent Bungee"), respects Bungee's stated requirement, and requires no extra patches. A clear configure message pointing users to upgrade CMake or install a package is more maintainable than a patched minimum.

If ExternalProject fallback is not approved by Mixxx maintainers (Option C), the decision still defaults to a clean error.

**Impact if deferred:** BNG-21 is blocked. BNG-19 (CMake discovery) and BNG-20 (vcpkg port) can proceed in parallel.

---

## Recommended target version for the PR

**v2.4.24** (latest, 2026-05-03) — API-compatible, latest bug fixes.

The vcpkg port in microsoft/vcpkg currently pins v2.4.15. Updating the port to v2.4.24 is a
separate concern for the vcpkg PR (BNG-20); Mixxx's ExternalProject fallback (BNG-21) can pin
v2.4.24 directly.
