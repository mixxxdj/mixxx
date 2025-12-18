#!/bin/bash
# This is a cross-distribution script that sets up a Flatpak build environment for Mixxx.
set -o pipefail

# Script filename
SCRIPT=$(basename "$0")

# Required runtime and SDK
REQUIRED_PACKAGES=("org.kde.Platform//6.10" "org.kde.Sdk//6.10")

# Empty selected subcommand
COMMAND=""

# Install option without a default
INSTALL_OPTION=""

# Print usage information
print_usage() {
    echo ""
    echo "Usage:"
    echo "  $SCRIPT  name"
    echo "  $SCRIPT  setup (--system | --user) [--builder]"
    echo "  $SCRIPT  -h | --help"
    echo ""
    echo "Commands:"
    echo "  name         Display the name of the required build environment."
    echo "  setup        Install the build environment."
    echo ""
    echo "Options:"
    echo "  -h | --help  Display this help message."
    echo "  --builder    Install optional org.flatpak.Builder package."
    echo "  --system     Install packages system-wide."
    echo "  --user       Install packages for the current user."
}

# No user input
if [[ $# -eq 0 ]]; then
    echo "Error: Please select a command." >&2
    print_usage >&2
    exit 1
fi

# Parse user input
while [[ $# -gt 0 ]]; do
    case "$1" in
        name | setup)
            if [[ -n $COMMAND ]]; then
                echo "Error: Please select only one command." >&2
                print_usage >&2
                exit 1
            else
                COMMAND="$1"
                shift
            fi
            ;;

        -h | --help)
            echo "Flatpak build environment script for Mixxx."
            print_usage
            exit 0
            ;;

        --builder)
            if [[ $COMMAND == "setup" ]]; then
                REQUIRED_PACKAGES+=("org.flatpak.Builder")
                shift
            else
                echo "Error: Option '$1' can only be used with the 'setup' command." >&2
                print_usage >&2
                exit 1
            fi
            ;;

        --system | --user)
            if [[ $COMMAND == "setup" ]]; then
                if [[ -n "$INSTALL_OPTION" ]]; then
                    echo "Error: Multiple '--system' or '--user' options selected." >&2
                    print_usage >&2
                    exit 1
                else
                    INSTALL_OPTION="$1"
                    shift
                fi
            else
                echo "Error: Option '$1' can only be used with the 'setup' command." >&2
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

# Print build environment name info
if [[ $COMMAND == "name" ]]; then
    echo "No build environment name required for Flatpak installations." >&2
    echo "This script installs the build dependencies via flatpak using the 'setup' command." >&2
    exit 0
fi

# Setup command selected without an install type
if [[ $COMMAND == "setup" && -z $INSTALL_OPTION ]]; then
    echo "Error: Command 'setup' requires '--system' or '--user' option." >&2
    print_usage >&2
    exit 1
fi

# Check if flatpak is available
if ! command -v flatpak > /dev/null 2>&1; then
    echo "Error: Program 'flatpak' is not available." >&2
    echo "Please use the system package manager to install it." >&2
    exit 1
fi

# Add Flathub as a repository if it doesn't exist
if ! flatpak remotes "$INSTALL_OPTION" --columns url 2>/dev/null | \
    grep -q "https://dl.flathub.org/repo/" > /dev/null 2>&1; then
    echo "Adding Flathub as a repository..."
    flatpak remote-add "$INSTALL_OPTION" --if-not-exists \
    flathub https://dl.flathub.org/repo/flathub.flatpakrepo
fi

# Install Flatpak runtime and SDK
echo "Installing required packages..."
flatpak install -y "$INSTALL_OPTION" "${REQUIRED_PACKAGES[@]}"
