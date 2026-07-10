# AGENTS.md — Mixxx Behave E2E UI Tests

> **New to BDD or this test suite?** See [README.md](README.md) for a
> human-friendly introduction to Gherkin, Behave, the step DSL, test tags, and
> the overall setup. This file is the detailed technical reference.

## Architecture Overview

```text
mixxx_test_runner.py   ← CTest entry point (CMake add_test target "mixxx-behave-*")
  │
  ├─ mixxx_profile.py   ← Profile creation, track download, MixxxProcess lifecycle
  ├─ environment.py    ← behave before_all/after_all/scenario hooks + autoretry
  ├─ steps/
  │   └─ mixxx_steps.py  ← Gherkin step definitions (Given/When/Then)
  ├─ features/
  │   ├─ deck.feature
  │   └─ library.feature
  └─ test_tracks.json   ← Static track metadata (url, title, artist, bpm, etc.)
```

## How to Run Tests (Iterating)

### Single feature, headed (local dev — uses your display)

```bash
src/test/behave/.venv/bin/python \
  src/test/behave/mixxx_test_runner.py \
  --binary build/mixxx-test \
  src/test/behave/features/library.feature
```

### Single feature, offscreen (no display needed)

```bash
# Using --headless flag (preferred — uses Xvfb virtual display on Linux)
src/test/behave/.venv/bin/python \
  src/test/behave/mixxx_test_runner.py \
  --binary build/mixxx-test \
  --headless \
  src/test/behave/features/library.feature

# Or via QPA env var (equivalent, but no video recording)
QT_QPA_PLATFORM=offscreen src/test/behave/.venv/bin/python \
  src/test/behave/mixxx_test_runner.py \
  --binary build/mixxx-test \
  src/test/behave/features/library.feature
```

### Single feature, named scenario only (fastest iteration)

behave forwards extra args after the feature file. Use `-n` (a.k.a. `--name`) to
run just one scenario:

```bash
src/test/behave/.venv/bin/python \
  src/test/behave/mixxx_test_runner.py \
  --binary build/mixxx-test \
  --headless \
  src/test/behave/features/library.feature \
  -n "Maximize library with toggle"
```

### Custom retry count

By default, each scenario is retried up to 3 times on failure. Override with
`--retry N`:

```bash
src/test/behave/.venv/bin/python \
  src/test/behave/mixxx_test_runner.py \
  --binary build/mixxx-test \
  --headless \
  --retry 5 \
  src/test/behave/features/library.feature
```

### All features via CTest

```bash
# Headed (local dev, uses existing display)
ctest -R mixxx-behave- --output-on-failure

# Headless (CI — sets MIXXX_TEST_HEADLESS=1 for Xvfb+ffmpeg)
MIXXX_TEST_HEADLESS=1 ctest -R mixxx-behave- --output-on-failure

# Custom retry count (default: 3)
MIXXX_BEHAVE_RETRY=5 ctest -R mixxx-behave- --output-on-failure
```

### Tips for iterating

- **Application output artifact**: every `mixxx-test` spawn writes its
  complete stdout/stderr (`--log-level debug`) to
  `src/test/behave/artifacts/mixxx-output.<pid>.log` — one file per spawn,
  wrapped in `===== spawn =====` / `===== exited ... code=... =====`
  banners, so crashes and kill/restart cycles can be investigated after the
  fact. The live terminal does **not** show this output; on a startup
  failure the runner prints a bounded tail (last 40 lines) plus the path to
  the full file. On Windows the launcher's stream additionally mirrors the
  same output — see the Debugging section.
- **Video recording**: When `--record` is passed (or `MIXXX_TEST_RECORD=1`),
  ffmpeg records the X display. Chapter metadata is embedded into the video
  from behave's scenario lifecycle hooks.
- **Session reuse**: scenarios sharing the same `Background` profile type reuse
  the same `mixxx-test` process — only the first scenario pays startup cost.
- After editing QML or `main.cpp`, rebuild:
  `cmake --build build --target mixxx-test -j$(nproc)`
- After editing Python (`steps/`, `environment.py`, `mixxx_profile.py`, runner), no
  rebuild needed — just re-run the command.
- If spix patches fail to apply during build, delete
  `build/libspix-prefix` and `build/lib/libspix-install`, then reconfigure.

## Running the Tests on Windows

> *This section was drafted autonomously by an AI agent (for a human reviewer
> to check, amend, and submit).*

Everything above assumes a POSIX shell and a real terminal. Windows needs a
launcher, and the invocation is different enough to be worth its own section.

### Why a launcher is required

`mixxx-test` is a GUI application. An SSH session on Windows lands in session 0
(non-interactive), where no window station/desktop exists, so the process either
fails to start or starts with no usable display.
`tools/Start-InteractiveProcessOnWindows.ps1` works around this: it registers
a temporary scheduled task with `-LogonType Interactive` and the currently
logged-on user's identity, which runs the child on the real desktop, then
streams the child's stdout/stderr back to the caller while polling for exit.

Consequences worth remembering:

- A human must be **logged in on the console**. If the machine is locked or
  nobody is logged in, the launcher throws
  `No interactive desktop user is currently logged in.` Check with
  `query session` — the `console` row must show `Active`.
- `-WaitMode Exit` blocks until the run finishes (correct for CI/SSH).
  `-WaitMode Start` detaches and is only useful for interactive tinkering.
- **Killing the SSH connection orphans the run.** The launcher cleans up its
  scheduled task and temp dir in `finally`, which only runs if the launcher
  itself survives. Drop the SSH connection (Ctrl-C, network blip, closing the
  terminal) and both leak, and they accumulate:

  ```powershell
  # Find leftovers
  Get-ScheduledTask -TaskName 'SSH-Interactive-Launcher-*'
  Get-ChildItem "$env:TEMP\SSHLauncher"

  # Remove them
  Get-ScheduledTask -TaskName 'SSH-Interactive-Launcher-*' |
      Unregister-ScheduledTask -Confirm:$false
  Remove-Item "$env:TEMP\SSHLauncher\*" -Recurse -Force
  ```

  A dropped connection means **no result** — re-run it, and check the leftovers
  before the next run so you are not looking at a stale screen.

### Prerequisites

