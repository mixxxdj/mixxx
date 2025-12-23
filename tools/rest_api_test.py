#!/usr/bin/env python3
"""Exercise Mixxx's REST API endpoints.

Example usage:
  python3 tools/rest_api_test.py --help
  python3 tools/rest_api_test.py --list-checks
  python3 tools/rest_api_test.py --scheme http --host localhost --check health --check status --show-response
  python3 tools/rest_api_test.py --scheme https --host localhost --token <token> --insecure --check schema --show-response
  python3 tools/rest_api_test.py --check control --control command=play,group=[Channel1]
  python3 tools/rest_api_test.py --check control --control command=seek,deck=1,position=0.5
  python3 tools/rest_api_test.py --check control --control key=volume,group=[Master],value=0.75
  python3 tools/rest_api_test.py --check playlists_read
  python3 tools/rest_api_test.py --check playlists_read --playlist-read-id 12
  python3 tools/rest_api_test.py --list-playlists
  python3 tools/rest_api_test.py --check playlists \\
    --playlist-action send_to_autodj --playlist-id 12 --playlist-autodj-position top
  python3 tools/rest_api_test.py --check control --play-deck 1
  python3 tools/rest_api_test.py --check control --pause-deck 2
  python3 tools/rest_api_test.py --check autodj_write --autodj-enable
  python3 tools/rest_api_test.py --check control --check autodj_write \\
    --payload control=control.json --payload autodj_write='{"action": "enable"}'
"""

from __future__ import annotations

import argparse
import json
import ssl
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Callable, Iterable, List, Mapping, Optional, Sequence
from urllib.error import HTTPError, URLError
from urllib.parse import urlencode
from urllib.request import Request, urlopen

DEFAULT_HTTP_PORT = 8989
DEFAULT_HTTPS_PORT = 8990
JSON_HEADERS = {
    "Accept": "application/json",
    "Content-Type": "application/json",
}


@dataclass
class ApiConfig:
    scheme: str
    host: str
    port: int
    token: Optional[str]
    timeout: float
    insecure_tls: bool

    @property
    def base_url(self) -> str:
        return f"{self.scheme}://{self.host}:{self.port}"


@dataclass
class ApiResponse:
    payload: Any
    status: int
    content_type: str
    duration_ms: float


@dataclass
class CheckDefinition:
    name: str
    method: str
    path: str
    description: str
    required_scopes: Sequence[str]
    validator: Callable[[Any, str], None]


class ApiTestError(Exception):
    pass


def build_ssl_context(insecure: bool) -> Optional[ssl.SSLContext]:
    if insecure:
        context = ssl.create_default_context()
        context.check_hostname = False
        context.verify_mode = ssl.CERT_NONE
        return context
    return None


def parse_headers(header_args: Optional[Sequence[str]]) -> Mapping[str, str]:
    headers: dict[str, str] = {}
    if not header_args:
        return headers
    for raw in header_args:
        if ":" not in raw:
            raise ApiTestError(f"Invalid header format: {raw}")
        key, value = raw.split(":", 1)
        key = key.strip()
        value = value.strip()
        if not key:
            raise ApiTestError(f"Invalid header format: {raw}")
        headers[key] = value
    return headers


def parse_query(query_args: Optional[Sequence[str]]) -> Mapping[str, str]:
    params: dict[str, str] = {}
    if not query_args:
        return params
    for raw in query_args:
        if "=" not in raw:
            raise ApiTestError(f"Invalid query format: {raw}")
        key, value = raw.split("=", 1)
        key = key.strip()
        value = value.strip()
        if not key:
            raise ApiTestError(f"Invalid query format: {raw}")
        params[key] = value
    return params


def parse_json_payload(raw: Optional[str], file_path: Optional[str]) -> Optional[Any]:
    if raw and file_path:
        raise ApiTestError("Use either --data or --data-file, not both")
    if raw:
        try:
            return json.loads(raw)
        except json.JSONDecodeError as exc:
            raise ApiTestError(f"Invalid JSON in --data: {exc}") from exc
    if file_path:
        try:
            payload_text = Path(file_path).read_text(encoding="utf-8")
        except OSError as exc:
            raise ApiTestError(f"Unable to read data file: {exc}") from exc
        try:
            return json.loads(payload_text)
        except json.JSONDecodeError as exc:
            raise ApiTestError(f"Invalid JSON in {file_path}: {exc}") from exc
    return None


