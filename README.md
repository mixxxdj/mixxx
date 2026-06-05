# DJ Sugar

[![Build status](https://github.com/winnerspiros/mixxx/actions/workflows/build.yml/badge.svg)](https://github.com/winnerspiros/mixxx/actions/workflows/build.yml)
[![License: GPL v2](https://img.shields.io/badge/License-GPLv2-blue.svg)](LICENSE)

**DJ Sugar** is a free, open-source DJ application for Android (and desktop)
that gives you everything you need to perform live DJ sets. Built on top of
the [Mixxx] codebase, DJ Sugar adds first-class Android support, built-in
YouTube search & streaming, SponsorBlock integration, and a streamlined
mobile experience.

## Features

- 🎵 **YouTube integration** — search, stream, and mix YouTube audio directly
  inside the app; no yt-dlp binary required on Android
- 📱 **Android-first** — optimised for Samsung Galaxy and other ARM64 devices;
  correct scaling, Oboe audio output, and MANAGE_EXTERNAL_STORAGE support
- ⚡ **Performance** — InnerTube ANDROID_VR client for direct (no-cipher)
  stream URLs; HTTP/1.1 forced on Android to avoid Qt HTTP/2 stalls
- 🚫 **SponsorBlock** — automatically skips sponsored segments in YouTube
  music videos
- 🎛️ **4-deck mixing** — pitch, EQ, effects, looping, and hot cues
- 🎚️ **External controller support** — MIDI and HID devices (USB permission
  prompt on Android 13+)
- 📦 **Local library** — scans device storage, SD card, and cloud-synced
  folders; supports MP3, FLAC, OGG, OPUS, WAV, AAC, M4A, and more

## Quick Start

### Android

Install the latest APK from the [Releases] page, grant storage and audio
permissions when prompted, and tap the **YouTube** tab to start searching.

### Desktop (Linux / Windows / macOS)

Clone and build following the [upstream Mixxx build instructions][contrib]:

```bash
git clone https://github.com/winnerspiros/mixxx.git
cd mixxx
# Follow CONTRIBUTING.md for cmake/vcpkg setup
```

## Bug Reports

Found an issue? [Open a GitHub issue][issues]. Please attach your `mixxx.log`
file — it lives at:

- **Android**: `Android/data/org.mixxx/files/Documents/Mixxx/`
- **Linux**: `~/.local/share/mixxx/`
- **Windows**: `%LOCALAPPDATA%\Mixxx\`
- **macOS**: `~/Library/Containers/org.mixxx.mixxx/Data/Library/Application Support/Mixxx/`

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md) for build instructions, code style
guidelines, and how to open a pull request.

## License

DJ Sugar is released under the GPLv2. See the [LICENSE](LICENSE) file for the
full text. This project is a fork of [Mixxx], copyright the Mixxx contributors.

[Mixxx]: https://github.com/mixxxdj/mixxx
[Releases]: https://github.com/winnerspiros/mixxx/releases
[issues]: https://github.com/winnerspiros/mixxx/issues/new/choose
[contrib]: https://github.com/mixxxdj/mixxx/blob/main/CONTRIBUTING.md