| Requirement | Path / check |
| --- | --- |
| Behave venv | `src\test\behave\.venv\Scripts\python.exe` (note: `Scripts`, not `bin`) |
| Test binary | `build\mixxx-test.exe`, built from the `mixxx-test` target |
| Launcher | `tools/Start-InteractiveProcessOnWindows.ps1` |
| Wrapper | `tools/Run-BehaveScenariosOnWindows.ps1` |
| Console session | `query session` shows `console ... Active` |
| ffmpeg (only for `-Record`) | resolved by the wrapper, see below |

ffmpeg is usually **not on `PATH`** on a Windows build machine, and the wrapper
does not look for it unless you ask. `-FfmpegBin` defaults to the empty string,
which short-circuits the lookup entirely, so a plain `-Record` run leaves
`MIXXX_FFMPEG_BIN` unset and the runner has to find ffmpeg itself. Pass
`-FfmpegBin` to change that; the resolution order is:

1. If `-FfmpegBin` is a path that exists, use it.
2. Otherwise `ffmpeg.exe` on `PATH`.
3. Otherwise the first `ffmpeg.exe` found by a recursive glob of
   `<repo>\buildenv`, where `buildenv` puts the Mixxx dependencies (this is
   where it usually turns up, and the glob costs well under a second).
4. Otherwise the wrapper warns `ffmpeg not found; -Record will fail` and sets
   nothing, leaving the failure to the runner.

So a literal path is not required — any non-empty placeholder is enough to
trigger the PATH/buildenv search, and the common case on a Mixxx build machine
is just `-FfmpegBin ffmpeg`. Passing `-FfmpegBin ""` disables the search
entirely. This matters because the `buildenv` glob is the wrapper's alone: the
runner's own `_find_ffmpeg` checks only an explicit path and then `PATH`, so
without `-FfmpegBin` a machine that has ffmpeg solely under `buildenv` fails
with `ffmpeg not found. Cannot perform video recording!` and exit 1.

There is no usable headless story on Windows. `--headless` and
`--display-backend` are Linux-only: on any other platform the runner prints
`--headless is Linux-only (current platform: win32) - using offscreen
platform` and then **sets `QT_QPA_PLATFORM=offscreen`**. That is not a no-op
— spix cannot see an offscreen widget tree, so passing `--headless` on Windows
breaks the very interactions the tests are driving. Combining `--headless`
with `--record` fails outright (`Using offscreen QPA. Cannot perform video
recording!`, exit 1). Just omit both flags. The tests drive the real desktop
through the `windows` QPA platform, so whatever is on screen is what spix sees.
A resolution change, a scaling change, or a different skin will affect results
— match the machine you validated on when comparing runs.

### Running (preferred): `Run-BehaveScenariosOnWindows.ps1`

`tools/Run-BehaveScenariosOnWindows.ps1` is the supported entry point. It
builds the runner argv as an array, applies the defaults, and calls the
launcher:

```powershell
# Defaults: the eight library-settings scenarios, retry 3
.\tools\Run-BehaveScenariosOnWindows.ps1

# One scenario — fastest iteration loop
.\tools\Run-BehaveScenariosOnWindows.ps1 -Name 'An empty music directory can be removed'

# Several scenarios at once
.\tools\Run-BehaveScenariosOnWindows.ps1 -Name 'A single directory with tracks can be added and saved','An empty music directory can be removed'

# A different feature file. The path is passed straight to behave and is
# resolved against the working directory, which the wrapper sets to the repo
# root — so it must include the src\test\behave prefix.
.\tools\Run-BehaveScenariosOnWindows.ps1 -Feature src\test\behave\features\settings\library.feature -Name 'History cleanup threshold can be changed'

# Capture video into src\test\behave\artifacts
# NOTE: needs -FfmpegBin (see above), and on Windows the run currently ends
# with a KeyboardInterrupt during teardown even when every scenario passed.
.\tools\Run-BehaveScenariosOnWindows.ps1 -Name 'An empty music directory can be removed' -Record -FfmpegBin ffmpeg

# Keep this run's artifacts separate from a concurrent/previous run
.\tools\Run-BehaveScenariosOnWindows.ps1 -Label wip -Name 'An empty music directory can be removed'

# Point at a checkout in another location
.\tools\Run-BehaveScenariosOnWindows.ps1 -RepoRoot D:\other\mixxx -Name 'An empty music directory can be removed'
```

`-Name` takes scenario *names*, not line numbers, and is passed through as
behave `-n`. `-Label` only affects artifact filenames.

> **Feature paths are not repo-relative.** A path like
> `-Feature features/settings/library.feature` looks reasonable and fails:
> `behave.exception.ConfigError: No steps directory in
> 'D:\dev\mixxx\features\settings\library.feature'`. Use
> `src\test\behave\features\...`, which is also what the wrapper's own default
> uses.

### Running (raw): `Start-InteractiveProcessOnWindows.ps1` directly

Equivalent, and useful when you want flags the wrapper does not expose:

```powershell
powershell.exe -ExecutionPolicy Bypass -File .\tools\Start-InteractiveProcessOnWindows.ps1 `
    -Program "D:\dev\mixxx\src\test\behave\.venv\Scripts\python.exe" `
    -Arguments "src\test\behave\mixxx_test_runner.py --no-capture --retry 3 --binary build\mixxx-test.exe src\test\behave\features\settings\library.feature -n 'An empty music directory can be removed'" `
    -WorkingDirectory "D:\dev\mixxx" `
    -WaitMode Exit -TimeoutSeconds 1800
```

> **Quoting trap.** `-Arguments` is a single string that is re-parsed by the
> Windows C runtime when the child starts. Scenario names **must** be quoted or
> every word of the name becomes its own argument and behave fails with
> `unrecognized arguments:`. The launcher rewrites `'` to `"` for you, so use
> single quotes in `-Arguments`.
>
> Build this string **in PowerShell only**. Typing it in `cmd.exe`, or
> generating it from a bash/SSH wrapper, strips the quotes before they reach
> PowerShell and the failure is silent and confusing. That is why
> `Run-BehaveScenariosOnWindows.ps1` exists — prefer it, and keep the quoting
> inside PowerShell.

### Driving a Windows host from Linux

