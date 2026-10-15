"""Write Native Instruments style ``.stem.mp4`` files.

Mixxx recognises a stem file by the MP4 atom ``moov/udta/stem`` (see
``src/track/steminfoimporter.cpp``) and plays it with ``soundsourcestem.cpp``,
which expects the NI stream layout: five stereo audio streams, the first one
being the premixed master and the remaining four the stems (drums, bass, other,
vocals).

This module muxes that with ffmpeg and then injects the manifest atom by hand,
because ffmpeg does not know about the ``stem`` atom. The manifest is plain
JSON, so injecting it means inserting one box inside ``moov/udta`` and shifting
the chunk offsets (``stco``/``co64``) that point behind it.

Command line usage:

    python stem_mp4.py write --original track.mp3 --stem drums=drums.wav ...
    python stem_mp4.py verify track.stem.mp4
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import struct
import subprocess
import sys
import tempfile
from typing import Iterator

STEM_NAMES = ("Drums", "Bass", "Other", "Vocals")
STEM_COLORS = ("#fd4a4a", "#ffff00", "#00e8e8", "#ad65ff")
MANIFEST_VERSION = 1

# Tags worth carrying over to the stem file, so Mixxx shows the same artist and
# title as for the track the stems were made from.
TAGGED_KEYS = ("title", "artist", "album", "album_artist", "genre", "date",
               "comment", "track", "composer")

# Mixxx refuses stem streams that are not stereo.
_CHANNEL_COUNT = 2

# A box region is always a (box offset, payload start, payload end) tuple, so
# that no caller has to juggle offsets and sizes.
Region = tuple[int, int, int]


class StemMp4Error(RuntimeError):
    """Raised when a container cannot be written or is not a stem file."""


def _box(box_type: bytes, payload: bytes) -> bytes:
    return struct.pack(">I", len(payload) + 8) + box_type + payload


def box_length(buf: bytes, offset: int) -> int:
    """Return the total length of the box starting at ``offset``."""
    size = struct.unpack(">I", buf[offset : offset + 4])[0]
    if size == 1:
        return struct.unpack(">Q", buf[offset + 8 : offset + 16])[0]
    return size


def iter_boxes(buf: bytes, start: int, end: int) -> Iterator[tuple[str, Region]]:
    """Yield (box type, region) for the boxes contained in [start, end)."""
    offset = start
    while offset + 8 <= end:
        size, box_type = struct.unpack(">I4s", buf[offset : offset + 8])
        header = 8
        if size == 1:
            size = struct.unpack(">Q", buf[offset + 8 : offset + 16])[0]
            header = 16
        elif size == 0:
            size = end - offset
        name = box_type.decode("latin1")
        if size < header or offset + size > end:
            raise StemMp4Error(f"truncated {name} box at {offset}")
        yield name, (offset, offset + header, offset + size)
        offset += size


def child_box(buf: bytes, area: Region, box_type: str) -> Region | None:
    """Region of a direct child box, or None. ``area`` starts at the container."""
    for name, region in iter_boxes(buf, area[1], area[2]):
        if name == box_type:
            return region
    return None


def find_box(buf: bytes, path: tuple[str, ...]) -> Region | None:
    """Region of a nested box, or None."""
    area = (0, 0, len(buf))
    for wanted in path:
        found = child_box(buf, area, wanted)
        if found is None:
            return None
        area = found
    return area


def build_manifest(names=STEM_NAMES, colors=STEM_COLORS) -> dict:
    if len(names) != len(colors):
        raise StemMp4Error("stem names and colours have different lengths")
    return {
        "mastering_dsp": {
            "compressor": {
                "ratio": 3,
                "output_gain": 0.5,
                "enabled": False,
                "threshold": 0,
                "attack": 0.003,
                "input_gain": 0.5,
                "release": 0.3,
                "hp_cutoff": 300,
                "dry_wet": 50,
            },
            "limiter": {
                "release": 0.05,
                "threshold": 0,
                "ceiling": -0.35,
                "enabled": False,
            },
        },
        "version": MANIFEST_VERSION,
        "stems": [{"color": color, "name": name} for name, color in zip(names, colors)],
    }


def chunk_offset_tables(buf: bytes, moov: Region) -> list[tuple[Region, int]]:
    """Return [(region of stco/co64 payload, entry width), ...] for all tracks."""
    tables = []
    for name, trak in iter_boxes(buf, moov[1], moov[2]):
        if name != "trak":
            continue
        area = trak
        for wanted in ("mdia", "minf", "stbl"):
            found = child_box(buf, area, wanted)
            if found is None:
                break
            area = found
        else:
            for table_name, table in iter_boxes(buf, area[1], area[2]):
                if table_name == "stco":
                    tables.append((table, 4))
                elif table_name == "co64":
                    tables.append((table, 8))
    return tables


def _patch_chunk_offsets(buf: bytearray, moov: Region,
                         inserted_at: int, delta: int) -> None:
    """Shift chunk offsets that point at media data moved by the insertion."""
    patched = 0
    for table, width in chunk_offset_tables(buf, moov):
        count = struct.unpack(">I", buf[table[1] + 4 : table[1] + 8])[0]
        for index in range(count):
            at = table[1] + 8 + index * width
            code = ">I" if width == 4 else ">Q"
            value = struct.unpack(code, buf[at : at + width])[0]
            if value >= inserted_at:
                buf[at : at + width] = struct.pack(code, value + delta)
                patched += 1
    if patched == 0:
        raise StemMp4Error("no chunk offsets were patched")


def inject_manifest(path: str, manifest: dict) -> None:
    """Insert the ``stem`` atom with the manifest into an existing MP4 file."""
    with open(path, "rb") as handle:
        buf = bytearray(handle.read())

    moov = find_box(buf, ("moov",))
    if moov is None:
        raise StemMp4Error("no moov box in the file")
    stem_box = _box(b"stem",
                    json.dumps(manifest, separators=(",", ":")).encode("utf-8"))

    # ``meta`` is a full box, so its children start four bytes later.
    udta = None
    for name, region in iter_boxes(buf, moov[1], moov[2]):
        if name == "udta":
            udta = region
            break

    if udta is None:
        # Append a new udta with only the manifest; nothing inside moov moves.
        head = bytes(buf[moov[1] : moov[2]])
        tail = b""
        new_udta = _box(b"udta", stem_box)
        moved_from = moov[2]
    else:
        # Reuse the existing udta and add the manifest as its last child.
        head = bytes(buf[moov[1] : udta[0]])
        tail = bytes(buf[udta[2] : moov[2]])
        new_udta = _box(b"udta", bytes(buf[udta[1] : udta[2]]) + stem_box)
        moved_from = udta[2]

    new_payload = head + new_udta + tail
    delta = len(new_payload) - (moov[2] - moov[1])
    old_box_length = box_length(buf, moov[0])
    buf[moov[0] : moov[0] + old_box_length] = _box(b"moov", new_payload)
    # The moov box grew, so every offset at or behind the insertion moved.
    _patch_chunk_offsets(buf, find_box(buf, ("moov",)), moved_from, delta)

    temporary = path + ".tmp"
    with open(temporary, "wb") as handle:
        handle.write(buf)
    os.replace(temporary, path)


# Mixxx decodes all of these with its bundled FFmpeg: AAC (Native Instruments'
# own choice, smallest files) and MP3 (VUMeter style) are lossy, ALAC is
# lossless but roughly six times bigger.
CODEC_NAMES = {
    "aac": "aac",
    "mp3": "libmp3lame",
    "alac": "alac",
}


def read_metadata(path: str, ffmpeg: str = "ffmpeg") -> dict[str, str]:
    """Container tags of ``path``, so a generated stem file keeps them.

    The muxed audio of a stem file is the mix the separator wrote, a plain WAV
    without tags, so the artist and title have to be taken from the file the
    user picked.
    """
    ffprobe = os.path.join(os.path.dirname(ffmpeg), "ffprobe.exe")
    if not os.path.isfile(ffprobe):
        ffprobe = os.path.join(os.path.dirname(ffmpeg), "ffprobe")
    if not os.path.isfile(ffprobe):
        ffprobe = shutil.which("ffprobe") or "ffprobe"
    result = subprocess.run(
        [ffprobe, "-v", "error", "-show_entries", "format_tags",
         "-of", "default=noprint_wrappers=1", path],
        capture_output=True, text=True)
    if result.returncode != 0:
        return {}
    tags: dict[str, str] = {}
    for line in result.stdout.splitlines():
        key, separator, value = line.partition("=")
        if not separator:
            continue
        key = key.removeprefix("TAG:").strip().lower()
        if key in TAGGED_KEYS and value:
            tags[key] = value
    return tags


def write_stem_mp4(
    original: str,
    stems: dict[str, str],
    out_path: str,
    ffmpeg: str = "ffmpeg",
    codec: str = "aac",
    bitrate: str = "256k",
    sample_rate: int = 48000,
    tags: dict[str, str] | None = None,
) -> None:
    """Mux ``original`` plus the four stems into a NI style stem container."""
    missing = [name for name in STEM_NAMES if name not in stems]
    if missing:
        raise StemMp4Error(f"missing stems: {', '.join(missing)}")
    if codec not in CODEC_NAMES:
        raise StemMp4Error(f"unsupported codec {codec!r}")
    ordered = [stems[name] for name in STEM_NAMES]

    arguments = [ffmpeg, "-y", "-hide_banner", "-loglevel", "error", "-i", original]
    for stem_path in ordered:
        arguments += ["-i", stem_path]
    for index in range(len(ordered) + 1):
        arguments += ["-map", f"{index}:a"]
    arguments += [
        "-c:a", CODEC_NAMES[codec],
        "-ar", str(sample_rate),
        "-ac", str(_CHANNEL_COUNT),
    ]
    if codec != "alac":
        arguments += ["-b:a", bitrate]
    for key, value in (tags or {}).items():
        arguments += ["-metadata", f"{key}={value}"]
    arguments += [
        # moov before mdat, like the reference NI files.
        "-movflags", "+faststart",
        "-f", "mp4",
    ]

    parent = os.path.dirname(os.path.abspath(out_path))
    os.makedirs(parent, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="stemtmp") as scratch:
        staged = os.path.join(scratch, "stems.mp4")
        result = subprocess.run(arguments + [staged], capture_output=True, text=True)
        if result.returncode != 0:
            raise StemMp4Error(
                f"ffmpeg failed ({result.returncode}): {result.stderr.strip()}")
        inject_manifest(staged, build_manifest())
        os.replace(staged, out_path)


def verify(path: str) -> dict:
    """Return a small report about a stem container, raising on structural errors."""
    with open(path, "rb") as handle:
        buf = handle.read()

    manifest_at = find_box(buf, ("moov", "udta", "stem"))
    if manifest_at is None:
        raise StemMp4Error(
            "no moov/udta/stem atom: Mixxx would treat this as a plain audio file")
    raw = buf[manifest_at[1] : manifest_at[2]].rstrip(b"\x00")
    manifest = json.loads(raw.decode("utf-8"))

    tracks = []
    for name, trak in iter_boxes(buf, *find_box(buf, ("moov",))[1:]):
        if name == "trak":
            tracks.append(trak)
    return {
        "traks": len(tracks),
        "version": manifest.get("version"),
        "stems": [stem.get("name") for stem in manifest.get("stems", [])],
        "manifest_bytes": len(raw),
        "chunk_offset_tables": len(chunk_offset_tables(buf, find_box(buf, ("moov",)))),
    }


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="NI style stem containers")
    subparsers = parser.add_subparsers(dest="command", required=True)

    write_parser = subparsers.add_parser("write")
    write_parser.add_argument("--original", required=True)
    write_parser.add_argument("--out", required=True)
    write_parser.add_argument("--codec", choices=sorted(CODEC_NAMES), default="aac")
    write_parser.add_argument("--bitrate", default="256k")
    write_parser.add_argument("--stem", action="append", required=True,
                              help="name=path, e.g. drums=drums.wav")

    verify_parser = subparsers.add_parser("verify")
    verify_parser.add_argument("path")

    args = parser.parse_args(argv)
    if args.command == "write":
        stems = {}
        for item in args.stem:
            name, _, stem_path = item.partition("=")
            stems[name.capitalize()] = stem_path
        write_stem_mp4(args.original, stems, args.out,
                       codec=args.codec, bitrate=args.bitrate,
                       tags=read_metadata(args.original))
        print(f"escrito: {args.out}")
        print(json.dumps(verify(args.out), indent=2))
    else:
        print(json.dumps(verify(args.path), indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
