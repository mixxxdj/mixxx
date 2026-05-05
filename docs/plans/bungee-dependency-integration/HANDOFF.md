# Handoff — Bungee dependency integration (mid-stream)

This file is a self-contained primer for a fresh Pi session continuing the
Bungee dependency-integration work. Read this first.

## Project goal

Move Mixxx from a checked-in `lib/bungee/` source copy to a normal dependency
integration that Mixxx maintainers will accept. End-state shape:

- `find_package(Bungee CONFIG)` first, vcpkg `unofficial-bungee` second,
  pkg-config/system third, **no vendored source** in the final tree.
- Windows/macOS resolve through `mixxxdj/vcpkg`.
- Linux uses system Bungee or an opt-in `ExternalProject_Add` fallback.
- `BUNGEE` defaults to `ON`.
- Every commit on the integration branch is independently green.

The full plan lives in `docs/plans/bungee-dependency-integration/`.

## Where we are

Tickets in `docs/tasks/`:

| ID | Status | Summary |
| --- | --- | --- |
| BNG-17 | done | Plan assumptions + decision-format captured |
| BNG-18 | done | Bungee/CMake compatibility matrix — see `bng-18-version-matrix.md` |
| BNG-19 | done | CMake discovery normalized to `Bungee::Bungee` |
| BNG-20 | done | In-tree vcpkg overlay port (Option C) implemented |
| BNG-21 | open | ExternalProject fallback with CMake-warning + version pin |
| BNG-22 | open | Remove vendored `lib/bungee/` |
| BNG-23 | open | Packaging / CI updates for non-vendored Bungee |
| BNG-24 | open | Trunk-driven PR history + per-commit validation record |

## Critical findings (read before touching CMake)

From BNG-18 (`bng-18-version-matrix.md`):

1. **Every Bungee release** (v2.1.8 … v2.4.24) declares
   `cmake_minimum_required(VERSION 3.30...3.31)`. There is no older Bungee
   release that supports lower CMake. The "older Bungee for older CMake" idea
   is **not viable** as version selection.
2. **API stable**: `bungee/Bungee.h` and `bungee/Modes.h` are byte-identical
   between v2.4.15 (current vendor) and v2.4.24 (latest).
3. **Upstream Bungee CMakeLists.txt does not install a CMake package** by
   itself. The vcpkg port adds `cmake-use-vcpkg-deps-and-install-layout.patch`
   to make it `find_package`-able. Any non-vcpkg fallback must do the same.
4. **mixxxdj/vcpkg does not yet have the Bungee port.** It is 477 commits
   behind microsoft/vcpkg as of the research date.
5. **The CMake 3.30 minimum is a policy choice, not a feature requirement.**
   The actual CMake commands Bungee uses all work in CMake 3.21+.

## User decisions captured

- `BUNGEE` defaults to `ON` for this PR.
- It is acceptable to **recommend** a newer CMake for Bungee-enabled builds.
  Do **not** raise Mixxx's global hard CMake minimum.
- For BNG-21 (ExternalProject fallback): patch Bungee's
  `cmake_minimum_required` down so older CMake can build it, **but emit a
  clear CMake warning** when host CMake is below Bungee's stated 3.30. Mixxx
  pins dependency versions, so any future Bungee bump is gated by upstream
  testing.
- For BNG-20 (vcpkg path): use Option C (in-tree overlay port) **as its own
  commit** so it can be reverted cleanly when Option A (cherry-pick into
  mixxxdj/vcpkg) replaces it.
- All future maintainer/user decision requests must be presented with a
  pros/cons list and a recommendation, not as open-ended yes/no.

## What is on disk right now

### Already changed (uncommitted)

```text
M  CMakeLists.txt
M  cmake/modules/FindBungee.cmake
?? cmake/vcpkg-overlay-ports/bungee/  (entire directory; 9 files)
?? docs/plans/bungee-dependency-integration/  (planning docs, 9 files)
?? docs/tasks/  (ticket index + open/* + done/*)
```

### Suggested commit slicing

Per the trunk-driven-development discipline in
`docs/plans/bungee-dependency-integration/06-branch-ci-discipline.md`,
split the in-flight work into focused commits before extending it:

1. **docs(bungee): plan + tickets for dependency integration**
   - `docs/plans/bungee-dependency-integration/**`
   - `docs/tasks/index.yaml` + `docs/tasks/open/*` + `docs/tasks/done/*`
2. **build(bungee): normalize CMake discovery to `Bungee::Bungee` (BNG-19)**
   - Touches only `CMakeLists.txt` BUNGEE block + `cmake/modules/FindBungee.cmake`
   - Vendored fallback retained, labelled for BNG-22 removal
