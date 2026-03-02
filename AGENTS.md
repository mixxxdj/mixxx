# AGENTS.md — Mixxx Project Instructions

See [README.md](README.md) for a project overview, and
[CONTRIBUTING.md](CONTRIBUTING.md) for build instructions, code style,
pre-commit setup, Git workflow, and pull request guidelines.

## Build

See [CONTRIBUTING.md](CONTRIBUTING.md) for platform-specific dependency setup.
Use these flags for a development build:

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DDEBUG_ASSERTIONS_FATAL=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build . --parallel $(nproc)
ctest  # run tests (Google Test)
```

## Code Style

See [CONTRIBUTING.md](CONTRIBUTING.md) for full conventions. Key points for
AI-generated code:

- **Formatting**: `.clang-format` (Google base, 4-space indent, 8-space continuation). Only format new/modified code — never mass-reformat.
- **Separate formatting commits** from logic commits.

### C++ Quick Reference

- Classes: `CamelCase`. Methods: `camelBack()`. Members: `m_prefix`. Pointers: `pPrefix`. Constants: `kPascalCase`. Enums: `enum class CamelCase`.
- CO/setting keys: `snake_case`.
- No naked `new`/`delete` — use `std::make_unique`, `std::make_shared`, or `make_parented`.
- `VERIFY_OR_DEBUG_ASSERT(cond) { recovery; }` for defensive checks.
- `override` on all virtual overrides; omit redundant `virtual`.
- `QStringLiteral("...")` for string literals. `tr("...")` for translatable strings.
- No `goto`. No `Q_UNUSED` (use unnamed params). No C-style enums.
- `#pragma once`, not include guards.
- Include order: matching header → system → Qt → library deps → Mixxx local → forward decls. Alphabetical within groups.
- Wrap new code in `namespace mixxx {}`. Anonymous namespace for file-local helpers in .cpp.
- Non-const ref out-params: use pointers, not references (legacy convention).
- Lambdas: use carefully — they get extra review scrutiny for lifetime/control-flow issues.

## Git & PR Workflow

See [CONTRIBUTING.md](CONTRIBUTING.md) for full details. Key rules:

- One branch per feature/bugfix. Every commit must build.
- Small commits. Imperative commit messages, 72-char wrap, describe what + why.
- Bug fixes → stable branch (e.g. `2.5`). Features → `main`.
- Don't rebase without reviewer agreement. Use `--fixup` commits, only squash before merge if requested.
- Keep PRs focused — no unrelated formatting, config, or refactoring changes mixed in.

## Key Architecture

- **ControlObject/ControlProxy**: `[Group], key_name` inter-component communication.
- **Engine thread**: Real-time audio — no allocations, no locks, no Qt signals.
- **parented_ptr/make_parented**: Qt object-tree ownership. Object must get a parent before `parented_ptr` destructs.

## Project Layout

```text
src/          C++ source (engine/, controllers/, library/, mixer/, effects/, qml/, preferences/, util/, test/)
res/          Resources (controllers/ JS/XML, skins/, qml/)
cmake/        CMake modules
tools/        Python helper scripts
```
