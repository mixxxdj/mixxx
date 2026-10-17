# Chameleon — Development Plan

> **Chameleon** is a mobile web interface for Mixxx, built with Alpine.js 3, DaisyUI 5, Tailwind CSS 4 and Vite.
> It communicates with the Mixxx backend via HTTP POST to `/rcontrol`.
>
> **Note**: This mobile interface is not intended to replace Mixxx's main desktop UI.
> The structure stays in `res/web/chameleon` to allow other users to develop alternative `web skins` in `res/web/` in parallel.

---

## 1. Architectural Vision

### High-Level Goals

| Goal | Description |
| ------ | ------------- |
| **Mobile-first** | Responsive, touch-friendly interface optimized for smartphones and tablets |
| **Real-time reactivity** | Deck state polling every 1s (only when visible), configurable AutoDJ refresh |
| **Feature coverage** | Mixer (gain/EQ/fx/volume), transport (play/cue/seek), AutoDJ, library (search + deck load) |
| **Extensibility** | Modular architecture for new panels (hot cue, library browser, pitch/key) |
| **Theming** | Full support for 13 DaisyUI themes, persisted in `localStorage` |

### Explicitly Out of Scope

| Feature | Reason |
| --------- | -------- |
| **Waveform** (SVG/canvas) | Would require a new backend endpoint for waveform data. Not needed for mobile remote control. |
| **Loop** (set/recall loop) | Advanced performance feature, outside mobile scope. |
| **Recording** (record control) | Not relevant to DJ set remote control. |
| **Broadcast** (stream control) | Not relevant to DJ set remote control. |
| **FX Panel** (FX racks per deck) | Too complex for mobile. Super FX knobs are sufficient. |
| **EQ Kill switches** (high/mid/bass kill) | Hardware controller feature, not for mobile. |
| **Replace Mixxx main UI** | Chameleon is a mobile complement, not a desktop replacement. |
| **TypeScript** | Project stays in pure JavaScript. |

### Tech Stack

```
Vite 8.x          → Build, HMR, modular bundling
Alpine.js 3.15    → Reactive data model, declarative templates
DaisyUI 5         → Mobile-ready UI components on Tailwind CSS 4
Tailwind CSS 4    → Utility-first styling
SortableJS        → Drag & drop for AutoDJ queue
Iconify           → MDI icons (Material Design Icons)
```

### Communication Pattern

```
[Alpine component] ──calls──> app.js (component methods)
       │
       ▼
proxy.rcontrol() ──fetch POST──> /rcontrol (Mixxx QHttpServer)
       │                              │
       ▼                              ▼
                             remote.cpp (C++)
                                     │
                                     ▼
                               ControlObject / SQLite / PlaylistDAO
```

> **Note**: DEMO_MODE no longer exists. The project is developed inside the Mixxx codebase
> and requires building a branch that includes `remote.cpp`.

---

## 2. Current State Analysis

### 2.1 What Works (Already Implemented)

| Area | Detail |
| ------ | -------- |
| **Authentication** | Password login, cookie session, redirect to `/chameleon/` |
| **Mixer – Deck** | 1s polling (mixer visible only): play/pause, position, duration, artist/title. Transport: play, stop, cue |
| **Mixer – Knob** | 6 knobs per deck (gain, high, mid, bass, super_fx, volume). ControlObject mapping via `setParameter/getParameter`. Double-click resets to `default` value (50 for gain/EQ, 100 for volume) |
| **Mixer – Crossfader** | Slider -1..+1, reactive binding via watcher |
| **Mixer – Main Output Gain** | Slider 0..100, visible in both Mixer tab (below crossfader) and AutoDJ tab (top) |
| **AutoDJ** | Toggle activation, drag & drop queue via SortableJS, add/remove tracks. Polling every N seconds (default 10, configurable) only when tab is visible |
| **Library** | Text search with 300ms debounce, track selection, FAB with buttons for all visible decks |
| **Themes** | 13 DaisyUI themes, persisted, selector in Settings drawer |
| **Settings** | Deck visibility/ordering via SortableJS, configurable AutoDJ refresh interval |
| **Generic backend** | `setParameter` / `getParameter` for any ControlObject group/key |
| **Knob** | Custom SVG component with tangential drag, touch support, double-click to default |
| **Build** | Vite with manifest for deployment on QHttpServer static under `/chameleon/` |

