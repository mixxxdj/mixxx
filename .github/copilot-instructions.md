# Mixxx — AI Coding Instructions

Mixxx is a free, open-source DJ application written primarily in **C++17** with
**Qt 6** (widgets and QML), **JavaScript/TypeScript** for MIDI/HID controller
mappings, and **CMake** as the build system.

Repository: <https://github.com/mixxxdj/mixxx>
License: GPL v2+

## Build System

- CMake ≥ 3.21; out-of-source builds in `cbuild/`.
- Generate: `cmake -DCMAKE_BUILD_TYPE=Debug -DDEBUG_ASSERTIONS_FATAL=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..`
- Build:   `cmake --build . --parallel $(nproc)`
- Tests:   `ctest --test-dir cbuild` (Google Test)
- C++ standard: **C++20** (`.clang-format` says `Standard: c++20`), conservative feature adoption.

## Code Formatting & Linting

- **clang-format** config lives at `.clang-format` (BasedOnStyle: Google, IndentWidth: 4).
  Run `pre-commit` hooks or `python tools/clang_format.py` (two-pass: reformat then line-break at 80 cols).
- **ESLint** for JS/TS controller scripts (`eslint.config.cjs`).
- **pre-commit** runs clang-format, ESLint, codespell, shellcheck, markdownlint,
  black (Python tools), gersemi (CMake), qsscheck, qmlformat, and more.
- Do NOT mass-reformat files. Only format new or modified code segments.
- Separate formatting-only changes into their own commit, distinct from logic changes.

## C++ Style Guide

### Indentation & Whitespace

- 4-space indent; **never** tabs.
- 8-space continuation indent for broken lines.
- Max column width: 80 characters (soft target; enforced by clang-format second pass).
- BinPackArguments/BinPackParameters: false — if you break a parameter list, put each parameter on its own line.

### Naming

| Entity | Convention | Example |
|---|---|---|
| Classes | `CamelCase` | `TrackCollection` |
| Member functions | `camelBack` | `loadTrack()` |
| Local variables | `camelBack` or `snake_case` | `hotcueName` / `hotcue_name` |
| Member variables | `m_` prefix | `m_pConfig` |
| Static class members | `s_` prefix | `s_instance` |
| Pointer variables | `p` prefix | `pTrack`, `m_pEngine` |
| Constants / constexpr | `k` prefix, PascalCase | `kSilenceThreshold` |
| Enums | `enum class`, CamelCase | `enum class ChannelLayout { Stereo }` |
| CO / setting keys | `snake_case` | `hotcue_1_activate` |

### Braces & Control Flow

- K&R brace style — opening brace on same line, separated by a space.
- **Always** use braces for `if`/`else`/`for`/`while`/`switch` bodies, even single-line.
- Space after `if`, `for`, `while`, `switch` keyword; space before opening brace.

### Headers

- Use `#pragma once` (not `#ifndef` guards).
- Include order (groups separated by blank line):
  1. Matching `.h` for this `.cpp`
  2. System includes
  3. Qt includes
  4. Library dependency includes
  5. Mixxx local includes (`"path/from/src/root.h"`)
  6. Forward declarations
- Alphabetical within each group.
- Never use relative includes.
- Use forward declarations to minimize header dependencies.

### Class Declarations

Order of sections:

1. `Q_OBJECT` macro
2. Public enums/constants/inner types
3. Public methods → public slots → signals
4. Protected enums/slots/methods/variables
5. Private enums/slots/methods/variables
6. Private member variables last

- Access specifiers indented 2 spaces.
- Document every public method in the header (only); implementation files should have implementation-only comments.
- Destructors: mark `override` in derived classes (not `virtual`).
- Mark single-argument constructors `explicit`.

### Ownership & Pointers

- **No naked `new`/`delete`**. Use `std::unique_ptr` (`std::make_unique`),
  `std::shared_ptr` (`std::make_shared`), or `parented_ptr` (`make_parented` for
  Qt object-tree ownership).
- Pass `const T&` or `T&` if no ownership transfer; raw `T*` only if `nullptr` is valid.
- Pass smart pointers by value when transferring ownership.
- For non-null pointer arguments, use `gsl::not_null<T*>` — but **always check for nullptr before constructing** `gsl::not_null`.

### Assertions

- `DEBUG_ASSERT(condition)` for debug-only checks.
- `VERIFY_OR_DEBUG_ASSERT(condition) { /* recovery */ }` when graceful recovery
  is needed in release builds.
- Do **not** use `Q_UNUSED`; leave parameters unnamed in definitions instead.
- Use `[[maybe_unused]]` for parameters in functions without separate declarations.

### Modern C++ Features