def parse_payload_overrides(payload_args: Optional[Sequence[str]]) -> Mapping[str, Any]:
    overrides: dict[str, Any] = {}
    if not payload_args:
        return overrides
    for raw in payload_args:
        if "=" not in raw:
            raise ApiTestError(f"Invalid payload override: {raw}")
        check_name, value = raw.split("=", 1)
        check_name = check_name.strip()
        value = value.strip()
        if not check_name or not value:
            raise ApiTestError(f"Invalid payload override: {raw}")
        if check_name not in CHECKS:
            raise ApiTestError(f"Unknown check for payload override: {check_name}")
        if value.endswith(".json") and Path(value).exists():
            overrides[check_name] = parse_json_payload(None, value)
        else:
            overrides[check_name] = parse_json_payload(value, None)
    return overrides


def parse_control_value(key: str, value: str) -> Any:
    if key == "deck":
        try:
            return int(value)
        except ValueError as exc:
            raise ApiTestError(f"Invalid deck value: {value}") from exc
    if key in {"position", "value"}:
        try:
            return float(value)
        except ValueError as exc:
            raise ApiTestError(f"Invalid numeric value for {key}: {value}") from exc
    return value


def parse_control_entry(raw: str) -> Mapping[str, Any]:
    entry: dict[str, Any] = {}
    for part in raw.split(","):
        if "=" not in part:
            raise ApiTestError(
                f"Invalid control entry '{raw}'. Expected comma-separated key=value pairs."
            )
        key, value = part.split("=", 1)
        key = key.strip()
        value = value.strip()
        if not key or value == "":
            raise ApiTestError(
                f"Invalid control entry '{raw}'. Expected comma-separated key=value pairs."
            )
        entry[key] = parse_control_value(key, value)
    return entry


def validate_control_entry(entry: Mapping[str, Any]) -> None:
    if not entry.get("group") and "deck" not in entry:
        raise ApiTestError("Control entry requires 'group' or 'deck'")
    if not entry.get("command") and not entry.get("key"):
        raise ApiTestError("Control entry requires 'command' or 'key'")
    command = entry.get("command")
    if command == "seek" and "position" not in entry:
        raise ApiTestError("Seek command requires 'position'")
    if command == "gain" and "value" not in entry:
        raise ApiTestError("Gain command requires 'value'")


def parse_control_payload(control_args: Optional[Sequence[str]]) -> Optional[Any]:
    if not control_args:
        return None
    entries = [parse_control_entry(raw) for raw in control_args]
    for entry in entries:
        validate_control_entry(entry)
    if len(entries) == 1:
        return entries[0]
    return {"commands": entries}


def control_entry_for_command(command: str, deck: int) -> Mapping[str, Any]:
    if deck <= 0:
        raise ApiTestError(f"Deck must be a positive integer for {command}")
    return {"command": command, "group": f"[Channel{deck}]"}


def parse_control_shortcuts(args: argparse.Namespace) -> Optional[Any]:
    entries: List[Mapping[str, Any]] = []
    if args.play_deck is not None:
        entries.append(control_entry_for_command("play", args.play_deck))
    if args.pause_deck is not None:
        entries.append(control_entry_for_command("pause", args.pause_deck))
    if not entries:
        return None
    if len(entries) == 1:
        return entries[0]
    return {"commands": entries}


def parse_csv_list(raw_values: Optional[Sequence[str]], description: str) -> List[str]:
    if not raw_values:
        return []
    values: List[str] = []
    for raw in raw_values:
        for part in raw.split(","):
            part = part.strip()
            if not part:
                raise ApiTestError(f"Invalid {description} value: {raw}")
            values.append(part)
    return values