### 2.2 Bug Fixes Applied

| # | Problem | Fix |
| --- | --------- | ----- |
| 1 | **Legacy mixxx.js** | File removed (544 lines of deprecated JS) |
| 2 | **DEMO_MODE / MOCK** | Removed from endpoints.js, proxy.js, login.js |
| 3 | **Initial volume at 50%** | `new Channel(index)` → `volume: 100` (Mixxx default 1.0) |
| 4 | **Double-click resets to 50%** | Knob now has configurable `defaultVal`. Double-click → `defaultVal` (50 for gain/EQ, 100 for volume) |
| 5 | **Crossfader watcher redundant parseFloat** | Removed `parseFloat(val)`, value already arrives as number |
| 6 | **Hardcoded Test Zone buttons** | Removed Up/Down/Remove buttons with fixed parameters (0,0) |
| 7 | **FAB shows only deck 1** | FAB template now iterates over `settings.decks` filtering visible ones |
| 8 | **Deck polling always active** | Now polls only when `activeTab === 'mixer'` |
| 9 | **AutoDJ never updated** | New periodic polling (default 10s) only when `activeTab === 'autodj'` |
| 10 | **Main Output Gain slider missing** | Added in Mixer tab (below crossfader) and AutoDJ tab (top) |
| 11 | **Hardcoded redirect paths** | Updated to `/chameleon/login.html` and `/chameleon/` for consistency |
| 12 | **Volume knob double-click resets to 50** | Volume knob was missing `default: 100` — added in `index.html` and `deck-channel.html` |

### 2.3 What's Missing (To Implement)

| # | Feature | Priority | Dependencies |
| --- | --------- | ---------- | -------------- |
| 1 | **Knob initialization from backend** | **Medium** | `getParameter` for each KNOB_PARAMS in `loadInitialState` |
| 2 | **Pitch / Key control** (pitch slider, keylock, BPM display) | **Medium** | ControlObject: `rate`, `pitch`, `keylock`, `file_bpm` |
| 3 | **CUE and SYNC buttons** | **Medium** | ControlObject: `cue_default`, `sync_enabled` |
| 4 | **Offline overlay** (Mixxx not responding or connection lost) | **Medium** | Fetch timeout detection, UI overlay component |
| 5 | **Add to AutoDJ with modal** (next track or in two? if next, pause AutoDJ, add, re-enable) | **Medium** | Modal component, AutoDJ state management |
| 6 | **Hot cue** (save and recall) | **Low** | `HotCue` class already defined but unused |
| 7 | **Library Browser** (playlists, artists, albums, crates) | **Low** | New C++ endpoints |
| 8 | **Connection status indicator** (online/offline badge, auto-retry) | **Medium** | — |
| 9 | **Loading feedback** (spinner on slow operations) | **Low** | — |
| 10 | **BPM / Key display in deck card** | **Low** | Already returned by `getDeckState` |
| 11 | **Swipe-to-delete AutoDJ track** | **Low** | Touch gesture library (e.g. Swiper.js, Hammer.js) |

### 2.4 Architectural Gaps

| # | Gap | Description |
| --- | ----- | ------------- |
| 1 | ~~**Monolithic component**~~ | `app.js` — **REFACTORING COMPLETED**: models, API and utilities are in separate modules. Alpine logic stays in `app.js` to avoid getter issues. |
| 2 | **No API/State/View separation** | API calls and presentation logic are still mixed in `app.js`. Acceptable for current size. |
| 3 | **Simple polling** | `setInterval` without backoff, no cancellation when tab is in background. |
| 4 | **No tests** | Zero automated tests. |
| 5 | **Backend lacks push** | Everything is client-pull. WebSocket or SSE would eliminate polling. |

