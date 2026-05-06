---
id: BNG-22
title: "Remove vendored Bungee source and direct-build wiring"
status: done
priority: high
effort: medium
group: BNG
package: root
labels: [bungee, vendor-removal, cmake]
blocked_by: [BNG-21, BNG-26]
created: '2026-05-05'
completed: '2026-05-06'
---

## BNG-22: Remove vendored Bungee source and direct-build wiring

## Context

The current branch still contains `lib/bungee/` and top-level CMake code that directly compiles Bungee, PFFFT, and Eigen submodule sources. This is the main shape that is unlikely to match Mixxx maintainer preferences.

Vendored `lib/bungee/` is currently the only Bungee provider that requires no system Eigen3/pffft. It cannot be deleted until BNG-26 wires the BNG-25 Eigen3 + pffft ExternalProjects into `bungee_external` and removes the fallthrough, otherwise stock Linux dev boxes (no system Eigen3, no system pffft) will regress from "configures clean and builds" to "configure fails." BNG-22 must therefore land *after* BNG-26, even though the work itself (delete vendored tree, remove the now-unreachable fallback CMake block) is mechanical.

BNG-22's commit message should also surface the new GNU `patch` build-time requirement (see BNG-23 umbrella for context) so distro packagers and Mixxx wiki maintainers can update prerequisites lists.

## Scope

- [x] Remove direct `add_library(bungee-pffft ...)` and `add_library(bungee ...)` CMake wiring.
- [x] Remove vendored include paths for `lib/bungee`, `lib/bungee/bungee`, and `lib/bungee/submodules/eigen`.
- [x] Remove configure-time `git apply` patching against `lib/bungee` (the MSVC-compatibility patch block).
- [x] Delete `lib/bungee/` from the repository (`git rm -r lib/bungee/`).
- [x] Delete the `# 5. Vendored fallback` block in `CMakeLists.txt` (post-BNG-21 file, locate via grep for `Bungee: no package found, using vendored lib/bungee/ as fallback`).
- [x] Delete the `add_library(Bungee::Bungee ALIAS bungee)` alias line in the vendored branch.
- [x] Update the discovery-order comment from "5. Vendored lib/bungee/ fallback (temporary — removed in BNG-22)" to drop step 5.
- [x] Keep Mixxx-owned scaler code, tests, docs, and CI.
- [x] Update documentation references that still describe vendored Bungee as the intended integration shape.

## Acceptance criteria

- No checked-in Bungee/Eigen/PFFFT source remains under `lib/bungee/`.
- Configuring/building Mixxx does not dirty the source tree.
- `BUNGEE=ON` uses package/vcpkg/ExternalProject discovery only (no vendored path).
- `BUNGEE=OFF` builds cleanly.
- QML and non-QML builds link consistently.
- On a host with no system Eigen3 and no system pffft, `cmake -DBUNGEE=ON ..` still configures and builds (proves BNG-23 plumbing is exercised, not just present).

## Completion notes

- Removed tracked `lib/bungee/` vendor sources and cleaned up the leftover nested `.git` directory from the working tree.
- Deleted the dead vendored CMake fallback branch, including `bungee-pffft`, direct `bungee`, the MSVC `git apply` patch block, vendored include paths, and the `Bungee::Bungee ALIAS bungee` normalization.
- Dropped discovery-order step 5 and updated `docs/bungee-integration.md` to describe dependency-provider/source-fetch patching instead of a checked-in vendor tree.
- Validation: source-fetch configure with local Bungee discovery disabled, `bungee_external`, `mixxx-lib`, `BUNGEE=OFF`, and packaged-only `BUNGEE_FETCH_FALLBACK=OFF` configure all passed.
