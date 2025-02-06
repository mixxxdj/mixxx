#!/bin/bash

set -e
cd "$(dirname "$0")/../.."

if ! command -v inkscape &> /dev/null; then
    echo "Please make sure to have 'inkscape' on your PATH!"
    exit 1
fi

input_svg="res/images/icons/scalable/apps/mixxx_macos.svg"
tmp_dir="$(mktemp -dt mixxx_icon)"
output_dir="$tmp_dir.iconset"
output_icns=(
    "res/osx/application.icns"
    "res/osx/VolumeIcon.icns"
)

mv "$tmp_dir" "$output_dir"

# We want $output_dir to expand now, therefore we disable the check
# shellcheck disable=SC2064
trap "rm -rf '$output_dir'" EXIT

echo "==> Generating icons from $input_svg..."

for size in 16 32 64 128 256 512 1024; do
    echo "Generating icon of size $size..."
    icon_prefix="icon_${size}x${size}"
    inkscape -o "$output_dir/$icon_prefix.png"    -w $size         "$input_svg"
    inkscape -o "$output_dir/$icon_prefix@2x.png" -w $((size * 2)) "$input_svg"
done

echo "==> Updating ICNS icons..."

for icns in "${output_icns[@]}"; do
    echo "Updating $icns..."
    iconutil -c icns -o "$icns" "$output_dir"
done