def parse_int_list(raw_values: Optional[Sequence[str]], description: str) -> List[int]:
    values: List[int] = []
    for raw in parse_csv_list(raw_values, description):
        try:
            values.append(int(raw))
        except ValueError as exc:
            raise ApiTestError(f"Invalid {description} value: {raw}") from exc
    return values


def parse_playlist_payload(args: argparse.Namespace) -> Optional[Mapping[str, Any]]:
    if not args.playlist_action:
        return None
    action = args.playlist_action
    payload: dict[str, Any] = {"action": action}

    if action == "create":
        if not args.playlist_name:
            raise ApiTestError("Playlist create requires --playlist-name")
        payload["name"] = args.playlist_name
        return payload

    if action in {"delete", "set_active"}:
        if args.playlist_id is None:
            raise ApiTestError(f"Playlist {action} requires --playlist-id")
        payload["id"] = args.playlist_id
        return payload

    if action == "rename":
        if args.playlist_id is None or not args.playlist_name:
            raise ApiTestError("Playlist rename requires --playlist-id and --playlist-name")
        payload["id"] = args.playlist_id
        payload["name"] = args.playlist_name
        return payload

    if action == "add":
        if args.playlist_id is None:
            raise ApiTestError("Playlist add requires --playlist-id")
        track_ids = parse_csv_list(args.playlist_track_ids, "playlist track id")
        if not track_ids:
            raise ApiTestError("Playlist add requires --playlist-track-id(s)")
        payload["id"] = args.playlist_id
        payload["track_ids"] = track_ids
        if args.playlist_position is not None:
            payload["position"] = args.playlist_position
        return payload

    if action == "remove":
        if args.playlist_id is None:
            raise ApiTestError("Playlist remove requires --playlist-id")
        positions = parse_int_list(args.playlist_positions, "playlist position")
        if not positions:
            raise ApiTestError("Playlist remove requires --playlist-positions")
        payload["id"] = args.playlist_id
        payload["positions"] = positions
        return payload

    if action == "reorder":
        if args.playlist_id is None:
            raise ApiTestError("Playlist reorder requires --playlist-id")
        if args.playlist_from is None or args.playlist_to is None:
            raise ApiTestError("Playlist reorder requires --playlist-from and --playlist-to")
        payload["id"] = args.playlist_id
        payload["from"] = args.playlist_from
        payload["to"] = args.playlist_to
        return payload

    if action == "send_to_autodj":
        if args.playlist_id is None:
            raise ApiTestError("Playlist send_to_autodj requires --playlist-id")
        if not args.playlist_autodj_position:
            raise ApiTestError(
                "Playlist send_to_autodj requires --playlist-autodj-position"
            )
        payload["id"] = args.playlist_id
        payload["position"] = args.playlist_autodj_position
        return payload

    raise ApiTestError(f"Unsupported playlist action: {action}")


def parse_playlist_query(args: argparse.Namespace) -> Mapping[str, str]:
    if args.playlist_read_id is None:
        return {}
    return {"id": str(args.playlist_read_id)}


def request_json(
    config: ApiConfig,
    method: str,
    path: str,
    data: Optional[Any],
    headers: Mapping[str, str],
    query: Mapping[str, str],
) -> ApiResponse:
    query_string = f"?{urlencode(query)}" if query else ""
    url = f"{config.base_url}{path}{query_string}"
    request_headers = dict(JSON_HEADERS)
    request_headers.update(headers)
    if config.token:
        request_headers["Authorization"] = f"Bearer {config.token}"

    body: Optional[bytes] = None
    if data is not None:
        body = json.dumps(data).encode("utf-8")

    request = Request(url, headers=request_headers, method=method, data=body)
    try:
        start_time = time.monotonic()
        with urlopen(
            request,
            timeout=config.timeout,
            context=build_ssl_context(config.insecure_tls),
        ) as response:
            payload = response.read().decode("utf-8")
            status = response.status
            content_type = response.headers.get("Content-Type", "")
        duration_ms = (time.monotonic() - start_time) * 1000
    except HTTPError as exc:
        body = exc.read().decode("utf-8", errors="replace")
        raise ApiTestError(
            f"HTTP {exc.code} for {path}: {body or exc.reason}"
        ) from exc
    except URLError as exc:
        raise ApiTestError(f"Network error for {path}: {exc.reason}") from exc

    if not (200 <= status < 300):
        raise ApiTestError(f"Unexpected status {status} for {path}")
    if "application/json" not in content_type:
        raise ApiTestError(f"Unexpected content type for {path}: {content_type}")

    try:
        return ApiResponse(json.loads(payload), status, content_type, duration_ms)
    except json.JSONDecodeError as exc:
        raise ApiTestError(f"Invalid JSON response for {path}: {exc}") from exc


