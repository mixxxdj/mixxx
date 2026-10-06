#!/usr/bin/env python3
"""CTest entry point for the Mixxx behave UI tests.

Usage: mixxx_test_runner.py [options] [feature_file...]

Sets up a global tracks cache, optional headless display (Linux) and
optional video recording, then runs behave.
Profile setup and mixxx-test lifecycle are handled by the behave steps.
"""

import argparse
import atexit
import datetime
import json
import os
import signal
import shutil
import subprocess
import sys
import re
import tempfile
import time

from behave.configuration import Configuration
from behave.runner import Runner
from behave.formatter.pretty import PrettyFormatter

# Wall-clock time at the start of video recording. Written by main() after
# spawning the recorder; read by XFailPrettyFormatter to append a video
# timecode to each step's source comment. Stays ``None`` when ``--record`` is
# not set, in which case the formatter behaves like its parent.
RECORDING_START = None

LIBRARY_TRACK_COUNT = 50


def _format_timecode(seconds):
    if seconds is None or seconds < 0:
        return None
    total = int(seconds)
    return f"{total // 60}:{total % 60:02d}"


def _find_ffmpeg(explicit_path=None):
    if explicit_path and os.path.isfile(explicit_path):
        return explicit_path
    return shutil.which("ffmpeg")


def _detect_backend(choice, recording):
    """Resolve --display-backend to a concrete backend name.

    Returns one of "xvfb", "xwayland" or "offscreen". "xwayland" uses cage
    (a wlroots nested compositor with Xwayland support) plus, when recording
    is requested, wf-recorder (which needs the wlroots wlr-screencopy
    protocol not provided by weston). "auto" prefers xwayland, then xvfb,
    then offscreen.
    """
    if sys.platform != "linux":
        print(
            f"--headless is Linux-only (current platform: {sys.platform}) - "
            "using offscreen platform",
            file=sys.stderr,
        )
        return "offscreen"
    xvfb_ok = shutil.which("Xvfb") is not None
    cage_ok = shutil.which("cage") is not None
    wfr_ok = shutil.which("wf-recorder") is not None
    # xwayland backend needs cage always; needs wf-recorder only when recording.
    # (ffmpeg for chapter muxing is checked separately at record time.)
    xwayland_ok = cage_ok and (not recording or wfr_ok)

    if choice == "xvfb":
        if not xvfb_ok:
            print(
                "Requested xvfb backend but Xvfb not found - "
                "using offscreen platform",
                file=sys.stderr,
            )
            return "offscreen"
        return "xvfb"
    if choice == "xwayland":
        missing = []
        if not cage_ok:
            missing.append("cage")
        if recording and not wfr_ok:
            missing.append("wf-recorder")
        if missing:
            print(
                f"Requested xwayland backend but {'/'.join(missing)} not found "
                "- using offscreen platform",
                file=sys.stderr,
            )
            return "offscreen"
        return "xwayland"
    # auto: prefer xwayland, fall back to xvfb, then offscreen
    if xwayland_ok:
        return "xwayland"
    if xvfb_ok:
        return "xvfb"
    print(
        "No headless display backend available (need cage+wf-recorder or "
        "Xvfb) - using offscreen platform",
        file=sys.stderr,
    )
    return "offscreen"


