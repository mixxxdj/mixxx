"""DSL for setting up Mixxx test profiles and managing Mixxx process lifecycle."""

import collections
import json
import os
import shutil
import socket
import sqlite3
import subprocess
import glob
import sys
import tempfile
import threading
import random
import time
import urllib.request
import urllib.parse
import xml.etree.ElementTree as ET

TRACK_MANIFEST = os.path.join(os.path.dirname(__file__), "test_tracks.json")
SETTINGS_FILE = "mixxx.cfg"
DATABASE_FILE = "mixxxdb.sqlite"
SCHEMA_FILE = os.path.join(
    os.path.dirname(__file__),
    os.pardir,
    os.pardir,
    os.pardir,
    "res",
    "schema.xml",
)


def profile_dir():
    return os.path.join(os.path.expanduser("~"), ".mixxx-test-profile")


def database_path():
    return os.path.join(profile_dir(), DATABASE_FILE)


def mixxx_cfg_path():
    return os.path.join(profile_dir(), SETTINGS_FILE)


def load_track_manifest():
    with open(TRACK_MANIFEST) as f:
        return json.load(f)


def _parse_schema(schema_path):
    tree = ET.parse(schema_path)
    root = tree.getroot()
    statements = []
    for revision in root.findall("revision"):
        sql_el = revision.find("sql")
        if sql_el is None or not sql_el.text or not sql_el.text.strip():
            continue
        statements.append(sql_el.text.strip())
    return statements


def _read_schema_version(schema_path):
    """Return (latest_version, min_compatible_version) from the schema XML."""
    tree = ET.parse(schema_path)
    root = tree.getroot()
    latest_version = 0
    min_compatible = 0
    for revision in root.findall("revision"):
        ver = int(revision.get("version", "0"))
        if ver > latest_version:
            latest_version = ver
            min_compatible = int(revision.get("min_compatible", "0"))
    return latest_version, min_compatible


def _database_has_tables(db_path):
    try:
        cur = sqlite3.connect(db_path).cursor()
        cur.execute("SELECT count(*) FROM sqlite_master WHERE type='table'")
        count = cur.fetchone()[0]
        cur.connection.close()
        return count > 0
    except Exception:
        return False


def _download_file(entry, dest):
    url = entry["url"]
    req = urllib.request.Request(entry["url"], headers={"User-Agent": "MixxxTestProfile/1.0"})
    with tempfile.NamedTemporaryFile(delete_on_close=False) as f, tempfile.NamedTemporaryFile(delete_on_close=False) as c:
        with urllib.request.urlopen(req, timeout=30) as response:
            f.write(response.read())
            f.close()
        if "artwork" in entry and entry["artwork"]:
            req = urllib.request.Request(entry["artwork"], headers={"User-Agent": "MixxxTestProfile/1.0"})
            with urllib.request.urlopen(req, timeout=30) as response:
                c.write(response.read())
                c.close()
            p = subprocess.run([
                os.getenv("MIXXX_FFMPEG_BIN") or "ffmpeg", "-nostdin", "-hide_banner", "-loglevel", "info",
                "-i", f.name,
                "-i", c.name,
                "-map", "0:a", "-map", "1",
                "-c:a", "copy",
                "-c:v", "mjpeg",
                # No shell is involved (subprocess.run with a list of args),
                # so the value is passed verbatim to ffmpeg's -metadata
                # key=value. Do not use repr()/quotes: ffmpeg stores metadata
                # values literally, so 'Foo' would embed the quotes.
                "-metadata", f"title={entry['title']}",
                "-metadata", f"artist={entry['artist']}",
                "-metadata:s:v", "title=Album cover",
                "-metadata:s:v", "comment=Cover (front)",
                dest
            ], text=True, capture_output=True)
            if p.returncode:
                print(p.returncode)
                print(p.stdout)
                print(p.stderr)
                sys.stdout.write(
                    f"  ffmpeg failed to mux artwork for {entry['url']}, "
                    f"falling back to raw MP3\n"
                )
                sys.stdout.flush()
                os.unlink(f.name)
                raise RuntimeError("Unable to set the metadata for test track")
        else:
            os.replace(f.name, dest)


def track_filename(entry):
    """Name of the downloaded file for a manifest entry."""
    return os.path.basename(urllib.parse.urlparse(entry["url"]).path)


def available_tracks(target_dir):
    """Catalog of the tracks actually present in target_dir.

    Builds a copy of each manifest entry whose downloaded file exists
    (non-empty) in target_dir, adding its absolute path as ``location``.
    """
    # Entries that share a downloaded filename with another entry cannot be
    # attributed to a single file unambiguously and are excluded.
    counts = collections.Counter(
        track_filename(entry) for entry in load_track_manifest()
    )
    ambiguous = {name for name, count in counts.items() if count > 1}
    catalog = []
    for entry in load_track_manifest():
        filename = track_filename(entry)
        if filename in ambiguous:
            continue
        filepath = os.path.join(target_dir, filename)
        if not os.path.isfile(filepath) or os.path.getsize(filepath) == 0:
            continue
        entry = dict(entry)
        entry["location"] = filepath
        catalog.append(entry)
    return catalog