When the checkout you are editing is on Linux but the test machine is Windows,
`tools/win_ssh_run.py` carries the invocation over SSH and leaves the actual
GUI launch to the interactive scheduled task. It needs `paramiko`, which is
**not** in the `src/test/behave/.venv` — run it with a `python3` that has
`paramiko` installed. Credentials come from the environment only, never from a
file in the repo:

```bash
export MIXXX_WIN_HOST=192.168.1.10
export MIXXX_WIN_USER=someuser
export MIXXX_WIN_PASSWORD=...
export MIXXX_WIN_ROOT='D:\dev\mixxx'   # optional, only for relative paths
```

```bash
# Run the wrapper remotely, mirroring the remote output into a local log
python3 tools/win_ssh_run.py run /tmp/behave.log 1800 -q -- \
    'powershell -NoProfile -ExecutionPolicy Bypass -File D:\dev\mixxx\tools\Run-BehaveScenariosOnWindows.ps1 -Name "An empty music directory can be removed"'

# Push edited sources during a debug loop, then fetch the artifacts back
python3 tools/win_ssh_run.py sync src/test/behave
python3 tools/win_ssh_run.py get 'D:\dev\mixxx\src\test\behave\artifacts\behave-output-default.txt' /tmp/out.txt
```

Subcommands are `run LOG TIMEOUT -- COMMAND`, `put LOCAL REMOTE`, `get REMOTE
LOCAL` and `sync LOCAL_DIR`. Two properties matter when scripting against it:

- **`run` returns the remote exit status,** or `124` if the local timeout fires
  first. Output is written to `LOG` and streamed unless `-q`; a UTF-8 character
  split across a chunk boundary is carried over instead of being mangled, which
  matters because the logs are UTF-8 and may contain non-ASCII titles.
- **`sync` is one-directional and does overwrite.** It creates directories as
  needed, and skips a file only when the remote mtime already matches the local
  mtime to within one second. Otherwise it uploads — including over a remote
  file that is *newer* than the local one. Nothing is ever deleted, but nothing
  is protected either, so do not point it at a tree with local work you care
  about, and commit before syncing a file you may need back. Symlinks are
  skipped, and so are `.venv`, `venv`, `__pycache__`, `.git` and `*.pyc` —
  uploading a Linux virtualenv over the Windows one replaces `pyvenv.cfg` with
  `home = /usr/bin` and the remote interpreter stops resolving
  (`did not find executable at '/usr/bin\python.exe'`, exit 103).

Host keys are refused unless already known. `--trust-new-host` accepts an
unknown key without verification; use it only on a lab network you control,
where the win32-OpenSSH service generates a fresh `ssh_host_*` key pair on
first boot and a legitimate first connection would otherwise be blocked.

### Exit codes

`Run-BehaveScenariosOnWindows.ps1` and `Start-InteractiveProcessOnWindows.ps1`
both propagate the child's exit status, so `echo %ERRORLEVEL%` /
`$LASTEXITCODE` is trustworthy:

| Code | Meaning | Source |
| --- | --- | --- |
| `0` | All selected scenarios passed | runner |
| `1` | A scenario failed, **or** the run never got as far as running one | runner |
| `3` | The child exited but never wrote its exit code — a harness failure, not a pass | launcher |
| `58` | Windows `cmd.exe` truncation of `0xC000013A` — the runner was killed by Ctrl-C. See the `-Record` note under Debugging | runner |
| `124` | `TimeoutSeconds` elapsed; the child was killed | launcher |

`mixxx_test_runner.py` only ever returns `0` or `1` — there is no separate code
for a bad command line. An argument or feature-path mistake surfaces as `1`
with a `ConfigError` traceback, so read the traceback rather than the number
when a run fails instantly.

Code `3` exists so that a broken launcher can never masquerade as a green run.
The launcher writes the child's exit code to `exitcode.txt` in its temp dir and
refuses to guess when that file is missing or empty. If you ever see it, the
launcher's `finally` block keeps the temp dir (check `Write-Verbose` output) —
its `stdout.log` / `stderr.log` are the only record.

### Reading the results

Everything lands in `src\test\behave\artifacts\`:

- `behave-output-<label>.txt` — the full behave transcript, identical to stdout.
  Written even when the live stream is lost (locked screen, killed SSH), so
  **always check this file** before concluding a run produced nothing.
- `results-<label>.json` — per-scenario `outcome`, `tags`, `error`, and
  `start_time`/`end_time`. The quickest way to see which scenarios failed and
  how long each attempt took.
- `mixxx-ui-test-<label>-<timestamp>.mkv` — video, only with `-Record`. Chapters
  are muxed from the scenario timings, so the timecode in a failure's
  `# src/test/behave/steps/...` source comment seeks straight to it.
- `mixxx-output.<pid>.log` — the complete application output of one
  `mixxx-test` spawn (`--log-level debug`), with `===== spawn =====` and
  `===== exited ... code=... =====` banners around it. The quickest way to
  see what the app did around a failure, and which crash code it died with;
  one file per spawn, so a run with restarts produces several.

### Debugging

- **mixxx-test's own logs are in the artifacts on every platform.** Each
  spawn writes the full `--log-level debug` output (roughly 2500
  `debug [Main] ...` lines for a single short scenario) to
  `mixxx-output.<pid>.log` in the artifacts dir; grep it rather than reading
  it. On Windows the launcher's captured stream additionally mirrors the
  same output, because `src/util/console.cpp` reattaches Mixxx to the parent
  console.
- **Read the spawn/exit banners** to diagnose crashes and kill/restart
  problems: every `mixxx-output.<pid>.log` starts with a `===== spawn =====`
  line and ends with a `===== exited ... code=... =====` line once the
  runner reaped the process. An access-violation crash on Windows shows up
  as `code=3221225477 (0xC0000005)`; a missing exit banner means the runner
  itself died before it could reap the process. The transcript also carries
  a one-line `mixxx-test stopped: pid=... exit=...` notice per stop, so
  restarts can be correlated with the behave output without opening the
  logs.
- **Unbuffered output is mandatory over SSH.** The launcher sets
  `PYTHONUNBUFFERED=1` for the child. Without it CPython block-buffers a
  redirected stdout, so a run can sit silent for minutes and then dump
  everything at exit. If output ever looks frozen, check that variable first.
