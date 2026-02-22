#!/bin/bash

set -e
cd "$(dirname "$0")/../.."

if ! command -v inkscape &> /dev/null; then
    echo "Please make sure to have 'inkscape' on your PATH!"
    exit 1
fi

input_svg="res/images/icons/scalable/apps/mixxx_ios.svg"
output_dir="packaging/ios/Assets.xcassets/AppIcon.appiconset"

echo "==> Generating icons from $input_svg..."

size=1024
icon_prefix="${size}x${size}"
inkscape -b FFFFFF -o "$output_dir/$icon_prefix.png" -w $size "$input_svg"
