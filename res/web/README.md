# Mixxx HTTP Remote Controller

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

> **Security Warning:** This feature exposes your Mixxx instance to the local network. Use it only on trusted, protected networks. Set a strong password in Mixxx Preferences -> Remote Control. The authors provide no guarantees regarding security or data integrity. Use at your own risk.

## Overview

HTTP Remote Controller enables remote control of Mixxx DJ software via web browsers and mobile devices. It consists of a Qt6 HTTP backend (`src/remote/`) and a mobile-first web interface called Chameleon (`res/web/chameleon/`), built with Alpine.js 3, DaisyUI 5, and Tailwind CSS 4.

Chameleon is a complementary mobile interface -- it does not aim to replace Mixxx's main desktop UI. It features a modern design with multiple light and dark color themes. All settings, including theme choice, deck visibility, and polling preferences, are persisted across sessions via cookies.

## Motivation

During dinners, parties, or social events, the DJ may be away from the mixer -- sitting at the table, mingling with guests, or taking a break. Having a lightweight remote control on a smartphone or tablet allows basic supervision of the set without being tied to the booth.

- Remote control without additional hardware -- control your DJ set from a smartphone, tablet, or any device with a browser
- Web-based interface -- no app installation required, accessible from any browser on the local network
- Open API -- REST endpoints allow third-party integrations and custom frontends

## Features

### Deck Controls

Monitor and control each deck: play/pause, stop, cue, seek, and load tracks. The interface shows current play state, position, elapsed time, artist, title, BPM, and key. Multiple decks can be shown or hidden from the settings panel.

### Mixer Controls

Adjust gain, EQ (high/mid/bass), Super FX, and volume per channel via touch-friendly knobs. Control crossfader and main output gain. Knob values are polled from the engine every 2 seconds when the mixer tab is visible, using a single batch request for all visible decks.

### AutoDJ

Enable or disable AutoDJ, view the upcoming queue, reorder tracks via drag-and-drop, add tracks from the library, and remove tracks. The queue refreshes automatically at a configurable interval (default 10 seconds) when the AutoDJ tab is visible.

### Library

Search tracks by artist, title, or album. Results show duration, BPM, key, and play count. Selected tracks can be loaded directly to a deck or added to the AutoDJ queue.

## Getting Started

HTTP Remote is experimental and may not be integrated into upstream Mixxx releases due to security considerations that are difficult to anticipate and manage. You need to build Mixxx from the pull request: [#16682](https://github.com/mixxxdj/mixxx/pull/16682)

```bash
# Configure with HTTP Remote enabled
cmake -B build -DHTTP_REMOTE=ON

# Build
cmake --build build -j$(nproc)

# Run Mixxx
./build/mixxx
```

In Mixxx, go to Preferences -> Remote Control, enable the server and set the port (default 8080). Then open in your browser:

`http://localhost:8080/chameleon/index.html`

## Contributing

Contact us on Zulip in the thread **# development > http remote support**.

See [CONTRIBUTING.md](https://github.com/mixxxdj/mixxx/blob/main/CONTRIBUTING.md) for code style, build instructions, and Git workflow.

## Project Structure (Draft)

```text
res/web/chameleon/
├── index.html              Main application shell
├── login.html              Login page
├── README.md               This file
├── PLAN.md                 Development roadmap
├── src/
│   ├── main.js             Alpine init, plugins, theme store
│   ├── app.js              Alpine component (main logic)
│   ├── styles.css          Custom styles
│   ├── api/
│   │   ├── proxy.js        rcontrol() fetch wrapper
│   │   ├── endpoints.js    Command constants
│   │   ├── login.js        Authentication API
│   │   ├── decks.js        Deck API calls
│   │   ├── autodj.js       AutoDJ API calls
│   │   └── library.js      Library search API
│   ├── models/
│   │   ├── channel.js      Channel state model
│   │   └── deck.js         Deck state model
│   ├── utils/
│   │   ├── format.js       Time formatting utilities
│   │   └── knobs.js        Knob parameter mappings
│   └── x-components/
│       └── knob.js         Alpine x-component for SVG knob
├── package.json
├── vite.config.js
└── tailwind.config.js
```

### Frontend Build

```bash
cd res/web/chameleon
npm install
npx vite build
```

## License

GPL v3 -- see [LICENSE](https://github.com/mixxxdj/mixxx/blob/main/LICENSE).