- **The launcher also pins `PYTHONIOENCODING=utf-8`.** Without it a redirected
  stdout makes CPython encode with the machine's ANSI codepage, so any
  non-ASCII text in a scenario name, step failure message or track title is
  mangled into replacement characters when read as UTF-8. This does not affect
  Mixxx's own C++ output. Override `-Environment` if you need to.
- **Narrow to one scenario** with `-Name`; the eight-scenario default retries
  each scenario up to three times, which multiplies wall-clock time by up to 4x
  when things fail.
- **Timestamps are in the log**: each `Scenario:` block is followed by
  `start_time`/`end_time` in `results-<label>.json`. A scenario that never
  appears there died before behave recorded it — usually a launcher or
  `mixxx-test` startup problem rather than a test problem.
- **Stuck processes**: `taskkill /IM mixxx-test.exe /F`, then re-run. The
  launcher reuses a session only within one process, so a leftover from a
  killed run will not be reused but will still hold the profile directory.
- **Video is the ground truth** for spix interaction bugs. If a click "did
  nothing", re-run with `-Record` before changing the step. On Windows the
  recorder is ffmpeg `gdigrab` capturing the `desktop` input, hardcoded to
  `1920x1080` at 30 fps, so the video is letterboxed/scaled on any other
  resolution — judge timing and cursor position, not pixel geometry.
- **`-Record` currently dies during teardown on Windows, after the scenarios
  have already passed.** `_stop_recorder` in `mixxx_test_runner.py` calls
  `recorder_proc.send_signal(signal.CTRL_C_EVENT)`, but the ffmpeg child is
  started without `creationflags=CREATE_NEW_PROCESS_GROUP`, so it shares the
  runner's console. `GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0)` targets group
  0 — *every* process attached to that console — so the runner receives the
  Ctrl-C too, raises `KeyboardInterrupt` out of `recorder_proc.wait(timeout=5)`,
  and dies with `0xC000013A` (reported as exit 58 through `cmd.exe`). The
  behave transcript in that run says `1 scenario passed, 0 failed`, so **the
  video is usually fine and the exit code is the misleading part.** Reproduced
  with a minimal probe: a plain `Popen` child plus `send_signal(CTRL_C_EVENT)`
  raises `KeyboardInterrupt` in the parent whenever a real console is attached,
  and does nothing at all in session 0 (no console), which is why it only
  shows up through the launcher. Until this is fixed, treat a `-Record` run's
  video as valid and ignore its exit code.

### Extending coverage on Windows

Adding features and steps is platform-independent — write the Gherkin, add the
step definitions, add `objectName`s in QML, rebuild `mixxx-test`. Specifically:

- **Steps are shared.** `steps/mixxx_steps.py` is the same file on both
  platforms; do not add Windows-only branches to a step. Platform differences
  belong in the feature's tags.
- **Tag platform-specific scenarios** (`@xpass` for environment flakiness,
  `@xfail(gh_issue=N)` for known bugs) rather than skipping them, so the Linux
  CI still exercises them. Note that autoretry applies to tagged scenarios too.
- **The QML under test is the same**, so a step that works on Linux should work
  on Windows. If it does not, suspect one of the known limitations above
  (`TapHandler`, `Menu` overlays, `TableView` drag) or a geometry/theme
  difference, and confirm with video before changing the step.
- **Keep new scenarios cheap to iterate on.** Anything you add to the
  `Run-BehaveScenariosOnWindows.ps1` default `-Name` list multiplies everyone's
  Windows feedback loop; add it to the feature file first and promote it only
  once it is stable.

> *End of autonomously AI-generated section.*

## How Tests Run

1. **CMake** finds `python3` + `behave`, globs `features/*.feature`, registers
   each as `mixxx-behave-<name>`.
2. `mixxx_test_runner.py` sets up a global tracks
   cache at `/tmp/mixxx-test-tracks/` (downloading if needed), optionally
   starts Xvfb+ffmpeg for headless CI, then invokes `behave` with the feature
   file.
3. behave invokes `environment.py` hooks, then matches step definitions in
   `steps/mixxx_steps.py`.
4. **Autoretry**: `before_feature` patches every scenario's `run()` method with
   a custom wrapper. If a scenario fails, it is retried up to `--retry N` times
   (default 3). Each attempt triggers `before_scenario`/`after_scenario` hooks
   normally. Only the final outcome is recorded in `context.results` (with a
   `retries` field summarizing previous attempts). `had_failure` / `fail_early`
   is only set when all retries are exhausted.

## mixxx-test --serve Mode

The `mixxx-test` binary built with `USE_TEST_UI=ON` supports `--serve` flag:

- Parses `--settings-path <dir>` via `CmdlineArgs::Instance()` (singleton — see
  `src/test/mixxxtest.cpp:29`). **Important**: `ApplicationScope` must call
  `CmdlineArgs::Instance().parse()` rather than constructing a local
  `CmdlineArgs`, otherwise `--settings-path` is lost.
- Creates `CoreServices` + `QmlApplication` to load the full Mixxx QML UI.
- Registers a **generic command handler** (see below) and starts an
  **AnyRpcServer** (spix) on **port 9000**.

## Custom spix Command: `getControlValue`

The `--serve` mode registers a custom handler via
`server.setGenericCommandHandler(...)` in `src/test/main.cpp`.

| Command           | Payload               | Result                                                                 | How to read result                                        |
|-------------------|-----------------------|------------------------------------------------------------------------|-----------------------------------------------------------|
| `getControlValue` | `"[Group],key"`       | `double` value stored in `mainWindow.lastControlValue` window property | `rpc.getStringProperty("mainWindow", "lastControlValue")` |
| `setControlValue` | `"[Group],key,value"` | Sets a ControlObject value                                             | None                                                      |
| `loadTrack`       | `"deck,filepath"`     | Loads a track file onto the specified deck                             | None                                                      |
| `library`         | JSON (see below)      | Adds/removes library directories, optionally triggers a scan           | None                                                      |
| `reloadQml`       | (empty)               | Tears down and rebuilds the QML engine                                 | None                                                      |

### `library` command

JSON payload:

```json
{
  "action": "addDirectory" | "removeDirectory",
  "path": "/absolute/path/to/directory",
  "scan": true | false
}
```

- `addDirectory`: calls `TrackCollectionManager::addDirectory()`. Silently
  skips if the directory is already watched (`AlreadyWatching`).
