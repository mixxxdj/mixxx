#!/bin/bash

set -e

DMG_FILE="${1}"
[ -z "${DMG_FILE}" ] && echo "Pass DMG file name as first argument." >&2 && exit 1
[ -z "${APPLE_CODESIGN_IDENTITY}" ] && echo 'Please set the APPLE_CODESIGN_IDENTITY env var.' >&2 && exit 1
[ -z "${APPLE_ID_USERNAME}" ] && echo 'Please set the APPLE_ID_USERNAME env var.' >&2 && exit 1
[ -z "${APPLE_APP_SPECIFIC_PASSWORD}" ] && echo 'Please set the APPLE_APP_SPECIFIC_PASSWORD env var.' >&2 && exit 1
[ -z "${APPLE_TEAM_ID}" ] && echo 'Please set the APPLE_TEAM_ID env var.' >&2 && exit 1

echo "Signing $DMG_FILE"
codesign --verbose=4 --options runtime \
    --sign "${APPLE_CODESIGN_IDENTITY}" "$(dirname "$0")/Mixxx.entitlements" "${DMG_FILE}"

CREDENTIALS=(
    --apple-id "${APPLE_ID_USERNAME}"
    --password "${APPLE_APP_SPECIFIC_PASSWORD}"
    --team-id "${APPLE_TEAM_ID}"
)

echo "Notarizing $DMG_FILE"
xcrun notarytool submit "${CREDENTIALS[@]}" --output-format plist --wait "${DMG_FILE}" \
    > notarize_status.plist

trap 'rm notarize_status.plist' EXIT
cat notarize_status.plist

ID="$(/usr/libexec/PlistBuddy -c 'Print id' notarize_status.plist)"
STATUS="$(/usr/libexec/PlistBuddy -c 'Print status' notarize_status.plist)"

print_notary_log() {
    xcrun notarytool log "${CREDENTIALS[@]}" "$ID"
}

case "${STATUS}" in
    Accepted)
        echo "Notarization succeeded"
        ;;
    Invalid|Rejected)
        echo "Notarization failed: ${STATUS}"
        print_notary_log
        exit 1
        ;;
    *)
        echo "Unknown notarization status: ${STATUS}"
        print_notary_log
        exit 1
        ;;
esac

echo "Stapling $DMG_FILE"
xcrun stapler staple -q "${DMG_FILE}"