---

## 3. Implementation Priorities

### Phase 1 — Bug Fixes & Corrections (Completed)

- [x] Gain/EQ formula: `(2*x)²` applied in `mapToRange_0_1_4`
- [x] Volume starts at 100 (Mixxx default 1.0 = 100%)
- [x] Main Output Gain slider in Mixer and AutoDJ UI
- [x] Knob double-click resets to `default` value
- [x] Removed Test Zone with hardcoded parameters
- [x] Removed legacy `mixxx.js`
- [x] Removed redundant `parseFloat(val)` in crossfader watcher
- [x] Removed DEMO_MODE and MOCK (code and dependencies)
- [x] Fixed FAB to show all visible decks
- [x] Deck polling only when mixer visible
- [x] Configurable periodic AutoDJ polling
- [x] Login page with DaisyUI styling
- [x] Redirect path `/chameleon/`

### Phase 2 — Architectural Refactoring (Completed)

```
Goal: Make the code maintainable and extensible.
```

#### 2A — Frontend Modularization (JavaScript)

```
src/
├── main.js                    Alpine init, plugins, theme store
├── app.js                     Main Alpine component
├── models/
│   ├── channel.js             Channel class
│   └── deck.js                Deck, HotCue classes
├── api/
│   ├── proxy.js               rcontrol() wrapper
│   ├── endpoints.js           CMD constants
│   ├── login.js               Auth
│   ├── decks.js               Deck API calls
│   ├── autodj.js              AutoDJ API calls
│   └── library.js             Library API calls
├── utils/
│   ├── format.js              formatTime, findInResponse
│   └── knobs.js               KNOB_PARAMS, mapToRange_*, getKnobParam
├── components/                [BLUEPRINT] Not imported — documentation
│   ├── deck.js                Deck logic blueprint
│   ├── mixer.js               Mixer logic blueprint
│   ├── autodj.js              AutoDJ logic blueprint
│   ├── library.js             Library logic blueprint
│   └── settings.js            Settings logic blueprint
└── x-components/
    ├── knob.js                Alpine.data('knob', ...) — IMPORTED
    ├── knob.html              SVG knob blueprint
    ├── deck-card.html         [BLUEPRINT] Deck card
    ├── deck-channel.html      [BLUEPRINT] Mixer channel
    ├── track-list-row.html    [BLUEPRINT] Track list row
    └── now-playing.html       [BLUEPRINT] "Now Playing"
```

#### 2B — Critical Rules to Avoid Breaking the App

1. **NEVER use spread operator (`...`) to mix objects with getters in Alpine**
   - `get playingDecks() { ... }` is a getter
   - `...deckMethods` copies the **value** at spread time, not the definition
   - Alpine loses references to `settings`, `activeTab`, `Mixer`, etc.
   - **Solution**: all Alpine code must stay in the factory function's literal object

2. **`Alpine.$persist()` must be called inside the factory function**, not at top-level of an imported module
   - At import time, `Alpine` may not be available yet
   - Original `settings.js` called `Alpine.$persist()` at top-level → crash

3. **Import `Alpine` explicitly** in every file that uses it
   - `import Alpine from "alpinejs"` in `app.js`
   - Even if `main.js` does `window.Alpine = Alpine`, ES6 modules have their own scope

4. **Files in `components/` and `x-components/*.html` are BLUEPRINTS**
   - They serve as documentation of HTML/logic structure
   - NOT imported by `app.js` or `main.js`
   - Can be activated in the future only if the getter problem is solved (e.g. with ES6 classes or `Object.defineProperty`)

5. **The only active x-component is `knob.js`** (imported in `main.js` via `Alpine.data("knob", x_comp_knob)`)
   - Because it's a pure component without getters, based on SVG rendering

