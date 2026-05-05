# 01 — Upstream Bungee Package Readiness

This step makes Bungee consumable as a normal dependency.

## 1.1 Confirm the target upstream version

Current local Bungee checkout reports approximately:

```text
v2.4.15-2-g613d3ba
```

Action items:

- choose a stable Bungee tag/commit to integrate;
- prefer a released tag over a commit if possible;
- document the exact version in Mixxx CMake and packaging files.

## 1.2 CMake requirement and Bungee version strategy

Mixxx currently requires CMake `3.21`. Upstream Bungee's current CMake file appears to require CMake `3.30...3.31`.

Current implementation direction:

- it is acceptable for Bungee-enabled builds to recommend a newer CMake when consuming the most recent Bungee release;
- do not raise Mixxx's global hard CMake minimum without a deliberate pros/cons decision, because that affects builders even when `-DBUNGEE=OFF`;
- investigate whether an older Bungee release supports Mixxx's current CMake minimum and remains API-compatible with Mixxx's scaler integration;
- if an older compatible release exists, allow the fallback/dependency path to choose that version for older installed CMake environments;
- if no older safe release exists, document the tradeoff and prefer a clear configure message over hidden build failure.

Action items:

- build a version/CMake/API compatibility table before changing build logic;
- ask Bungee upstream whether the current CMake minimum can be lowered or whether older-release consumption is expected for older build environments;
- if patching is needed, prefer packaging/fallback patch files over raising Mixxx's global CMake minimum;
- present any remaining choice to maintainers with pros/cons and a recommendation.

## 1.3 Make Bungee install a proper CMake package

Preferred upstream target:

```cmake
find_package(Bungee CONFIG REQUIRED)
target_link_libraries(mixxx-lib PUBLIC Bungee::Bungee)
```

Bungee should install:

- `include/bungee/*.h`;
- `libbungee` static/shared library;
- `BungeeConfig.cmake`;
- `BungeeConfigVersion.cmake`;
- exported target `Bungee::Bungee`;
- correct transitive dependencies.

## 1.4 De-vendor Bungee dependencies

Preferred dependency model:

- `Eigen3::Eigen` from system/vcpkg package;
- `pffft` from system/vcpkg package, or a small Bungee-owned fallback only outside Mixxx;
- `cxxopts` only if building the Bungee command-line tool; Mixxx should not need it.

Action items:

- make Bungee library build independent from the CLI target;
- add options to disable CLI/examples/tests in dependency builds;
- ensure static library builds expose any required transitive link dependencies.

## 1.5 Upstream the MSVC compatibility patch

Current Mixxx patch handles Windows/MSVC issues such as POSIX-only includes.

Preferred order:

1. submit the fix upstream to Bungee;
2. if not accepted yet, apply the patch in the vcpkg port and ExternalProject fallback;
3. do not apply it to a checked-in Mixxx vendor tree during configure.

## 1.6 Result of this phase

Exit criteria:

- Bungee can be built standalone as static library on Linux, Windows, macOS;
- Bungee can be installed and found with `find_package(Bungee CONFIG)`;
- dependency builds do not require copied source inside Mixxx;
- any remaining patch is isolated in packaging/fallback build files.
