---
id: BNG-23
title: "Update packaging and CI paths for non-vendored Bungee (umbrella)"
status: done
priority: high
effort: large
group: BNG
package: root
labels: [bungee, packaging, ci, umbrella]
blocked_by: [BNG-25, BNG-26, BNG-27, BNG-28]
created: '2026-05-05'
completed: '2026-05-06'
---

## BNG-23: Umbrella ticket — packaging + CI for non-vendored Bungee

## Status

This is an **umbrella ticket**. The implementation work has been split into four reviewable child tickets so each lands as a focused PR:

| Ticket | Surface | Blocks |
| --- | --- | --- |
| **BNG-25** | Add `eigen3_external` + `pffft_external` ExternalProjects | BNG-26 |
| **BNG-26** | Wire them into `bungee_external`; remove fallthrough | BNG-22, BNG-27, BNG-28 |
| **BNG-27** | Add Bungee/Eigen3/pffft as Flatpak modules | — |
| **BNG-28** | Update GitHub Actions workflows | — |
| BNG-29 | (deferred) Reorganize patches to `cmake/patches/` | — |

BNG-23 closes when all four children (BNG-25, BNG-26, BNG-27, BNG-28) are done.

## Context

After vendor removal (BNG-22), every official build target needs either an explicit Bungee dependency path or an explicit `BUNGEE=OFF` decision. Because this PR keeps `BUNGEE=ON` by default, unsupported platforms need deliberate handling rather than accidental breakage.

BNG-21 added an `ExternalProject_Add(bungee_external)` source-fetch fallback, but it falls through to the vendored `lib/bungee/` tree when system Eigen3 or system pffft is missing — a stock Linux dev box. The four implementation children close that gap: BNG-25 + BNG-26 make the ExternalProject path self-contained; BNG-27 covers Flatpak (offline build); BNG-28 covers GitHub Actions.

## Pin research (canonical home)

These pins are referenced by BNG-25 and BNG-27. Recorded once here so children don't re-research:

- **Eigen 3.4.0 stable**
  URL: `https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz`
  SHA256: `8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72`
  Bungee v2.4.15 actually pins commit `c29c8001…` from Eigen master (`EIGEN_WORLD.MAJOR.MINOR = 3.4.90` — a development snapshot). We deliberately deviate to 3.4.0 stable because: (a) Bungee's upstream `CMakeLists.txt` has zero Eigen version constraint — the master pin is convenience, not requirement; (b) Mixxx convention strongly prefers stable release tarballs (libdjinterop 0.27.0, rubberband 4.0.0); (c) Bungee uses only stable Eigen Core API + long-standing `EIGEN_RUNTIME_NO_MALLOC` machinery, all present in 3.4.0. **BNG-25 must smoke-test this** — if compile fails, fall back to pinning the c29c8001 commit and document the deviation.

- **pffft commit `02fe7715a5bf8bfd914681c53429600f94e0f536`** (jpommier upstream from `bitbucket.org/jpommier/pffft.git`)
  URL: `https://bitbucket.org/jpommier/pffft/get/02fe7715a5bf8bfd914681c53429600f94e0f536.tar.gz`
  SHA256: compute at implementation time
  Confirmed via `git -C lib/bungee ls-tree v2.4.15:submodules`. Not `marton78/pffft` — we mirror exactly what Bungee was tested with.

- **Bungee v2.4.15**
  URL: `https://github.com/bungee-audio-stretch/bungee/archive/refs/tags/v2.4.15.tar.gz`
  SHA256: compute at implementation time (BNG-21 already records this — read from `CMakeLists.txt` `bungee_external` URL_HASH).

## New patch-tool requirement (surface in PR description + Mixxx wiki)

After BNG-26 lands, the ExternalProject branch hard-fails when GNU `patch` is missing (no more vendored fallthrough). Mixxx's documented build prerequisites need to mention `patch` for Linux and (optionally) for Windows MSVC + macOS dev environments. Updating the wiki / `docs/build/` is part of BNG-22's commit message.

## Suggested commit / PR sequence

1. **BNG-25** — Eigen3 + pffft ExternalProjects (no `bungee_external` changes). One PR.
2. **BNG-26** — wire them in, remove fallthrough. One PR. **Unblocks BNG-22.**
3. **BNG-22** — delete `lib/bungee/` and dead vendored CMake block. One PR.
4. **BNG-27** — Flatpak modules. One PR (independent of BNG-22; can land in parallel).
5. **BNG-28** — GitHub Actions updates. One PR (independent of BNG-22; can land in parallel).

## Acceptance criteria (rolled up)

- All four implementation children done.
- BNG-22 lands cleanly without regressing any platform.
- Stock Linux dev box builds end-to-end with `BUNGEE=ON`.
- `BUNGEE=OFF` clean on all platforms.
- `BUNGEE_FETCH_FALLBACK=OFF` produces a strict packaged-only build path.
- Flatpak build green; `bungee-asan.yml` green; `build.yml` matrix green.