def assert_keys(payload: Any, keys: Iterable[str], path: str) -> None:
    if not isinstance(payload, dict):
        raise ApiTestError(f"Expected JSON object for {path}")
    missing = [key for key in keys if key not in payload]
    if missing:
        raise ApiTestError(f"Missing keys in {path}: {', '.join(missing)}")


def assert_type(payload: Any, expected: type, description: str) -> None:
    if not isinstance(payload, expected):
        raise ApiTestError(f"Expected {description} to be {expected.__name__}")


def assert_optional_number(payload: Any, description: str) -> None:
    if not isinstance(payload, (int, float)):
        raise ApiTestError(f"Expected {description} to be numeric")


def assert_optional_int_string(payload: Any, description: str) -> None:
    if payload is None:
        return
    if not isinstance(payload, str):
        raise ApiTestError(f"Expected {description} to be an integer string")
    if not payload.isdigit():
        raise ApiTestError(f"Expected {description} to be numeric digits only")


def format_response(response: Any) -> str:
    return json.dumps(response, indent=2, sort_keys=True)


def report_response(path: str, response: Any, show: bool) -> None:
    if not show:
        return
    print(f"Response for {path}:")
    print(format_response(response))


def write_response(output_dir: Optional[Path], name: str, response: Any) -> None:
    if not output_dir:
        return
    output_dir.mkdir(parents=True, exist_ok=True)
    output_path = output_dir / f"{name}.json"
    output_path.write_text(format_response(response) + "\n", encoding="utf-8")


def validate_schema(payload: Any, path: str) -> None:
    assert_keys(payload, ["base_path", "links"], path)
    assert_type(payload["base_path"], str, "schema.base_path")
    assert_type(payload["links"], dict, "schema.links")


def validate_system(payload: Any, path: str) -> None:
    assert_keys(payload, ["logical_cores", "cpu_usage_percent", "rss_bytes"], path)
    assert_type(payload["logical_cores"], int, f"{path}.logical_cores")
    assert_optional_int_string(payload["cpu_usage_percent"], f"{path}.cpu_usage_percent")
    assert_optional_number(payload["rss_bytes"], f"{path}.rss_bytes")


def validate_health(payload: Any, path: str) -> None:
    assert_keys(
        payload,
        ["status", "uptime", "uptime_unix", "timestamp", "timestamp_unix", "system"],
        path,
    )
    assert_type(payload["status"], str, "health.status")
    assert_type(payload["uptime"], str, "health.uptime")
    assert_optional_number(payload["uptime_unix"], "health.uptime_unix")
    assert_type(payload["timestamp"], str, "health.timestamp")
    assert_optional_number(payload["timestamp_unix"], "health.timestamp_unix")
    validate_system(payload["system"], "health.system")


def validate_ready(payload: Any, path: str) -> None:
    assert_keys(payload, ["ready", "issues"], path)
    assert_type(payload["ready"], bool, "ready.ready")
    assert_type(payload["issues"], list, "ready.issues")


def validate_status(payload: Any, path: str) -> None:
    assert_keys(
        payload,
        [
            "app",
            "decks",
            "mixer",
            "uptime",
            "uptime_unix",
            "timestamp",
            "timestamp_unix",
            "system",
        ],
        path,
    )
    assert_type(payload["app"], dict, "status.app")
    assert_type(payload["decks"], list, "status.decks")
    assert_type(payload["mixer"], dict, "status.mixer")
    assert_type(payload["uptime"], str, "status.uptime")
    assert_optional_number(payload["uptime_unix"], "status.uptime_unix")
    assert_type(payload["timestamp"], str, "status.timestamp")
    assert_optional_number(payload["timestamp_unix"], "status.timestamp_unix")
    validate_system(payload["system"], "status.system")


