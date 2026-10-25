#!/bin/bash
# Deploy the locally-built signed APK to a connected phone over ADB.
# From JaviLabarumDJ/mixxx-android (tools/install_to_phone.sh), translated
# to English and pointed at this repo's defaults.
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$DIR"

APK="${1:-mixxx-android-release-signed.apk}"
if [ ! -f "$APK" ]; then
    echo "APK not found: $APK"
    echo "Usage: $0 [path-to-apk]"
    echo "Build it first, e.g.: ./tools/android_buildenv.sh ... && <gradle assemble>"
    exit 1
fi

echo "Waiting for device..."
adb wait-for-device
DEV=$(adb devices | grep -v 'List' | grep 'device' | awk '{print $1}' | head -n 1)
echo "Device detected: $DEV"

echo "Ensuring controller/recordings dir exists on device..."
adb shell mkdir -p /storage/emulated/0/Mixxx/controllers

echo "Installing $APK ..."
adb install -r -d "$APK"

echo "Copying APK to /sdcard/Download/mixxx-android.apk ..."
adb push "$APK" /sdcard/Download/mixxx-android.apk

echo "Launching Mixxx on device..."
adb shell am start -S -n org.mixxx/.MainActivity

echo "Done."