- `removeDirectory`: calls `TrackCollectionManager::removeDirectory()`.
- `scan`: when `true`, calls `startLibraryScan()` after the operation.

### `reloadQml` command

No payload needed. Calls `QmlApplication::loadQml(mainFilePath())` to
destroy and recreate the entire QML engine. Use this to reset QML state
between scenarios instead of abusing the auto-reload feature.

### Why two-step?

spix's `command()` RPC method returns `void`. The handler runs on the main
thread, sets the result as a Qt property on the window. A subsequent
`getStringProperty()` call (also processed on the main thread, in order)
reads it back. Since spix processes commands sequentially, this is race-free.

### Python helpers

```python
def _get_control_value(rpc, group, key):
    rpc.command("getControlValue", f"{group},{key}")
    return float(rpc.getStringProperty("mainWindow", "lastControlValue"))

def _set_control_value(rpc, group, key, value):
    rpc.command("setControlValue", f"{group},{key},{value}")
```

### Adding new custom commands

To add a new C++ command, extend the lambda in `src/test/main.cpp`:

```cpp
if (command == "myCommand") {
    // parse payload, do work
    // store result: window->setProperty("myResult", value);
}
```

Then read from Python:

```python
rpc.command("myCommand", payload)
result = rpc.getStringProperty("mainWindow", "myResult")
```

## Profile Setup (`mixxx_profile.py`)

### `create_empty_profile(settings_dir)`

- Creates the `settings_dir` directory (profile folder). Does **not** create a
  database — the `mixxx-test --serve` binary creates the database and applies
  schema migrations on startup.

### Profile types

- `empty`: schema-only database, no tracks (the only type currently implemented)

### `MixxxProcess` class

- `start()`: spawns `mixxx-test --serve --settings-path <dir> --developer
  --log-level debug`
- When constructed with `output_dir=<dir>`, every spawn persists its full
  stdout/stderr (stderr is merged into stdout) to
  `<dir>/mixxx-output.<pid>.log` — one file per spawn, so crash and
  kill/restart cycles stay separated. The file opens with a
  `===== spawn <timestamp> pid=... binary=... profile=... =====` banner and
  `stop()` appends `===== exited <timestamp> pid=... code=<n> (0x...) =====`,
  where the hex form makes Windows crash codes recognizable (3221225477 is
  0xC0000005, an access violation). The runner passes the artifacts
  directory as `output_dir`; without it, output stays in memory only.
- Stdout is still read via `subprocess.PIPE` + daemon thread; lines go both
  to the output file (flushed per line) and to a bounded in-memory deque
  (last 200 lines) used for startup diagnostics. Nothing is printed live.
- On early exit or RPC-port timeout, a bounded tail (last 40 lines) plus
  the output-file path is printed to the transcript instead of the full log.
- Waits up to 20s for port 9000 to be reachable
- `stop()`: terminate with 15s grace, then kill; appends the exit banner and
  prints a `mixxx-test stopped: pid=... exit=...` line, which is how
  kill/restart cycles become visible in the behave transcript.

### Track downloads

- Manifest in `test_tracks.json` — each entry has `url`, `title`, `artist`,
  `tags`, `bpm`, `first_beat`, `samplerate`, and optionally `artwork` (URL to
  cover image)
- Downloaded on first run to `/tmp/mixxx-test-tracks/` (global cache, shared
  across all CTest invocations)

## Spix RPC API (`TestServer.h`)

Available via XML-RPC on `http://localhost:9000/`:

| Method | Signature | Purpose |
| --- | --- | --- |
| `mouseClick` | `(path)` | Left click at item center |
| `mouseClick` | `(path, proportion, offset?)` | Click at relative position |
| `mouseClickWithButton` | `(path, button, modifiers)` | button: 1=Left, 2=Right; modifiers: 0=None, 2=Ctrl |
| `mouseClickWithProportion` | `(path, x, y)` | Click at relative (x, y) position |
| `mouseClickAndHold` | `(path, button, modifiers, holdTime)` | Hold button for `holdTime` ms |
| `mouseBeginDrag` | `(startPath, startX, startY, endPath, endY, endY)` | Begin drag from one item to another |
| `mouseDrag` | `(path, startX, startY, deltaX, deltaY, time)` | Drag by pixel delta over `time` ms |
| `mouseEndDrag` | `(path)` | End drag at item center |
| `mouseDropUrls` | `(path, urls)` | Drag-drop URLs onto item |
| `command` | `(command, payload)` | Custom C++ handler (see `main.cpp`) |
| `enterKey` | `(path, key, modifiers)` | Send a keyboard key press |
| `inputText` | `(path, text)` | Type text into item |
| `getStringProperty` | `(path, propertyName)` | Read QML property as string |
| `setStringProperty` | `(path, propertyName, value)` | Write QML property |
| `invokeMethod` | `(path, method, args)` | Call Q_INVOKABLE method |
| `getBoundingBox` | `(path)` | Returns `[x, y, w, h]` |
| `existsAndVisible` | `(path)` | Check item exists and visible |
| `wait` | `(ms)` | Sleep on main thread |
| `takeScreenshot` | `(path, filePath)` | Capture screenshot |
| `quit` | `()` | Quit application |
| `getErrors` | `()` | Get collected errors |

## Path Resolution

Spix resolves items by their `objectName` in the QML object tree, using `/`
as separator. The path `mainWindow/libraryContent/trackList/columnHeader/Title`
means:

- Find the `ApplicationWindow` with `objectName: "mainWindow"`
- Find its child with `objectName: "libraryContent"` (Loader → Library.qml root)
- Find its descendant with `objectName: "trackList"` (TrackList.qml root
  Rectangle)
- Find child `HorizontalHeaderView` with `objectName: "columnHeader"`
- Find the column delegate whose `objectName == "Title"` (set to `display`
  property of the model)

### ObjectName Map

