# Contributing to Mixxx

Thank you for contributing to [Mixxx](https://mixxx.org/)! Mixxx is a free, open-source DJ
application built with C++17, Qt 6, and JavaScript (for controller mappings). We are global, all
volunteer team that works by consensus and we are excited to have you join us. Your work helps DJs
all over the world!  This document specifies technical aspects of our workflow. For social aspects
please refer to [CODE_OF_CONDUCT.md](https://github.com/mixxxdj/mixxx/blob/main/CODE_OF_CONDUCT.md)
in this repository. We encourage you to introduce yourself on our [Zulip
chat](https://mixxx.zulipchat.com/) before starting to contribute code to Mixxx.

**Contents:**

1. [Orientation](#orientation)
   - [Important Guidelines and Policies](#important-guidelines-and-policies)
   - [Git Repositories](#git-repositories)
2. [Getting Started](#getting-started)
3. [Pre-commit Setup](#pre-commit)
4. [Code Style](#code-style)
   - [Code Formatting](#code-formatting)
   - [C++ Conventions](#c-conventions)
   - [Advanced Conventions](#advanced-conventions)
   - [JavaScript (Controller Mappings)](#javascript-controller-mappings)
   - [QML](#qml)
5. [Git Workflow](#git-workflow)
6. [Pull Requests](#pull-requests)
7. [Code Review](#code-review)
8. [Merging](#merging)
9. [Core Team](#core-team)
10. [Further Reading](#further-reading)

## Orientation

We have lots more helpful information for users and developers on the [Mixxx wiki](https://github.com/mixxxdj/mixxx/wiki) and elsewhere, including [detailed build instructions](https://github.com/mixxxdj/mixxx/wiki#compile-mixxx-from-source-code). This document should be enough to get you off the ground.

### Important Guidelines and Policies

- [Git Workflow](#git-workflow)
- [Coding Guidelines](https://github.com/mixxxdj/mixxx/wiki/Coding-Guidelines) & [Setting up `pre-commit`](#pre-commit)
- [Minimum Requirements Policy](https://github.com/mixxxdj/mixxx/wiki/Coding-Guidelines)
- [Internationalization Workflow](https://github.com/mixxxdj/mixxx/wiki/Internationalization)
- [Release Checklist](https://github.com/mixxxdj/mixxx/wiki/Release-Checklist-2.5.0)

### Git Repositories

This repository contains the Mixxx source code, skins, controller mappings, and some helpful scripts. We have a few other Git repositories too:

- [mixxxdj/website](https://github.com/mixxxdj/website): content for the main [mixxx.org](https://mixxx.org/) website which is generated with the [Pelican static site generator](https://getpelican.com/)
- [mixxxdj/manual](https://github.com/mixxxdj/manual): content for the Mixxx manual, which uses [Sphinx](https://www.sphinx-doc.org/)
- [mixxxdj/vcpkg](https://github.com/mixxxdj/vcpkg): dependencies packaged for macOS, Windows, Android and Linux (statically linked builds)

All of these are automatically built and deployed by [GitHub Actions](https://github.com/features/actions) whenever a change is committed. You are welcome to open a pull request in any of these repositories.

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

     Detailed build instructions for each target OS can be found [on the wiki](https://github.com/mixxxdj/mixxx/wiki#compile-mixxx-from-source-code)

6. **Run tests**:

    `ctest`

## Pre-commit Setup {#pre-commit}

Install [pre-commit](https://pre-commit.com/#install) to automatically ensure that your commits comply with our code style for both C++ and JavaScript. This saves time reviewing so we don't have to point out nitpicky style issues. Once you have pre-commit installed on your computer, set it up in your local Git repository:

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

## Git Workflow

- **One branch per feature or bug fix**. Each feature/bug fix should be done on its own Git branch so they can be reviewed and merged independently. Refer to [Using Git](https://github.com/mixxxdj/mixxx/wiki/using-git) for how to do this. Please ask for help on [Zulip](https://mixxx.zulipchat.com/) if you have questions about using Git after reading that page.
- **Every commit must build.** The smaller the commit, the easier it is to review. This is also important so [git bisect](https://git-scm.com/book/en/v2/Git-Tools-Debugging-with-Git#_binary_search) works. If you have lots of changes that you need to commit, a [GUI Git client](https://git-scm.com/downloads/guis) can be helpful for picking out specific changes for multiple small commits.
- **Small PRs.** The smaller the pull request, the easier it is to review and revert.
- Write commit messages in the **imperative mood**. Commit messages should succinctly describe what is changed in that commit and why. Lines should wrap at 72 characters so they show fully in GitHub and other Git tools. For example, this is a good commit message:

  Good:

      DlgPrefEffects: add QListWidget to set order of chains

      This order will soon be used by new ControlObjects to load them
      from controllers.

  Not so good:

      address comments from PR review

  Refer to [How to Write a Git Commit Message](https://chris.beams.io/posts/git-commit/) for more details.

- **Separate formatting commits** from logic commits. Use
  `SKIP=clang-format` to commit logic first, then commit formatting.
- Use `git commit --fixup=<sha>` for changes that should be squashed before
  merge. Only squash when asked by a reviewer.
- Generally, prefer merging over rebasing. Do not rebase unless you have discussed that with whoever is reviewing the pull request. When you rebase a branch with an open pull request, it is no longer possible to distinguish your latest changes from already reviewed parts, resulting in unnecessary extra work for the reviewer. Comments made directly to a single commit will be lost. Rebased commits are likely not tested and there is a risk that building fails in a later `git bisect` run. If you want to correct minor mistakes with a rebase within a few minutes of pushing commits, that is okay as long as no one has started reviewing those commits yet. A `git commit --amend` is possible at any time as long the commit has the limited scope of one topic.
- If you plan to rebase your branch before merge to eliminate forth and back commits or such, you can commit them with the --fixup option or manually add "fixup!" as the first word in the commit message. This prevents mergeing to upstream before the rebase.
- If you are helping with someone else's pull request that is not yet merged, open a pull request targeted at their fork. Leave a comment on the upstream pull request (which targets mixxxdj/mixxx) with a link to your pull request so other Mixxx contributors are aware of your changes.
- Low risk bug fixes should be targeted at the stable branch (e.g., `2.3`). However, bug fixes for the stable branches must have a direct impact on users. If you spot a minor bug reading the code or only want to clean up the code, target that at the `main` or beta branch.
- Controller mappings should be targeted at the stable branch unless they use features that are new in the beta or `main` branch.
- If you are making changes to the GUI with a pull request, please post before and after screenshots of the changes.
- Please help review other people's pull requests. When others review your pull requests, please return the favor. The continued progress of Mixxx depends on all of us working together. Even if you are not familiar with the area of the code being changed in a pull request, you can be helpful by building the branch, verifying that it works as described, and commenting with feedback about the user experience design.
- If you demonstrate good coding skills, help review pull requests, contribute major features, and show a commitment to Mixxx over time, we may invite you to the core team.

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

- Enable [two-factor authentication (2FA)](https://help.github.com/en/github/authenticating-to-github/securing-your-account-with-two-factor-authentication-2fa) for your GitHub account.
- *Never* force push to an upstream repository (mixxxdj). If you encounter an error from Git saying you would need to force push, stop what you are doing and discuss the situation on Zulip.
- All non-trivial contributions should be made with a pull request, just like any other contributor who does not have write access. Do not merge your own pull requests.
- You may merge someone else's pull request as the only reviewer if no other contributors have expressed concerns about the changes or said they want to review the code. Please do not merge pull requests immediately; allow at least a day or two for others to comment. Remember we are all volunteers and cannot respond to everything immediately.
- If there is disagreement about changes in a pull request, do not merge it until a consensus has been reached.
- Check CI to ensure builds work and tests pass before merging. If CI timed out, either manually restart it or build the branch and run the tests locally before merging.
- When you merge a pull request to a stable branch, merge the stable branch to the beta branch afterwards. If you merge a pull request to a beta branch, merge the beta branch to `main` afterwards. When backporting, cherry-pick or rebase rather than merge.
- Merge PRs using a merge, to keep the original commits valid. Keep the default commit message "Merge pull request ..." with the reference to the pull request. In case where the PR contains broken (non-building) commits, back-and-forth commits or commits without a meaningful commit message that are not worth keeping, ask the author to squash the commits before merge. Alternatively you may ask the contributor to check "Allow edits and access to secrets by maintainers". Then you can squash locally or use the `/softfix` comment to squash remotely. See [Softfix](https://github.com/daschuer/softfix/?tab=readme-ov-file#softfix-a-pull-request)
- Default to open; only post in the private Zulip stream for discussions that have a reason to be private. Most of the time, post to a public Zulip stream so anyone can participate in the discussion.
- When Mixxx participates in Google Summer of Code, you may volunteer as a mentor if you like.
- Benchmark runs will occur on release branches only by default. It also possible to request a benchmark run in the PR by using the comment `/benchmark`.

## Further Reading

- [Using Git](https://github.com/mixxxdj/mixxx/wiki/Using-Git) — branching,
  rebasing, and common workflows
- [Coding Guidelines](https://github.com/mixxxdj/mixxx/wiki/Coding-Guidelines)
  — full C++ style guide
- [Contribution Guidelines](https://github.com/mixxxdj/mixxx/wiki/Contribution-Guidelines)
  — complete contributor and core-team policies
- [Developer Guide](https://github.com/mixxxdj/mixxx/wiki/Developer-Guide)
- [Mixxx Wiki](https://github.com/mixxxdj/mixxx/wiki)
