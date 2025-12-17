from urllib.request import Request, urlopen
from urllib.parse import urlparse
import os
import sys
import csv
import sqlite3
import multiprocessing
import subprocess
import json

"""Generated protocol buffer code for beats.proto."""
from google.protobuf.internal import builder as _builder
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database

# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()

DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(
    b'\n\x0b\x62\x65\x61ts.proto\x12\x0emixxx.track.io"g\n\x04\x42\x65\x61t\x12\x16\n\x0e\x66rame_position\x18\x01 \x01(\x05\x12\x15\n\x07\x65nabled\x18\x02 \x01(\x08:\x04true\x12\x30\n\x06source\x18\x03 \x01(\x0e\x32\x16.mixxx.track.io.Source:\x08\x41NALYZER"D\n\x03\x42pm\x12\x0b\n\x03\x62pm\x18\x01 \x01(\x01\x12\x30\n\x06source\x18\x02 \x01(\x0e\x32\x16.mixxx.track.io.Source:\x08\x41NALYZER"-\n\x07\x42\x65\x61tMap\x12"\n\x04\x62\x65\x61t\x18\x01 \x03(\x0b\x32\x14.mixxx.track.io.Beat"V\n\x08\x42\x65\x61tGrid\x12 \n\x03\x62pm\x18\x01 \x01(\x0b\x32\x13.mixxx.track.io.Bpm\x12(\n\nfirst_beat\x18\x02 \x01(\x0b\x32\x14.mixxx.track.io.Beat*3\n\x06Source\x12\x0c\n\x08\x41NALYZER\x10\x00\x12\x11\n\rFILE_METADATA\x10\x01\x12\x08\n\x04USER\x10\x02\x42\x02H\x03'
)

_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, "beats_pb2", globals())
if _descriptor._USE_C_DESCRIPTORS == False:

    DESCRIPTOR._options = None
    DESCRIPTOR._serialized_options = b"H\003"
    _SOURCE._serialized_start = 341
    _SOURCE._serialized_end = 392
    _BEAT._serialized_start = 31
    _BEAT._serialized_end = 134
    _BPM._serialized_start = 136
    _BPM._serialized_end = 204
    _BEATMAP._serialized_start = 206
    _BEATMAP._serialized_end = 251
    _BEATGRID._serialized_start = 253
    _BEATGRID._serialized_end = 339
# @@protoc_insertion_point(module_scope)
"""End generated protocol buffer code."""

datasets = []

with open(sys.argv[1]) as f:
    datasets = list(csv.DictReader(f))
    for record in datasets:
        record.update(
            dict(
                first_beat=(
                    int(record["first_beat"]) if record["first_beat"] else 0
                ),
                bpm=float(record["bpm"]) if record["bpm"] else 0,
                samplerate=(
                    float(record["samplerate"]) if record["samplerate"] else 0
                ),
            )
        )

DOWNLOAD_FOLDER = os.getenv("DOWNLOAD_FOLDER") or os.getcwd()
DB_PATH = os.getenv("DB_PATH") or os.path.expanduser("~/.mixxx/mixxxdb.sqlite")

con = sqlite3.connect(DB_PATH)


def fetch(record):
    url_parsed = urlparse(record["source"])
    filename = None
    if url_parsed.scheme in ("file", ""):
        if not os.path.exists(url_parsed.path):
            raise Exception(f"Cannot find file {url_parsed.path}")
        filename = url_parsed.path
    else:
        headers = {
            "User-Agent": "Mozilla/5.0 (X11; Linux x86_64; rv:146.0) Gecko/20100101 Firefox/146.0",
            "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
        }
        filename = f"{record['title']} - {record['artist']}.mp3".replace(
            "/", "_"
        )
        filename = f"{DOWNLOAD_FOLDER}/{filename}"
        if not os.path.exists(filename):
            req = Request(record["source"], headers=headers)
            with urlopen(req) as response:
                print(f"Downloading {filename}...")
                with open(filename, "wb") as f:
                    f.write(response.read())

    if (
        not record.get("bpm")
        or not record.get("first_beat")
        or not record.get("samplerate")
    ):
        cur = con.cursor()
        res = cur.execute(
            "SELECT l.location, a.bpm, a.artist, a.title, a.color, a.samplerate, a.beats FROM track_locations l, library a WHERE l.location LIKE ? AND l.id = a.location",
            (filename,),
        )
        data = res.fetchone()
        if data is not None:
            location, bpm, artist, title, color, samplerate, beats = data
            beatgrid = BeatGrid()
            beatgrid.ParseFromString(beats)
            record.update(
                dict(
                    bpm=float(beatgrid.bpm.bpm),
                    first_beat=int(beatgrid.first_beat.frame_position),
                    color=color,
                    samplerate=samplerate,
                )
            )

    return filename


def process(record):
    filename = fetch(record)

    if record.get("color") is not None:
        # A color suggest that this track cannot use a static grid
        return record, [
            f"{record['title']} - {record['artist']}]({record['source']})",
            0,
            0,
            "-",
            "-",
            0,
        ]

    proc = subprocess.Popen(
        ["/workspaces/mixxx/build/mixxx-test", "--analyser", filename],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    proc.wait()
    if proc.returncode != 0:
        print(f"Got error code {proc.returncode} on {filename}")
        raise Exception(proc.stderr.read())
    output = json.loads(proc.stdout.read())
    expected_bpm = record["bpm"]

    delta = "?"
    offset = "?"
    runtime = output["runtime"]
    try:
        beat_length = record["samplerate"] * 60 / expected_bpm
        grid_offset = (
            abs(record["first_beat"] - output["offset"]) / beat_length
        ) % 1
        offset = abs(2 * grid_offset - 1)
        delta = expected_bpm - output["bpm"]
        delta = (
            f"+{delta:.2f}"
            if delta > 0
            else (f"{delta:.2f}" if delta < 0 else "-")
        )
        offset = int(offset * 100)
    except:
        print(
            f"File {repr(filename)} doesn't have an expected BPM and a grid. Load it in Mixxx and set the right metadata before re-running this script!",
            file=sys.stderr,
        )
        pass
    return record, [
        f"{record['title']} - {record['artist']}]({record['source']})",
        expected_bpm,
        output["bpm"],
        delta,
        offset,
        runtime / 1000 / (output["track_duration"] / 60),
    ]


with multiprocessing.Pool(multiprocessing.cpu_count()) as p:
    header = list(datasets[0].keys())
    results = p.map(process, datasets)
    datasets = []
    report = []
    for record, data in results:
        datasets.append(record)
        report.append(data)
    # Filter out records without expected BPM
    records = list(filter(lambda record: record[1], report))
    # Order "poor" result first
    records.sort(
        key=lambda record: (
            100000 if record[3] == "-" else (1 / abs(float(record[3])))
        )
        * (1 + record[4])
    )
    # TODO array max
    print(
        "\n".join(
            [
                f"| [{record[0]:<145} | {record[1]:<6.2f} | {record[2]:<6.2f} | {record[3]:<6} | {record[4]:>3}% | {record[5]:>6.2f} sec/min |"
                for record in records
            ]
        )
    )
    with open(sys.argv[1], "w") as f:
        c = csv.DictWriter(f, header)
        c.writeheader()
        c.writerows(
            [
                {k: v for k, v in record.items() if k in header}
                for record in datasets
            ]
        )
