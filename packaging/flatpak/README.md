# Mixxx Flatpak instructions

This document provides instructions for building, packaging and installing Mixxx as a Flatpak application using the **tools/flatpak_buildenv.sh** and **packaging/flatpak/flatpak_build.sh** Bash scripts. The scripts provide several options for configuring the build, so please read the documentation before using them.

If you already have Flatpak set up, you can simply use the recommended build options. These commands will install a system-wide build environment, compile a Mixxx Flatpak bundle and install it for the current user. The process requires 15 GB of disk space and usually takes about 20 minutes.

`tools/flatpak_buildenv.sh setup --system`

`packaging/flatpak/flatpak_build.sh bundle`

`flatpak install --user Mixxx.Flatpak`

## Requirements

To build Mixxx as a Flatpak, you need to have **flatpak** and **flatpak-builder** installed on your system. Most Linux distributions publish these as official packages that can be installed using the system package manager. Installing **ccache** is also recommended as it makes rebuilds much faster.

If you're using an immutable distribution (e.g. Silverblue, Kinoite, Bazzite), you can install the packages within a Toolbx or a Distrobox container and build from there.

## Flatpak build environment

Building Mixxx requires two Flatpak packages: **org.kde.Platform** and **org.kde.Sdk**. These are commonly referred to as Flatpak runtime and Flatpak SDK.

Most Flatpak runtimes and SDKs are quite large, but they are shared between all applications. If you're using other Qt-based Flatpak applications, you may already have the required runtime installed. You can check what packages are currently installed with `flatpak list --all`.

## Build environment setup

`tools/flatpak_buildenv.sh setup (--system | --user) [--builder] [--debug]`

The build environment setup script performs the following actions:

- Configures Flathub as a Flatpak repository
- Installs org.kde.Platform package
- Installs org.kde.Sdk package
- Optionally installs org.flatpak.Builder package
- Optionally installs org.kde.Sdk.Debug extension

### Setup script options

The `--system` option configures Flathub as a system repository and installs packages system-wide, making them available to all users. This is the recommended option for build environment setup.

The `--user` option configures Flathub as a user repository and installs packages only for the current user. Using either this or the system-wide install option above is required.

The `--builder` option installs the optional **org.flatpak.Builder** package. This package is maintained by Flathub and provides a Flatpak version of the build tools. It's especially useful with immutable distributions as it can build Mixxx directly on the host system.

The `--debug` option installs the optional **org.kde.Sdk.Debug** extension. The extension contains debug symbols for the Flatpak SDK as well as various debugging tools. This option is meant for development purposes and is not required for building or running Mixxx.

## Building Mixxx

`packaging/flatpak/flatpak_build.sh (bundle | debug | install | repo) [--builder] [--manifest <file>]`

The build script uses subcommands for choosing between four Flatpak build variations. Note that the script places build directories and package files on the current working directory and it should be run from the root of the Mixxx source tree.

### Build script options

The `--builder` option uses the **org.flatpak.Builder** Flatpak package for building Mixxx. This option automatically grants the builder filesystem access for the current and the Flatpak manifest directory.

The `--manifest <file>` option accepts a custom Flatpak build manifest file as an argument. The default *packaging/flatpak/org.mixxx.Mixxx.yaml* manifest is used if this option is not passed.

## User bundle build

`packaging/flatpak/flatpak_build.sh bundle [--builder] [--manifest <file>]`

`flatpak install [--system | --user] Mixxx.Flatpak`

The **bundle** command builds Mixxx as a single-file Flatpak user bundle that can be easily installed to multiple systems. This build method is recommended for all Mixxx users.

## Debug extension build

`packaging/flatpak/flatpak_build.sh debug [--builder] [--manifest <file>]`

`flatpak install [--system | --user] Mixxx.flatpak`

`flatpak install [--system | --user] Mixxx.Debug.Flatpak`

The **debug** command builds the same single-file user bundle as above, but also creates an additional debug extension. This extension contains debug symbols and is intended for development purposes. It's not needed for running Mixxx.

## Direct Flatpak install

`packaging/flatpak/flatpak_build.sh install [--builder] [--manifest <file>]`

The **install** command builds and installs Mixxx directly as Flatpak application for the current user. Note that if you're building Mixxx using **org.flatpak.Builder**, a debug extension will also be installed.

## Local Flatpak repository build

`packaging/flatpak/flatpak_build.sh repo [--builder] [--manifest <file>]`

The **repo** command builds Mixxx and creates a Flatpak repository in *flatpak_repo* directory. This directory can be used as a local repository, providing easy package installs and updates. The same directory is also created by **bundle** and **debug** build options.

`flatpak --user remote-add --no-gpg-verify local-mixxx-repo ~/mixxxdj/mixxx/flatpak_repo`

The example above uses **local-mixxx-repo** as repository name and *~/mixxxdj/mixxx/flatpak_repo* for location. Installing Mixxx is done by specifying the repository name and the application ID **org.mixxx.Mixxx**. Updating after a rebuild requires only the application ID.

`flatpak install [--system | --user] local-mixxx-repo org.mixxx.Mixxx`

`flatpak update org.mixxx.Mixxx`