| QML File | objectName | Notes |
| --- | --- | --- |
| `main.qml` | `"mainWindow"` | Root `ApplicationWindow` |
| `main.qml` | `"splashScreen"` | Splash `Rectangle` (fades, then hides) |
| `MainWindow.qml` | `"library"` | Toolbar "Library" toggle button |
| `MainWindow.qml` | `"libraryContent"` | Loader that shows `Library.qml` |
| `Library/TrackList.qml` | `"trackList"` | Root Rectangle |
| `Library/TrackList.qml` | `"columnHeader"` | `HorizontalHeaderView` |
| `Library/TrackList.qml` | `"columnPickerMenu"` | Column visibility `Menu` |
| `Library/TrackList.qml` | `"trackTableView"` | `TableView` |
| `Library/TrackList.qml` | `<display value>` | Column header delegate: `objectName: display` |
| `Library/Track.qml` | `"trackContextMenu"` | Right-click context `Menu` |
| `Library/Cell.qml` | `"trackRow_" + row` | Row `Rectangle`, dynamic per-row |

### Path Constants (from `mixxx_steps.py`)

```python
BUTTON_PATHS = {
    "LIBRARY": "mainWindow/library",
    "4DECKS": "mainWindow/show4DecksButton",
    "EDIT": "mainWindow/editDeckButton",
}
LIBRARY_CONTENT = "mainWindow/libraryContent"
TRACKLIST_PATH = "mainWindow/libraryContent/trackList"
COLUMN_HEADER_PATH = "mainWindow/libraryContent/trackList/columnHeader"
COLUMN_PICKER_MENU_PATH = "mainWindow/columnPickerMenu"
TRACK_TABLE_PATH = "mainWindow/libraryContent/trackList/trackTableView"
TRACK_ROW_PATH = "mainWindow/libraryContent/trackList/trackTableView"
TRACK_CONTEXT_MENU_PATH = "mainWindow/trackContextMenu"
```

> **Note**: `COLUMN_PICKER_MENU_PATH` and `TRACK_CONTEXT_MENU_PATH` use
> `mainWindow/` (not `trackList/`) because Qt `Menu` popups render in the
> window's `Overlay`, not as children of their parent item.

### Lookup Maps

```python
DECK_PROPERTY_MAP = {
    "BPM": "bpm",
    "tempo rate": "rate",
}

REMEMBERED_CO_KEYS = {
    "beatloop size": "beatloop_size",
    "rate range": "rateRange",
    "playback position": "playposition",
}

TRACK_ACTIONS = {
    "click": _click,
    "double-click": _double_click,
    "right-click": _right_click,
    "long-press": _long_press,
}

LOOP_BUTTONS = {
    ("halve", "beatloop"): "loop_halve",
    ("double", "beatloop"): "loop_double",
}
```

- `DECK_PROPERTY_MAP` translates user-friendly property names to ControlObject
  keys. Used by `the {prop} on deck {deck:d} is set to {value:f}` and
  `deck {deck_a:d} {prop} should be equal to deck {deck_b:d}`.
- `REMEMBERED_CO_KEYS` maps prop names to CO keys for the remember/compare
  pattern (`I remember the "{prop}"` / `the "{prop}" should have changed`).
- `TRACK_ACTIONS` maps action verbs to helper functions for the generic
  `I {action} the track at row {row:d}` step.
- `LOOP_BUTTONS` maps `(direction, component)` tuples to button paths for the
  generic `I {direction} the "{component}" size on deck {deck:d}` step.

## Step DSL (`steps/mixxx_steps.py`)

### Constants

- `QT_LEFT_BUTTON = 1`, `QT_RIGHT_BUTTON = 2`, `QT_CONTROL_MODIFIER = 2`
- `QT_KEY_ENTER`, `QT_KEY_SPACE`, `QT_KEY_DOWN`, `QT_KEY_UP`, `QT_KEY_TAB` — Qt key codes
- `KNOWN_COLUMNS` — list of all possible library column display labels

### Helper Functions

| Helper | Purpose |
| --- | --- |
| `_wait_for_visible(rpc, path, timeout=5)` | Poll `existsAndVisible` until true |
| `_wait_for_hidden(rpc, path, timeout=60)` | Poll `existsAndVisible` until false |
| `_click(rpc, path)` | `rpc.mouseClick(path)` |
| `_right_click(rpc, path)` | `rpc.mouseClickWithButton(path, QT_RIGHT_BUTTON, 0)` |
| `_long_press(rpc, path, button=QT_LEFT_BUTTON, hold_ms=1000)` | `rpc.mouseClickAndHold(path, button, 0, hold_ms)` |
| `_is_visible(rpc, path)` | `existsAndVisible` + bounding box area > 0 |
| `_is_column_visible(rpc, col)` | Resolves the model column index via `trackList.columnLabels` (see `_column_index`) + `columnWidth` via `invokeMethod` |
| `_column_index(rpc, col)` | Parses `trackList.columnLabels` (JSON array of labels in model order) and returns the index, or `-1`. Works for hidden columns whose header delegates do not exist |
| `_get_bb(rpc, path)` | `getBoundingBox` normalized to dict |
| `_get_control_value(rpc, group, key)` | Custom command: reads ControlObject via `command("getControlValue", ...)` + `getStringProperty("mainWindow", "lastControlValue")` |
| `_set_control_value(rpc, group, key, value)` | Custom command: writes ControlObject via `command("setControlValue", ...)` |
| `_load_track(rpc, deck, filepath)` | Custom command: loads track file via `command("loadTrack", ...)` |
| `_library_command(rpc, action, path, scan=False)` | Custom command: adds/removes library directories via `command("library", ...)` with JSON payload |
| `_get_property(rpc, path, prop)` | `rpc.getStringProperty(path, prop)` |
| `_set_property(rpc, path, prop, value)` | `rpc.setStringProperty(path, prop, str(value))` |
| `_ensure_profile(context, type)` | Manages profile lifecycle across scenarios (session reuse) |

### Given Steps

| Pattern | Implementation |
| --- | --- |
| `a new empty profile` | `_ensure_profile(context, "empty")` |
| `Mixxx is open and ready to operate` | Starts Mixxx, waits for mainWindow, waits for splash to hide, caches `_column_idx` and `_default_props` |
| `the 4 decks view is enabled` | Sets `show4DecksButton.checked = true` |
| `a track is loaded on deck {deck:d}` | Picks random track from `MIXXX_TEST_TRACKS_DIR`, calls `loadTrack` C++ command |
| `no track is loaded on deck {deck:d}` | Ejects track via `eject` ControlObject if `track_loaded` is set |
| `the {prop} on deck {deck:d} is set to {value:f}` | `_set_control_value` to set a ControlObject (Given-only, not for When steps) |
| `the sync_on deck {deck:d} is {state}` | `_set_control_value` for `sync_enabled` (Given-only) |
| `I wait for {second:d} second` | `time.sleep(second)` (also available as @when and @then) |

