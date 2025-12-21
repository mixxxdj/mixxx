from urllib.request import Request, urlopen
from urllib.parse import urlparse
import os
import sys
import csv
import sqlite3
import multiprocessing
import subprocess
import json
import dataclasses

from typing import Optional

"""Generated protocol buffer code for beats.proto. Using # noqa"""
from google.protobuf.internal import builder as _builder
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database

# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()

DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(
    b'\n\x0b\x62\x65\x61ts.proto\x12\x0emixxx.track.io"g\n\x04\x42\x65\x61t\x12\x16\n\x0e\x66rame_position\x18\x01 \x01(\x05\x12\x15\n\x07\x65nabled\x18\x02 \x01(\x08:\x04true\x12\x30\n\x06source\x18\x03 \x01(\x0e\x32\x16.mixxx.track.io.Source:\x08\x41NALYZER"D\n\x03\x42pm\x12\x0b\n\x03\x62pm\x18\x01 \x01(\x01\x12\x30\n\x06source\x18\x02 \x01(\x0e\x32\x16.mixxx.track.io.Source:\x08\x41NALYZER"-\n\x07\x42\x65\x61tMap\x12"\n\x04\x62\x65\x61t\x18\x01 \x03(\x0b\x32\x14.mixxx.track.io.Beat"V\n\x08\x42\x65\x61tGrid\x12 \n\x03\x62pm\x18\x01 \x01(\x0b\x32\x13.mixxx.track.io.Bpm\x12(\n\nfirst_beat\x18\x02 \x01(\x0b\x32\x14.mixxx.track.io.Beat*3\n\x06Source\x12\x0c\n\x08\x41NALYZER\x10\x00\x12\x11\n\rFILE_METADATA\x10\x01\x12\x08\n\x04USER\x10\x02\x42\x02H\x03'  # noqa: E501
)

_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, "beats_pb2", globals())
if _descriptor._USE_C_DESCRIPTORS == False:  # noqa: E712

    DESCRIPTOR._options = None
    DESCRIPTOR._serialized_options = b"H\003"
    _SOURCE._serialized_start = 341  # noqa: F821
    _SOURCE._serialized_end = 392  # noqa: F821
    _BEAT._serialized_start = 31  # noqa: F821
    _BEAT._serialized_end = 134  # noqa: F821
    _BPM._serialized_start = 136  # noqa: F821
    _BPM._serialized_end = 204  # noqa: F821
    _BEATMAP._serialized_start = 206  # noqa: F821
    _BEATMAP._serialized_end = 251  # noqa: F821
    _BEATGRID._serialized_start = 253  # noqa: F821
    _BEATGRID._serialized_end = 339  # noqa: F821
# @@protoc_insertion_point(module_scope)
"""End generated protocol buffer code."""

datasets = []

DOWNLOAD_FOLDER = os.getenv("DOWNLOAD_FOLDER") or os.getcwd()
DB_PATH = os.getenv("DB_PATH") or os.path.expanduser("~/.mixxx/mixxxdb.sqlite")
MIXXX_TEST_PATH = os.getenv("MIXXX_TEST_PATH") or os.path.expanduser(
    "./mixxx-test"
)
UPDATE_DATASET = not os.getenv("DISABLE_DATASET_UPDATE")
BPM_ACCURATE_THRESHOLD = 0.02
GRID_ACCURATE_THRESHOLD = 0.98

con = sqlite3.connect(DB_PATH)


