#!/bin/bash

DMG_FILE="${1}"
[ -z "${DMG_FILE}" ] && echo "Pass DMG file name as first argument." >&2 && exit 1
[ -z "${APPLE_CODESIGN_IDENTITY}" ] && echo 'Please set the APPLE_CODESIGN_IDENTITY env var.' >&2 && exit 1
[ -z "${APPLE_BUNDLE_ID}" ] && echo 'Please set the APPLE_BUNDLE_ID env var.' >&2 && exit 1
[ -z "${APPLE_ID_USERNAME}" ] && echo 'Please set the APPLE_ID_USERNAME env var.' >&2 && exit 1
[ -z "${APPLE_APP_SPECIFIC_PASSWORD}" ] && echo 'Please set the APPLE_APP_SPECIFIC_PASSWORD env var.' >&2 && exit 1
[ -z "${APPLE_TEAM_ID}" ] && echo 'Please set the APPLE_TEAM_ID env var.' >&2 && exit 1

echo "Signing $DMG_FILE"
codesign --verbose=4 --options runtime \
    --sign "${APPLE_CODESIGN_IDENTITY}" "$(dirname "$0")/Mixxx.entitlements" "${DMG_FILE}"

echo "Notarizing $DMG_FILE"
xcrun notarytool submit \
    --apple-id "${APPLE_ID_USERNAME}" \
    --password "${APPLE_APP_SPECIFIC_PASSWORD}" \
    --team-id "${APPLE_TEAM_ID}" \
    --output-format plist \
    --wait \
    "${DMG_FILE}" > notarize_status.plist

# shellcheck disable=SC2181
if [ "$?" != "0" ]; then
    echo "Notarization failed:"
    cat notarize_status.plist
    curl "$(/usr/libexec/PlistBuddy -c 'Print notarization-info:LogFileURL' notarize_status.plist)"
    exit 1
fi

NOTARIZATION_STATUS="$(/usr/libexec/PlistBuddy -c 'Print notarization-info:Status' notarize_status.plist)"
rm notarize_status.plist

case "${NOTARIZATION_STATUS}" in
    success)
        echo "Notarization succeeded"
        ;;
    invalid)
        echo "Notarization failed with status: ${NOTARIZATION_STATUS}"
        exit 1
        ;;
    *)
        echo "Unknown notarization status: ${NOTARIZATION_STATUS}"
        exit 1
        ;;
esac

echo "Stapling $DMG_FILE"
xcrun stapler staple -q "${DMG_FILE}"