- Use `override` on all overridden methods; omit redundant `virtual`.
- Use `nullptr`, never `NULL`.
- Prefer `std::unique_ptr` over `QScopedPointer`.
- Prefer range-based `for` loops over `foreach`.
- Use `auto` to avoid repetition, but **not** in function signatures, return types, or where the type is not obvious from context.
- Use strongly-typed `enum class`, not C-style enums.
- Use `constexpr` where possible; prefer over `#define`.
- Use `QStringLiteral("...")` for string literals, not raw `QString("...")`.
- Use `noexcept` only on move operations / special members where clang-tidy recommends it; do not sprinkle it everywhere.
- Use closures/lambdas carefully — they obscure control flow and can cause lifetime bugs. Expect extra scrutiny in review.
- `goto` is **not allowed**.

### Non-Const References

- Avoid non-const reference parameters for out-params (legacy Google/Qt style). Use pointers instead.
- R-value references (`auto&&`) are fine in range-for over mutable containers.

### Comments

- C++ style (`//`), not C-style (`/* */`) or Javadoc-style.
- `///` for documentation comments in headers.
- `// TODO(username)` or `// TODO(XXX)` for team TODOs.
- Date and attribute comments for warnings: `// Reason -- username MM/YYYY`.
- Do not comment out code without an explanatory text comment.

### Namespaces

- Wrap new code in the `mixxx` namespace. Avoid deep namespace hierarchies.
- Put file-local helpers in anonymous namespaces in `.cpp` files.

### QString

- Use `QStringLiteral("...")`.
- Escape non-ASCII: `QStringLiteral("Hello I\u2019ve to go")`.
- Use `+` for concatenation (QStringBuilder enabled via `QT_USE_QSTRINGBUILDER`).

## JavaScript / Controller Mappings

- Controller mapping JS lives under `res/controllers/`.
- ESLint is enforced via pre-commit — follow `eslint.config.cjs` rules.
- Use JSDoc comments for public functions.
- Use the Components JS library (`res/controllers/common-controller-scripts.js`, `components.js`) when writing HID/MIDI mappings.

## QML

- QML files live under `res/qml/` and `src/qml/`.
- `qmlformat` and `qmllint` are available as pre-commit hooks (manual stage).

## Git Workflow & PR Conventions

- Each feature/bugfix on its own branch; small, building commits.
- **Every commit must build.** This is critical for `git bisect`.
- Commit messages: imperative mood, 72-char line wrap, describe *what* and *why*.
  Good: `DlgPrefEffects: add QListWidget to set order of chains`
  Bad: `address comments from PR review`
- Prefer merging over rebasing. Do not rebase without reviewer agreement.
- Use `--fixup` commits if you plan to squash before merge.
- Bug fixes → stable branch (e.g. `2.5`). New features → `main` or beta branch.
- When changing GUI, post before/after screenshots.
- Squash fixup commits before merge when asked.
- Don't include unrelated changes (e.g. `.pre-commit-config.yaml` version bumps) in your PR.
- Sign the Mixxx Contributor Agreement before your first contribution.

## Common Review Feedback Patterns

These are issues frequently raised by Mixxx reviewers:

1. **Keep PRs focused.** Don't mix formatting fixes, unrelated refactors, or config changes with feature work.
2. **Pre-commit must pass.** Install pre-commit locally; fix issues before pushing.
3. **Use `std::chrono::duration`** for time values instead of raw numeric types.
4. **Use Q_ENUM** for enums exposed to Qt's type system; avoid manual `static_cast<int>` when `QVariant::fromValue` works directly.
5. **No `.DS_Store` or IDE files** in commits. Check `.gitignore`.
6. **Squash commits** when asked — reviewers expect clean history before merge.
7. **Rebase onto the correct target branch** (stable for bugfixes, `main` for features).
8. **Document the "why"** in both commit messages and code comments.
9. **Test edge cases** — reviewers will check border conditions, null pointers, and thread safety.
10. **SVG assets** for waveform/skin markers should be full-size, borderless, and match existing conventions.

## Project Structure (Key Directories)

```
src/                    # C++ source code
  engine/               # Audio engine (real-time thread)
  controllers/          # Controller backend (MIDI, HID)
  library/              # Track library, database
  mixer/                # Mixing, EQ, crossfader
  effects/              # Effects framework
  skin/                 # Legacy widget skin loader
  qml/                  # QML-based UI backend
  preferences/          # Preferences dialogs
  util/                 # Shared utilities (assert.h, memory.h, etc.)
  test/                 # Google Test unit tests
res/                    # Resources
  controllers/          # Controller mapping JS/XML files
  skins/                # Widget-based skins (LateNight, Shade, etc.)
  qml/                  # QML UI files
cmake/                  # CMake modules
tools/                  # Python helper scripts (clang_format.py, etc.)
```

## Important APIs & Patterns

- **ControlObject / ControlProxy**: The primary mechanism for inter-component
  communication. Keys use `[Group], key_name` format with `snake_case` keys.
- **Engine thread**: Real-time audio processing. No allocations, no locks, no
  Qt signals in the audio callback.
- **parented_ptr / make_parented**: For Qt object-tree managed objects. The
  pointed-to object must receive a parent before the `parented_ptr` is destroyed.
- **VERIFY_OR_DEBUG_ASSERT**: The idiomatic way to handle "should never happen"
  conditions with graceful recovery in release builds.
