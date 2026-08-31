# AGENTS.md — Mixxx Project Instructions

See [README.md](README.md) for a project overview, and
[CONTRIBUTING.md](CONTRIBUTING.md) for build instructions, code style,
pre-commit setup, Git workflow, and pull request guidelines.

## AI Agent Policy

> **Important:** The Mixxx project only accepts contributions driven by human developers.
> Automated AI agents acting autonomously on a user's behalf are not welcome and may result
> in pull requests being closed without comment.
> But using AI tools that **assist** the human contributor with code generation, refactoring,
> documentation and reviewing the work before submission is welcome.

### No Autonomous Pull Requests

AI agents **must not** open, update, or re-open pull requests autonomously on behalf of a user.
All pull requests must be explicitly reviewed, approved, and submitted by the human developer
themselves. An agent may prepare and stage changes locally, but `git commit`, `git push`, and PR
creation must be deliberate human actions. Before this, a test of the code change by a human,
using a real DJ setup, is required.

### No Automated Responses to PR Review Comments

AI agents **must not** post replies to review comments, questions, or change requests left on
a pull request or its commits. All communication in the PR thread must come directly from the human contributor.

### AI Code-Reviews

AI agents **may** and **should** perform code reviews on explicitly request in scope of the PR.

### No Autonomous Creation of Issues, Bug Reports or Feature Requests

AI agents **must not** open, update, or re-open Issues, Bug Reports or Feature Requests autonomously
or on behalf of a user.
All Issues, Bug Reports, and Feature Requests must be explicitly reviewed, approved, and submitted by
the human user themselves.

### AI-Generated Text Must Carry a Disclaimer

Any text (PR description, commit message body, code comments, documentation) that was written
autonomously by an AI Agent **must** be framed with an disclaimer that the text is autonomously generated
by the AI Agent, at both the start and the end of that text block.

## Key Architecture

- **ControlObject/ControlProxy**: `[Group], key_name` inter-component communication.
- **Engine thread**: Real-time audio — no allocations, no locks, may emit Qt signals but cannot receive them.
- **parented_ptr/make_parented**: Qt object-tree ownership. Object must get a parent before `parented_ptr` destructs.

## Project Layout

```text
src/          C++ source (engine/, controllers/, library/, mixer/, effects/, qml/, preferences/, util/, test/)
res/          Resources (controllers/ JS/XML, skins/, qml/)
cmake/        CMake modules
tools/        Python helper scripts
```
