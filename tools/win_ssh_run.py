#!/usr/bin/env python3
r"""Run commands on, and move files to and from, a Windows host over SSH.

This script was drafted autonomously by an AI agent and is offered for a human
reviewer to check, amend and submit.

Mixxx's behave UI tests drive a real GUI window, and a GUI application cannot
start in the SSH session that lands in Windows session 0. So this bridge only
carries the *invocation*; Start-InteractiveProcessOnWindows.ps1 re-launches the
actual test process on the logged-on desktop through an interactive scheduled
task. See src/test/behave/AGENTS.md, "Running the tests on Windows".

Credentials come from the environment so that no secret is ever written to the
repository:

    export MIXXX_WIN_HOST=192.168.1.10
    export MIXXX_WIN_USER=someuser
    export MIXXX_WIN_PASSWORD=...
    export MIXXX_WIN_ROOT='D:\dev\mixxx'      # optional, for relative paths

Subcommands:

    run   LOG TIMEOUT -- COMMAND     stream COMMAND on the remote cmd.exe into
                                     LOG, return the remote exit status
    put   LOCAL REMOTE               upload LOCAL via SFTP
    get   REMOTE LOCAL               download REMOTE via SFTP
    sync  LOCAL_DIR                  upload LOCAL_DIR recursively over the
                                     remote root, creating directories as
                                     needed and skipping files whose mtime
                                     already matches. Intended for pushing
                                     edited test/QML sources during a debug
                                     loop without a commit per iteration.
                                     One-directional: nothing is ever deleted.
"""

import argparse
import codecs
import os
import select
import sys
import threading
import time

try:
    import paramiko
except ImportError:  # pragma: no cover - depends on the caller's environment
    sys.exit("win_ssh_run.py needs paramiko: pip install paramiko")

ENV_HOST = "MIXXX_WIN_HOST"
ENV_USER = "MIXXX_WIN_USER"
ENV_PASSWORD = "MIXXX_WIN_PASSWORD"
ENV_PORT = "MIXXX_WIN_PORT"
ENV_ROOT = "MIXXX_WIN_ROOT"

# Exit status reported when the local timeout fires. Matches the convention
# already used by the PowerShell launcher, and `timeout(1)`.
EXIT_TIMEOUT = 124

# Never mirror these into a remote tree with "sync": they are per-host, and
# copying a Linux virtualenv over a Windows one breaks the remote interpreter.
_SYNC_SKIP_DIRS = frozenset(
    {
        ".venv",
        "venv",
        "__pycache__",
        ".git",
        ".mypy_cache",
        ".pytest_cache",
        ".ruff_cache",
    }
)
_SYNC_SKIP_SUFFIXES = (".pyc", ".pyo", ".so", ".pyd")


def _credentials():
    host = os.environ.get(ENV_HOST)
    user = os.environ.get(ENV_USER)
    password = os.environ.get(ENV_PASSWORD)

    missing = [
        name
        for name, value in (
            (ENV_HOST, host),
            (ENV_USER, user),
            (ENV_PASSWORD, password),
        )
        if not value
    ]
    if missing:
        names = ", ".join(missing)
        raise SystemExit(
            f"missing environment variable(s): {names}\n"
            f"Set {ENV_HOST}, {ENV_USER} and {ENV_PASSWORD} before "
            "running this script."
        )

    port = int(os.environ.get(ENV_PORT, "22") or 22)
    return host, port, user, password


def _connect(trust_new_host=False):
    host, port, user, password = _credentials()

    client = paramiko.SSHClient()
    client.load_system_host_keys()
    if trust_new_host:
        # Blindly accepts an unknown host key, so it is vulnerable to a
        # machine-in-the-middle. Only for a throwaway lab VM on a trusted LAN.
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    else:
        client.set_missing_host_key_policy(paramiko.RejectPolicy())

    try:
        client.connect(
            host,
            port=port,
            username=user,
            password=password,
            timeout=30,
            look_for_keys=False,
            allow_agent=False,
        )
    except paramiko.ssh_exception.SSHException as error:
        if (
            not trust_new_host
            and "not found in known_hosts" in str(error).lower()
        ):
            raise SystemExit(
                f"{error}\n"
                "Add the host key once with ssh, or pass --trust-new-host "
                "to accept it without verification "
                "(machine-in-the-middle risk)."
            )
        raise

    return client


def _resolve_remote(path):
    """Return an absolute remote path.

    A leading / is treated as the remote root.
    """
    if path.startswith("/") or (len(path) > 1 and path[1] == ":"):
        return path
    root = os.environ.get(ENV_ROOT)
    if not root:
        raise SystemExit(
            f"{path!r} is relative, so it would land under the SSH "
            f"account's home directory. Set {ENV_ROOT} to the remote "
            "repository root, or pass an absolute path."
        )
    return _win_join(root, path)


