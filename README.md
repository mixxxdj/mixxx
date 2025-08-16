# Mixxx

[![GitHub latest tag](https://img.shields.io/github/tag/mixxxdj/mixxx.svg)](https://mixxx.org/download)
[![Packaging status](https://repology.org/badge/tiny-repos/mixxx.svg)](https://repology.org/metapackage/mixxx/versions)
[![Build status](https://github.com/mixxxdj/mixxx/actions/workflows/build.yml/badge.svg)](https://github.com/mixxxdj/mixxx/actions/workflows/build.yml)
[![Coverage status](https://coveralls.io/repos/github/mixxxdj/mixxx/badge.svg)](https://coveralls.io/github/mixxxdj/mixxx)
[![Zulip chat](https://img.shields.io/badge/zulip-join_chat-brightgreen.svg)](https://mixxx.zulipchat.com)
[![Donate](https://img.shields.io/opencollective/all/mixxx?label=Donate)](https://mixxx.org/donate)

[Mixxx] is Free DJ software that gives you everything you need to perform live
DJ mixes. Mixxx works on GNU/Linux, Windows, and macOS.

## Quick Start

To get started with Mixxx:

1. For live use, [download the latest stable version][download-stable].
2. For experimentation and testing, [download a development release][download-testing].
3. To live on the bleeding edge, clone the repo: `git clone https://github.com/mixxxdj/mixxx.git`

## Bug tracker

The Mixxx team uses [Github Issues][issues] to manage Mixxx development.

Have a bug or feature request? [File a bug on Github][fileabug].

Want to get involved in Mixxx development? Assign yourself a bug from the [easy
bug list][easybugs] and get started!
Read [CONTRIBUTING](CONTRIBUTING.md) for more information.

## Building Mixxx

First, open a terminal (on Windows, use "**x64 Native Tools Command Prompt for
[VS 2022][visualstudio2022]**"), download the mixxx
source code and navigate to it:

    $ git clone https://github.com/mixxxdj/mixxx.git
    $ cd mixxx

Fetch the required dependencies and set up the build environment by running the
corresponding command for your operating system:

| OS | Command |
| -- | ------- |
| Windows | `tools\windows_buildenv.bat` |
| macOS | `source tools/macos_buildenv.sh setup` |
| Debian/Ubuntu | `tools/debian_buildenv.sh setup` |
| Fedora | `tools/rpm_buildenv.sh setup` |
| Other Linux distros | See the [wiki article](https://github.com/mixxxdj/mixxx/wiki/Compiling%20on%20Linux) |

To build Mixxx, run

    $ mkdir build
    $ cd build
    $ cmake ..
    $ cmake --build .

There should now be a `mixxx` executable in the current directory that you can
run. Alternatively, can generate a package using `cpack`.

Detailed build instructions for each target OS can be found [on the wiki](https://github.com/mixxxdj/mixxx/wiki#compile-mixxx-from-source-code)

## Documentation

For help using Mixxx, there are a variety of options:

- [Mixxx manual][manual]
- [Mixxx wiki][wiki]
- [Hardware Compatibility]
- [Creating Skins]

## Translation

Help to spread Mixxx with translations into more languages, as well as to update and ensure the accuracy of existing translations.

- [Help translate content]
- [Mixxx i18n wiki]
- [Mixxx localization forum]
- [Mixxx glossary]

## Community

Mixxx is a vibrant community of hackers, DJs and artists. To keep track of
development and community news:

- Chat with us on [Zulip][zulip].
- Follow us on [Mastodon], [Bluesky] and [Facebook].
- Subscribe to the [Mixxx Blog][blog].
- Post on the [Mixxx forums][discourse].

## License

Mixxx is released under the GPLv2. See the LICENSE file for a full copy of the
license.

[mixxx]: https://mixxx.org
[download-stable]: https://mixxx.org/download/#stable
[download-testing]: https://mixxx.org/download/#testing
[issues]: https://github.com/mixxxdj/mixxx/issues
[fileabug]: https://github.com/mixxxdj/mixxx/issues/new/choose
[mastodon]: https://floss.social/@mixxx
[Bluesky]: https://bsky.app/profile/mixxx.bsky.social
[facebook]: https://www.facebook.com/pages/Mixxx-DJ-Software/21723485212
[blog]: https://mixxx.org/news/
[manual]: https://manual.mixxx.org/
[wiki]: https://github.com/mixxxdj/mixxx/wiki
[visualstudio2022]: https://docs.microsoft.com/visualstudio/install/install-visual-studio?view=vs-2022
[easybugs]: https://github.com/mixxxdj/mixxx/issues?q=is%3Aopen+is%3Aissue+label%3Aeasy
[creating skins]: https://mixxx.org/wiki/doku.php/Creating-Skins
[help translate content]: https://www.transifex.com/projects/p/mixxxdj
[Mixxx i18n wiki]: https://github.com/mixxxdj/mixxx/wiki/Internationalization
[Mixxx localization forum]: https://mixxx.discourse.group/c/translation/13
[Mixxx glossary]: https://www.transifex.com/projects/p/mixxxdj/glossary/l/en/
[hardware compatibility]: https://manual.mixxx.org/2.3/en/hardware/manuals.html
[zulip]: https://mixxx.zulipchat.com/
[discourse]: https://mixxx.discourse.group/
