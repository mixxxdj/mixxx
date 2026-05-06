# Draft PR description — Bungee dependency integration

> Working draft for the GitHub PR body. Keep the PR as **draft** until branch-head CI and any maintainer-requested per-commit CI evidence are green.

## Summary

This branch keeps Mixxx's Bungee keylock/scaler integration but removes the checked-in `lib/bungee/` vendor tree. Bungee is now resolved as a normal dependency:

1. `find_package(Bungee CONFIG)` for a package that exports `Bungee::Bungee`.
2. `find_package(unofficial-bungee CONFIG)` for the vcpkg port (`unofficial::bungee::bungee`).
3. `FindBungee.cmake` module mode for system/pkg-config installs.
4. Source-fetch fallback via `ExternalProject_Add` when no package is available.

The Linux source-fetch fallback is self-contained: it downloads pinned Bungee, Eigen3, and pffft archives into the build tree, applies the same vcpkg Bungee install-layout patches, and exposes `Bungee::Bungee` to the rest of Mixxx. Windows/macOS continue to resolve through vcpkg. Flatpak gets explicit Eigen3, pffft, and Bungee modules ordered before the Mixxx module.

## Dependency provenance

| Dependency | Version / pin | Source | Hash / provenance |
| --- | --- | --- | --- |
| Bungee | `2.4.15` | `https://github.com/bungee-audio-stretch/bungee/archive/refs/tags/v2.4.15.tar.gz` | `SHA256=aa94ffe8ba49bcb916f454c5221d3480ae880f199e1516de31585924398ca67a`; matches the microsoft/vcpkg Bungee port version used by the overlay. |
| Eigen3 | `3.4.0` | `https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz` | `SHA256=8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72`; stable release preferred over Bungee's submodule commit. |
| pffft | `02fe7715a5bf8bfd914681c53429600f94e0f536` | `https://bitbucket.org/jpommier/pffft/get/02fe7715a5bf8bfd914681c53429600f94e0f536.tar.gz` | `SHA256=9adeb18ac7bb52e9fb921c31c0c6a4e9ae150cc6fcb20a899d4b3a2275176ded`; exact pffft submodule pin from Bungee `2.4.15`. |
| vcpkg Bungee overlay | microsoft/vcpkg port at merge `ae92331` | `cmake/vcpkg-overlay-ports/bungee/` | Kept as a separate commit so it can be cleanly removed once `mixxxdj/vcpkg` contains the upstream port. |

Hash notes: Bungee/Eigen3/pffft hashes are the values committed into the Mixxx CMake/Flatpak integration and were checked locally during the BNG implementation chain with `sha256sum`/manifest verification. Bungee version provenance is cross-checked against the microsoft/vcpkg `bungee` port copied under `cmake/vcpkg-overlay-ports/bungee/`.

## Maintainer-facing decisions already applied

- `BUNGEE` remains default `ON`, with `BUNGEE=OFF` still supported.
- Mixxx's global CMake minimum is unchanged. Bungee's source-fetch build applies a Mixxx patch lowering Bungee's upstream `cmake_minimum_required(3.30...3.31)` to Mixxx's current minimum, and emits a warning on CMake < 3.30.
- The vcpkg overlay is temporary Option C: it is isolated in one commit for later removal when the Mixxx vcpkg fork catches up.
- The source-fetch path requires a `patch`/`gpatch` executable and fails clearly if missing.
- No checked-in Bungee/Eigen/pffft source remains in `lib/bungee/`.

## BNG-24 local spot checks

Run on branch `feature/bungee` during BNG-24 preparation:

| Check | Result |
| --- | --- |
| `git branch --show-current` | `feature/bungee` |
| `git ls-files 'lib/bungee/**' \| wc -l` | `0` tracked vendored Bungee files |
| generated/build artifact grep over tracked files (`build/`, `build_*`, `CMakeFiles/`, `meta_types/`, `src/version.h`) | `0` tracked generated/build artifacts |
| Ruby YAML parse for Flatpak modules, Flatpak manifest, build workflow, Bungee ASan workflow, and task index | Passed |
| `pre-commit run --files .github/workflows/bungee-asan.yml .github/workflows/build.yml docs/tasks/index.yaml docs/tasks/transitions.jsonl docs/tasks/done/BNG-28.md` | Passed during BNG-28 |
| BNG-28 adversarial review | Addressed: added `cmake/modules/FindBungee.cmake` path trigger and clarified sanitizer comments. |
| `build.yml` read-through | No change needed: Ubuntu matrix configures `-DBUNGEE=ON`; build step is `cmake --build . --config RelWithDebInfo` with no `--target` filter, so ExternalProject dependencies are reachable through the normal build graph. |

