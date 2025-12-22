# REST API developer notes

Mixxx ships with an optional REST API for lightweight control and monitoring.
This document summarizes defaults, security, and available routes.

## Defaults

- The REST server is disabled by default. Enable it from Preferences ➜ REST API.
- When enabled, the default listeners are:
  - HTTP: `localhost:8989`
  - HTTPS: `localhost:8990` (when TLS is enabled)
- HTTP access is allowed unless you disable it. TLS is off by default and must be
  explicitly turned on. Control routes are accessible without TLS but HTTPS is strongly recommended.

## Authentication and authorization

- Multiple bearer tokens are supported (up to 16). Each token stores:
  - Scopes: granular permissions such as `status:read`, `autodj:write`, or `control:write`.
  - Description: optional user-provided label.
  - Created at: recorded in UTC.
  - Expiration: optional (UTC). Leave blank for “never expires”.
- Manage tokens in Preferences ➜ REST API: add, remove, regenerate, edit description/scopes/expiration.
- Leave all tokens empty to allow unauthenticated access (not recommended).
- Include tokens as `Authorization: Bearer <token>`.
- Using tokens without HTTPS exposes them; enable TLS whenever tokens are set, especially off localhost.

### Scopes

Each endpoint declares one or more required scopes. Tokens must include all required scopes to access
the route.

Available scopes:
- `status:read` — health, ready, and status endpoints.
- `decks:read` — deck status endpoints.
- `autodj:read` / `autodj:write` — AutoDJ status or control.
- `playlists:read` / `playlists:write` — playlist listing and mutations.
- `control:write` — `POST /api/v1/control`.

## TLS certificates

- Enable HTTPS to serve the API over TLS. If you opt into automatic generation and OpenSSL is available,
  Mixxx creates a self-signed certificate at `<settings>/rest/rest_certificate.pem` with a matching
  private key at `<settings>/rest/rest_private_key.pem`.
- The same files are reused on future launches. You can also point Mixxx at existing certificate and key paths
  instead of auto-generation.
- Startup logs report whether the certificate was generated or loaded, and log errors if TLS preparation fails.

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

Control flow for a control request (for example, `POST /api/v1/control`) looks like:

```
POST /api/v1/control
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

## Endpoints

All routes are served under the versioned base path `/api/v1` (for example, `/api/v1/health`).

### Schema

- `GET /schema` — static JSON description of the REST API, including `base_path`, links to core endpoints, required scopes,
  and supported actions for AutoDJ/playlists plus the control payload styles. Requires `status:read`.

### Health and status

- `GET /api/v1/health` — liveness, uptime (ISO-8601 UTC), uptime_unix (seconds since Unix epoch), timestamp (ISO-8601 UTC), timestamp_unix (seconds since Unix epoch), readiness issues, and system metrics (CPU usage as an integer percent string when available, RSS bytes). Requires `status:read`.
- `GET /api/v1/ready` — readiness summary with dependency issues. Requires `status:read`.
- `GET /api/v1/status` — application info, decks, mixer state, broadcast/recording status, AutoDJ overview, uptime (ISO-8601 UTC), uptime_unix (seconds since Unix epoch), timestamp (ISO-8601 UTC), timestamp_unix (seconds since Unix epoch),
  and system metrics (CPU usage as an integer percent string when available, RSS bytes). Requires `status:read`.
- `GET /api/v1/stream/status` — server-sent events stream that emits status deltas at the configured interval. Requires `status:read`.
  Each event uses `event: status` and a JSON payload in `data:` containing only changed top-level fields since the last
  update (removed fields are sent as `null`). The first event includes the full status payload.

### Deck status

- `GET /api/v1/decks` — status for all decks (same payload as the deck list in `/api/v1/status`). Requires `decks:read`.
- `GET /api/v1/decks/<n>` — status for a single deck (1-based index). Requires `decks:read`.

### Control

- `POST /api/v1/control` — control Mixxx via JSON body. Requires `control:write`. Supported styles include:
  - Commands: `{ "command": "play", "group": "[Channel1]" }`, `{ "command": "seek", "position": 0.5 }`.
  - Direct control values: `{ "group": "[Master]", "key": "gain", "value": 1.2 }`.
  - Multiple commands: `{ "commands": [ { "command": "play", "group": "[Channel1]" }, { "group": "[Master]", "key": "gain", "value": 1.2 } ] }`.
    Responses include a `results` array of per-command payloads with a `status` field. Mixed success/error
    responses return HTTP 207 (Multi-Status); all errors return the first error status.

### AutoDJ

- `GET /api/v1/autodj` — fetch AutoDJ status and a sample of queued tracks. Requires `autodj:read`.
- `POST /api/v1/autodj` — manage AutoDJ with an `action` field. Requires `autodj:write`:
  - `enable`, `disable`, `skip`, `fade`, `shuffle`, `add_random`, `clear`
  - `add` with `track_ids` and optional `position`
  - `move` with `from`/`to`

### Playlists

- `GET /api/v1/playlists` — list playlists with metadata and the active playlist id. Requires `playlists:read`.
- `GET /api/v1/playlists?id=<id>` — fetch tracks for a specific playlist. Requires `playlists:read`.
- `POST /api/v1/playlists` — manage playlists with an `action` field. Requires `playlists:write`:
  - `create` with `name`
  - `delete`
  - `rename`
  - `set_active`
  - `add` with `track_ids` and optional `position`
  - `remove` with `positions`
  - `reorder` with `from`/`to`
  - `send_to_autodj` with `top`, `bottom`, or `replace` (pushes an entire playlist into the AutoDJ queue)

TLS is strongly recommended whenever authentication or control endpoints are used, especially outside localhost.