### When Steps

| Pattern | Implementation |
| --- | --- |
| `I resize the window's width to {width:d}px` | `setStringProperty("mainWindow", "width", value)` |
| `I check on the button "{button}" in the main toolbar` | Asserts button visible via `_is_visible` |
| `I click the "{button}" button` | Looks up in `BUTTON_PATHS`, clicks |
| `I turn off "4 decks" mode` | Clicks `show4DecksButton` |
| `I click the "{button}" button on deck {deck:d}` | `_deck_button_path(deck, button)` + `_click` |
| `I long-press the "{button}" button on deck {deck:d}` | `_deck_button_path(deck, button)` + `_long_press` (or `invokeMethod` for sync) |
| `I {direction} the "{component}" size on deck {deck:d}` | Looks up in `LOOP_BUTTONS` dict, clicks button |
| `I remember the "{prop}" of deck {deck:d}` | Reads CO via `_get_control_value`, stores in `context._remembered` |
| `I seek to {position:f} in deck {deck:d}` | `mouseClickWithProportion` on the overview widget |
| `I set the rate of deck {deck:d} to {value:d}%` | `mouseDrag` on the tempo fader handle |
| `I set hotcue {hotcue:d} on deck {deck:d}` | Clicks on `hotcue_{hotcue}` button path |
| `I clear hotcue {hotcue:d} on deck {deck:d}` | Clicks on same hotcue button to toggle |
| `I click the column header "{column}"` | Clicks header at `COLUMN_HEADER_PATH/{column}` |
| `I drag the column "{column}" before the column "{target}"` | `invokeMethod(trackTableView, "moveColumn", [idx_column, idx_target])` |
| `I open the column picker menu` | Right-clicks on Title column header, waits for menu |
| `I toggle the column "{column}" in the column picker` | Use the keyboard to choose the nth item, based on column index |
| `I {action} the track at row {row:d}` | Looks up in `TRACK_ACTIONS` dict (`click`, `double-click`, `right-click`, `long-press`) |
| `I select {path} on the track menu` | Keyboard-navigates context menu using `enterKey` |
| `I move the "{component}" component in deck {deck:d} after the "{target}" component` | `mouseDrag` from component to target position |
| `I move the selected group in deck {deck:d} after the "{target}" component` | `mouseDrag` from selected group overlay to target position |
| `I select the group containing "{component}" in deck {deck:d} with a {action}` | Long press or ctrl+click for group selection |
| `I wait for {second:d} second` | `time.sleep(second)` |

### Then Steps

| Pattern | Implementation |
| --- | --- |
| `the library is shown for {operator} than {percent:d}% of the Window's height` | Compares `libraryContent` height ratio to `mainWindow` height |
| `only the column "{columns}" are shown` | Splits on `,`/`.`, checks each expected column visible, each unexpected column not visible |
| `the column "{column}" should {state} visible` | `_is_column_visible` check |
| `the column "{column}" should appear before the column "{other}"` | Compares `x` positions of headers |
| `the results should be sorted by "{column}" in "{order}" order` | Reads `sortingColumn`/`sortingOrder` from `columnHeader` |
| `the track at row {row:d} should be selected` | Reads `selected` property from Cell |
| `the track context menu should be visible` | `existsAndVisible` on `trackContextMenu` path |
| `the deck for "{group}" should {assertion} visible` | `_wait_for_visible`/`_wait_for_hidden` on `DECK_PATH_MAP` |
| `the deck {deck:d} should be {playing\|stopped}` | Reads `play` CO via `_get_control_value` |
| `the play button on deck {deck:d} should be {pressed\|released}` | Reads `highlight` property on play button |
| `deck {deck_a:d} {prop} should be equal to deck {deck_b:d}` | Compares CO values between two decks (uses `DECK_PROPERTY_MAP`) |
| `the cue point should be set on deck {deck:d}` | Asserts `cue_point > 0` |
| `hotcue {hotcue:d} should {assertion} set on deck {deck:d}` | Asserts `hotcue_{n}_status` CO |
| `the loop should {assertion} enabled on deck {deck:d}` | Asserts `loop_enabled` CO |
| `the "{prop}" of deck {deck:d} should have changed` | Compares current to saved value in `context._remembered` (uses `REMEMBERED_CO_KEYS`) |
| `sync should be {assertion} on deck {deck:d}` | Asserts `sync_enabled` CO |
| `deck {deck:d} should be the sync leader` | Asserts `sync_leader` CO |
| `the rate of deck {deck:d} should be near {target}` | Reads `rate` CO, asserts `abs(actual - target) < 0.01` |
| `the rate ratio of deck {deck:d} should be near {target}` | Reads `rate_ratio` CO |
| `edit mode should {assertion} enabled` | Reads `editDeckButton.checked` property |
| `the edit overlay should {action} visible on the "{component}" component in deck {deck:d}` | `_is_visible` on `componentItem/overlayItem` path |
| `the "{component}" component should appear after the "{target}" component in deck {deck:d}` | Compares `x` positions of component bounding boxes |
| `the "{group_name}" group should appear after the "{target}" component in deck {deck:d}` | Not yet implemented (`NotImplementedError`) |
| `a track {assertion} loaded on deck {deck:d}` | Reads `track_loaded` CO |
| `I wait for {second:d} second` | `time.sleep(second)` |

## Test Fixtures (`environment.py`)

- `before_all`: initiates `context._session` dict for cross-scenario state
- `before_feature`: patches every scenario with the custom autoretry wrapper
  (max attempts from `--retry`, default 3)
- `before_scenario`: restores `context.mixxx`, `context.mixxx_rpc`,
  `context.profile_dir`, `context.active_profile_type` from session; skips
  if `fail_early` is set and a previous scenario failed all retries
- `after_scenario`: computes outcome, records timing, stores pending result on
  `scenario._pending_result` for the autoretry wrapper to finalize
