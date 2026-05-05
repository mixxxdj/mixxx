---
id: BNG-22
title: "Remove vendored Bungee source and direct-build wiring"
status: open
priority: high
effort: medium
group: BNG
package: root
labels: [bungee, vendor-removal, cmake]
blocked_by: [BNG-19, BNG-20]
created: '2026-05-05'
---

## BNG-22: Remove vendored Bungee source and direct-build wiring

## Context

The current branch still contains `lib/bungee/` and top-level CMake code that directly compiles Bungee, PFFFT, and Eigen submodule sources. This is the main shape that is unlikely to match Mixxx maintainer preferences.

## Scope

- Remove direct `add_library(bungee-pffft ...)` and `add_library(bungee ...)` CMake wiring.
- Remove vendored include paths for `lib/bungee`, `lib/bungee/bungee`, and `lib/bungee/submodules/eigen`.
- Remove configure-time `git apply` patching against `lib/bungee`.
- Delete `lib/bungee/` from the repository once package/fallback discovery is working.
- Keep Mixxx-owned scaler code, tests, docs, and CI.
- Update documentation references that still describe vendored Bungee as the intended integration shape.

## Acceptance criteria

- No checked-in Bungee/Eigen/PFFFT source remains under `lib/bungee/`.
- Configuring/building Mixxx does not dirty the source tree.
- `BUNGEE=ON` uses package/vcpkg/fallback discovery only.
- `BUNGEE=OFF` builds cleanly.
- QML and non-QML builds link consistently.