@dataclasses.dataclass
class Result:
    title: Optional[str] = None
    artist: Optional[str] = None
    source: str = ""
    expected_bpm: float = 0
    actual_bpm: float = 0
    expected_first_beat: int = 0
    actual_first_beat: int = 0
    samplerate: int = 0
    runtime: float = 0

    @property
    def is_valid(self):
        return (
            self.source != ""
            and self.expected_bpm != 0
            and self.actual_bpm != 0
            and self.expected_first_beat != 0
            and self.actual_first_beat != 0
            and self.samplerate != 0
            and self.runtime != 0
        )

    @property
    def sorting_key(self):
        bpm_weight = 1000
        if self.bpm_error_multiplier:
            bpm_weight = 10 / self.bpm_error_multiplier
        elif not self.is_bpm_accurate:
            bpm_weight = 1 / abs(self.bpm_error_delta)

        return bpm_weight * (1 + self.grid_offset)

    @property
    def grid_offset(self):
        beat_length = self.samplerate * 60 / self.expected_bpm
        grid_offset = (
            abs(self.expected_first_beat - self.actual_first_beat)
            / beat_length
        ) % 1
        return abs(2 * grid_offset - 1)

    @property
    def bpm_error_multiplier(self):
        values = [self.expected_bpm, self.actual_bpm]
        ratio = max(values) / min(values)

        if int(ratio) == ratio and ratio > 1:
            return int(ratio)
        return 0

    @property
    def bpm_error_delta(self):
        delta = self.expected_bpm - self.actual_bpm
        return delta

    @property
    def is_bpm_accurate(self):
        return (
            abs(self.expected_bpm - self.actual_bpm) <= BPM_ACCURATE_THRESHOLD
        )

    @property
    def is_grid_accurate(self):
        return self.grid_offset >= GRID_ACCURATE_THRESHOLD

    @property
    def label_column(self):
        return f"[{self.title} - {self.artist}]({self.source})".replace(
            "|", "\\|"
        )

    @property
    def expected_bpm_column(self):
        return f"{self.expected_bpm:<6.2f}"

    @property
    def actual_bpm_column(self):
        return f"{self.actual_bpm:<6.2f}"

    @property
    def bpm_error_column(self):
        if self.is_bpm_accurate:
            return "="

        ratio = self.bpm_error_multiplier
        if ratio:
            return f"x {ratio}"
        else:
            delta = self.bpm_error_delta
            return f"{'+' if delta > 0 else ''}{delta:.2f}"

    @property
    def grid_offset_column(self):
        return f"{int(self.grid_offset * 100):>3}%"

    @property
    def runtime_column(self):
        return f"{(self.runtime / 1000):>6.2f} sec/min"


class StatsWrapper:
    def __init__(self, key, func, current, previous):
        self.__key = key
        self.__func = func
        self.__current = current
        self.__previous = previous

    def __str__(self):
        current = self.__func(self.__current[self.__key])
        if self.__key not in self.__previous.keys():
            return str(current)
        previous = self.__func(self.__previous[self.__key])
        if current != previous:
            return f"{current} **(was {previous})**"
        return str(current)


class Wrapper(object):
    _result = None
    _previous = None
    _WRAPPED_COLUMNS = {
        "bpm_error": "bpm_error_delta",
        "grid_offset": "grid_offset",
    }

    def __init__(self, result, previous):
        self._result = result
        self._previous = previous

    def __getattribute__(self, attr):
        result = super(Wrapper, self).__getattribute__("_result")
        previous = super(Wrapper, self).__getattribute__("_previous")
        _WRAPPED_COLUMNS = super(Wrapper, self).__getattribute__(
            "_WRAPPED_COLUMNS"
        )

        col_attr = attr.removesuffix("_column")
        if attr.endswith("_column") and col_attr in _WRAPPED_COLUMNS.keys():
            prop = _WRAPPED_COLUMNS[col_attr]
            ret = getattr(result, attr).strip()
            if previous and getattr(result, prop) != getattr(previous, prop):
                return f"**{ret} (was {getattr(previous, attr).strip()})**"
            return ret

        try:
            return super(Wrapper, self).__getattribute__(attr)
        except AttributeError:
            result = super(Wrapper, self).__getattribute__("_result")
            return getattr(result, attr)