def ensure_track_catalog(target_dir, nb_tracks, seed=None):
    """Download missing tracks from the manifest into target_dir.

    Downloads only as many tracks as needed to reach nb_tracks files, then
    returns the resulting catalog (see available_tracks): the manifest
    metadata of every track file present in target_dir, with ``location``.

    ``seed`` pins which manifest entries are sampled for download, so two
    machines (or a wiped cache) fetch the same set for the same run seed;
    without it the selection is random. Already-present files are kept
    regardless of the seed.
    """
    os.makedirs(target_dir, exist_ok=True)
    existing_track_count = len(glob.glob(f'{target_dir}/*.mp3'))
    if existing_track_count < nb_tracks:
        rng = random.Random(seed) if seed is not None else random
        manifest = available_tracks(target_dir)
        catalog = load_track_manifest()
        candidates = [v for v in catalog if all(map(lambda e: e['url'] != v['url'], manifest))]
        while len(manifest) < nb_tracks:
            if not candidates:
                raise ValueError("unable to find a new track. Has the pool starved?")
            entry = rng.choice(candidates)
            candidates.remove(entry)

            url = entry.get("url")
            assert url, f"Track {entry} has no URL"
            filename = track_filename(entry)
            dest = os.path.join(target_dir, filename)
            if os.path.exists(dest) and os.path.getsize(dest) > 0:
                continue
            sys.stdout.write(f"Downloading {filename}...\n")
            sys.stdout.flush()
            try:
                _download_file(entry, dest)
                if not os.path.exists(dest) or os.path.getsize(dest) == 0:
                    raise RuntimeError(f"Download failed, no output at {dest}")
                manifest.append(entry)
            except Exception as e:
                sys.stdout.write(f"  FAILED: {e}\n")
                sys.stdout.flush()
    return available_tracks(target_dir)


def create_empty_profile(settings_dir=None):
    """Create a profile with schema-only database (no data rows)."""
    if settings_dir is None:
        settings_dir = profile_dir()
    os.makedirs(settings_dir, exist_ok=True)


def add_directory_to_db(track_dir, settings_dir=None):
    """Add a directory to the library's directory table."""
    if settings_dir is None:
        settings_dir = profile_dir()
    db_path = os.path.join(settings_dir, DATABASE_FILE)
    if not os.path.exists(db_path):
        return
    conn = sqlite3.connect(db_path)
    conn.execute(
        "INSERT OR IGNORE INTO directories (directory) VALUES (?)",
        (track_dir,),
    )
    conn.commit()
    conn.close()


_MIXXX_PROCESS = None


def _print_output_tail(lines, output_path, tail=40):
    """Print a bounded tail of the application output to the transcript.

    The complete output of every spawn is persisted as a test artifact
    (``mixxx-output.<pid>.log``), so the transcript only carries the last
    lines to stay readable on failure while still pointing at the full log.
    """
    lines = list(lines)
    if lines:
        print(
            f"--- mixxx-test output tail (last {min(len(lines), tail)} lines) ---",
            file=sys.stderr,
        )
        for line in lines[-tail:]:
            sys.stderr.write(line)
        print("--- end mixxx-test output tail ---", file=sys.stderr)
    if output_path:
        print(f"Full application output: {output_path}", file=sys.stderr)