3. **build(bungee): in-tree vcpkg overlay port (BNG-20, Option C)**
   - Adds `cmake/vcpkg-overlay-ports/bungee/**`
   - Adds the `list(APPEND VCPKG_OVERLAY_PORTS …)` line in `CMakeLists.txt`
   - Will be reverted when mixxxdj/vcpkg gains the port (Option A)

Verify each commit configures cleanly with both `-DBUNGEE=ON` and
`-DBUNGEE=OFF` before moving on.

## Branch state — IMPORTANT

The current local branch is **`main`**, not `feature/bungee`. Per dev-memory,
`feature/bungee` is no longer the working surface; relevant fixes are
cherry-picked into `main` (which is the upstream-fork trunk). Confirm this
matches the user's intent before pushing or opening a PR.

The mixxxdj/vcpkg fork itself is on branch `2.6` (default) and 477 commits
behind microsoft/vcpkg.

## Constraints / invariants — DO NOT BREAK

- **Do not modify `lib/bungee/` source files.** Treat Bungee as a vendor
  library. Only the existing Windows-compatibility patch may be touched, and
  preferably not even that — upstream the patch instead.
- **Engine-thread safety**: no avoidable allocations, locks, or hidden state
  churn in the real-time scaler path (`enginebufferscalebungee.{h,cpp}`).
- **`BUNGEE=OFF` must build cleanly** at every commit — verify with
  `cd build && cmake .. -DBUNGEE=OFF`.
- **No source-tree mutation at configure time.** The current vendored fallback
  applies a patch into `lib/bungee/` on MSVC — that pattern is the reason for
  this whole rework. Do not repeat it in the new fallback path.

## Next ticket: BNG-21 — ExternalProject fallback

`docs/tasks/open/BNG-21.md` has the scope. Concretely you need to:

1. Pin a Bungee version. Recommend **v2.4.15** to match the vcpkg overlay
   (BNG-20) so a future bump can be coordinated across both paths in one PR.
   API is identical to v2.4.24, so no functional difference.
2. Add an `ExternalProject_Add(bungee_external …)` call inside the BUNGEE
   block of `CMakeLists.txt` in the existing `else()` branch (the one
   currently labelled "Vendored fallback — build Bungee directly from
   lib/bungee/"). Place it **before** the vendored-source `add_library(bungee
   STATIC …)` so that the fallback order becomes:
   `package CONFIG → vcpkg CONFIG → MODULE → ExternalProject → vendored`.
   Vendored stays as the absolute last resort until BNG-22 deletes it.
3. URL: `https://github.com/bungee-audio-stretch/bungee/archive/refs/tags/v${BUNGEE_FETCH_VERSION}.tar.gz`
   with `URL_HASH SHA256=…` (compute from the tarball).
4. `DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/downloads"`,
   stable `DOWNLOAD_NAME "bungee-${BUNGEE_FETCH_VERSION}.tar.gz"`.