- `after_all`: writes `chapters.json` to artifacts directory, sends
  `rpc.quit()`, stops Mixxx process, cleans up temp profile dirs

### Autoretry wrapper (`patch_scenario_with_autoretry`)

Based on `behave.contrib.scenario_autoretry.patch_scenario_with_autoretry`.
Wraps `scenario.run()` so that failed scenarios are retried up to N times.
For each attempt: `before_scenario` → steps → `after_scenario` runs normally.
After the last attempt the wrapper:

1. Reads `scenario._pending_result` (set by `after_scenario`) to build the
   final result entry, adding a `retries` field when there were multiple
   attempts.
2. Appends the single result to `context.results`.
3. Sets `had_failure = True` **only** when all retries are exhausted and the
   scenario still failed (excluding `@xfail`/`@xpass`).

## How to Add a New Step

1. **Follow step naming conventions** — Given/Then text must be
   behavior-oriented (what the user sees or expects). When text should favour
   language-oriented actions; implementation-specific details are acceptable
   when no clean alternative exists. See [README.md](README.md#writing-steps)
   for the full guide with examples.
2. Define Gherkin pattern in a `.feature` file
3. Add step implementation function in `steps/mixxx_steps.py` (or a new file
   in `steps/`)
4. Use `@given`, `@when`, or `@then` decorator from `behave`
5. Use helpers: `_rpc()` for spix proxy, `_wait_for_visible` for sync,
   `_get_control_value` for ControlObject reads

### Rules for `When` steps

- **Favour language-oriented action text** — `I drag the volume fader of deck 1
  to 50%` is preferred over `I click on volume fader and drag to 0.05`.
  Implementation-specific text is acceptable when no clean behavior-oriented
  alternative exists (e.g. precise CO values like `I set the rate of deck 1 to
  0.05`).
- **`_set_control_value` is forbidden in `When` steps** — it bypasses the UI
  and does not test real user interaction. Use only in `Given` steps (context
  setup).
- **`_get_control_value` is allowed in `Then` steps** for assertions. It is a
  read-only operation that verifies state.
- **`When` steps must use real UI interactions** — spix `mouseClick`, drag,
  etc. If a step genuinely cannot use a UI interaction (e.g. seeking to an
  exact position on a slider), tag the scenario with `@test/missing-ui-interaction`
  and add a comment annotation explaining why.
- **Custom C++ commands** (e.g. `loadTrack`, `getControlValue`) that bypass
  UI are subject to the same tagging rule as `_set_control_value`.

### Rules for `Then` steps

- **Always behavior-oriented** — step text must describe what the user sees or
  expects, not internal state. `the loop should be enabled on deck 1` is
  correct; `the loop_enabled CO is 1` is not.
- **Never expose control object names** — use domain language (e.g. "the deck
  should be playing") rather than CO keys (e.g. "the play CO should be 1").
- **Implementation reads are fine internally** — `_get_control_value` in the
  step function body is allowed for assertions, but the Gherkin pattern itself
  must not leak it.

## How to Add a New Feature

1. Create `features/<name>.feature` with `Feature:` header and
   `Background:` / `Scenario:` blocks
2. Add objectNames in QML as needed (`objectName: "myElement"`)
3. CMake auto-discovers `*.feature` files — no registration needed

## How to Read a ControlObject from a Test

Use `_get_control_value(rpc, group, key)`:

```python
from steps.mixxx_steps import _get_control_value

@then('the play button should be pressed')
def step_play_pressed(context):
    value = _get_control_value(context.mixxx_rpc, "[Channel1]", "play")
    assert float(value) > 0, f"Play is not pressed (value={value})"
```

Or use `command("getControlValue", "[Group],key")` directly:

```python
context.mixxx_rpc.command("getControlValue", "[Channel1],play")
value = context.mixxx_rpc.getStringProperty("mainWindow", "lastControlValue")
```

## Tagging

- **`@xpass`** — Marks **flaky tests** (intermittent failures due to timing,
  platform quirks, etc.). The runner suppresses captured output for these and
  labels them `FLAKY` in video chapters. Use this when the test logic is
  correct but the environment is unreliable.

- **`@xfail(gh_issue=N)`** — Marks **known bugs**. Always include the
  `gh_issue=` annotation linking to the relevant GitHub issue so failures are
  traceable. The runner labels chapters `EXPECTED FAILURE` or `UNEXPECTED PASS`
  and prints the issue link on failure.

- **Autoretry applies to all scenarios**, including `@xpass` and `@xfail`.
  For `@xfail` scenarios that always fail, the retry attempts are harmless
  (just add time). `had_failure` is never set for `@xfail`/`@xpass` scenarios
  regardless of retry outcome.

## Known Limitations / Blockers

1. **spix mouseClick does not trigger Qt6 TapHandler** — `TapHandler` (used in
   TrackList.qml header delegates and Track.qml) doesn't respond to synthetic
   mouse events from spix. A patch (`spix.patch`) adds `globalPos` to Qt
   mouse events which helps, but the fix remains fragile.
2. **QML Menu popups (Overlay)** — `Menu` components like
   `columnPickerMenu` and `trackContextMenu` render outside the parent item in
   a window-level `Overlay`. spix cannot find them as children of the expected
   parent path. The alternative path `"mainWindow/columnPickerMenu"` works for
   visibility checks but click interaction may fail. Menu must override their
   `contentItem` with a custom `ListView` which explicitly set a `objectName` and
   allow interaction with the Menu's action (see `columnSelectionMenu` and
   `contextMenu` Menu for reference).
3. **spix drag-and-drop doesn't work with TableView column drag** —
   `mouseBeginDrag`/`mouseEndDrag` events don't trigger QML `TableView` column
   reorder mechanics. Current workaround: direct `invokeMethod("moveColumn",
   ...)` call.
4. **Window property `width` set via RPC doesn't trigger QML layout** —
   `setStringProperty("mainWindow", "width", "1200")` doesn't cause Column QML layout
   recalculation. Current workaround: direct `invokeMethod("columnWidth",...)` to detect if the column is now hidden.
5. **spix cannot seek to an exact playposition on a slider** — seeking to a
   precise position (e.g. 0.05) uses `_set_control_value` on the `playposition`
   CO. No spix API can target a fractional position on a `Slider` reliably.
   Tagged `@test/missing-ui-interaction`.
