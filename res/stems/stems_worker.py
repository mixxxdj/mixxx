"""Separate a track into stems and write a ``.stem.mp4`` for Mixxx.

This is the compute side of Mixxx's stem generation: Mixxx spawns it as a child
process and reads one line per message from stdout, so the protocol is tiny and
must stay stable:

    STATUS <text>          human readable phase, shown in the queue
    PROGRESS <0-100>       integer percentage of the whole job
    RESULT {json}          final numbers (seconds, realtime factor, provider)
    DONE <path>            the container is written and ready to load
    ERROR <text>           something failed, exit code is non zero

``probe`` prints a single JSON object describing the environment (ONNX Runtime
providers, GPU, ffmpeg, installed packages) so the user interface can warn
before starting a long job.

The heavy lifting is done by the ``demucs-onnx`` package (ONNX Runtime, no
PyTorch) and by ``stem_mp4.py``, which builds the NI style container.
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import stem_mp4  # noqa: E402  (needs the path above when run as a script)

STEM_NAMES = ("drums", "bass", "other", "vocals")
STEM_TITLES = stem_mp4.STEM_NAMES

# Measured on the development machine with the shipped default model
# (htdemucs, fp32) over a 243 s track: 22.4 s on an RTX 3060 through CUDA and
# 102.4 s on an i5-12400F. They are only used to estimate a job duration.
REALTIME_FACTOR_GPU = 10.9
REALTIME_FACTOR_CPU = 2.4

# Part of the progress bar that belongs to the separation itself; the rest is
# encoding and muxing.
_SEPARATION_SHARE = 90


def emit(kind: str, value: object) -> None:
    print(f"{kind} {value}", flush=True)


def gpu_name() -> str:
    for tool in ("nvidia-smi", "nvidia-smi.exe"):
        if shutil.which(tool) is None:
            continue
        try:
            result = subprocess.run(
                [tool, "--query-gpu=name", "--format=csv,noheader"],
                capture_output=True, text=True, timeout=10)
        except (OSError, subprocess.SubprocessError):
            continue
        if result.returncode == 0 and result.stdout.strip():
            return result.stdout.strip().splitlines()[0]
    return ""


def probe(ffmpeg: str) -> dict:
    """Describe the environment; never raises."""
    report: dict = {"python": sys.version.split()[0], "errors": []}
    try:
        import onnxruntime as ort

        providers = ort.get_available_providers()
        report["onnxruntime"] = ort.__version__
        report["providers"] = providers
        report["has_cuda"] = "CUDAExecutionProvider" in providers
        report["has_directml"] = "DmlExecutionProvider" in providers
    except Exception as error:  # noqa: BLE001 - reported to the user
        report["has_cuda"] = False
        report["errors"].append(f"onnxruntime: {error}")

    try:
        import demucs_onnx

        report["demucs_onnx"] = getattr(demucs_onnx, "__version__", "?")
    except Exception as error:  # noqa: BLE001 - reported to the user
        report["errors"].append(f"demucs_onnx: {error}")

    resolved_ffmpeg = shutil.which(ffmpeg)
    report["ffmpeg"] = resolved_ffmpeg or ""
    if not resolved_ffmpeg:
        report["errors"].append(f"ffmpeg not found: {ffmpeg}")

    report["gpu"] = gpu_name()
    report["provider"] = "cuda" if report.get("has_cuda") else "cpu"
    report["realtime_factor"] = (
        REALTIME_FACTOR_GPU if report["provider"] == "cuda" else REALTIME_FACTOR_CPU)
    report["ok"] = not report["errors"]
    return report


def install_progress_hook(callback) -> bool:
    """Report the separator's chunk progress to ``callback``.

    ``demucs_onnx`` only draws a tqdm bar, which is useless for a parent
    process, so its internal chunk iterator is wrapped. If the library changes
    its internals the separation still works, only the fine grained progress is
    lost.
    """
    try:
        from demucs_onnx import inference as demucs_inference
    except Exception:  # noqa: BLE001 - optional optimisation
        return False
    if not hasattr(demucs_inference, "_iter_with_progress"):
        return False

    def patched(iterable, *, total, desc, enabled):  # noqa: ARG001 - signature
        total = max(int(total or 1), 1)
        for index, item in enumerate(iterable):
            yield item
            callback((index + 1) / total)

    demucs_inference._iter_with_progress = patched
    return True


def choose_provider(requested: str) -> str:
    if requested in ("cuda", "cpu"):
        return requested
    try:
        import onnxruntime as ort

        if "CUDAExecutionProvider" in ort.get_available_providers():
            return "cuda"
    except Exception:  # noqa: BLE001 - fall back to the CPU
        pass
    return "cpu"


def separate(args) -> int:
    started = time.perf_counter()
    if not os.path.isfile(args.input):
        emit("ERROR", f"input file not found: {args.input}")
        return 2

    try:
        import demucs_onnx
    except Exception as error:  # noqa: BLE001 - reported to the user
        emit("ERROR", f"demucs_onnx is not installed: {error}")
        return 3

    provider = choose_provider(args.provider)
    emit("STATUS", f"provider={provider}")

    def on_chunk(fraction: float) -> None:
        emit("PROGRESS", int(max(0.0, min(1.0, fraction)) * _SEPARATION_SHARE))

    hook = install_progress_hook(on_chunk)
    emit("STATUS", "separating" + ("" if hook else " (without fine progress)"))

    with tempfile.TemporaryDirectory(prefix="mixxx-stems-") as scratch:
        try:
            started_separation = time.perf_counter()
            demucs_onnx.separate(
                args.input,
                scratch,
                model=args.model,
                providers=[provider],
                precision=args.precision,
                output_format="wav",
                # The master track must line up sample exactly with the stems,
                # so it is the sum the separator already computes for us.
                mix_stems=list(STEM_NAMES),
                mix_output_name="master",
                progress=False,
                verbose=False,
            )
            separation_seconds = time.perf_counter() - started_separation
        except Exception as error:  # noqa: BLE001 - reported to the user
            emit("ERROR", f"separation failed: {type(error).__name__}: {error}")
            return 4

        stems = {}
        for name in STEM_NAMES + ("master",):
            path = os.path.join(scratch, f"{name}.wav")
            if not os.path.isfile(path):
                emit("ERROR", f"the separator did not write {name}.wav")
                return 5
            stems[name] = path

        emit("PROGRESS", _SEPARATION_SHARE + 2)
        emit("STATUS", f"encoding {args.codec} and writing the container")
        try:
            stem_mp4.write_stem_mp4(
                stems["master"],
                {title: stems[name] for title, name in zip(STEM_TITLES, STEM_NAMES)},
                args.out,
                ffmpeg=args.ffmpeg,
                codec=args.codec,
                bitrate=args.bitrate,
                # The master is a plain WAV without tags, so the tags the deck
                # shows come from the track the user picked.
                tags=stem_mp4.read_metadata(args.input, args.ffmpeg),
            )
        except Exception as error:  # noqa: BLE001 - reported to the user
            emit("ERROR", f"writing the container failed: {error}")
            return 6

        emit("PROGRESS", 100)
        emit("STATUS", "done")
        emit("RESULT", json.dumps({
            "provider": provider,
            "model": args.model,
            "codec": args.codec,
            "separation_seconds": round(separation_seconds, 2),
            "total_seconds": round(time.perf_counter() - started, 2),
            "bytes": os.path.getsize(args.out),
        }))
        emit("DONE", args.out)
    return 0


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Mixxx stem generation worker")
    subparsers = parser.add_subparsers(dest="command", required=True)

    probe_parser = subparsers.add_parser("probe")
    probe_parser.add_argument("--ffmpeg", default="ffmpeg")

    separate_parser = subparsers.add_parser("separate")
    separate_parser.add_argument("--input", required=True)
    separate_parser.add_argument("--out", required=True)
    separate_parser.add_argument("--provider", default="auto",
                                 choices=("auto", "cuda", "cpu"))
    separate_parser.add_argument("--model", default="htdemucs")
    separate_parser.add_argument("--precision", default="fp32",
                                 choices=("fp32", "fp16weights"))
    separate_parser.add_argument("--codec", default="aac",
                                 choices=sorted(stem_mp4.CODEC_NAMES))
    separate_parser.add_argument("--bitrate", default="256k")
    separate_parser.add_argument("--ffmpeg", default="ffmpeg")

    args = parser.parse_args(argv)
    if args.command == "probe":
        # No trailing newline on purpose: the parent process turns every
        # complete line into a progress/status message, so printing the report
        # without one leaves it in the buffer that is parsed when the worker
        # exits (see StemGenerator::slotWorkerFinished).
        sys.stdout.write(json.dumps(probe(args.ffmpeg)))
        sys.stdout.flush()
        return 0
    return separate(args)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