## Focused dependency-removal validation record

| Commit | Purpose | Local validation | CI result |
| --- | --- | --- | --- |
| `b8149fd354` | BNG-19: normalize discovery to `Bungee::Bungee` | `BUNGEE=OFF` configure clean; module discovery exercised with local Bungee checkout; downstream links use `Bungee::Bungee`. | Pending CI URL |
| `ab304c284b` | BNG-20: add temporary vcpkg overlay | Overlay copied from microsoft/vcpkg Bungee port, kept isolated for clean future removal. | Pending CI URL |
| `e14787a19c` | BNG-21: add Bungee `ExternalProject_Add` fallback | `BUNGEE=ON` configure variants exercised; `BUNGEE=OFF` configure clean; patch sequence applies to v2.4.15; full Eigen+pffft host build deferred to later self-contained fallback work. | Pending CI URL |
| `cf86559a5c` | BNG-25: add Eigen3 + pffft ExternalProject targets | Pinned hashes verified; pffft archive and template layout checked; targets added `EXCLUDE_FROM_ALL`. | Pending CI URL |
| `1b9df1d450` | BNG-26: wire Eigen3/pffft into Bungee fallback | `bungee_external` and `mixxx-lib` path validated locally; source-fetch path now self-contained. | Pending CI URL |
| `fa1883c7cd` | BNG-22: remove vendored Bungee source | `lib/bungee/` removed from tracked tree; dead direct-build fallback deleted; source-fetch and `mixxx-lib` validation passed. | Pending CI URL |
| `e89b238989` | BNG-27: add Flatpak Eigen3/pffft/Bungee modules | Ruby YAML parse, pre-commit, manifest-relative source/patch paths, tarball hashes, pffft layout, Bungee patch sequence, and `/tmp` `find_package(unofficial-bungee CONFIG REQUIRED)` shape passed. `flatpak-builder` not installed locally; defer full build to CI. | Pending CI URL |
| `46378cdf61` | BNG-28: update Bungee ASan workflow paths/wiring | Ruby YAML parse and pre-commit passed; exact ASan flag verified as `-DSANITIZE_ADDRESS=ON`; ExternalProject sanitizer instrumentation added via job environment; adversarial review addressed. | Pending CI URL |

## Full branch commit ledger

