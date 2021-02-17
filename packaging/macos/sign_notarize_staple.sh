#!/bin/bash

DMG_FILE="${1}"
[ -z "${DMG_FILE}" ] && echo "Pass DMG file name as first argument." >&2 && exit 1
[ -z "${APPLE_CODESIGN_IDENTITY}" ] && echo "Please set the $APPLE_CODESIGN_IDENTITY env var." >&2 && exit 1
[ -z "${APPLE_BUNDLE_ID}" ] && echo "Please set the $APPLE_BUNDLE_ID env var." >&2 && exit 1
[ -z "${APPLE_ID_USERNAME}" ] && echo "Please set the $APPLE_ID_USERNAME env var." >&2 && exit 1
[ -z "${APPLE_APP_SPECIFIC_PASSWORD}" ] && echo "Please set the $APPLE_APP_SPECIFIC_PASSWORD env var." >&2 && exit 1
[ -z "${ASC_PROVIDER}" ] && echo "Please set the $ASC_PROVIDER env var." >&2 && exit 1

echo "Signing $DMG_FILE"
codesign --verbose=4 --options runtime \
    --sign "${APPLE_CODESIGN_IDENTITY}" "$(dirname "$0")/entitlements.plist" "${DMG_FILE}"

echo "Notarizing $DMG_FILE"
xcrun altool --notarize-app --primary-bundle-id "${APPLE_BUNDLE_ID}" --username "${APPLE_ID_USERNAME}" \
    --password "${APPLE_APP_SPECIFIC_PASSWORD}" --asc-provider "${ASC_PROVIDER}" --file "${DMG_FILE}" \
    --output-format xml > notarize_result.plist
UUID="$(/usr/libexec/PlistBuddy -c 'Print notarization-upload:RequestUUID' notarize_result.plist)"
echo "Notorization UUID: $UUID"
rm notarize_result.plist

# wait for confirmation that notarization finished
while true; do
    xcrun altool --notarization-info "$UUID" \
    --username "${APPLE_ID_USERNAME}" --password "${APPLE_APP_SPECIFIC_PASSWORD}" \
    --output-format xml > notarize_status.plist

    # shellcheck disable=SC2181
    if [ "$?" != "0" ]; then
        echo "Notarization failed:"
        cat notarize_status.plist
        curl "$(/usr/libexec/PlistBuddy -c 'Print notarization-info:LogFileURL' notarize_status.plist)"
        exit 1
    fi

    NOTARIZATION_STATUS="$(/usr/libexec/PlistBuddy -c 'Print notarization-info:Status' notarize_status.plist)"
    if [ "${NOTARIZATION_STATUS}" == "in progress" ]; then
        echo "Waiting another 10 seconds for notarization to complete"
        sleep 10
    elif [ "${NOTARIZATION_STATUS}" == "success" ]; then
        echo "Notarization succeeded"
        break
    else
        echo "Notarization status: ${NOTARIZATION_STATUS}"
    fi
done

rm notarize_status.plist

echo "Stapling $DMG_FILE"
xcrun stapler staple -q "${DMG_FILE}"
