# REST API developer notes

Mixxx ships with an optional REST API for lightweight control and monitoring.
Use this page to find the available routes, authentication expectations, TLS
support, and the default network configuration.

## Defaults

- The REST server is disabled by default. Enable it from Preferences ➜ REST
  API.
- When enabled, HTTP listens on `localhost:8989` unless you change the host or
  port. HTTPS uses `localhost:8990` when TLS is enabled.
- HTTP access is allowed unless you disable it. TLS is off by default and must
  be explicitly turned on. Control routes can be forced to require TLS.
- Authentication is optional. When a bearer token is configured, include it in
  requests as `Authorization: Bearer <token>`.

## TLS certificates

- Enable HTTPS to serve the API over TLS. If you opt into automatic generation
  and OpenSSL is available, Mixxx creates a self-signed certificate at
  `<settings>/rest/rest_certificate.pem` with a matching private key at
  `<settings>/rest/rest_private_key.pem`.
- The same files are reused on future launches. You can also point Mixxx at
  existing certificate and key paths instead of auto-generation.
- Startup logs report whether the certificate was generated or loaded, and
  log errors if TLS preparation fails.

## Endpoints

All routes are available with and without the `/api` prefix (for example,
`/health` and `/api/health`).

- `GET /health` — basic liveness check returning `{ "status": "ok" }`.
- `GET /status` — summary including application info, decks, mixer state,
  broadcast/recording status, and AutoDJ overview.
- `POST /control` — control Mixxx by JSON body. Supports commands like
  `{ "command": "play", "group": "[Channel1]" }`, `{ "command": "seek", "position": 0.5 }`,
  or direct control key/value pairs such as `{ "group": "[Master]", "key": "gain", "value": 1.2 }`.
- `GET /autodj` — fetch AutoDJ status and a sample of queued tracks.
- `POST /autodj` — manage AutoDJ with an `action` field. Supported actions
  include `enable`, `disable`, `skip`, `fade`, `shuffle`, `add_random`,
  `clear`, `add` (with `track_ids` and optional `position`), and `move`
  (with `from`/`to`).
- `GET /playlists` — list playlists with metadata and the active playlist id.
- `GET /playlists?id=<id>` — fetch tracks for a specific playlist.
- `POST /playlists` — manage playlists with an `action` field. Supported
  actions include `create` (with `name`), `delete`, `rename`, `set_active`,
  `add` (with `track_ids` and optional `position`), `remove` (with
  `positions`), and `reorder` (with `from`/`to`).

TLS is recommended whenever authentication or control endpoints are used,
especially outside of localhost.