| Commit | Purpose | Local validation | CI result |
| --- | --- | --- | --- |
| `131abc3a30` | Complete Bungee integration into Mixxx by implementing real Bungee API calls and wiring it into the keylock engine system | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `a934bde3b9` | Enable BUNGEE by default for CI testing | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `08417eb16f` | Added Bungee library and dependencies to lib/ and fixed build file | Legacy vendor-add commit: at this SHA the checked-in `lib/bungee/` provider is intentional and must be validated as that historical build shape; BNG-22 later removes it. This needs CI evidence or history linearization before ready. | Pending CI URL after push |
| `d5d2987e6a` | Fix std::min type mismatch in EngineBufferScaleBungee::processGrain | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `93766bb4f4` | Merge pull request #6 from 0cwa/cto/fix-bungee-std-min-type | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `2d08d941f0` | Convert lib/bungee/ MSVC fixes into patch file | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `4030ffcdc7` | fix: disable Dual-threaded Stereo checkbox for Bungee keylock engine | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `72d93a8ec6` | Integrate MSVC compatibility patch for bungee library | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `0c090f4b79` | Merge pull request #11 from 0cwa/cto/integrate-msvc-bungee-patch | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `3ee32289bd` | Merge branch 'feature/bungee' into cto/goaldisable-the-dual-threaded-stereo-checkbox-for-bungee-key, check pr still builds | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `1c5dc0bf06` | Merge pull request #10 from 0cwa/cto/goaldisable-the-dual-threaded-stereo-checkbox-for-bungee-key | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `8ed62a13a0` | style: Fix pre-commit formatting issues on feature/bungee | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `0165d555c3` | style: Fix pre-commit formatting issues on feature/bungee | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `5ca9a8e869` | Fix bungee signal | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `0b0a5580c0` | fix: Bungee engine waveform lockup/jiggle and crash issues | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `4bdec2907f` | fix: Bungee engine waveform lockup/jiggle and crash issues | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `4142927b46` | Merge pull request #12 from 0cwa/cto/fix-bungee-waveform-lockup-crash | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `a8e32b07f7` | Merge remote-tracking branch 'mine/mixxx/main' into feature/bungee | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `6583b5f4c6` | Merge remote-tracking branch 'upstream/main' into feature/bungee | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `a3f99cdd19` | simplify patch, update bungee | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `57e98589de` | simplify patch, but not too much lol | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `e6d2c4c5f2` | simplify patch further | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `14ad2e9176` | Fix Bungee audio glitches: correct buffer size and channel pointer handling | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `7d5fedd072` | code style | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `9820cc095c` | Merge pull request #15 from 0cwa/cto/implement-fixes-for-bungee-audio-glitches-based-on-identifie | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `e5f09c10cc` | Fix Bungee grain position tracking: use actual frame consumption instead of fixed advance | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `8acbfeb365` | Fix bungee playback speed | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `ec020ed6c5` | fix: Bungee keylock speedup issue in Release builds | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `145e9e2e6d` | engine/bungee: fix InputChunk contract, ReadAheadManager accounting, build wiring, docs and tests | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `c6ffea9062` | engine/bungee: add EngineBuffer integration tests for keylock scaler selection (BNG-06) | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `4090f3a73a` | engine/bungee: performance, UI, docs, CI, and final cleanup (BNG-08 through BNG-12) | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `dd97c258b9` | Merge pull request #19 from 0cwa/cto/fix-bungee-return-rate-e01 | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `d1006cba49` | lint markdown | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `87e48497c1` | engine/bungee: fix heap corruption from discardBufferedInputBefore + analyseGrain overflow | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `28c87ad520` | crash fix | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `c24a6252e9` | Add opt-in Bungee petrify debug mode | Legacy feature/bungee commit; CI validation pending after draft push. | Pending CI URL after push |
| `429db6b89d` | BNG-15/16: seed Bungee defaults and refresh prefs UI | Historical Bungee test/pre-commit evidence captured in dev-memory; re-run via CI before ready. | Pending CI URL after push |
| `14eff8a399` | BNG-13: buffer-window regression test + ASan CI | Historical Bungee test/pre-commit evidence captured in dev-memory; re-run via CI before ready. | Pending CI URL after push |
| `ec91ad31ec` | docs/bungee-integration: BNG-14 post-fix review pass | Historical Bungee test/pre-commit evidence captured in dev-memory; re-run via CI before ready. | Pending CI URL after push |
| `07c63339d6` | docs(bungee): dependency integration plan and BNG-17..24 execution tickets | Docs/ticket content; markdown/pre-commit expected, re-run in branch CI before ready. | Pending CI URL after push |
| `b8149fd354` | build(bungee): normalize CMake dependency discovery to Bungee::Bungee (BNG-19) | Dependency-integration local checks recorded in dev-memory/commit body; re-run CI before ready. | Pending CI URL after push |
| `ab304c284b` | build(bungee): add in-tree vcpkg overlay port for Bungee (BNG-20 Option C) | Dependency-integration local checks recorded in dev-memory/commit body; re-run CI before ready. | Pending CI URL after push |
| `e14787a19c` | build(bungee): ExternalProject_Add source-fetch fallback (BNG-21) | Dependency-integration local checks recorded in dev-memory/commit body; re-run CI before ready. | Pending CI URL after push |
| `b8df7f263f` | docs(bungee): split BNG-23 into BNG-25..28 (+BNG-29 deferred) | Local checks recorded in commit body; see focused validation table below. | Pending CI URL after push |
| `cf86559a5c` | build(bungee): add Eigen3 + pffft ExternalProject_Add targets (BNG-25) | Local checks recorded in commit body; see focused validation table below. | Pending CI URL after push |
| `1b9df1d450` | build(bungee): wire Eigen3 + pffft into bungee_external (BNG-26) | Local checks recorded in commit body; see focused validation table below. | Pending CI URL after push |
| `fa1883c7cd` | build(bungee): remove vendored source fallback (BNG-22) | Dependency-integration local checks recorded in dev-memory/commit body; re-run CI before ready. | Pending CI URL after push |
| `e89b238989` | feat(BNG): BNG-27 | Local checks recorded in commit body; see focused validation table below. | Pending CI URL after push |
| `46378cdf61` | ci(BNG): BNG-28 | Local checks recorded in commit body; see focused validation table below. | Pending CI URL after push |