5. `PATCH_COMMAND` applies, in order:
   - `cmake/vcpkg-overlay-ports/bungee/cmake-use-vcpkg-deps-and-install-layout.patch`
     (reusing the already-downloaded patch — no duplication).
   - `cmake/vcpkg-overlay-ports/bungee/pffft-include-path.patch`.
   - On MSVC: `assert-win32-compat.patch` and `resample-msvc-noinline.patch`.
   - A new patch that lowers `cmake_minimum_required(VERSION 3.30...3.31)`
     to e.g. `3.21` (Mixxx's current minimum). Place it under
     `cmake/patches/bungee/lower-cmake-minimum.patch` (new directory; do not
     mix it into `cmake/vcpkg-overlay-ports/`, which must stay verbatim from
     microsoft/vcpkg for clean removal).
6. `CMAKE_ARGS` forwarding (mirror libdjinterop's pattern at CMakeLists.txt
   ~3340): `CMAKE_BUILD_TYPE`, `CMAKE_INSTALL_PREFIX`, `CMAKE_PREFIX_PATH`
   (pipe-delimited), `CMAKE_FIND_ROOT_PATH`, `CMAKE_MODULE_PATH`,
   `CMAKE_TOOLCHAIN_FILE`, `CMAKE_OSX_DEPLOYMENT_TARGET`,
   `CMAKE_OSX_ARCHITECTURES`, `CMAKE_SYSTEM_NAME`, `CMAKE_SYSTEM_PROCESSOR`,
   `BUNGEE_BUILD_SHARED_LIBRARY=OFF`, `BUNGEE_INSTALL_FRAMEWORK=OFF`,
   `BUNGEE_VERSION=${BUNGEE_FETCH_VERSION}`, `BUNGEE_PRESET=`.
7. Set `BUILD_BYPRODUCTS` to the static library install path (look at the
   `cmake-use-vcpkg-deps-and-install-layout.patch` to see the install layout
   — `lib/${CMAKE_STATIC_LIBRARY_PREFIX}bungee${CMAKE_STATIC_LIBRARY_SUFFIX}`).
8. After the ExternalProject block, create the imported `Bungee::Bungee`
   target pointing at the installed artifact, plus its include directory.
   `file(MAKE_DIRECTORY …)` for the include dir before setting
   `INTERFACE_INCLUDE_DIRECTORIES` (matches the existing libdjinterop
   pattern).
9. **CMake-version warning** — at the top of the BUNGEE block, before
   discovery starts:

   ```cmake
   if(BUNGEE AND CMAKE_VERSION VERSION_LESS 3.30)
     message(
       WARNING
       "Bungee upstream declares cmake_minimum_required(VERSION 3.30...3.31). "
       "You are running CMake ${CMAKE_VERSION}. Mixxx will patch the Bungee "
       "build to allow this older CMake when using the source fallback, but "
       "the build is unsupported by the Bungee maintainers. If the Bungee "
       "build fails, install CMake 3.30 or newer (Kitware PPA on Linux, "
       "Homebrew on macOS) or use a system/vcpkg Bungee package instead."
     )
   endif()
   ```

10. Add `ExternalProject_Add` (look near line 3321 in CMakeLists.txt) — first
    `include(ExternalProject)` is already done at the top of the file; verify.
11. Hook the new `Bungee::Bungee` target into the existing
    `target_link_libraries(mixxx-lib PUBLIC Bungee::Bungee)` flow (no change
    needed — that line already uses the normalized target).

### Acceptance checklist for BNG-21

- [ ] `cmake .. -DBUNGEE=ON` configures clean on a host with no system
      Bungee package.
- [ ] `cmake --build build --target bungee_external` (or whatever target
      name you choose) actually downloads, patches, builds, and installs.
- [ ] `Bungee::Bungee` resolves to the externally-built artifact, not the
      vendored one (verify by removing `lib/bungee/` temporarily and
      re-configuring).
- [ ] `cmake .. -DBUNGEE=OFF` still configures clean.
- [ ] Configure-time mutation of the source tree is **not** introduced.
- [ ] The CMake version warning fires when run with CMake < 3.30.

## After BNG-21

Order to follow:

1. **BNG-22** — Once BNG-19/20/21 prove the fallback chain works end-to-end,
   delete `lib/bungee/` and remove the vendored `else()` branch from the
   BUNGEE block. The `Bungee::Bungee` ALIAS line goes away with it; the rest
   of the BUNGEE block needs no other change because everything already links
   through `Bungee::Bungee`.
2. **BNG-23** — Walk through `packaging/debian/`, `packaging/flatpak/`,
   `tools/*_buildenv.*`, and `.github/workflows/` to verify each platform
   has an explicit Bungee story (vcpkg, system package, ExternalProject, or
   explicit `BUNGEE=OFF` with rationale).
3. **BNG-24** — Final history clean-up. Confirm working tree has no
   generated/build artifacts (the current untracked list includes `build/`,
   `build_asan/`, `CMakeFiles/`, `meta_types/`, `src/version.h`,
   `*.midi.xml`, etc. — those must not be committed). Compose the PR
   description with the validation matrix from `06-branch-ci-discipline.md`.

## Tooling notes for the next session

- `manage_ticket_manifest` is already initialized at `docs/tasks/index.yaml`.
- `dev_memory` has the BNG-19 and BNG-20 completion decisions recorded; recall
  via `dev_memory { action: "read_decisions" }`.
- Network: `web_search` and `web_extract` were unreliable in this environment;
  `curl` from `Bash` worked for raw GitHub content. Use that to fetch upstream
  files (e.g., to compute SHA256 for the Bungee tarball in BNG-21).
- Configure-time check that has been the smoke-test all session:

  ```bash
  cd build && cmake .. -DBUNGEE=ON
  grep -c '__BUNGEE__' build/CMakeFiles/mixxx-lib.dir/flags.make
  ```

  Should print `1`. With `-DBUNGEE=OFF`, the count should be `0` and
  `enginebufferscalebungee.cpp.o` must not appear in
  `build/CMakeFiles/mixxx-lib.dir/link.txt`.

## Open decisions still owed to the user

None right now. BNG-21 has explicit direction. BNG-22/23/24 may surface new
maintainer-facing decisions; present them with the pros/cons template from
`07-execution-tickets.md`.
