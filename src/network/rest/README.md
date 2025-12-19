# REST API developer notes

Mixxx ships with an optional REST API for lightweight control and monitoring.
This document summarizes defaults, security, and available routes.

## Defaults

- The REST server is disabled by default. Enable it from Preferences ➜ REST API.
- When enabled, HTTP listens on `localhost:8989` unless you change the host or port.
  HTTPS uses `localhost:8990` when TLS is enabled.
- HTTP access is allowed unless you disable it. TLS is off by default and must be
  explicitly turned on. Control routes are accessible without TLS but HTTPS is strongly recommended.

## Authentication and authorization

- Multiple bearer tokens are supported (up to 16). Each token stores:
  - Permission: `read` (health/status/GET routes) or `full` (all routes including control/mutation).
  - Description: optional user-provided label.
  - Created at: recorded in UTC.
  - Expiration: optional (UTC). Leave blank for “never expires”.
- Manage tokens in Preferences ➜ REST API: add, remove, regenerate, edit description/permission/expiration.
- Leave all tokens empty to allow unauthenticated access (not recommended).
- Include tokens as `Authorization: Bearer <token>`.
- Using tokens without HTTPS exposes them; enable TLS whenever tokens are set, especially off localhost.

## TLS certificates

- Enable HTTPS to serve the API over TLS. If you opt into automatic generation and OpenSSL is available,
  Mixxx creates a self-signed certificate at `<settings>/rest/rest_certificate.pem` with a matching
  private key at `<settings>/rest/rest_private_key.pem`.
- The same files are reused on future launches. You can also point Mixxx at existing certificate and key paths
  instead of auto-generation.
- Startup logs report whether the certificate was generated or loaded, and log errors if TLS preparation fails.

## Endpoints

All routes are available with and without the `/api` prefix (for example, `/health` and `/api/health`).

## REST API architecture and control flow

The REST API is a thin HTTP façade that routes requests into Mixxx's existing
control and library subsystems. It is split into a server component that handles
HTTP transport and authentication, plus a gateway component that translates
requests into ControlProxy and library actions.

```
HTTP client
    |
    v
RestServer (QHttpServer, auth, TLS, routing)
    |
    | invokeGateway() via queued connection
    v
RestApiGateway (JSON parsing + response shaping)
    |
    +--> ControlProxy -> ControlObject system -> Engine/PlayerManager
    |
    +--> TrackCollectionManager/DAO -> Library database
```

Control flow for a control request (for example, `POST /control`) looks like:

```
POST /control
  -> RestServer::route()
  -> authorize() / parse JSON
  -> invokeGateway()
  -> RestApiGateway::control()
  -> ControlProxy(group, key).set(value)
  -> ControlObject system updates engine state
  -> JSON response to client
```

### Why ControlProxy (and not D-Bus)

Mixxx already exposes its internal state and actions through the ControlObject
system. Using ControlProxy keeps the REST API aligned with that existing model:

- **Reuse of stable control names:** REST commands map directly to the same
  `group`/`key` pairs used throughout the app, avoiding a parallel API surface.
- **Thread-safe integration:** RestServer hands work to RestApiGateway through a
  queued Qt connection, and ControlProxy is the standard way to read/write
  ControlObjects from the UI thread.
- **Cross-platform reach:** HTTP works consistently on Linux, Windows, and
  macOS. A D-Bus-based API would be Linux-specific and require a separate IPC
  service definition and client bindings.

This keeps the REST API lightweight, consistent with existing control logic, and
available on all Mixxx-supported platforms.

### Health and status

- `GET /health` — liveness, uptime, timestamp, readiness issues, and system metrics (CPU usage when available, RSS bytes).
- `GET /ready` — readiness summary with dependency issues.
- `GET /status` — application info, decks, mixer state, broadcast/recording status, AutoDJ overview, uptime, timestamp,
  and system metrics.

### Deck status

- `GET /decks` — status for all decks (same payload as the deck list in `/status`).
- `GET /decks/<n>` — status for a single deck (1-based index).

### Control

- `POST /control` — control Mixxx via JSON body. Supported styles include:
  - Commands: `{ "command": "play", "group": "[Channel1]" }`, `{ "command": "seek", "position": 0.5 }`.
  - Direct control values: `{ "group": "[Master]", "key": "gain", "value": 1.2 }`.

### AutoDJ

- `GET /autodj` — fetch AutoDJ status and a sample of queued tracks.
- `POST /autodj` — manage AutoDJ with an `action` field:
  - `enable`, `disable`, `skip`, `fade`, `shuffle`, `add_random`, `clear`
  - `add` with `track_ids` and optional `position`
  - `move` with `from`/`to`

### Playlists

- `GET /playlists` — list playlists with metadata and the active playlist id.
- `GET /playlists?id=<id>` — fetch tracks for a specific playlist.
- `POST /playlists` — manage playlists with an `action` field:
  - `create` with `name`
  - `delete`
  - `rename`
  - `set_active`
  - `add` with `track_ids` and optional `position`
  - `remove` with `positions`
  - `reorder` with `from`/`to`
  - `send_to_autodj` with `top`, `bottom`, or `replace` (pushes an entire playlist into the AutoDJ queue)

TLS is strongly recommended whenever authentication or control endpoints are used, especially outside localhost.