## Draft-readiness / history caveats

This document is sufficient for a **draft PR body**, not for marking the PR ready. The current branch still carries historical development artifacts that conflict with the strictest reading of `06-branch-ci-discipline.md`:

- The branch has 49 commits after `upstream/main`; 41 legacy/pre-plan commits do not yet have per-commit CI URLs or local command logs in this document. They must either receive CI evidence after the draft push or be squashed/linearized into a smaller review set before marking ready.
- The branch has 9 merge commits, including private working-branch/provenance messages (`mine/mixxx/main`, `0cwa/cto/*`, and `3ee32289bd`'s cross-branch "check pr still builds" merge). Maintainers may reasonably require an interactive rebase/squash before review.
- The early vendor-add commit `08417eb16f` is a historical intermediate state where vendored `lib/bungee/` is live; it is not representative of the final non-vendored dependency shape. It needs explicit CI evidence at that SHA or removal via history rewrite before the PR is ready.

Merge commits currently present:

```text
93766bb4f4 Merge pull request #6 from 0cwa/cto/fix-bungee-std-min-type
0c090f4b79 Merge pull request #11 from 0cwa/cto/integrate-msvc-bungee-patch
3ee32289bd Merge branch 'feature/bungee' into cto/goaldisable-the-dual-threaded-stereo-checkbox-for-bungee-key, check pr still builds
1c5dc0bf06 Merge pull request #10 from 0cwa/cto/goaldisable-the-dual-threaded-stereo-checkbox-for-bungee-key
4142927b46 Merge pull request #12 from 0cwa/cto/fix-bungee-waveform-lockup-crash
a8e32b07f7 Merge remote-tracking branch 'mine/mixxx/main' into feature/bungee
6583b5f4c6 Merge remote-tracking branch 'upstream/main' into feature/bungee
9820cc095c Merge pull request #15 from 0cwa/cto/implement-fixes-for-bungee-audio-glitches-based-on-identifie
dd97c258b9 Merge pull request #19 from 0cwa/cto/fix-bungee-return-rate-e01
```

## Experimental branch audit

| Branch | Status / rationale |
| --- | --- |
| `bungee-engine` | `git log --oneline bungee-engine ^feature/bungee` shows one unmerged commit: `d919f9db23 Add Bungee library with CMake build integration`. Intentionally excluded because this PR removes the vendored Bungee source and supersedes that build shape; no separate engine-logic commits are missing from this local audit. |
| `feat/bungee-petrify-debug` | Contains hundreds of commits from an older base plus an alternate petrify-debug commit; intentionally excluded as an experimental/debug branch. The user-facing petrify debug feature is already represented in this branch by `c24a6252e9`. |
| Other local `*bungee*` branches | No commits ahead of `feature/bungee` in the local audit. |

## Known limitations before marking ready

- CI URLs must be filled after pushing the branch/draft PR; until then the PR should stay draft-only.
- Full Flatpak build is deferred to CI because `flatpak-builder` is not installed locally.
- Windows/macOS vcpkg CI must confirm the in-tree overlay is active. `CMakeLists.txt` appends `cmake/vcpkg-overlay-ports` to `VCPKG_OVERLAY_PORTS` when `MIXXX_VCPKG_ROOT` is used and `VCPKG_OVERLAY_PORTS` is not already defined; CI should verify no prebuilt buildenv path bypasses that.
- GitHub positive/negative path-filter smoke tests for `bungee-asan.yml` require pushed commits/PR events.
- Maintainers may still request a linearized/squashed history. This draft preserves the existing feature branch history and documents validation status rather than rewriting history locally.
