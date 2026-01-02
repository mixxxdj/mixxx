#!/usr/bin/env python3
import re
import sys

from datetime import datetime
from urllib.request import urlopen
from urllib.error import HTTPError

MIXXX_DOWNLOAD_BASE = "https://downloads.mixxx.org/dependencies/"
DEFAULT_PATTERN = (
    r'BUILDENV_NAME="(mixxx-deps-[0-9]\.[0-9]-{triplet}-)'
    r'([a-z0-9]+)"\n(\W+)BUILDENV_SHA256="([^"]+)"'
)
DEFAULT_REPLACE = (
    r'BUILDENV_NAME="\g<1>{version}"\n\g<3>BUILDENV_SHA256="{shasum}"'
)
PLATFORMS = [
    {
        "os": "Windows",
        "triplet": "arm64-windows",
        "release_triplet": "arm64-windows-rel",
        "file": "tools/windows_buildenv.bat",
        "pattern": (
            r"BUILDENV_NAME=(mixxx-deps-[0-9]\.[0-9]-{triplet}-)"
            r"([a-z0-9]+)\n(\W+)SET BUILDENV_SHA256=([^\W]+)"
        ),
        "replace": (
            r"BUILDENV_NAME=\g<1>{version}\n\g<3>"
            r"SET BUILDENV_SHA256={shasum}"
        ),
    },
    {
        "os": "Windows",
        "triplet": "x64-windows",
        "release_triplet": "x64-windows-rel",
        "file": "tools/windows_buildenv.bat",
        "pattern": (
            r"BUILDENV_NAME=(mixxx-deps-[0-9]\.[0-9]-{triplet}-)"
            r"([a-z0-9]+)\n(\W+)SET BUILDENV_SHA256=([^\W]+)"
        ),
        "replace": (
            r"BUILDENV_NAME=\g<1>{version}\n\g<3>"
            r"SET BUILDENV_SHA256={shasum}"
        ),
    },
    {
        "os": "macOS",
        "triplet": "arm64-osx-cross",
        "release_triplet": "arm64-osx-cross-rel",
        "file": "tools/macos_buildenv.sh",
    },
    {
        "os": "macOS",
        "triplet": "x64-osx",
        "release_triplet": "x64-osx-rel",
        "file": "tools/macos_buildenv.sh",
    },
    {
        "os": "macOS",
        "triplet": "arm64-osx",
        "release_triplet": "arm64-osx-rel",
        "file": "tools/macos_buildenv.sh",
    },
    {
        "os": "Linux",
        "triplet": "arm64-android",
        "release_triplet": "arm64-android-release",
        "file": "tools/android_buildenv.sh",
    },
]


def update(defs, channel, data):
    with open(defs["file"], "r") as file:
        content = file.read()

    # Perform the replacement
    pattern = defs.get("pattern", DEFAULT_PATTERN).format(**data)
    replace = defs.get("replace", DEFAULT_REPLACE).format(**data)
    content = re.sub(pattern, replace, content)

    with open(defs["file"], "w") as file:
        file.write(content)


def get_raw_releases(platform, version):
    url = f"{MIXXX_DOWNLOAD_BASE}{version}/{platform}/"
    with urlopen(url) as f:
        return f.read().decode()


def get_sha256sum(platform, version, filename):
    with urlopen(
        f"{MIXXX_DOWNLOAD_BASE}{version}/{platform}/{filename}.sha256sum"
    ) as f:
        return f.read().decode().split(" ")


def parse_files(data):
    return [
        re.split(r"([<>]|\W{2,})", d)
        for d in data.split("\n")
        if d.strip().startswith("<a href")
    ]


def extra_info(data):
    filename = data[4]
    components = filename.removesuffix(".zip").split("-")
    return dict(
        filename=filename,
        triplet="-".join(components[3:-1]),
        version=components[-1],
        timestamp=datetime.strptime(data[-3].strip(), "%d-%b-%Y %H:%M"),
    )


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(
            (
                "Usage: %s <channel>\n  <channel> is Mixxx's "
                "VCPKG release branch, e.g. 2.6."
            )
            % sys.argv[0],
            file=sys.stderr,
        )
        sys.exit(-1)
    channel_prefix = sys.argv[1]

    for platform in PLATFORMS:
        os = platform["os"]
        data = None
        for is_release in [True, False]:
            triplet = platform[
                "triplet" if not is_release else "release_triplet"
            ]
            channel = (
                channel_prefix if not is_release else f"{channel_prefix}-rel"
            )
            latest = None
            try:
                data = get_raw_releases(os, channel)
                files = list(map(extra_info, parse_files(data)))
                releases = list(
                    filter(
                        lambda d: not d["filename"].endswith("sha256sum")
                        and triplet == d["triplet"],
                        files,
                    )
                )
                releases.sort(key=lambda d: d["timestamp"])
                latest = releases[-1]
            except (HTTPError, IndexError):
                print(
                    (
                        f"Cannot find release for channel {channel} "
                        f"on platform {os}/{triplet}"
                    ),
                    file=sys.stderr,
                )
                continue
            latest.update(
                dict(
                    shasum=get_sha256sum(
                        platform["os"], channel, latest["filename"]
                    )[0]
                )
            )
            print(
                f"Latest version for {os}/{triplet}/{channel} is "
                f"{latest['filename']} with shasum: {latest['shasum']}"
            )

            update(platform, channel, latest)
