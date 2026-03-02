# Contributing to Mixxx

Thank you for contributing to [Mixxx](https://mixxx.org/)! Mixxx is a free,
open-source DJ application built with C++17, Qt 6, and JavaScript (for
controller mappings). We are a global, all-volunteer team that works by
consensus. Please introduce yourself on our
[Zulip chat](https://mixxx.zulipchat.com/) and review our
[Code of Conduct](CODE_OF_CONDUCT.md) before getting started.

## Getting Started

1. **Fork** the repository on GitHub by clicking the **Fork** button on
    <https://github.com/mixxxdj/mixxx>.

2. **Clone** your fork locally:

    ```shell
    git clone https://github.com/<your-username>/mixxx.git
    cd mixxx
    ```

3. **Create a branch** for your work:

      ```shell
      git checkout -b my-feature
      ```

      Bug fixes go to the current stable branch (e.g. `2.5`). New features go
      to `main`.

4. **Install dependencies** for your platform:

    Mixxx requires many libraries to build. We have convenience scripts to make installing these dependencies easier:

    | Platform | Command | Requirements |
    | -- | ------- | ------------ |
    | Windows | `tools\windows_buildenv.bat` | ~2.5 GB download, ~9 GB disk space |
    | macOS | `source tools/macos_buildenv.sh setup` | ~1.5 GB download, ~3 GB disk space |
    | Debian/Ubuntu | `tools/debian_buildenv.sh setup` | ~200 MB download, ~1 GB disk space |
    | Fedora | `tools/rpm_buildenv.sh setup` | ~200 MB download, ~1 GB disk space |
    | Flatpak | `tools/flatpak_buildenv.sh setup` | ~2.6 GB download, ~5 GB disk space |
    | Android | `tools/android_buildenv.sh setup` (see the [wiki article](<https://github.com/mixxxdj/mixxx/wiki/> Building-for-Android)) | ~3.4 GB download, 13GB disk space |
    | Other Linux distros | See the [wiki article](https://github.com/mixxxdj/mixxx/wiki/Compiling%20on%20Linux) | |

    Other platforms: see the wiki's
    [build instructions](https://github.com/mixxxdj/mixxx/wiki#compile-mixxx-from-source-code).

5. **Build**:

     ```shell
     mkdir build && cd build
     cmake ..
     cmake --build . --parallel $(nproc)

     There should now be a `mixxx` executable in the current directory that you can run. Alternatively, can generate a package using   `cpack`.

     For building and installing Mixxx as a Flatpak, check the documentation in [packaging/flatpak/README.md](packaging/flatpak/  README.md).

     Detailed build instructions for each target OS can be found [on the wiki](https://github.com/mixxxdj/mixxx/  wiki#compile-mixxx-from-source-code)

6. **Run tests**:

    `ctest`

## Pre-commit Setup

[pre-commit](https://pre-commit.com/) hooks enforce clang-format, ESLint,
codespell, markdownlint, gersemi, and other checks automatically. **Install
these before your first commit** — pull requests that fail pre-commit checks
will not be merged.

    ```shell
    pip install pre-commit   # or use your system package manager
    cd /path/to/mixxx
    pre-commit install
    pre-commit install -t pre-push
    ```

To skip a specific hook when needed:

    ```shell
    SKIP=clang-format,end-of-file-fixer git commit
    ```

This is preferable to `git commit --no-verify` because it still runs the
other hooks. You can use `SKIP` to separate formatting changes from logic
changes into two sequential commits (see [Code Formatting](#code-formatting)
below).

## Code Style

### Code Formatting

Formatting is defined by the [`.clang-format`](.clang-format) file in the
project root (Google base, 4-space indent, 8-space continuation). **Only
format new or modified code** — do not mass-reformat unrelated code. Keep
formatting commits separate from logic commits.

### C++ Conventions

- 4 spaces, never tabs. 100-column hard limit, 80-column soft target.
- **Naming**: Classes `CamelCase`, methods `camelBack()`, members `m_prefix`,
  pointers `pPrefix`, constants `kPascalCase`, enums `enum class CamelCase`,
  CO/setting keys `snake_case`.
- Google-style braces (opening brace on the same line). Always use braces on control
  flow bodies, even single-line ones.
- `#pragma once` instead of include guards.
- **Include order** (separated by blank lines, alphabetical within each
  group): matching header → system → Qt → library deps → Mixxx local →
  forward declarations.
- No naked `new`/`delete` — use `std::make_unique`, `std::make_shared`, or
  `make_parented`.
- No `goto`, no `Q_UNUSED` (use unnamed parameters), no C-style enums.
- `///` doc comments in headers. `// TODO(username)` or
  `// TODO(issue URL)`.

### Advanced Conventions

First-time contributors probably don't need to worry about these:

- Wrap new code in `namespace mixxx {}`. Use anonymous namespaces for
  file-local helpers in `.cpp` files.
- `QStringLiteral("...")` for string literals; `tr("...")` for translatable
  strings.
- `override` on all virtual overrides; omit the redundant `virtual` keyword.
- Non-const reference out-parameters: use pointers, not references (legacy
  convention).
- Lambdas: use carefully — they receive extra scrutiny in review for lifetime
  and control-flow issues.
- `VERIFY_OR_DEBUG_ASSERT(cond) { recovery; }` for defensive checks.

For the full C++ style guide, see the
[Coding Guidelines](https://github.com/mixxxdj/mixxx/wiki/Coding-Guidelines)
wiki page.

### JavaScript (Controller Mappings)

- Scripts live in `res/controllers/`. ESLint is enforced via pre-commit
  ([`eslint.config.cjs`](eslint.config.cjs)).
- Use the Components JS library and JSDoc comments.

### QML

The Mixxx UI is currently being rewritten in QML. This new code is in a high state of flux and will change rapidly.

- Source in `res/qml/` and `src/qml/`.
- `qmlformat` and `qmllint` are available via pre-commit.

## Commits

- **One branch per feature or bug fix** so they can be reviewed
  independently.
- **Every commit must build.** This is essential for `git bisect`.
- **Small PRs.** The smaller the pull request, the easier it is to review and
  revert.
- Write commit messages in the **imperative mood**. First line ≤ 50
  characters, body wrapped at 72 characters. Describe *what* changed and
  *why*.

  Good:

      DlgPrefEffects: add QListWidget to set order of chains

      This order will soon be used by new ControlObjects to load them
      from controllers.

  Bad:

      address comments from PR review

- **Separate formatting commits** from logic commits. Use
  `SKIP=clang-format` to commit logic first, then commit formatting.
- Use `git commit --fixup=<sha>` for changes that should be squashed before
  merge. Only squash when asked by a reviewer.
- Refer to
  [How to Write a Git Commit Message](https://chris.beams.io/posts/git-commit/)
  for more details.

## Pull Requests

- Keep PRs focused — no unrelated formatting, config, or refactoring mixed
  in.
- Target the correct branch: bug fixes → stable (e.g. `2.5`), features →
  `main`. Controller mappings go to the stable branch unless they use new
  features from `main`.
- If your PR changes the GUI, post before-and-after screenshots.
- **Do not rebase** without agreement from your reviewer. Rebasing loses the
  connection between inline review comments and the code. If you need to
  correct a recent commit that hasn't been reviewed yet, amending is fine.
- CI must pass (builds + tests) before a PR can be merged.
- No `.DS_Store`, IDE files, or other untracked artifacts.
- If you are helping with someone else's PR that is not yet merged, open a PR
  targeted at their fork and leave a comment on the upstream PR linking to
  yours.

## Code Review

- **Review others' PRs.** Even if you're unfamiliar with the area, you can
  build the branch, test it, and give UX feedback. When others review your
  pull requests, please return the favor.
- Reviewers commonly flag:
  1. Pre-commit / CI not passing.
  2. Missing `std::chrono::duration` for time values.
  3. Manual `static_cast<int>` instead of `Q_ENUM` +
     `QVariant::fromValue`.
  4. Insufficiently descriptive commit messages — document *why*, not just
     *what*.
  5. SVG assets not matching existing conventions (full-size, borderless).

## Merging

Merging will be performed by a core member after the pull request has LGTM.
A reviewer who is a core member is allowed to also merge the PR.
Be sure to add a message indicating if your PR is not ready to merge.

## Core Team

Core team members have write access to the
[upstream mixxxdj repositories](https://github.com/mixxxdj/). See the
[Contribution Guidelines](https://github.com/mixxxdj/mixxx/wiki/Contribution-Guidelines)
wiki page for core-team policies on merging, force-pushing, and branch
management.

## Further Reading

- [Using Git](https://github.com/mixxxdj/mixxx/wiki/Using-Git) — branching,
  rebasing, and common workflows
- [Coding Guidelines](https://github.com/mixxxdj/mixxx/wiki/Coding-Guidelines)
  — full C++ style guide
- [Contribution Guidelines](https://github.com/mixxxdj/mixxx/wiki/Contribution-Guidelines)
  — complete contributor and core-team policies
- [Developer Guide](https://github.com/mixxxdj/mixxx/wiki/Developer-Guide)
- [Mixxx Wiki](https://github.com/mixxxdj/mixxx/wiki)