# The remote filesystem is Windows while this client is usually Linux, so
# os.path cannot be used for remote paths: posixpath treats "\" as an ordinary
# character and would join "D:\dev" with "mixxx" as "D:\dev/mixxx".


def _win_join(directory, name):
    return directory.rstrip("\\/") + "\\" + name


def _win_dirname(path):
    """Directory part of a Windows path, or "" at the drive root."""
    trimmed = path.rstrip("\\/")
    index = trimmed.rfind("\\")
    if index < 0:
        # A bare drive ("D:") or a relative path with no separator.
        return trimmed[:2] if len(trimmed) >= 2 and trimmed[1] == ":" else ""
    if index <= 2 and trimmed[1:2] == ":":
        # "D:\dev" -> "D:"
        return trimmed[:2]
    return trimmed[:index]


# --------------------------------------------------------------------------- #
# run
# --------------------------------------------------------------------------- #


def _pump(channel, stream, log, lock, tag):
    """Drain one of the channel's two streams into the log and the console."""
    is_stderr = tag == "stderr"
    ready = channel.recv_stderr_ready if is_stderr else channel.recv_ready
    recv = channel.recv_stderr if is_stderr else channel.recv

    # Decode incrementally. A 64 KiB chunk boundary can land in the middle of
    # a multi-byte UTF-8 sequence, and decoding each chunk on its own would
    # turn the tail of such a character into a replacement glyph.
    decoder = codecs.getincrementaldecoder("utf-8")(errors="replace")

    while True:
        if not ready():
            if channel.eof_received and not ready():
                break
            select.select([channel], [], [], 0.1)
            continue

        data = recv(65536)
        if not data:
            if channel.eof_received:
                break
            continue

        text = decoder.decode(data)
        if not text:
            continue

        with lock:
            stream.write(text)
            stream.flush()
            log.write(text)
            log.flush()

    # Flush anything the decoder was still holding back.
    tail = decoder.decode(b"", final=True)
    if tail:
        with lock:
            stream.write(tail)
            stream.flush()
            log.write(tail)
            log.flush()


def cmd_run(args):
    client = _connect(args.trust_new_host)
    try:
        channel = client.get_transport().open_session()
        channel.exec_command(args.command)

        with open(args.log, "w", encoding="utf-8", errors="replace") as log:
            log.write(f"# {args.command}\n\n")
            log.flush()

            lock = threading.Lock()
            stdout = sys.stdout if not args.quiet else _NullStream()
            stderr = sys.stderr if not args.quiet else _NullStream()
            threads = [
                threading.Thread(
                    target=_pump,
                    args=(channel, stdout, log, lock, "stdout"),
                    daemon=True,
                ),
                threading.Thread(
                    target=_pump,
                    args=(channel, stderr, log, lock, "stderr"),
                    daemon=True,
                ),
            ]
            for thread in threads:
                thread.start()

            start = time.time()
            timed_out = False
            while not channel.exit_status_ready():
                if time.time() - start > args.timeout:
                    with lock:
                        log.write(
                            f"\n[local timeout after {args.timeout}s, "
                            "closing channel]\n"
                        )
                        log.flush()
                    timed_out = True
                    channel.close()
                    break
                time.sleep(0.2)

            if not timed_out:
                status = channel.recv_exit_status()
                for thread in threads:
                    thread.join(timeout=5)
                with lock:
                    log.write(f"\n[exit={status}]\n")
                    log.flush()
                return status

            return EXIT_TIMEOUT
    finally:
        client.close()


class _NullStream:
    def write(self, _text):
        return 0

    def flush(self):
        pass


# --------------------------------------------------------------------------- #
# put / get
# --------------------------------------------------------------------------- #


def cmd_put(args):
    remote = _resolve_remote(args.remote)
    client = _connect(args.trust_new_host)
    try:
        sftp = client.open_sftp()
        try:
            _put(sftp, args.local, remote)
        finally:
            sftp.close()
    finally:
        client.close()

    if not args.quiet:
        print(f"{args.local} -> {remote}")
    return 0


def _put(sftp, local, remote):
    _mkdirs(sftp, _win_dirname(remote))
    sftp.put(local, remote)
    mtime = os.path.getmtime(local)
    sftp.utime(remote, (mtime, mtime))


def _mkdirs(sftp, remote_dir):
    """Create remote_dir and every missing parent, tolerating existing ones."""
    if not remote_dir:
        return
    parent = _win_dirname(remote_dir)
    if parent and parent != remote_dir:
        _mkdirs(sftp, parent)
    try:
        sftp.stat(remote_dir)
    except OSError:
        sftp.mkdir(remote_dir)