def fetch(record):
    url_parsed = urlparse(record["source"])
    filename = None
    if url_parsed.scheme in ("file", ""):
        if not os.path.exists(url_parsed.path):
            raise Exception(f"Cannot find file {url_parsed.path}")
        filename = url_parsed.path
    else:
        headers = {
            "User-Agent": (
                "Mozilla/5.0 (X11; Linux x86_64; rv:146.0) "
                "Gecko/20100101 Firefox/146.0"
            ),
            "Accept": (
                "text/html,application/xhtml+xml,"
                "application/xml;q=0.9,*/*;q=0.8"
            ),
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
            (
                "SELECT l.location, a.bpm, a.artist, a.title, a.color, "
                "a.samplerate, a.beats FROM track_locations l, library a "
                "WHERE l.location LIKE ? AND l.id = a.location"
            ),
            (filename,),
        )
        data = res.fetchone()
        if data is not None:
            location, bpm, artist, title, color, samplerate, beats = data
            beatgrid = BeatGrid()  # noqa: F821
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


def process(args):
    record, previous = args
    filename = fetch(record)

    if record.get("color") is not None:
        # A color suggest that this track cannot use a static grid
        return record, Result()

    proc = subprocess.Popen(
        [MIXXX_TEST_PATH, "--analyser", filename],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    proc.wait()
    if proc.returncode != 0:
        print(f"Got error code {proc.returncode} on {filename}")
        raise Exception(proc.stderr.read())
    output = json.loads(proc.stdout.read())

    result = Result(
        title=record["title"],
        artist=record["artist"],
        source=record["source"],
        expected_bpm=record.get("bpm", 0),
        actual_bpm=output.get("bpm", 0),
        expected_first_beat=record.get("first_beat", 0),
        actual_first_beat=output.get("first_beat", 0),
        samplerate=output["track_samplerate"],
        runtime=output["runtime"],
    )
    return record, result if not previous else Wrapper(result, previous)


if __name__ == "__main__":
    if len(sys.argv) < 2 or len(sys.argv) > 5:
        print(
            (
                f"Usage: {sys.argv[0]} <PathToCSVDataset> "
                "[<PathToOutputReport>] [PathToManifest] "
                "[PathTPreviousManifest]"
            ),
            file=sys.stderr,
        )
        sys.exit(1)

    datasets = []
    with open(sys.argv[1]) as f:
        datasets = list(csv.DictReader(f))
        for record in datasets:
            record.update(
                dict(
                    first_beat=(
                        int(record["first_beat"])
                        if record["first_beat"]
                        else 0
                    ),
                    bpm=float(record["bpm"]) if record["bpm"] else 0,
                    samplerate=(
                        float(record["samplerate"])
                        if record["samplerate"]
                        else 0
                    ),
                )
            )

    previous_stats = {}
    previous_results = {}
    if len(sys.argv) == 5:
        with open(sys.argv[4], "r") as f:
            data = json.load(f)
            previous_results = {
                d["source"]: Result(**d) for d in data.get("results", [])
            }
            previous_stats = data["stats"]

    header = list(datasets[0].keys())
    output = []
    args = [(data, previous_results.get(data["source"])) for data in datasets]

    with multiprocessing.Pool(multiprocessing.cpu_count()) as p:
        output = p.map(process, args)

    datasets = []
    results = []
    for record, result in output:
        datasets.append(record)
        results.append(result)

    # Filter out records without expected BPM
    results = list(filter(lambda record: result.is_valid, results))

    # Order "poor" results first
    results.sort(key=lambda result: result.sorting_key)

    columns = {
        "label": "Label",
        "expected_bpm": "Expected BPM",
        "actual_bpm": "Actual BPM",
        "bpm_error": "BPM Error",
        "grid_offset": "Grid Offset",
        "runtime": "Runtime",
    }
    columns_size = [
        max(
            [
                len(getattr(result, f"{col_name}_column"))
                for result in results
                if result.is_valid
            ]
            + [len(col_label)]
        )
        for col_name, col_label in columns.items()
    ]

    stats = {
        "bpm_accuracy": 0,
        "grid_accuracy": 0,
        "grid_accuracy_avg": 0,
        "bpm_offset_avg": 0,
        "bpm_offset_max": 0,
        "runtime_avg": 0,
    }

    f = (
        open(sys.argv[2], "w")
        if len(sys.argv) >= 3 and sys.argv[2] != "-"
        else sys.stdout
    )

    result_table = []
    incomplete_table = []
    for result in results:
        if not result.is_valid:
            incomplete_table.append(
                f"| {result.label_column.ljust(columns_size[0])} |"
            )
            continue
        if result.is_bpm_accurate:
            stats["bpm_accuracy"] += 1
        if result.is_bpm_accurate:
            stats["grid_accuracy"] += 1
        stats["grid_accuracy_avg"] += result.grid_offset
        stats["bpm_offset_avg"] += result.bpm_error_delta
        stats["runtime_avg"] += result.runtime
        if not result.bpm_error_multiplier:
            stats["bpm_offset_avg"] += result.bpm_error_delta
        stats["bpm_offset_max"] = max(
            abs(result.bpm_error_delta), stats["bpm_offset_max"]
        )
        # stats["grid_accuracy_avg"] += result.grid_offset
        result_table.append(
            "| "
            + " | ".join(
                [
                    getattr(result, f"{col_name}_column").ljust(col_size)
                    for col_name, col_size in zip(columns, columns_size)
                ]
            )
            + " |"
        )

    stats["grid_accuracy_avg"] /= len(result_table)
    stats["bpm_offset_avg"] /= len(result_table)
    stats["runtime_avg"] /= len(result_table)

    stats_items = {
        "Accurate BPM results": (
            "bpm_accuracy",
            lambda value: (
                f"{value}/{len(result_table)} "
                "({(100 * value/len(result_table)):.2f}%)"
            ),
        ),
        "Accurate grid results": (
            "grid_accuracy",
            lambda value: (
                f"{value}/{len(result_table)} "
                "({(100 * value/len(result_table)):.2f}%)"
            ),
        ),
        "Grid accuracy average": (
            "grid_accuracy_avg",
            lambda value: f"{(100 * value):.2f}%",
        ),
        "BPM offset average": ("bpm_offset_avg", lambda value: f"{value:.2f}"),
        "BPM maximum offset": ("bpm_offset_max", lambda value: f"{value:.2f}"),
        "Runtime average": (
            "runtime_avg",
            lambda value: f"{(value / 1000):.2f} sec/min",
        ),
    }

    print("# BPM and grid analyzer report\n", file=f)
    for label, defs in stats_items.items():
        print(
            f"**{label}**: {StatsWrapper(*defs, stats, previous_stats)}",
            file=f,
        )
    print("\n", file=f)
    print("## Result table\n", file=f)

    print(
        "| "
        + " | ".join(
            [
                col_name.ljust(col_size)
                for col_name, col_size in zip(columns.values(), columns_size)
            ]
        )
        + " |",
        file=f,
    )
    print(
        "|"
        + "|".join(["-" * (col_size + 2) for col_size in columns_size])
        + "|",
        file=f,
    )
    print("\n".join(result_table), file=f)

    if incomplete_table:
        print(
            (
                "## Incomplete data\n\nThe following files do not have an "
                "expected BPM and/or a grid. Load it in Mixxx and set the "
                "right metadata before re-running this script to fill the "
                "gap in the dataset.\n"
            ),
            file=f,
        )

        print(f"| {next(columns.values()).ljust(columns_size[0])} |", file=f)
        print(f'|{"-" * columns_size[0]}|', file=f)
        print("\n".join(incomplete_table), file=f)

    print("\n", file=f)
    f.close()

    if len(sys.argv) >= 4:

        def serialize(obj):
            d = obj.__dict__
            return d.get("_result", d)

        with open(sys.argv[3], "w") as f:
            json.dump(
                dict(results=results, stats=stats),
                f,
                sort_keys=True,
                indent=4,
                default=serialize,
            )

    if UPDATE_DATASET:
        with open(sys.argv[1], "w") as f:
            c = csv.DictWriter(f, header)
            c.writeheader()
            c.writerows(
                [
                    {k: v for k, v in record.items() if k in header}
                    for record in datasets
                ]
            )