def _start_xvfb(display):
    proc = subprocess.Popen(
        [
            "Xvfb",
            display,
            "-ac",
            "-nocursor",
            "-screen",
            "0",
            "1920x1080x24",
            "-auth",
            "/dev/null",
        ],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    time.sleep(3)
    # A dead Xvfb (portability issue, wrong arguments, host display state)
    # would make mixxx-test fail with a confusing RPC timeout; fail here
    # with the actual reason instead.
    if proc.poll() is not None:
        raise RuntimeError(
            f"Xvfb {display} exited during startup (code {proc.returncode})"
        )
    return proc


# Cage's wlroots headless backend only advertises a single 1280x720 mode and
# cage has no CLI flag to override it, so after the compositor socket appears
# we resize its single output to 1920x1080 via wlr-randr --custom-mode.
_DEFAULT_CAGE_WIDTH = 1920
_DEFAULT_CAGE_HEIGHT = 1080


def _start_cage():
    """Spawn cage (wlroots nested compositor, Xwayland-capable) headlessly.

    Returns (proc, wayland_display, runtime_dir). The caller is responsible
    for setting WAYLAND_DISPLAY + XDG_RUNTIME_DIR on any child process.
    """
    runtime_dir = tempfile.mkdtemp(prefix="mixxx-cage-")
    os.chmod(runtime_dir, 0o700)
    wayland_display = "wayland-0"
    env = {
        **os.environ,
        "XDG_RUNTIME_DIR": runtime_dir,
        "WAYLAND_DISPLAY": wayland_display,
        "WLR_BACKENDS": "headless",
        "WLR_OUTPUTS": "1",
    }
    proc = subprocess.Popen(
        ["cage", "-d"],
        env=env,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    socket_path = os.path.join(runtime_dir, wayland_display)
    deadline = time.time() + 10
    while time.time() < deadline:
        if proc.poll() is not None:
            raise RuntimeError(f"cage exited during startup (code {proc.returncode})")
        if os.path.exists(socket_path):
            break
        time.sleep(0.2)
    else:
        raise RuntimeError(
            f"cage failed to start (no socket at {socket_path})"
        )

    _resize_cage_output(env, _DEFAULT_CAGE_WIDTH, _DEFAULT_CAGE_HEIGHT)
    return proc, wayland_display, runtime_dir


def _resize_cage_output(env, width, height):
    """Switch cage's single headless output to width x height via wlr-randr.

    wlroots' headless backend only advertises 1280x720 by default; wlr-randr
    talks the wlr-output-management-v1 protocol that cage implements and lets
    us set a custom mode arbitrarily. Soft warning on missing binary or
    failed switch.
    """
    if width == 1280 and height == 720:
        # Already the default; nothing to do.
        return
    wlr_randr = shutil.which("wlr-randr")
    if not wlr_randr:
        print(
            f"wlr-randr not found - cage output stays at the wlroots default "
            f"(1280x720) instead of {width}x{height}. Install wlr-randr to "
            f"control the headless output size.",
            file=sys.stderr,
        )
        return
    # Discover cage's single output name (e.g. "HEADLESS-1") so the
    # --custom-mode call targets the right sink even if wlroots ever changes
    # the naming scheme.
    try:
        listing = subprocess.run(
            [wlr_randr],
            env=env,
            capture_output=True, text=True, timeout=5,
        )
    except Exception as e:
        print(f"wlr-randr listing failed: {e}", file=sys.stderr)
        return
    output_name = None
    for line in listing.stdout.splitlines():
        line = line.strip()
        if line and not line.startswith(" ") and not line.startswith("\t"):
            # First token is the output name (e.g. 'HEADLESS-1 "Headless output 1"')
            output_name = line.split()[0]
            break
    if not output_name:
        print("wlr-randr: could not find cage output name", file=sys.stderr)
        return
    try:
        subprocess.run(
            [wlr_randr, "--output", output_name,
             "--custom-mode", f"{width}x{height}"],
            env=env,
            capture_output=True, text=True, timeout=5,
            check=True,
        )
    except Exception as e:
        print(
            f"wlr-randr --custom-mode {width}x{height} on {output_name} failed: "
            f"{e}; cage output remains at default resolution",
            file=sys.stderr,
        )


def _stop_display_proc(proc):
    if proc is None:
        return
    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait()


def _build_record_cmd(backend, ffmpeg_bin, wf_recorder_bin, display, artifacts_dir, video):
    """Build the recorder argv for the requested backend.

    For the xvfb backend we use ffmpeg x11grab (needs DISPLAY, set by caller);
    ``-video_size`` stays at 1920x1080 to match the Xvfb screen set by
    ``_start_xvfb``.
    For the xwayland backend we use wf-recorder (needs WAYLAND_DISPLAY +
    XDG_RUNTIME_DIR, set by the caller on the recorder process env); it
    follows the cage output geometry set by ``_resize_cage_output`` and needs
    no explicit size argument.
    """
    output = os.path.join(artifacts_dir, video)
    if backend == "xwayland" and sys.platform == "linux":
        # wf-recorder talks the wlr-screencopy protocol; it captures the
        # (single) cage output regardless of size. --no-damage forces a
        # constant framerate so chapter timing matches behave's wall clock.
        return [
            wf_recorder_bin,
            "--codec=libx264",
            "--pixel-format=yuv420p",
            "--no-damage",
            "-r", "30",
            "-f", output,
        ]
    common = [ffmpeg_bin, "-nostdin", "-hide_banner", "-loglevel", "error"]
    if sys.platform == "linux":
        return common + [
            "-f", "x11grab",
            "-draw_mouse", "0",
            "-video_size", "1920x1080",
            "-i", display,
            "-map", "0:v",
            "-r", "30",
            "-c:v", "libx264",
            "-preset", "veryfast",
            "-b:v", "2000k",
            output,
            "-y",
        ]
    elif sys.platform == "darwin":
        return common + [
            "-f", "avfoundation",
            "-video_size", "1920x1080",
            "-r", "30",
            "-i", "0:none",
            "-codec:v", "mpeg4",
            "-b:v", "10000k",
            output,
            "-y",
        ]
    elif sys.platform == "win32":
        return common + [
            "-f", "gdigrab",
            "-video_size", "1920x1080",
            "-framerate", "30",
            "-probesize", "20M",
            "-i", "desktop",
            "-codec:v", "mpeg4",
            "-r", "30",
            "-b:v", "10000k",
            output,
            "-y",
        ]
    else:
        print(f"Platform '{sys.platform}' unsupported for recording.", file=sys.stderr)
    return None


def _stop_recorder(recorder_proc, recorder_name="ffmpeg"):
    """Stop the recorder (ffmpeg or wf-recorder) gracefully.

    Both tools honor SIGINT and produce a clean final flush when interrupted
    rather than terminated, so prefer SIGINT over SIGTERM for the graceful
    path to avoid truncated/corrupt MKV tails.
    """
    if recorder_proc is None:
        return
    # First, give it a chance to exit on SIGINT (clean flush).
    try:
        if sys.platform == "win32":
            recorder_proc.send_signal(signal.CTRL_C_EVENT)
        else:
            recorder_proc.send_signal(signal.SIGINT)
        recorder_proc.wait(timeout=5)
        return
    except subprocess.TimeoutExpired:
        pass
    except ValueError:
        # Process already finished
        return
    # SIGINT didn't work: fall back to terminate / kill.
    recorder_proc.terminate()
    try:
        recorder_proc.wait(timeout=3)
    except subprocess.TimeoutExpired:
        recorder_proc.kill()
    # Surface any recorded stderr if it died early (was still running).
    ret = recorder_proc.poll()
    if ret is not None and recorder_proc.stderr is not None:
        try:
            stderr_output = recorder_proc.stderr.read().decode(
                "utf-8", errors="replace"
            )
        except Exception as e:
            stderr_output = f"Python: {e}"
        if stderr_output:
            print(
                f"\n=== {recorder_name} output (exit code: {ret}) ===",
                file=sys.stderr,
            )
            print(stderr_output, file=sys.stderr)
            print(f"=== end {recorder_name} output ===\n", file=sys.stderr)


def _generate_ffmetadata(results, dest):
    """Write an FFMetadata1 file for ffmpeg chapter muxing."""
    with open(dest, "w") as f:
        f.write(";FFMETADATA1\n")
        for ch in results:
            f.write("[CHAPTER]\n")
            f.write("TIMEBASE=1/1000\n")
            f.write(f"START={ch['start_ms']}\n")
            f.write(f"END={ch['end_ms']}\n")
            f.write(f"title={ch['title']}\n")


def _mux_chapters(video_path, results, recording_start, ffmpeg_bin):
    """Add chapter metadata from chapters_path into video_path in-place."""
    if not ffmpeg_bin:
        # Can happen on the xwayland backend (which uses wf-recorder for
        # capture): chapter muxing needs ffmpeg itself, so just skip.
        print(
            "ffmpeg not found - skipping chapter muxing of video chapters",
            file=sys.stderr,
        )
        return
    for ch in results:
        ch["start_ms"] = int((ch["start_time"] - recording_start) * 1000)
        ch["end_ms"] = int((ch["end_time"] - recording_start) * 1000)

    tmp = video_path + ".tmp.mp4"
    meta_file = video_path + ".ffmeta.txt"
    try:
        _generate_ffmetadata(results, meta_file)
        cmd = [
            ffmpeg_bin, "-nostdin", "-hide_banner", "-loglevel", "info",
            "-i", video_path,
            "-i", meta_file,
            "-map_metadata", "1",
            "-codec", "copy",
            "-y", tmp,
        ]
        # text=True so subprocess returns str, not bytes -- no .decode() needed.
        p = subprocess.run(cmd, capture_output=True, text=True)
        if p.returncode != 0:
            print(
                f"\n=== ffmpeg chapter muxing failed "
                f"(exit code: {p.returncode}) ===",
                file=sys.stderr,
            )
            print(p.stdout, p.stderr, file=sys.stderr)
        os.replace(tmp, video_path)
    except Exception as e:
        print(
            f"\n=== ffmpeg chapter muxing raised {type(e).__name__}: {e} ===",
            file=sys.stderr,
        )
    finally:
        if os.path.exists(tmp):
            os.remove(tmp)
        if os.path.exists(meta_file):
            os.remove(meta_file)

class XFailPrettyFormatter(PrettyFormatter):
    XFAIL_ARGS_RE = re.compile(r"xfail\((.*)\)")

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if sys.platform == "win32":
            # Force the use of monochrome output
            self.colored = False
        # Tee formatted output to a summary file so the test job can attach
        # the timecoded, source-annotated behave transcript to the GitHub
        # Actions job summary in addition to the live CI log.
        self._summary_file = None
        self._summary_stream = None
        summary_path = (self.config.userdata or {}).get("summary_path")
        if summary_path:
            try:
                self._summary_file = open(summary_path, "w")
                self._summary_stream = _TeeStream(self.stream, self._summary_file)
                # Install the tee as both the formatter stream and the
                # stream opener's stream so behave's close_stream() identity
                # assert (self.stream is self.stream_opener.stream) holds.
                # Keep should_close_stream False so behave never tries to
                # close the tee (which would close the live stdout too).
                self.stream = self._summary_stream
                self.stream_opener.stream = self._summary_stream
                self.stream_opener.should_close_stream = False
                atexit.register(self._close_summary)
            except OSError:
                # Fall back to normal stdout if the summary path is not
                # writable; the live log keeps its timecodes either way.
                self._summary_file = None
        self._current_step_offset = None

    def close(self):
        # Flush/close the summary tee before base close() nulls self.stream,
        # then restore the original stream so base.close_stream()'s identity
        # assert still passes.
        super().close()
        self._close_summary()

    def _close_summary(self):
        try:
            if self._summary_stream is not None:
                self._summary_stream.flush()
        except Exception:
            pass
        if self._summary_file is not None:
            try:
                self._summary_file.close()
            except Exception:
                pass
        self._summary_file = None
        self._summary_stream = None

    def match(self, match):
        # Record the wall-clock offset of each step start relative to the
        # recorder start so print_step() can append a (M:SS) timecode.
        self._current_step_offset = (
            time.time() - RECORDING_START if RECORDING_START is not None else None
        )
        super().match(match)

    def print_step(self, status, arguments, location, proceed):
        if self.show_source and location is not None and self._current_step_offset is not None:
            tc = _format_timecode(self._current_step_offset)
            if tc is not None:
                location = _LocationWithTimecode(location, tc)
        # Once consumed, clear so replayed/skipped steps (which call
        # print_step with proceed=True but no fresh match()) don't inherit a
        # stale offset.
        self._current_step_offset = None
        super().print_step(status, arguments, location, proceed)

    def _print_current_scenario_captured_output(self):
        for tag in self.current_scenario.tags:
            if tag.startswith("xpass"):
                # If this is a known flaky test, not bothering with any captured log output
                return
            if not tag.startswith("xfail"):
                continue
            match = self.XFAIL_ARGS_RE.fullmatch(tag)
            if match:
                params = dict([tuple(item.split("=", 1)) for item in match.group(1).split(",")])
                if "gh_issue" in params:
                    print(f"Tracked as https://github.com/mixxxdj/mixxx/issues/{params['gh_issue']}", file=sys.stderr)
                    return
            print("WARNING: xfailed test has no Github issue attached! Use the 'xfail(gh_issue=...)' to track the relevant issue")
            return

        super()._print_current_scenario_captured_output()


class _LocationWithTimecode(object):
    """Wrap a behave step match location so its textual representation carries
    the video timecode, e.g. ``src/test/behave/steps/mixxx_steps.py:372 (2:45)``.
    Behave's PrettyFormatter only requires ``six.text_type(location)`` to render
    the source comment, so a ``__str__`` is enough."""

    __slots__ = ("_location", "_timecode")

    def __init__(self, location, timecode):
        self._location = location
        self._timecode = timecode

    def __str__(self):
        return f"{self._location} ({self._timecode})"

    def __repr__(self):
        return f"_LocationWithTimecode({self._location!r}, {self._timecode!r})"


class _TeeStream(object):
    """Minimal stream wrapper that mirrors ``write`` and ``flush`` calls to two
    underlying streams. Used to send the behave transcript to both stdout (the
    live CI log) and a summary file attached to the job summary."""

    def __init__(self, primary, secondary):
        self._primary = primary
        self._secondary = secondary

    def write(self, data):
        self._primary.write(data)
        try:
            self._secondary.write(data)
        except Exception:
            pass

    def flush(self):
        self._primary.flush()
        try:
            self._secondary.flush()
        except Exception:
            pass

    def __getattr__(self, name):
        # Any attribute access other than write/flush is delegated to the
        # primary stream (covers ``encoding``, ``isatty()``, ``fileno()`` in
        # case behave inspects them).
        return getattr(self._primary, name)

def main():
    this_dir = os.path.dirname(os.path.abspath(__file__))
    # Needed so behave can resolve the "mixxx_test_runner:XFailPrettyFormatter"
    # formatter by module name and so "mixxx_profile" imports unambiguously
    # (it must never fall back to the stdlib "profile" module).
    sys.path.insert(0, this_dir)

    import mixxx_profile as profile

    parser = argparse.ArgumentParser(description="Mixxx UI test runner")
    parser.add_argument(
        "--headless",
        action="store_true",
        default=bool(os.environ.get("MIXXX_TEST_HEADLESS")),
        help="Linux only: run under a headless virtual display (see --display-backend)",
    )
    parser.add_argument(
        "--display-backend",
        choices=["auto", "xvfb", "xwayland"],
        default=os.environ.get("MIXXX_TEST_DISPLAY_BACKEND", "auto"),
        help="Linux only: headless display backend. 'xvfb' uses Xvfb + ffmpeg "
             "x11grab; 'xwayland' uses cage (a nested wlroots compositor with "
             "Xwayland support) + wf-recorder. 'auto' prefers xwayland, then "
             "xvfb, then offscreen QPA (default: auto)",
    )
    parser.add_argument(
        "--no-capture",
        action="store_true",
        default=bool(os.environ.get("MIXXX_BEHAVE_NO_CAPTURE")),
        help="Don't capture standard streams and let Mixxx logs being outputted in realtime",
    )
    parser.add_argument(
        "--fail-early",
        action="store_true",
        default=bool(os.environ.get("MIXXX_BEHAVE_FAIL_EARLY")),
        help="Stop running the tests as soon as failure occurs",
    )
    parser.add_argument(
        "--retry",
        type=int,
        default=int(os.environ.get("MIXXX_BEHAVE_RETRY", "1")),
        help="Max retry attempts for autoretry per scenario (default: 1)",
    )
    parser.add_argument(
        "--record",
        action="store_true",
        default=bool(os.environ.get("MIXXX_TEST_RECORD")),
        help="Record video via ffmpeg (any platform)",
    )
    parser.add_argument(
        "--ffmpeg-path",
        default=os.environ.get("MIXXX_FFMPEG_BIN"),
        help="Path to ffmpeg binary (used for x11grab recording on xvfb/mac/win "
             "and for chapter muxing on every backend)",
    )
    parser.add_argument(
        "--wf-recorder-path",
        default=os.environ.get("MIXXX_WF_RECORDER_BIN"),
        help="Path to wf-recorder binary (used by the xwayland display backend "
             "when --record is set)",
    )
    parser.add_argument(
        "--artifacts-dir",
        default=os.environ.get("MIXXX_TEST_ARTIFACTS"),
        help="Directory for screenshots and video output",
    )
    parser.add_argument(
        "--binary",
        default=os.environ.get("MIXXX_TEST_BINARY", "mixxx-test"),
        help="Path to the mixxx-test binary",
    )
    parser.add_argument(
        "--label",
        help="Label to include in the video filename (e.g. feature name)",
    )
    parser.add_argument(
        "behave_args",
        nargs=argparse.REMAINDER,
        help="Arguments forwarded to behave (e.g. feature file paths)",
    )

    args = parser.parse_args()


    # Set up global tracks cache (shared across all feature tests)
    tracks_cache = os.environ.get("MIXXX_TEST_TRACKS_DIR")
    if not tracks_cache:
        tracks_cache = os.path.join(tempfile.gettempdir(), "mixxx-test-tracks")
    os.makedirs(tracks_cache, exist_ok=True)

    # Download tracks (if not already cached) and build the catalog:
    # manifest metadata for every track file available to the tests.
    tracks_catalog = profile.ensure_track_catalog(
        tracks_cache, nb_tracks=LIBRARY_TRACK_COUNT)

    artifacts = args.artifacts_dir
    if not artifacts:
        artifacts = os.path.join(this_dir, "artifacts")
    os.makedirs(artifacts, exist_ok=True)

    recorder_proc = None
    recorder_name = "ffmpeg"
    display_proc = None
    cage_runtime_dir = None
    backend = "offscreen"
    video_path = None
    result_path = os.path.join(artifacts, f"results-{args.label or 'default'}.json")
    recording_start = None

    # --headless: Linux-only virtual display. Backend selection deferred to
    # _detect_backend so callers don't need to know whether Xvfb or cage is
    # installed. Both backends target a 1920x1080 framebuffer.
    if args.headless:
        backend = _detect_backend(args.display_backend, args.record)
        if backend == "xvfb":
            display_num = os.environ.get("MIXXX_TEST_DISPLAY", ":44")
            os.environ["DISPLAY"] = display_num
            os.environ["QT_QPA_PLATFORM"] = "xcb"
            display_proc = _start_xvfb(display_num)
        elif backend == "xwayland":
            display_proc, wayland_display, cage_runtime_dir = _start_cage()
            os.environ["WAYLAND_DISPLAY"] = wayland_display
            os.environ["XDG_RUNTIME_DIR"] = cage_runtime_dir
            os.environ["QT_QPA_PLATFORM"] = "wayland"
        else:
            os.environ["QT_QPA_PLATFORM"] = "offscreen"

    # --record: start the recorder on any platform with a usable display.
    if args.record:
        ffmpeg_bin = _find_ffmpeg(args.ffmpeg_path)
        wf_recorder_bin = args.wf_recorder_path or shutil.which("wf-recorder")
        if os.getenv("QT_QPA_PLATFORM") == "offscreen":
            print(
                "Using offscreen QPA. Cannot perform video recording!",
                file=sys.stderr,
            )
            return 1
        if backend == "xwayland":
            if not wf_recorder_bin:
                print(
                    "wf-recorder not found. Cannot record under xwayland backend!",
                    file=sys.stderr,
                )
                return 1
        elif not ffmpeg_bin:
            print(
                "ffmpeg not found. Cannot perform video recording!",
                file=sys.stderr,
            )
            return 1

        display = os.environ.get("DISPLAY", ":44")
        timestamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
        if args.label:
            video = f"mixxx-ui-test-{args.label}-{timestamp}.mkv"
        else:
            video = f"mixxx-ui-test-{timestamp}.mkv"
        result = video[:-4] + ".json"
        cmd = _build_record_cmd(
            backend, ffmpeg_bin, wf_recorder_bin, display, artifacts, video
        )
        if cmd is None:
            return 1

        video_path = os.path.join(artifacts, video)
        result_path = os.path.join(artifacts, result)
        recorder_name = cmd[0]
        recorder_env = None
        if backend == "xwayland":
            # wf-recorder must connect to the per-test cage compositor, not any
            # host session: explicit env ensures it picks up WAYLAND_DISPLAY.
            recorder_env = os.environ.copy()
        recorder_proc = subprocess.Popen(
            cmd,
            env=recorder_env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        recording_start = time.time()
        global RECORDING_START
        RECORDING_START = recording_start
        time.sleep(1)
        if recorder_proc.poll() is not None:
            print(
                f"{os.path.basename(recorder_name)} exited early with code "
                f"{recorder_proc.returncode}",
                file=sys.stderr,
            )
            stderr = recorder_proc.stderr.read().decode(
                "utf-8", errors="replace"
            )
            stdout = recorder_proc.stdout.read().decode(
                "utf-8", errors="replace"
            )
            if stderr:
                print(
                    f"\n=== {os.path.basename(recorder_name)} stderr ===",
                    file=sys.stderr,
                )
                print(stderr, file=sys.stderr)
                print("=== end recorder output ===\n", file=sys.stderr)
            if stdout:
                print(
                    f"\n=== {os.path.basename(recorder_name)} stdout ===",
                    file=sys.stderr,
                )
                print(stdout, file=sys.stderr)
                print("=== end recorder output ===\n", file=sys.stderr)
            return 1

    exit_code = 1
    try:
        config = Configuration(args.behave_args)
        runner = Runner(config)
        runner.config.format = ["mixxx_test_runner:XFailPrettyFormatter"]
        runner.config.show_timings = True
        runner.config.userdata = dict(
            tracks_dir=tracks_cache,
            tracks_catalog=tracks_catalog,
            binary=args.binary,
            fail_early=args.fail_early,
            retry_max_attempts=args.retry,
            summary_path=os.path.join(
                artifacts, f"behave-output-{args.label or 'default'}.txt"
            ),
            mixxx_output_dir=artifacts,
        )

        if args.no_capture:
            runner.config.capture_stdout = False
            runner.config.capture_stderr = False
        runner.run()

        for feature in runner.features:
            for scenario in feature.walk_scenarios():
                if scenario.status.name not in ["failed", "error"] or any(map(lambda t: t.startswith("xfail") or t.startswith("xpass"), scenario.tags)):
                    continue
                return exit_code
        exit_code = 0
    finally:
        if recorder_proc:
            recorder_name_pretty = os.path.basename(recorder_name)
            _stop_recorder(recorder_proc, recorder_name_pretty)
            _mux_chapters(video_path, getattr(runner.context, "results"), recording_start, _find_ffmpeg(args.ffmpeg_path))
        if "runner" in locals() and hasattr(runner.context, "results"):
            with open(result_path, "w") as f:
                json.dump(runner.context.results, f, indent=2)
        if display_proc:
            _stop_display_proc(display_proc)
        if cage_runtime_dir and os.path.isdir(cage_runtime_dir):
            shutil.rmtree(cage_runtime_dir, ignore_errors=True)

    return exit_code


if __name__ == "__main__":
    sys.exit(main())
