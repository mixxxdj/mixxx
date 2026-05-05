---
id: BNG-19
title: "Normalize Mixxx CMake discovery to Bungee::Bungee"
status: done
priority: high
effort: medium
group: BNG
package: root
labels: [bungee, cmake, find-package]
blocked_by: [BNG-18]
created: '2026-05-05'
completed: '2026-05-05'
---

## BNG-19: Normalize Mixxx CMake discovery to Bungee::Bungee

## Context

Mixxx currently builds Bungee directly from `lib/bungee/`. The target shape preferred by the plan is that Mixxx code links only to a normalized `Bungee::Bungee` target, regardless of whether the provider is upstream Bungee config, vcpkg's `unofficial-bungee`, pkg-config, or an approved fallback.

## Scope

- Update `cmake/modules/FindBungee.cmake` and/or top-level CMake glue to try dependency discovery in this order:
  1. `find_package(Bungee CONFIG QUIET)`
  2. `find_package(unofficial-bungee CONFIG QUIET)` from vcpkg PR #50120, wrapped as `Bungee::Bungee`
  3. module/pkg-config discovery that also creates `Bungee::Bungee`
- Preserve the user-facing `option(BUNGEE "Enable the Bungee engine for pitch-bending" ON)`.
- Make `-DBUNGEE=OFF` build exactly as before.
- Produce a clear configure error or fallback handoff when `BUNGEE=ON` but no dependency is found.
- Keep the vendored build path in place only if needed as a temporary compatibility step for this ticket; removal is BNG-22.

## Acceptance criteria

- Mixxx-side scaler code links through `Bungee::Bungee`, not provider-specific targets.
- vcpkg's `unofficial::bungee::bungee` can be wrapped without leaking that name across Mixxx CMake.
- System/package/pkg-config discovery remains possible.
- `BUNGEE=OFF` configures and builds cleanly.
- No source-tree patching or mutation is introduced.
