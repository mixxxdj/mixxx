#!/bin/bash
# This is a cross-distribution script that builds Mixxx as a Flatpak.
set -o pipefail

# Script filename
SCRIPT=$(basename "$0")

# Required runtime and SDK
REQUIRED_PACKAGES=("org.kde.Platform//6.10" "org.kde.Sdk//6.10")

# Default manifest
MANIFEST="packaging/flatpak/org.mixxx.Mixxx.yaml"

# Build and repo directories
BUILD_DIR="build_flatpak"
REPO_DIR="flatpak_repo"

# Empty selected subcommand
COMMAND=""

# Native builder is the default
BUILDER="flatpak-builder"

# Default Flatpak filesystem permissions
FS_PERMISSIONS=("--filesystem=$(pwd)")

# Default build options
BUILD_OPTIONS=("--force-clean")

# Prints usage information
print_usage() {
    echo ""
    echo "Usage:"
    echo "  $SCRIPT  bundle [--builder] [--manifest <file>]"
    echo "  $SCRIPT  debug [--builder] [--manifest <file>]"
    echo "  $SCRIPT  install [--builder] [--manifest <file>]"
    echo "  $SCRIPT  repo [--builder] [--manifest <file>]"
    echo "  $SCRIPT  -h | --help"
    echo ""
    echo "Commands:"
    echo "  bundle             Standard single-file bundle build."
    echo "  debug              Debug single-file bundle build."
    echo "  install            Build and install for the current user."
    echo "  repo               Local repository build."
    echo ""
    echo "Options:"
    echo "  -h | --help        Display this help message."
    echo "  --builder          Build using org.flatpak.Builder."
    echo "  --manifest <file>  Use a custom manifest file."
}

# Checks that the manifest is readable
check_manifest() {
    if ! [[ -r $MANIFEST ]]; then
        echo "Error: Unable to read manifest file." >&2
        echo "Selected manifest: $MANIFEST" >&2
        echo "Run this script from Mixxx source root to use the default manifest." >&2
        echo "If you're using a custom manifest, make sure the file exists and is readable." >&2
        exit 1
    fi
}