def validate_decks(payload: Any, path: str) -> None:
    assert_keys(payload, ["decks"], path)
    assert_type(payload["decks"], list, "decks.decks")
    for index, deck in enumerate(payload["decks"]):
        validate_deck_summary(deck, f"{path}.decks[{index}]")


def validate_track_summary(payload: Any, path: str) -> None:
    assert_type(payload, dict, path)
    if "id" in payload:
        assert_type(payload["id"], str, f"{path}.id")
    if "title" in payload:
        assert_type(payload["title"], str, f"{path}.title")
    if "artist" in payload:
        assert_type(payload["artist"], str, f"{path}.artist")
    if "album" in payload:
        assert_type(payload["album"], str, f"{path}.album")
    if "duration" in payload:
        assert_optional_number(payload["duration"], f"{path}.duration")
    if "bpm" in payload:
        assert_optional_number(payload["bpm"], f"{path}.bpm")


def validate_deck_summary(payload: Any, path: str) -> None:
    assert_keys(
        payload,
        [
            "deck",
            "group",
            "playing",
            "track_loaded",
            "position",
            "rate",
            "gain",
            "volume",
            "sync",
            "keylock",
            "loop_enabled",
        ],
        path,
    )
    assert_type(payload["deck"], int, f"{path}.deck")
    assert_type(payload["group"], str, f"{path}.group")
    assert_type(payload["playing"], bool, f"{path}.playing")
    assert_type(payload["track_loaded"], bool, f"{path}.track_loaded")
    assert_optional_number(payload["position"], f"{path}.position")
    assert_optional_number(payload["rate"], f"{path}.rate")
    assert_optional_number(payload["gain"], f"{path}.gain")
    assert_optional_number(payload["volume"], f"{path}.volume")
    assert_type(payload["sync"], bool, f"{path}.sync")
    assert_type(payload["keylock"], bool, f"{path}.keylock")
    assert_type(payload["loop_enabled"], bool, f"{path}.loop_enabled")

    has_track = "track" in payload
    has_duration = "duration" in payload
    if payload["track_loaded"]:
        if not has_track or not has_duration:
            raise ApiTestError(f"Expected {path} to include track details when loaded")
    if has_track:
        validate_track_summary(payload["track"], f"{path}.track")
    if has_duration:
        assert_optional_number(payload["duration"], f"{path}.duration")


def validate_autodj(payload: Any, path: str) -> None:
    assert_keys(payload, ["enabled", "queue"], path)
    assert_type(payload["enabled"], bool, "autodj.enabled")
    assert_type(payload["queue"], list, "autodj.queue")


def validate_playlists_read(payload: Any, path: str) -> None:
    assert_type(payload, dict, path)
    if "playlist_id" in payload:
        assert_keys(payload, ["playlist_id", "tracks"], path)
        assert_type(payload["playlist_id"], int, f"{path}.playlist_id")
        assert_type(payload["tracks"], list, f"{path}.tracks")
        for index, entry in enumerate(payload["tracks"]):
            assert_type(entry, dict, f"{path}.tracks[{index}]")
            if "id" in entry:
                assert_type(entry["id"], str, f"{path}.tracks[{index}].id")
            if "track" in entry:
                validate_track_summary(entry["track"], f"{path}.tracks[{index}].track")
            if "duration" in entry:
                assert_optional_number(entry["duration"], f"{path}.tracks[{index}].duration")
        return

    assert_keys(payload, ["active_playlist_id", "playlists"], path)
    assert_type(payload["active_playlist_id"], int, f"{path}.active_playlist_id")
    assert_type(payload["playlists"], list, f"{path}.playlists")
    for index, entry in enumerate(payload["playlists"]):
        assert_type(entry, dict, f"{path}.playlists[{index}]")
        if "id" in entry:
            assert_type(entry["id"], int, f"{path}.playlists[{index}].id")
        if "name" in entry:
            assert_type(entry["name"], str, f"{path}.playlists[{index}].name")
        if "locked" in entry:
            assert_type(entry["locked"], bool, f"{path}.playlists[{index}].locked")
        if "track_count" in entry:
            assert_type(entry["track_count"], int, f"{path}.playlists[{index}].track_count")