#### 2C — Backend Enhancement

- **`#include <QUuid>`**: QUuid is the Qt class for generating and managing UUIDs. Used in `remote.cpp` for unique session identifiers (`QUuid::createUuid()`). Without the explicit include, the compiler may not find the class definition. Even if indirectly included via other Qt dependencies, it's good practice to include it explicitly.
- Add optional CORS header route (`Access-Control-Allow-Origin: *` when enabled).
- Validate input on all commands (prevent crashes on malformed values).
- Add handlers for missing commands if needed.

### Phase 3 — New Features (Medium Priority)

- [ ] **Knob initialization from backend**: after loading initial state, call `getParam` for each KNOB_PARAMS of visible decks and update Channel models.
- [ ] **Pitch/Key panel**: pitch slider -1..+1, keylock toggle, BPM/key display.
- [ ] **CUE and SYNC buttons**: add cue and sync controls to deck transport.
- [ ] **Offline overlay**: detect connection loss (fetch timeout / network error), show overlay with "Mixxx not responding or connection closed" message.
- [ ] **Add to AutoDJ with modal**: when adding a track to AutoDJ, show a modal asking "Play next or in two tracks?" If "next", temporarily disable AutoDJ, add track at top, re-enable AutoDJ.
- [ ] **Connection indicator**: online/offline badge with automatic retry on fetch failure.
- [ ] **Loading indicator**: spinner/placeholder during fetch.

### Phase 4 — Performance & UX (Medium-Low Priority)

- [ ] **Intelligent polling**: use `requestAnimationFrame` + exponential backoff on error + suspend when tab is not visible (`document.visibilitychange`).
- [ ] **Virtual scrolling** for AutoDJ queue and search results (when > 50 items).
- [ ] **Reduce backend requests**: aggregate multiple `getParameter` into a single `getParameters` call (array of group/key). **Draft — not working yet (high priority).**
- [ ] **Debounce on watchers**: crossfader and Main Output Gain should be throttled to 100ms instead of sending on every slider event.

### Phase 5 — Backend Push (Optional, Low Priority)

- [ ] Evaluate SSE (Server-Sent Events) for engine state push → frontend.
- [ ] Would eliminate client-side polling.
- [ ] Would require a second persistent HTTP connection.

---

## 4. Data Schemas and API Contracts

### 4.1 Backend Commands (JSON Schema)

Category: **Mixer**

```json
// GET / SET crossfader
{"getcrossfader": "true"}                          → {"crossfader": 0.0}
{"setcrossfader": {"value": 0.5}}

// GET / SET Main Output Gain
{"getmastergain": "true"}                          → {"mastergain": 1.0}
{"setmastergain": {"gain": 0.8}}

// GET / SET deck knob parameter (via generic setParameter/getParameter)
{"getParameter": {"group": "[Channel1]", "key": "pregain"}}
  → {"group": "[Channel1]", "key": "pregain", "value": 1.0}
{"setParameter": {"group": "[Channel1]", "key": "pregain", "value": 1.0}}
```

Category: **Deck**

```json
{"getdeckstate": {"deck": 1}}
  → {"deck": 1, "playing": true, "position": 0.35, "duration": 245.0, "elapsed": 85.75, "artist": "...", "title": "..."}

{"setdeckplay": {"deck": 1, "playing": true}}
{"setdeckposition": {"deck": 1, "position": 0.5}}
{"deckcue": {"deck": 1}}
{"deckstop": {"deck": 1}}
{"loaddeck": {"deck": 1, "trackid": 42, "play": false}}
```

Category: **AutoDJ**

```json
{"getautodjenabled": "true"}                       → {"autodjenabled": true}
{"setautodjenabled": {"enabled": true}}
{"getautotracklist": "true"}                       → {"tracklist": [{"id": 101, "position": 0, "artist": "...", "title": "..."}]}
{"addautodj": {"trackid": "101", "position": "end"}}
{"delautodj": {"position": "0", "trackid": "101"}}
{"moveautotracklist": {"position": "0", "newposition": "2"}}
```

