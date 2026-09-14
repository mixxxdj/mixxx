# Mixxx End-to-End UI Tests

This directory contains BDD (Behavior-Driven Development) tests that exercise the
Mixxx QML user interface end-to-end. Tests drive a real Mixxx instance via
spix XML-RPC and verify UI state, making them effective at catching visual
regressions, broken interactions, and layout issues that unit tests miss.

## What is BDD?

BDD is a testing approach where scenarios are written in natural language using
the **Given/When/Then** structure:

```gherkin
Scenario: Deck play button toggles playback
  Given a track is loaded on deck 1
  When  I click the play button on deck 1
  Then  the deck 1 should be playing
```

This makes tests readable as living specifications -- contributors can add or
modify scenarios without understanding the underlying automation layer.

## What is Gherkin?

Gherkin is the plain-language syntax used in `.feature` files. Key keywords:

| Keyword        | Purpose                                            |
|----------------|----------------------------------------------------|
| `Feature:`     | Names the area under test                          |
| `Background:`  | Steps run before **every** scenario (common setup) |
| `Scenario:`    | A single test case                                 |
| `Given`        | Preconditions (initial state)                      |
| `When`         | Actions (user interactions)                        |
| `Then`         | Assertions (expected outcomes)                     |
| `And`          | Continues the previous keyword                     |

Feature files live under `features/` and are auto-discovered by CMake -- no
registration needed when adding a new file.

## What is Behave?