def validate_json_object(payload: Any, path: str) -> None:
    assert_type(payload, dict, path)


CHECKS = {
    "schema": CheckDefinition(
        name="schema",
        method="GET",
        path="/schema",
        description="Fetch API schema document.",
        required_scopes=["status:read"],
        validator=validate_schema,
    ),
    "health": CheckDefinition(
        name="health",
        method="GET",
        path="/api/v1/health",
        description="Validate liveness and uptime payload.",
        required_scopes=["status:read"],
        validator=validate_health,
    ),
    "ready": CheckDefinition(
        name="ready",
        method="GET",
        path="/api/v1/ready",
        description="Validate readiness issues payload.",
        required_scopes=["status:read"],
        validator=validate_ready,
    ),
    "status": CheckDefinition(
        name="status",
        method="GET",
        path="/api/v1/status",
        description="Validate status payload shape.",
        required_scopes=["status:read"],
        validator=validate_status,
    ),
    "decks": CheckDefinition(
        name="decks",
        method="GET",
        path="/api/v1/decks",
        description="Validate decks list payload.",
        required_scopes=["decks:read"],
        validator=validate_decks,
    ),
    "autodj": CheckDefinition(
        name="autodj",
        method="GET",
        path="/api/v1/autodj",
        description="Validate AutoDJ status payload.",
        required_scopes=["autodj:read"],
        validator=validate_autodj,
    ),
    "control": CheckDefinition(
        name="control",
        method="POST",
        path="/api/v1/control",
        description="Send a control action payload.",
        required_scopes=["control:write"],
        validator=validate_json_object,
    ),
    "autodj_write": CheckDefinition(
        name="autodj_write",
        method="POST",
        path="/api/v1/autodj",
        description="Send an AutoDJ action payload.",
        required_scopes=["autodj:write"],
        validator=validate_json_object,
    ),
    "playlists_read": CheckDefinition(
        name="playlists_read",
        method="GET",
        path="/api/v1/playlists",
        description="List playlists or fetch playlist tracks.",
        required_scopes=["playlists:read"],
        validator=validate_playlists_read,
    ),
    "playlists": CheckDefinition(
        name="playlists",
        method="POST",
        path="/api/v1/playlists",
        description="Send a playlists action payload.",
        required_scopes=["playlists:write"],
        validator=validate_json_object,
    ),
}


def list_checks() -> None:
    for check in CHECKS.values():
        scopes = ", ".join(check.required_scopes) or "none"
        print(f"{check.name}")
        print(f"  {check.method} {check.path}")
        print(f"  Scopes: {scopes}")
        print(f"  {check.description}\n")


def list_playlists(
    config: ApiConfig,
    headers: Mapping[str, str],
    query: Mapping[str, str],
    show_responses: bool,
    output_dir: Optional[Path],
    show_timing: bool,
) -> int:
    check = CHECKS["playlists_read"]
    response = request_json(
        config,
        check.method,
        check.path,
        None,
        headers,
        query,
    )
    report_response(check.path, response.payload, show_responses)
    write_response(output_dir, check.name, response.payload)
    validate_playlists_read(response.payload, check.path)

    playlists = response.payload.get("playlists", [])
    print("Playlists:")
    for entry in playlists:
        playlist_id = entry.get("id")
        name = entry.get("name", "")
        print(f"- {playlist_id}: {name}")

    timing = f" ({response.duration_ms:.1f}ms)" if show_timing else ""
    print(f"playlists_read OK{timing}")
    return 0