def cmd_get(args):
    remote = _resolve_remote(args.remote)
    client = _connect(args.trust_new_host)
    try:
        sftp = client.open_sftp()
        try:
            local_dir = os.path.dirname(os.path.abspath(args.local))
            os.makedirs(local_dir, exist_ok=True)
            sftp.get(remote, args.local)
        finally:
            sftp.close()
    finally:
        client.close()

    if not args.quiet:
        print(f"{remote} -> {args.local}")
    return 0


# --------------------------------------------------------------------------- #
# sync
# --------------------------------------------------------------------------- #


def cmd_sync(args):
    """Mirror a local directory tree onto the remote root.

    Deliberately one-directional and non-destructive: it pushes files that are
    missing or stale remotely and never deletes anything. The remote side is a
    live git working tree, so removing "files that no longer exist locally"
    would be a footgun rather than a convenience.
    """
    remote_root = _resolve_remote(args.remote)
    local_root = os.path.abspath(args.local_dir)

    if not os.path.isdir(local_root):
        raise SystemExit(f"{local_root} is not a directory")

    client = _connect(args.trust_new_host)
    try:
        sftp = client.open_sftp()
        try:
            copied, skipped = _sync_tree(
                sftp, local_root, remote_root, args.quiet
            )
        finally:
            sftp.close()
    finally:
        client.close()

    print(f"synced {copied} file(s), {skipped} already up to date")
    return 0


def _sync_tree(sftp, local_dir, remote_dir, quiet):
    _mkdirs(sftp, remote_dir)

    copied = 0
    skipped = 0
    for name in sorted(os.listdir(local_dir)):
        local_path = os.path.join(local_dir, name)

        if name in _SYNC_SKIP_DIRS or name.endswith(_SYNC_SKIP_SUFFIXES):
            # A virtualenv or a __pycache__ tree is host-specific: uploading
            # the Linux one over the Windows one replaces pyvenv.cfg with
            # "home = /usr/bin" and the remote interpreter stops resolving.
            # Byte-compiled files are tied to their build host too.
            continue

        if os.path.islink(local_path):
            # The Windows box runs git and Ninja with core.symlinks off, so a
            # symlink here would land as a plain file. Skipping is safer than
            # writing something that looks right locally and wrong remotely.
            continue

        if os.path.isdir(local_path):
            nested_copied, nested_skipped = _sync_tree(
                sftp, local_path, _win_join(remote_dir, name), quiet
            )
            copied += nested_copied
            skipped += nested_skipped
            continue

        if not os.path.isfile(local_path):
            continue

        remote_path = _win_join(remote_dir, name)
        local_mtime = int(os.path.getmtime(local_path))

        try:
            remote_mtime = int(sftp.stat(remote_path).st_mtime)
        except OSError:
            remote_mtime = None

        if remote_mtime is not None and abs(remote_mtime - local_mtime) <= 1:
            skipped += 1
            continue

        _put(sftp, local_path, remote_path)
        copied += 1
        if not quiet:
            print(f"  put {name}")

    return copied, skipped


# --------------------------------------------------------------------------- #
# entry point
# --------------------------------------------------------------------------- #


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    _add_global_flags(parser)

    sub = parser.add_subparsers(dest="subcommand", required=True)

    p_run = sub.add_parser(
        "run", help="stream a remote command into a local log"
    )
    _add_global_flags(p_run)
    p_run.add_argument(
        "log", help="local log file to write the remote output to"
    )
    p_run.add_argument("timeout", type=int, help="local timeout in seconds")
    p_run.add_argument(
        "command", help="command line, passed verbatim to the remote cmd.exe"
    )
    p_run.add_argument(
        "-q", "--quiet", action="store_true", help="do not echo output locally"
    )
    p_run.set_defaults(func=cmd_run)

    p_put = sub.add_parser("put", help="upload one file")
    _add_global_flags(p_put)
    p_put.add_argument("local")
    p_put.add_argument("remote")
    p_put.add_argument("-q", "--quiet", action="store_true")
    p_put.set_defaults(func=cmd_put)

    p_get = sub.add_parser("get", help="download one file")
    _add_global_flags(p_get)
    p_get.add_argument("remote")
    p_get.add_argument("local")
    p_get.add_argument("-q", "--quiet", action="store_true")
    p_get.set_defaults(func=cmd_get)

    p_sync = sub.add_parser(
        "sync", help="mirror a local directory onto the remote root"
    )
    _add_global_flags(p_sync)
    p_sync.add_argument("local_dir")
    p_sync.add_argument(
        "remote", help="remote target directory (relative to $MIXXX_WIN_ROOT)"
    )
    p_sync.add_argument("-q", "--quiet", action="store_true")
    p_sync.set_defaults(func=cmd_sync)

    args = parser.parse_args()
    return args.func(args)


def _add_global_flags(parser):
    parser.add_argument(
        "--trust-new-host",
        action="store_true",
        help="accept an unknown SSH host key without verification (MITM risk)",
    )


if __name__ == "__main__":
    sys.exit(main())