[Behave](https://behave.readthedocs.io/) is a Python BDD framework that:

1. Parses `.feature` files into scenarios
2. Matches step text (`Given a track is loaded on deck 1`) against step
   definition functions
3. Executes matched functions in order, passing a shared `context` object

Each step is a Python function decorated with `@given`, `@when`, or `@then`.
Decorators are a Python feature where a function is wrapped with `@decorator`
syntax -- see the [Python docs on decorators](https://docs.python.org/3/glossary.html#term-decorator)
for details. Behave uses them to register step implementations:

```python
from behave import given, when, then

@given("a track is loaded on deck {deck:d}")
def step_load_track(context, deck):
    # deck is automatically parsed as an integer from the step text
    ...
```

Behave supports [parameterized step types](https://behave.readthedocs.io/en/latest/tutorial/#step-matchers)
like `{name}` (string), `{count:d}` (integer), `{ratio:f}` (float) which are
matched and passed as arguments to the step function.

## Test Tags

Scenarios can be tagged to control runner behavior:

- **`@xpass`** -- Marks **flaky tests** (intermittent failures due to timing,
  platform quirks, etc.). The runner suppresses captured output for these and
  labels them `FLAKY` in video chapters if they fail. Use this when the test
  logic is correct but the environment is unreliable.

- **`@xfail(gh_issue=N)`** -- Marks **known bugs**. Always include the
  `gh_issue=` annotation linking to the relevant GitHub issue so failures are
  traceable. The runner labels chapters `EXPECTED FAILURE` or `UNEXPECTED PASS`
  and prints the issue link on failure.

## Writing Steps

Step text should read like a user specification, not an implementation
checklist. The three Gherkin keywords have different rules:

**Given (preconditions)** -- Describe the state the user expects, not how it
was achieved:

| Good | Notes |
| ------ | ------- |
| `the tempo rate on deck 1 is set to 0.0` | Describes state; internally calls `_set_control_value` |
| `a track is loaded on deck 1` | State-oriented |
| `the sync_on deck 1 is disabled` | State-oriented |

**When (actions)** -- Prefer language-oriented actions. Implementation-specific
text is acceptable when no clean behavior-oriented alternative exists:

| Good | Bad | Why |
| ------ | ----- | ----- |
| `I drag the column "Artist" before the column "Title"` | `I invokeMethod moveColumn with indices 2,0` | Language-oriented describes the user intent |
| `I click the "play" button on deck 1` | -- | Fine as-is, this is what a user would say |
| `I set the rate of deck 1 to 5%` | -- | Acceptable: no user-facing language maps cleanly to a precise CO value |

**Then (assertions)** -- Always behavior-oriented. Never expose control object
names, internal values, or implementation details. Describe what the user sees
or expects:

| Good | Bad | Why |
| ------ | ----- | ----- |
| `the loop should be enabled on deck 1` | `the loop_enabled CO should be 1` | Use domain language, not CO keys |
| `the deck 1 should be playing` | `the play control value should be 1.0` | User sees "playing", not a number |
| `the column "Artist" should appear before the column "Title"` | -- | Spatial, visible assertion |
| `the beatloop size of deck 1 should have changed` | `the beatloop_size should be 0.5` | "changed" is sufficient; exact values belong in When |

## How the Setup Works

The test runtime has three layers:

1. **`mixxx-test --serve`** -- A special build of the Mixxx binary that starts
   the full QML UI and a spix XML-RPC server on port 9000. Custom C++ command
   handlers (`getControlValue`, `setControlValue`, `loadTrack`, `library`,
   `reloadQml`) let the test harness read/write engine state and manage the
   application.

2. **Python test infrastructure** -- `mixxx_profile.py` manages temporary Mixxx
   profiles and the `MixxxProcess` lifecycle (start, wait for RPC, stop).
   `steps/mixxx_steps.py` provides the step definitions that drive the UI via
   spix. `environment.py` hooks manage session state across scenarios.

   Scenarios sharing the same profile and sound-device setup reuse one running
   Mixxx instance (no QML reload between them), so a scenario that relies on
   the default window size or library columns should declare it with
   `Given the window size is default` and/or
   `Given the library columns are in their default state`.

3. **Test runner** (`mixxx_test_runner.py`) -- Orchestrates everything: sets up
   a headless virtual display (Xvfb, or cage plus wf-recorder for the Xwayland
   backend), or offscreen QPA when no monitor is available, optionally records
   the display via ffmpeg or wf-recorder, downloads test audio tracks on first
   run, and invokes behave. Pass `--headless` to enable a virtual display and
   `--display-backend={auto,xvfb,xwayland}` to pick which one (`auto` prefers
   `xwayland` when `cage` and `wf-recorder` are available, then falls back to
   Xvfb, then offscreen). Linux CI runs with `MIXXX_TEST_DISPLAY_BACKEND=xwayland`
   by default; install `cage` and `wf-recorder` to use it locally.

**Audio dataset**: `test_tracks.json` contains metadata for 141 Creative
Commons tracks (from Pixabay). The runner downloads the first
`LIBRARY_TRACK_COUNT` (50) of them on first run, into a
`mixxx-test-tracks` folder inside the platform temp directory (`/tmp` on Linux,
`%TEMP%` on Windows), and reuses that cache on later runs. Set
`MIXXX_TEST_TRACKS_DIR` to point at a pre-seeded cache instead.

**CMake integration**: At configure time, CMake creates a Python venv, installs
behave, globs `features/*.feature`, and registers each as a `mixxx-behave-*`
CTest target with labels `e2e;ui;qml`.

## Quick Start

Run a single feature with the built binary:

```bash
src/test/behave/.venv/bin/python \
  src/test/behave/mixxx_test_runner.py \
  --binary build/mixxx-test \
  src/test/behave/features/deck.feature
```

On Windows, use `tools/Run-BehaveScenariosOnWindows.ps1` instead — a GUI
`mixxx-test` cannot start from an SSH session, so the script routes it through
`Start-InteractiveProcessOnWindows.ps1` and onto the logged-on user's desktop.
See
[AGENTS.md](AGENTS.md#running-the-tests-on-windows) for prerequisites, the
quoting rules, and how to read the results.

## CI Video Output

When run in CI, tests produce an `.mkv` video recording of the display.
Each scenario becomes a **chapter** in the video via ffmpeg chapter metadata,
making it easy to jump directly to a specific scenario's playback. The
`after_scenario` hook in `environment.py` records timing and outcome; after all
scenarios complete, the runner muxes this data into the video as chapter markers.
Videos are uploaded as CI artifacts for review.

Every `mixxx-test` spawn also writes its complete application output
(`--log-level debug`) to `mixxx-output.<pid>.log` in the artifacts directory,
so crashes and restarts can be investigated without re-running. (*This
paragraph was drafted autonomously by an AI agent for a human reviewer to
check, amend, and submit.*)