def run_checks(
    config: ApiConfig,
    checks: Iterable[str],
    show_responses: bool,
    output_dir: Optional[Path],
    data: Optional[Any],
    payload_overrides: Mapping[str, Any],
    headers: Mapping[str, str],
    query: Mapping[str, str],
    query_overrides: Mapping[str, Mapping[str, str]],
    show_timing: bool,
) -> List[str]:
    messages: List[str] = []

    for name in checks:
        check = CHECKS[name]
        payload_data = payload_overrides.get(name, data) if check.method == "POST" else None
        if check.method == "POST" and payload_data is None:
            raise ApiTestError(f"Check '{name}' requires --data or --data-file")
        effective_query = dict(query)
        if name in query_overrides:
            effective_query.update(query_overrides[name])
        response = request_json(
            config,
            check.method,
            check.path,
            payload_data,
            headers,
            effective_query,
        )
        report_response(check.path, response.payload, show_responses)
        write_response(output_dir, check.name, response.payload)
        check.validator(response.payload, check.path)
        timing = f" ({response.duration_ms:.1f}ms)" if show_timing else ""
        messages.append(f"{check.name} OK{timing}")

    return messages


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Exercise Mixxx REST API endpoints.")
    scheme_group = parser.add_mutually_exclusive_group()
    scheme_group.add_argument("--http", action="store_true", help="Use HTTP")
    scheme_group.add_argument("--https", action="store_true", help="Use HTTPS")
    parser.add_argument(
        "--scheme",
        choices=["http", "https"],
        help="Scheme to use (overrides --http/--https)",
    )
    parser.add_argument("--host", default="localhost", help="API host")
    parser.add_argument(
        "--port",
        type=int,
        help="API port (defaults to 8989 for HTTP, 8990 for HTTPS)",
    )
    parser.add_argument(
        "--token",
        help="Bearer token for Authorization header",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=10.0,
        help="Request timeout in seconds",
    )
    parser.add_argument(
        "--insecure",
        action="store_true",
        help="Disable TLS certificate verification for HTTPS",
    )
    parser.add_argument(
        "--check",
        action="append",
        choices=sorted(CHECKS.keys()),
        help=(
            "Endpoints to validate (repeatable). Defaults to all checks "
            "if omitted."
        ),
    )
    parser.add_argument(
        "--list-checks",
        action="store_true",
        help="List available checks and exit",
    )
    parser.add_argument(
        "--list-playlists",
        action="store_true",
        help="List available playlists and exit",
    )
    parser.add_argument(
        "--show-response",
        action="store_true",
        help="Print formatted JSON responses for each checked endpoint",
    )
    parser.add_argument(
        "--output-dir",
        help="Write formatted JSON responses to this directory",
    )
    parser.add_argument(
        "--data",
        help="JSON payload for POST checks",
    )
    parser.add_argument(
        "--data-file",
        help="Path to JSON file for POST checks",
    )
    parser.add_argument(
        "--payload",
        action="append",
        help=(
            "Override POST payload for a check as name=JSON or name=path.json "
            "(repeatable)"
        ),
    )
    parser.add_argument(
        "--control",
        action="append",
        help=(
            "Control command entry for /api/v1/control as comma-separated key=value "
            "pairs (repeatable). Example: --control command=play,group=[Channel1]"
        ),
    )
    parser.add_argument(
        "--play-deck",
        type=int,
        help="Send a play command to /api/v1/control for the given deck number",
    )
    parser.add_argument(
        "--pause-deck",
        type=int,
        help="Send a pause command to /api/v1/control for the given deck number",
    )
    parser.add_argument(
        "--autodj-enable",
        action="store_true",
        help="Send an AutoDJ enable action to /api/v1/autodj",
    )
    parser.add_argument(
        "--playlist-read-id",
        type=int,
        help="Playlist id to fetch tracks for with playlists_read",
    )
    parser.add_argument(
        "--playlist-action",
        choices=[
            "create",
            "delete",
            "rename",
            "set_active",
            "add",
            "remove",
            "reorder",
            "send_to_autodj",
        ],
        help="Playlist action for /api/v1/playlists",
    )
    parser.add_argument(
        "--playlist-id",
        type=int,
        help="Playlist id for playlist actions",
    )
    parser.add_argument(
        "--playlist-name",
        help="Playlist name for create/rename actions",
    )
    parser.add_argument(
        "--playlist-track-id",
        dest="playlist_track_ids",
        action="append",
        help="Track id to add to playlist (repeatable or comma-separated)",
    )
    parser.add_argument(
        "--playlist-position",
        type=int,
        help="Position for playlist add action",
    )
    parser.add_argument(
        "--playlist-positions",
        action="append",
        help="Comma-separated playlist positions for remove action",
    )
    parser.add_argument(
        "--playlist-from",
        type=int,
        help="Source position for playlist reorder action",
    )
    parser.add_argument(
        "--playlist-to",
        type=int,
        help="Destination position for playlist reorder action",
    )
    parser.add_argument(
        "--playlist-autodj-position",
        choices=["top", "bottom", "replace"],
        help="AutoDJ queue position for send_to_autodj action",
    )
    parser.add_argument(
        "--header",
        action="append",
        help="Additional HTTP header in the form Key:Value (repeatable)",
    )
    parser.add_argument(
        "--query",
        action="append",
        help="Query parameter in the form key=value (repeatable)",
    )
    parser.add_argument(
        "--timing",
        action="store_true",
        help="Include request timing in output",
    )
    return parser.parse_args()


