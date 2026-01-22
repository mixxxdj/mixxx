#!/usr/bin/env python3
"""
Create a STEM container (.stem.m4a) with multiple audio tracks and proper STEM metadata.
This creates a multi-track M4A file with drums, bass, other, and vocals as separate tracks,
plus the required STEM manifest atom using MP4Box (from compiled GPAC).
"""

import sys
import json
import subprocess
import os
import tempfile
from pathlib import Path


def find_mp4box():
    """Find MP4Box executable in various locations."""

    possible_locations = [
        "/usr/local/bin/MP4Box",
        "/usr/bin/MP4Box",
        "/opt/local/bin/MP4Box",
    ]

    for location in possible_locations:
        if os.path.exists(location):
            return location

    # Try to find it in PATH
    try:
        result = subprocess.run(["which", "MP4Box"], capture_output=True, text=True)
        if result.returncode == 0:
            return result.stdout.strip()
    except:
        pass

    return None


def create_stem_manifest():
    """Create the STEM manifest JSON with metadata."""
    stem_metadata = {
        "mastering_dsp": {
            "compressor": {
                "enabled": False,
                "ratio": 3,
                "output_gain": 0.5,
                "release": 0.3,
                "attack": 0.003,
                "input_gain": 0.5,
                "threshold": 0,
                "hp_cutoff": 300,
                "dry_wet": 50
            },
            "limiter": {
                "enabled": False,
                "release": 0.05,
                "threshold": 0,
                "ceiling": -0.35
            }
        },
        "version": 1,
        "stems": [
            {"color": "#009E73", "name": "Drums"},
            {"color": "#D55E00", "name": "Bass"},
            {"color": "#CC79A7", "name": "Other"},
            {"color": "#56B4E9", "name": "Vox"}
        ]
    }
    return json.dumps(stem_metadata)


def create_stem_container(mixdown_path, stems_dir, output_path, metadata_dict=None):
    """
    Create a STEM container file with multiple audio tracks and STEM manifest.

    Uses ffmpeg to create a multi-track M4A file with:
    - Track 1: Mixdown (master)
    - Track 2: Drums
    - Track 3: Bass
    - Track 4: Other (melody)
    - Track 5: Vocals

    Then uses MP4Box (from GPAC) to add the STEM manifest atom to moov/udta/stem.

    Args:
        mixdown_path: Path to the mixdown (full mix) M4A file
        stems_dir: Directory containing drums.m4a, bass.m4a, other.m4a, vocals.m4a
        output_path: Output path for the .stem.m4a file
        metadata_dict: Optional metadata dictionary with track info

    Returns:
        True if successful, False otherwise
    """

    try:
        if metadata_dict is None:
            metadata_dict = {}

        print(f"Creating STEM container from: {mixdown_path}")
        print(f"Stems directory: {stems_dir}")
        print(f"Output path: {output_path}")

        stem_files = {
            "drums.m4a": "Drums",
            "bass.m4a": "Bass",
            "other.m4a": "Other",
            "vocals.m4a": "Vocals"
        }

        ffmpeg_inputs = ["-i", mixdown_path]
        ffmpeg_map = ["-map", "0:a:0"]

        for stem_file, stem_name in stem_files.items():
            stem_path = os.path.join(stems_dir, stem_file)
            if not os.path.exists(stem_path):
                print(f"Warning: Stem file not found: {stem_path}")
                continue

            ffmpeg_inputs.extend(["-i", stem_path])
            track_index = len(ffmpeg_inputs) // 2 - 1
            ffmpeg_map.extend(["-map", f"{track_index}:a:0"])
            print(f"Added {stem_name} track: {stem_path}")

        ffmpeg_args = (
                ["ffmpeg", "-y"] +
                ffmpeg_inputs +
                ffmpeg_map +
                ["-c:a", "aac", "-b:a", "256k", "-movflags", "+faststart", output_path]
        )

        print(f"Running ffmpeg to create multi-track M4A...")

        result = subprocess.run(ffmpeg_args, capture_output=True, text=True)

        if result.returncode != 0:
            print(f"ffmpeg error: {result.stderr}", file=sys.stderr)
            if result.stdout:
                print(f"ffmpeg stdout: {result.stdout}", file=sys.stderr)
            return False

        print(f"Multi-track M4A created: {output_path}")

        mp4box_path = find_mp4box()
        if not mp4box_path:
            print("Error: MP4Box not found. Please ensure GPAC is compiled and installed.", file=sys.stderr)
            return False

        print(f"Using MP4Box: {mp4box_path}")

        stem_manifest = create_stem_manifest()

        with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
            f.write(stem_manifest)
            temp_json = f.name

        try:
            # Use MP4Box with -udta option to add stem metadata
            # Syntax: MP4Box -udta 0:type=stem:src=stem.json output.stem.m4a
            mp4box_args = [
                mp4box_path,
                "-udta", f"0:type=stem:src={temp_json}",
                output_path
            ]

            print(f"Adding STEM manifest atom with MP4Box...")
            print(f"Command: {' '.join(mp4box_args)}")

            result = subprocess.run(mp4box_args, capture_output=True, text=True)

            if result.returncode != 0:
                print(f"MP4Box error: {result.stderr}", file=sys.stderr)
                if result.stdout:
                    print(f"MP4Box stdout: {result.stdout}", file=sys.stderr)
                return False

            print(f"STEM container created successfully: {output_path}")
            return True

        finally:
            if os.path.exists(temp_json):
                os.remove(temp_json)

    except Exception as e:
        print(f"Error creating STEM container: {e}", file=sys.stderr)
        import traceback
        print(f"Traceback: {traceback.format_exc()}", file=sys.stderr)
        return False


def main():
    """Command line interface for creating STEM containers."""

    if len(sys.argv) < 4:
        print("Usage: python3 stems_create_container.py <mixdown.m4a> <stems_dir> <output.stem.m4a>")
        print()
        print("Example:")
        print("  python3 stems_create_container.py track_mixdown.m4a ./htdemucs_ft/track_name/ track.stem.m4a")
        sys.exit(1)

    mixdown_path = sys.argv[1]
    stems_dir = sys.argv[2]
    output_path = sys.argv[3]

    metadata = {}
    if len(sys.argv) > 4:
        try:
            metadata = json.loads(sys.argv[4])
        except json.JSONDecodeError:
            pass

    if not Path(mixdown_path).exists():
        print(f"Error: Mixdown file not found: {mixdown_path}", file=sys.stderr)
        sys.exit(1)

    if not Path(stems_dir).exists():
        print(f"Error: Stems directory not found: {stems_dir}", file=sys.stderr)
        sys.exit(1)

    success = create_stem_container(mixdown_path, stems_dir, output_path, metadata)

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
