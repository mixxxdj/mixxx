#!/bin/bash

set -e

DMG_FILE="${1}"
[ -z "${DMG_FILE}" ] && echo "Pass DMG file name as first argument." >&2 && exit 1
[ -z "${APPLE_CODESIGN_IDENTITY}" ] && echo 'Please set the APPLE_CODESIGN_IDENTITY env var.' >&2 && exit 1
[ -z "${APPLE_ID_USERNAME}" ] && echo 'Please set the APPLE_ID_USERNAME env var.' >&2 && exit 1
[ -z "${APPLE_APP_SPECIFIC_PASSWORD}" ] && echo 'Please set the APPLE_APP_SPECIFIC_PASSWORD env var.' >&2 && exit 1
[ -z "${APPLE_TEAM_ID}" ] && echo 'Please set the APPLE_TEAM_ID env var.' >&2 && exit 1

tmp_dir="$(mktemp -dt mixxx_notarize)"
# We want $tmp_dir to expand now, therefore we disable the check
# shellcheck disable=SC2064
trap "rm -rf '$tmp_dir'" EXIT

echo "==> Signing $DMG_FILE"
codesign --verbose=4 --sign "${APPLE_CODESIGN_IDENTITY}" "${DMG_FILE}"


echo "==> Notarizing $DMG_FILE"

credentials=(
    --apple-id "${APPLE_ID_USERNAME}"
    --password "${APPLE_APP_SPECIFIC_PASSWORD}"
    --team-id "${APPLE_TEAM_ID}"
)
submit_out="$tmp_dir/submit_out.txt"

xcrun notarytool submit "${credentials[@]}" "${DMG_FILE}" 2>&1 | tee "$submit_out"
REQUEST_ID="$(grep -e " id: " "$submit_out" | grep -oE '([0-9a-f-]{36})'| head -n1)"
rm "$submit_out"
xcrun notarytool wait "$REQUEST_ID" "${credentials[@]}" --timeout 15m ||:
xcrun notarytool log "$REQUEST_ID" "${credentials[@]}" ||:
xcrun notarytool info "$REQUEST_ID" "${credentials[@]}"

echo "==> Stapling $DMG_FILE"
xcrun stapler staple -q "${DMG_FILE}"
