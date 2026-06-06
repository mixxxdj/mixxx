<div align="center">

<img src="https://raw.githubusercontent.com/winnerspiros/djsugar/main/res/images/icons/scalable/apps/mixxx.svg" width="128" height="128" alt="DJ Sugar logo">

# DJ Sugar

**Free, open-source DJ software for everybody.**

[![CI](https://img.shields.io/github/actions/workflow/status/winnerspiros/djsugar/build.yml?branch=main&label=build&logo=github)](https://github.com/winnerspiros/djsugar/actions/workflows/build.yml)
[![Auto-Release](https://img.shields.io/github/actions/workflow/status/winnerspiros/djsugar/auto-release.yml?branch=main&label=release&logo=github)](https://github.com/winnerspiros/djsugar/actions/workflows/auto-release.yml)
[![License](https://img.shields.io/badge/license-GPLv2-blue.svg)](LICENSE)
[![Latest Release](https://img.shields.io/github/v/release/winnerspiros/djsugar?include_prereleases&label=release&logo=github)](https://github.com/winnerspiros/djsugar/releases)

[Download](#downloads) · [Features](#features) · [Building](#building) · [Contributing](#contributing)

</div>

---

## What is DJ Sugar?

DJ Sugar is a fork of [Mixxx](https://mixxxdj.org) — the free, open-source DJ software. It adds first-class **Android** support, built-in **YouTube** search & streaming, **SponsorBlock** integration, and a mobile-optimized experience while keeping everything that makes Mixxx great on desktop.

Available on **Android**, **Linux**, **macOS**, and **Windows**.

## Downloads

Grab the latest builds from the [Releases](https://github.com/winnerspiros/djsugar/releases) page. Every push to `main` publishes a new pre-release automatically.

| Platform | File | Notes |
|----------|------|-------|
| **Android** | `*.apk` | ARM64, side-load & play |
| **Windows** | `*.msi` | x64 + ARM64 |
| **macOS** | `*.dmg` | Intel + Apple Silicon |
| **Ubuntu** | `*.deb` | 24.04 LTS |

> Nightly builds are marked as pre-release. Tag a commit with `vX.Y.Z` for a stable release.

## Features

- **YouTube integration** — search, stream, and mix YouTube audio directly; no yt-dlp binary on Android
- **SponsorBlock** — auto-skips sponsored segments in YouTube videos
- **Android-optimized** — Oboe audio, correct scaling, MANAGE_EXTERNAL_STORAGE, controller USB permissions
- **InnerTube direct streams** — ANDROID_VR client for no-cipher stream URLs; HTTP/1.1 forced on Android
- **4-deck mixing** — pitch, EQ, effects, looping, hot cues
- **Controller support** — MIDI and HID devices (USB on Android 13+)
- **Local library** — scans device storage, SD card, cloud-synced folders; MP3, FLAC, OGG, OPUS, WAV, AAC, M4A, and more

## Building

### Android

```bash
git clone https://github.com/winnerspiros/djsugar.git
cd djsugar
# Install Android SDK/NDK, then:
cmake -B build -DANDROID=ON -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
# APK output: build/android-build/build/outputs/apk/release/
```

### Desktop (Linux / macOS / Windows)

Follow the [upstream Mixxx build instructions](https://github.com/mixxxdj/mixxx/blob/main/CONTRIBUTING.md) — DJ Sugar uses the same CMake/vcpkg toolchain.

```bash
git clone https://github.com/winnerspiros/djsugar.git
cd djsugar
# See CONTRIBUTING.md for full cmake/vcpkg setup
```

## Contributing

1. Fork and create a branch
2. Read [CONTRIBUTING.md](CONTRIBUTING.md) for code style and PR guidelines
3. Open a pull request

Found a bug? [Open an issue](https://github.com/winnerspiros/djsugar/issues/new/choose).
Attach `mixxx.log` from your device:

| Platform | Log location |
|----------|-------------|
| Android | `Android/data/org.mixxx/files/Documents/Mixxx/` |
| Linux | `~/.local/share/mixxx/` |
| Windows | `%LOCALAPPDATA%\Mixxx\` |
| macOS | `~/Library/Containers/org.mixxx.mixxx/Data/Library/Application Support/Mixxx/` |

## License

GPLv2. See [LICENSE](LICENSE). DJ Sugar is a fork of [Mixxx](https://github.com/mixxxdj/mixxx), copyright the Mixxx contributors.