# Checks that required commands are available
check_commands() {
    local missing_commands=()

    for command in "$@"; do
        if ! command -v "$command" > /dev/null 2>&1; then
            missing_commands+=("$command")
        fi
    done

    if [[ ${#missing_commands[@]} -gt 0 ]]; then
        echo "Error: Missing build dependencies." >&2
        echo "Commands not found:" "${missing_commands[@]}" >&2
        echo "Please install required programs with the system package manager." >&2
        exit 1
    fi
}

# Checks that Flathub is configured as a system or a user repository
check_flathub() {
    if ! flatpak remotes --system --columns url 2>/dev/null | grep -q "https://dl.flathub.org/repo/" && \
        ! flatpak remotes --user --columns url 2>/dev/null | grep -q "https://dl.flathub.org/repo/"; then
        echo "Error: Flathub is not configured as a Flatpak repository." >&2
        echo "Run tools/flatpak_build.sh to set up the build environment." >&2
        echo "" >&2
        echo "To configure Flathub manually, run:" >&2
        echo "    flatpak remote-add [--system | --user] --if-not-exists" \
            "flathub https://dl.flathub.org/repo/flathub.flatpakrepo" >&2
        exit 1
    fi
}

# Checks that required packages are installed
check_packages() {
    local missing_packages=()

    for package in "$@"; do
        if ! flatpak info "$package" > /dev/null 2>&1; then
            missing_packages+=("$package")
        fi
    done

    if [[ ${#missing_packages[@]} -gt 0 ]]; then
        echo "Error: Missing build dependencies." >&2
        echo "Packages not found:" "${missing_packages[@]}" >&2
        echo "Run tools/flatpak_build.sh to set up the build environment." >&2
        echo "" >&2
        echo "To install the packages manually, run:" >&2
        echo "    flatpak install [--system | --user]" "${missing_packages[@]}" >&2
        exit 1
    fi
}

# No user input
if [[ $# -eq 0 ]]; then
    echo "Error: Please select a command." >&2
    print_usage
    exit 1
fi

# Parse user input
while [[ $# -gt 0 ]]; do
    case "$1" in
        bundle | debug | install | repo)
            if [[ -n $COMMAND ]]; then
                echo "Error: Please select only one command." >&2
                print_usage >&2
                exit 1
            elif [[ $1 == "install" ]]; then
                BUILD_OPTIONS+=("--install" "--user")
            else
                BUILD_OPTIONS+=("--repo=$REPO_DIR")
            fi
            COMMAND="$1"
            shift
            ;;

        -h | --help)
            echo "Flatpak build script for Mixxx."
            print_usage
            exit 0
            ;;

        --builder)
            BUILDER="org.flatpak.Builder"
            shift
            ;;

        --manifest)
            shift
            if [ -n "$1" ]; then
                MANIFEST="$1"
                FS_PERMISSIONS+=("--filesystem=$(dirname "$(realpath "$MANIFEST")")")
                shift
            else
                echo "Error: Manifest file argument missing." >&2
                print_usage >&2
                exit 1
            fi
            ;;

        -*)
            echo "Error: '$1' is not a valid option." >&2
            print_usage >&2
            exit 1
            ;;

        *)
            echo "Error: '$1' is not a valid command." >&2
            print_usage >&2
            exit 1
            ;;
    esac
done

# No command selected
if [[ -z $COMMAND ]]; then
    echo "Error: Please select a command." >&2
    print_usage >&2
    exit 1
fi

# Run pre-build checks
check_manifest

check_commands "flatpak"

check_flathub

check_packages "${REQUIRED_PACKAGES[@]}"

if [[ $BUILDER == "org.flatpak.Builder" ]]; then
    check_packages "org.flatpak.Builder"
else
    check_commands "flatpak-builder"
fi

# Use ccache if available
if command -v ccache > /dev/null 2>&1; then
    BUILD_OPTIONS+=("--ccache")
fi

# Disable rofiles-fuse if we're in a container or using org.flatpak.Builder
if [[ -n $container || $BUILDER == "org.flatpak.Builder" ]]; then
    BUILD_OPTIONS+=("--disable-rofiles-fuse")
fi

# Run the build and exit if it fails
if [[ $BUILDER == "org.flatpak.Builder" ]]; then
    if ! flatpak run "${FS_PERMISSIONS[@]}" org.flatpak.Builder \
    "${BUILD_OPTIONS[@]}" "$BUILD_DIR" "$MANIFEST"; then
        exit 1
    fi
else
    if ! flatpak-builder "${BUILD_OPTIONS[@]}" "$BUILD_DIR" "$MANIFEST"; then
        exit 1
    fi
fi

# Finish repository build
case "$COMMAND" in
    bundle | debug | repo)
        if [[ -d $REPO_DIR ]]; then
            echo ""
            echo "Flatpak repository files are located at:"
            echo "    $(pwd)/$REPO_DIR"
        fi
        ;;
esac

# Create single-file bundle and finish standard build
case "$COMMAND" in
    bundle | debug)
        echo ""
        echo "Creating single-file Flatpak bundle..."
        flatpak build-bundle "$REPO_DIR" Mixxx.flatpak org.mixxx.Mixxx
        if [[ -f "Mixxx.flatpak" ]]; then
            echo ""
            echo "To install the single-file bundle, run:"
            echo "    flatpak install [--system | --user] Mixxx.flatpak"
        fi
        ;;
esac

# Create debug extension and finish debug build
if [[ $COMMAND == "debug" ]]; then
    echo ""
    echo "Creating Flatpak debug extension..."
    flatpak build-bundle "$REPO_DIR" --runtime Mixxx.Debug.flatpak org.mixxx.Mixxx.Debug
    if [[ -f "Mixxx.Debug.flatpak" ]]; then
        echo ""
        echo "To install the debug extension, run:"
        echo "    flatpak install [--system | --user] Mixxx.Debug.flatpak"
    fi
fi

# Finish direct install build
if [[ $COMMAND == "install" ]]; then
    echo ""
    echo "Direct install build finished."
    if flatpak info "org.mixxx.Mixxx" > /dev/null 2>&1; then
        echo ""
        echo "To start Mixxx, run:"
        echo "    flatpak run org.mixxx.Mixxx"
    fi
fi