def resolve_scheme(args: argparse.Namespace) -> str:
    if args.scheme:
        return args.scheme
    if args.https:
        return "https"
    return "http"


def main() -> int:
    args = parse_args()
    if args.list_checks:
        list_checks()
        return 0

    scheme = resolve_scheme(args)
    port = args.port or (DEFAULT_HTTPS_PORT if scheme == "https" else DEFAULT_HTTP_PORT)
    config = ApiConfig(
        scheme=scheme,
        host=args.host,
        port=port,
        token=args.token,
        timeout=args.timeout,
        insecure_tls=args.insecure,
    )
    checks = args.check or [
        "schema",
        "health",
        "ready",
        "status",
        "decks",
        "autodj",
    ]
    output_dir = Path(args.output_dir) if args.output_dir else None
    headers = parse_headers(args.header)
    query = parse_query(args.query)
    data = parse_json_payload(args.data, args.data_file)
    payload_overrides = parse_payload_overrides(args.payload)
    control_payload = parse_control_payload(args.control)
    shortcut_control_payload = parse_control_shortcuts(args)
    playlist_payload = parse_playlist_payload(args)
    playlist_query = parse_playlist_query(args)
    query_overrides: dict[str, Mapping[str, str]] = {}
    if playlist_query:
        if "id" in query:
            raise ApiTestError("Use either --playlist-read-id or --query id=...")
        query_overrides["playlists_read"] = playlist_query
    if control_payload is not None:
        if "control" in payload_overrides:
            raise ApiTestError("Use either --control or --payload control=..., not both")
        payload_overrides = dict(payload_overrides)
        payload_overrides["control"] = control_payload
    if shortcut_control_payload is not None:
        if "control" in payload_overrides:
            raise ApiTestError(
                "Use either --play-deck/--pause-deck or other control payload options"
            )
        payload_overrides = dict(payload_overrides)
        payload_overrides["control"] = shortcut_control_payload
    if args.autodj_enable:
        if "autodj_write" in payload_overrides:
            raise ApiTestError("Use either --autodj-enable or --payload autodj_write=...")
        payload_overrides = dict(payload_overrides)
        payload_overrides["autodj_write"] = {"action": "enable"}
    if playlist_payload is not None:
        if "playlists" in payload_overrides:
            raise ApiTestError("Use either --playlist-* options or --payload playlists=...")
        payload_overrides = dict(payload_overrides)
        payload_overrides["playlists"] = playlist_payload

    if args.list_playlists:
        try:
            return list_playlists(
                config,
                headers,
                query,
                args.show_response,
                output_dir,
                args.timing,
            )
        except ApiTestError as exc:
            print(f"FAIL: {exc}")
            return 1

    try:
        results = run_checks(
            config,
            checks,
            args.show_response,
            output_dir,
            data,
            payload_overrides,
            headers,
            query,
            query_overrides,
            args.timing,
        )
    except ApiTestError as exc:
        print(f"FAIL: {exc}")
        return 1

    print("PASS:")
    for message in results:
        print(f"- {message}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
