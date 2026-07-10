#!/usr/bin/env python3
"""CTest entry point for the Mixxx behave UI tests.

Usage: mixxx_test_runner.py [options] [feature_file...]

Sets up a global tracks cache, optional headless display (Linux) and
optional video recording, then runs behave.
Profile setup and mixxx-test lifecycle are handled by the behave steps.
"""

import argparse
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


def _find_ffmpeg(explicit_path=None):
    if explicit_path and os.path.isfile(explicit_path):
        return explicit_path
    return shutil.which("ffmpeg")

def _build_record_cmd(ffmpeg_bin, display, artifacts_dir, video):
    output = os.path.join(artifacts_dir, video)
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
            "-c:v", "libx264",
            "-preset", "veryfast",
            "-b:v", "2000k",
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


def _stop_ffmpeg(ffmpeg_proc):
    ffmpeg_proc.terminate()
    time.sleep(3)
    ffmpeg_ret = ffmpeg_proc.poll()
    if ffmpeg_ret is not None:
        stderr_output = ffmpeg_proc.stderr.read().decode(
            "utf-8", errors="replace"
        )
        if stderr_output:
            print(
                f"\n=== ffmpeg terminated unexpectedly "
                f"(exit code: {ffmpeg_ret}) ===",
                file=sys.stderr,
            )
            print(stderr_output, file=sys.stderr)
            print("=== end ffmpeg output ===\n", file=sys.stderr)
    else:
        try:
            ffmpeg_proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            if sys.platform == "win32":
                ffmpeg_proc.send_signal(signal.CTRL_C_EVENT)
            else:
                ffmpeg_proc.send_signal(signal.SIGINT)
            try:
                ffmpeg_proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                ffmpeg_proc.kill()


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
        p = subprocess.run(cmd, capture_output=True, text=True)
        if p.returncode != 0:
            print(
                f"\n=== ffmpeg chapter muxing failed (exit code: {e.returncode}) ===",
                file=sys.stderr,
            )
            print(p.stdout.decode(
                "utf-8", errors="replace"
            ), p.stderr.decode(
                "utf-8", errors="replace"
            ), file=sys.stderr)
        os.replace(tmp, video_path)
    except subprocess.CalledProcessError as e:
        print(
            f"\n=== ffmpeg chapter muxing failed (exit code: {e.returncode}) ===",
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
        help="Linux only: run with Xvfb virtual display",
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
        help="Path to ffmpeg binary",
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

    # Download tracks if not already cached
    profile.ensure_tracks_downloaded(tracks_cache)

    artifacts = args.artifacts_dir
    if not artifacts:
        artifacts = os.path.join(this_dir, "artifacts")
    os.makedirs(artifacts, exist_ok=True)

    ffmpeg_proc = None
    xvfb_proc = None
    video_path = None
    recording_start = None

    # --headless: Linux-only Xvfb setup
    if args.headless:
        if shutil.which("Xvfb"):
            display_num = os.environ.get("MIXXX_TEST_DISPLAY", ":44")
            os.environ["DISPLAY"] = display_num
            os.environ["QT_QPA_PLATFORM"] = "xcb"

            xvfb_proc = subprocess.Popen(
                [
                    "Xvfb",
                    display_num,
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
        else:
            if sys.platform == "linux":
                print(
                    "Xvfb not found - using offscreen platform",
                    file=sys.stderr,
                )
            else:
                print(
                    f"--headless is Linux-only (current platform: {sys.platform}) - "
                    "using offscreen platform",
                    file=sys.stderr,
                )
            os.environ["QT_QPA_PLATFORM"] = "offscreen"

    # --record: start ffmpeg on any platform
    if args.record:
        ffmpeg_bin = _find_ffmpeg(args.ffmpeg_path)
        if os.getenv("QT_QPA_PLATFORM") == "offscreen":
            print(
                "Using offscreen QPA. Cannot perform video recording!",
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
            ffmpeg_bin, display, artifacts, video
        )
        if cmd is None:
            return 1

        video_path = os.path.join(artifacts, video)
        result_path = os.path.join(artifacts, result)
        ffmpeg_proc = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        recording_start = time.time()
        time.sleep(1)
        if ffmpeg_proc.poll() is not None:
            print(
                f"ffmpeg exited early with code {ffmpeg_proc.returncode}",
                file=sys.stderr,
            )
            stderr = ffmpeg_proc.stderr.read().decode(
                "utf-8", errors="replace"
            )
            stdout = ffmpeg_proc.stdout.read().decode(
                "utf-8", errors="replace"
            )
            if stderr:
                print(
                    f"\n=== ffmpeg stderr ===",
                    file=sys.stderr,
                )
                print(stderr, file=sys.stderr)
                print("=== end ffmpeg output ===\n", file=sys.stderr)
            if stdout:
                print(
                    f"\n=== ffmpeg stdout ===",
                    file=sys.stderr,
                )
                print(stdout, file=sys.stderr)
                print("=== end ffmpeg output ===\n", file=sys.stderr)
            return 1

    exit_code = 1
    try:
        config = Configuration(args.behave_args)
        runner = Runner(config)
        runner.config.format = ["mixxx_test_runner:XFailPrettyFormatter"]
        runner.config.show_timings = True
        runner.config.userdata = dict(
            tracks_dir=tracks_cache,
            binary=args.binary,
            fail_early=args.fail_early,
            retry_max_attempts=args.retry,
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
        if ffmpeg_proc:
            _stop_ffmpeg(ffmpeg_proc)
            _mux_chapters(video_path, runner.context.results, recording_start, _find_ffmpeg(args.ffmpeg_path))
            # Saving after chapter muxing so we have relative timecode available
            with open(result_path, "w") as f:
                json.dump(runner.context.results, f, indent=2)
        if xvfb_proc:
            xvfb_proc.terminate()
            xvfb_proc.wait()

    return exit_code


if __name__ == "__main__":
    sys.exit(main())
