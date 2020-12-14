# Mixxx
[![GitHub latest tag](https://img.shields.io/github/tag/mixxxdj/mixxx.svg)](https://www.mixxx.org/download)
[![Packaging status](https://repology.org/badge/tiny-repos/mixxx.svg)](https://repology.org/metapackage/mixxx/versions)
[![Zulip chat](https://img.shields.io/badge/zulip-join_chat-brightgreen.svg)](https://mixxx.zulipchat.com)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=QSFMYWN2B3JD2&source=url)

[Mixxx] is Free DJ software that gives you everything you need to perform live
DJ mixes. Mixxx works on GNU/Linux, Windows, and macOS.

## Quick Start

To get started with Mixxx:

1. For live use, [download the latest stable version][download].
2. For experimentation and testing, [download a development release][builds].
3. To live on the bleeding edge, clone the repo: `git clone https://github.com/mixxxdj/mixxx.git`

## Roadmap

The Mixxx team is hard at work on Mixxx 2.3. The best place to keep track of
2.3 development is the [2.3.0 milestone page on Launchpad][launchpad 2.3.0].

A more general roadmap can be found on [the wiki][wiki roadmap].

## Bug tracker

The Mixxx team uses [Launchpad] to manage Mixxx development.

Have a bug or feature request? [File a bug on Launchpad][fileabug].

Want to get involved in Mixxx development? Assign yourself a bug from the [easy
bug list][easybugs] and get started!

## Compiling

* macOS [![Build Status](https://travis-ci.org/mixxxdj/mixxx.svg)](https://travis-ci.org/mixxxdj/mixxx)
* Ubuntu / Windows [![Build status](https://ci.appveyor.com/api/projects/status/j460rficblcaopwx?svg=true)](https://ci.appveyor.com/project/mixxxdj/mixxx)
* Jenkins [![Build status](https://img.shields.io/jenkins/s/https/builds.mixxx.org/job/master-release.svg)](https://builds.mixxx.org/job/master-release)

First, you must install all of Mixxx's dependencies. To compile Mixxx using
[CMake], run:

    $ mkdir build
    $ cd build
    $ cmake ..
    $ cmake --build .

Please see our helpful guides on the [wiki] for more information:
- [Compiling on Linux]
- [Compiling on macOS]
- [Compiling on Windows]

## Documentation

For help using Mixxx, there are a variety of options:

- [Mixxx manual][manual]
- [Mixxx wiki][wiki]
- [Frequently Asked Questions][FAQ]
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
- Follow us on [Twitter] and [Facebook].
- Subscribe to the [Mixxx Development Blog][blog].
- Join the developer [mailing list].
- Post on the [Mixxx forums][discourse].

## License

Mixxx is released under the GPLv2. See the LICENSE file for a full copy of the
license.

[mixxx]: https://www.mixxx.org
[download]: https://www.mixxx.org/download
[builds]: https://downloads.mixxx.org/builds/
[launchpad]: https://bugs.launchpad.net/mixxx
[fileabug]: https://bugs.launchpad.net/mixxx/+filebug
[twitter]: https://twitter.com/mixxxdj
[facebook]: https://www.facebook.com/pages/Mixxx-DJ-Software/21723485212
[blog]: https://mixxxblog.blogspot.com
[manual]: https://www.mixxx.org/manual/latest/
[wiki]: https://www.mixxx.org/wiki/
[faq]: https://mixxx.org/wiki/doku.php/faq
[forums]: https://www.mixxx.org/forums/
[compiling on linux]: https://mixxx.org/wiki/doku.php/compiling_on_linux
[compiling on macOS]: https://mixxx.org/wiki/doku.php/compiling_on_os_x
[compiling on windows]: https://mixxx.org/wiki/doku.php/compiling_on_windows
[mailing list]: https://lists.sourceforge.net/lists/listinfo/mixxx-devel
[CMake]: https://cmake.org/
[launchpad 2.3.0]: https://launchpad.net/mixxx/+milestone/2.3.0
[wiki roadmap]: https://mixxx.org/wiki/doku.php/development_roadmap
[easybugs]: https://bugs.launchpad.net/mixxx/+bugs?field.searchtext=&orderby=-importance&search=Search&field.status%3Alist=NEW&field.status%3Alist=CONFIRMED&field.status%3Alist=TRIAGED&field.status%3Alist=INPROGRESS&field.status%3Alist=INCOMPLETE_WITH_RESPONSE&field.status%3Alist=INCOMPLETE_WITHOUT_RESPONSE&assignee_option=any&field.assignee=&field.bug_reporter=&field.bug_commenter=&field.subscriber=&field.structural_subscriber=&field.tag=easy&field.tags_combinator=ANY&field.has_cve.used=&field.omit_dupes.used=&field.omit_dupes=on&field.affects_me.used=&field.has_patch.used=&field.has_branches.used=&field.has_branches=on&field.has_no_branches.used=&field.has_no_branches=on&field.has_blueprints.used=&field.has_blueprints=on&field.has_no_blueprints.used=&field.has_no_blueprints=on
[creating skins]: https://mixxx.org/wiki/doku.php/Creating-Skins
[help translate content]: https://www.transifex.com/projects/p/mixxxdj
[Mixxx i18n wiki]: https://mixxx.org/wiki/doku.php/internationalization
[Mixxx localization forum]: https://mixxx.org/forums/viewforum.php?f=10
[Mixxx glossary]: https://www.transifex.com/projects/p/mixxxdj/glossary/l/en/
[hardware compatibility]: https://mixxx.org/wiki/doku.php/Hardware-Compatibility
[zulip]: https://mixxx.zulipchat.com/
[discourse]: https://mixxx.discourse.group/