Category: **Library**

```json
{"searchtrack": "query"}                           → {"tracklist": [{"id": 201, "artist": "...", "title": "...", "duration": 320, "bpm": 123, "key": "8A", "timesplayed": 42}]}
```

Category: **Auth**

```json
{"login": {"password": "..."}}                     → {"sessionid": "uuid"}
{"sessionid": "uuid"}                              → {"sessionid": "uuid", "logintime": "..."}
```

### 4.2 Knob → ControlObject Mapping

| Knob | Group CO | Key CO | Engine Range | valueFn | UI Default |
| ------ | ---------- | -------- | ------------- | --------- | ------------ |
| gain | `[ChannelN]` | `pregain` | 0..1 (ControlAudioTaperPot -12..+12dB) | `(2*x)²` | 50 |
| high | `[EqualizerRack1_[ChannelN]_Effect1]` | `parameter3` | 0..1 | `(2*x)²` | 50 |
| mid | `[EqualizerRack1_[ChannelN]_Effect1]` | `parameter2` | 0..1 | `(2*x)²` | 50 |
| bass | `[EqualizerRack1_[ChannelN]_Effect1]` | `parameter1` | 0..1 | `(2*x)²` | 50 |
| super_fx | `[QuickEffectRack1_[ChannelN]]` | `super1` | 0..1 | `v/100` | 50 |
| volume | `[ChannelN]` | `volume` | 0..1 | `v/100` | 100 |

### 4.3 Frontend Data Model

```typescript
// Deck state (from polling)
interface DeckState {
  id: number;
  play: boolean;
  position: number;    // 0..1
  duration: number;    // seconds
  elapsed: number;     // seconds
  artist: string;
  title: string;
}

// Channel mixer state
interface ChannelState {
  index: number;       // 1..4
  gain: number;        // 0..100 (UI)
  high: number;        // 0..100 (UI)
  mid: number;         // 0..100 (UI)
  bass: number;        // 0..100 (UI)
  super_fx: number;    // 0..100 (UI)
  volume: number;      // 0..100 (UI) — default 100
}

// Track entry (search or AutoDJ queue)
interface TrackEntry {
  id: number;
  artist: string;
  title: string;
  duration: number;    // seconds
  bpm: number;
  key: string;
  timesplayed: number;
  position?: number;   // AutoDJ queue position
}

// Settings deck
interface DeckSetting {
  id: number;
  visible: boolean;
}
```

---

## 5. Quality Metrics

| Metric | Target | How to Measure |
| -------- | -------- | ---------------- |
| **Initialization time** | < 2s on local network | DevTools Performance tab |
| **Deck polling overhead** | < 50ms per cycle | `console.time` / `performance.now()` |
| **JS build size** | < 100KB gzip | `npx vite build && ls -lh dist/` |
| **Feature coverage** | 100% of planned features | User checklist |
| **Screen errors** | 0 | DevTools Console, Network tab |
| **Theme switch** | < 100ms | DevTools Paint profiling |

---

## 6. Glossary

| Term | Definition |
| ------ | ------------ |
| **CO** | ControlObject — Mixxx internal communication system based on `[Group], key` |
| **RControl** | HTTP endpoint `/rcontrol` for remote control of Mixxx |
| **AutoDJ** | Automatic track queuing and playback system in Mixxx |
| **FAB** | Floating Action Button (DaisyUI) |
| **Dock** | Fixed bottom navigation bar (mobile pattern) |
| **DaisyUI** | CSS component library for Tailwind CSS |
| **Alpine.js** | Reactive JavaScript framework for declarative interfaces |
| **Vite** | Frontend build tool with native HMR |
| **Blueprint** | Documentation file describing HTML/logic structure of a component, not actively imported |

---

*Document generated on 2026-07-28 — Base commit: `874b0d903bc43770703c93e682d56c38dca1081d`*