class MixxxProcess:
    """Manages a mixxx-test --serve subprocess for UI testing."""

    #: Number of recent output lines kept in memory for startup diagnostics.
    OUTPUT_TAIL_LINES = 200

    def __init__(self, binary, profile_dir, display=None, output_dir=None):
        self.binary = binary
        self.profile_dir = profile_dir
        self.display = display
        self.output_dir = output_dir
        self.process = None
        self.rpc = None
        self.output_path = None
        self._output_file = None
        self.last_pid = None
        self.last_exit_code = None

    def _open_output_log(self, pid):
        """Open the per-spawn application output log, if requested.

        Every spawn writes its own ``mixxx-output.<pid>.log`` so crash and
        kill/restart cycles stay separated. Returns None (keeping the
        in-memory-only behavior) when no output dir is configured or the
        file cannot be created.
        """
        if not self.output_dir:
            return None
        path = os.path.join(self.output_dir, f"mixxx-output.{pid}.log")
        try:
            f = open(path, "w", encoding="utf-8", errors="replace")
        except OSError as e:
            print(
                f"Warning: cannot open application output log {path}: {e}",
                file=sys.stderr,
            )
            return None
        self.output_path = path
        return f

    def _close_output_log(self):
        if self._output_file is None:
            return
        try:
            self._output_file.close()
        except Exception:
            pass
        self._output_file = None

    def _write_output_banner(self, text):
        if self._output_file is None:
            return
        try:
            self._output_file.write(text)
            self._output_file.flush()
        except Exception:
            self._output_file = None

    @staticmethod
    def _format_exit_code(code):
        if code is None:
            return "unknown"
        if code < 0:
            # POSIX: terminated by signal -code
            return f"{code} (signal {-code})"
        # Windows exit codes are large unsigned values (e.g. 3221225477 is
        # 0xC0000005, an access violation); the hex form makes them readable.
        return f"{code} (0x{code & 0xFFFFFFFF:08X})"

    # Fresh profiles apply schema migrations and load the full skin at first
    # start, which can exceed the historical 20 s on loaded systems.
    def start(self, timeout=20):
        env = os.environ.copy()
        if self.display:
            env["DISPLAY"] = self.display

        args = [
            self.binary,
            "--serve",
            "--settings-path",
            self.profile_dir,
            "--developer",
            "--log-level",
            "debug",
        ]
        RPC_PORT = 9000
        self.process = subprocess.Popen(
            args,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )

        output_file = self._open_output_log(self.process.pid)
        self._output_file = output_file
        self._write_output_banner(
            f"===== spawn {time.strftime('%Y-%m-%d %H:%M:%S')} "
            f"pid={self.process.pid} binary={self.binary} "
            f"profile={self.profile_dir} =====\n"
        )

        output_lines = collections.deque(maxlen=self.OUTPUT_TAIL_LINES)

        def _pipe_logger():
            nonlocal output_file
            assert self.process is not None
            assert self.process.stdout is not None
            for line in self.process.stdout:
                output_lines.append(line)
                if output_file is None:
                    continue
                try:
                    output_file.write(line)
                    output_file.flush()
                except Exception:
                    # Log persistence must never kill the reader thread.
                    output_file = None

        self._log_thread = threading.Thread(target=_pipe_logger, daemon=True)
        self._log_thread.start()

        deadline = time.time() + timeout
        while time.time() < deadline:
            try:
                s = socket.socket()
                s.settimeout(1)
                s.connect(("localhost", RPC_PORT))
                s.close()
                return
            except (OSError, ConnectionRefusedError):
                pass
            if self.process.poll() is not None:
                _print_output_tail(output_lines, self.output_path)
                raise RuntimeError(
                    f"mixxx-test exited early with code "
                    f"{self._format_exit_code(self.process.returncode)}"
                )
            time.sleep(1)
        # The process is still alive but never opened the RPC port. Dump the
        # tail of its output: it is the only clue to whether it is hung on a
        # database/lock migration, blocked on the audio device, or retrying
        # the port bind. Then reap it: an unresponsive instance left running
        # here outlives the scenario — holding port 9000 or the audio
        # devices — and turns every later fresh-profile spawn of the run
        # into another start() timeout.
        _print_output_tail(output_lines, self.output_path)
        pid = self.process.pid
        self.stop()
        raise RuntimeError(
            f"Timed out waiting for Mixxx RPC on port {RPC_PORT} "
            f"after {timeout}s (unresponsive process pid {pid} killed)"
        )

    def stop(self):
        """Reap the subprocess; returns True when a process was stopped.

        Appends an exit banner with the return code to the per-spawn output
        log (decimal plus hex, so Windows crash codes like 0xC0000005 are
        recognizable) and prints a one-line stop notice so kill/restart
        cycles are visible in the behave transcript, not only in the log
        file.
        """
        if self.process is None:
            self._close_output_log()
            return False
        self.process.terminate()
        try:
            self.process.wait(timeout=15)
        except subprocess.TimeoutExpired:
            self.process.kill()
            self.process.wait()
        pid = self.process.pid
        code = self._format_exit_code(self.process.returncode)
        self.process = None
        self.last_pid = pid
        self.last_exit_code = code
        # Let the reader thread drain and exit before closing its sink.
        thread = getattr(self, "_log_thread", None)
        if thread is not None:
            try:
                thread.join(timeout=5)
            except RuntimeError:
                pass
        self._write_output_banner(
            f"===== exited {time.strftime('%Y-%m-%d %H:%M:%S')} "
            f"pid={pid} code={code} =====\n"
        )
        self._close_output_log()
        print(f"mixxx-test stopped: pid={pid} exit={code}")
        return True

    def __enter__(self):
        self.start()
        global _MIXXX_PROCESS
        _MIXXX_PROCESS = self
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        global _MIXXX_PROCESS
        _MIXXX_PROCESS = None
        self.stop()


def current_mixxx():
    """Return the active MixxxProcess (set by context manager or start)."""
    return _MIXXX_PROCESS


def setup_profile(base_dir, profile_type):
    """Create a profile of the given type in base_dir.

    Types:
      'empty'   - Folder is ready
    """
    create_empty_profile(base_dir)
    return base_dir


def make_temp_profile(profile_type="empty"):
    """Create a temporary profile directory and return its path."""
    base = tempfile.mkdtemp(prefix="mixxx-test-profile-")
    return setup_profile(base, profile_type)


def cleanup_profile(profile_dir):
    """Remove a profile directory created by make_temp_profile."""
    if profile_dir and os.path.exists(profile_dir):
        shutil.rmtree(profile_dir, ignore_errors=True)
