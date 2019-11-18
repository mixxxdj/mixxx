# Changelog

## [2.3.0](https://launchpad.net/mixxx/+milestone/2.3.0) (Unreleased)

* Add support for multi-threaded track analysis. [lp:1641153](https://bugs.launchpad.net/mixxx/+bug/1641153)
* Add built-in support for MP3 encoding. LAME is now a required dependency. [lp:1294128](https://bugs.launchpad.net/mixxx/+bug/1294128)
* Add support for searching for empty fields. (e.g. crate:""). [lp:1788086](https://bugs.launchpad.net/mixxx/+bug/1788086)
* Unify skin controls for better consistency of settings across skins. [lp:1740513](https://bugs.launchpad.net/mixxx/+bug/1740513)
* Remove VAMP plugin support. vamp-plugin-sdk and vamp-hostsdk are no longer required dependencies.
* Remove SoundSource plugin support. [lp:1792747](https://bugs.launchpad.net/mixxx/+bug/1792747)
* Add Opus streaming and recording support. [lp:1338413](https://bugs.launchpad.net/mixxx/+bug/1338413)
* Add mapping for Roland DJ-505

## [2.2.2](https://launchpad.net/mixxx/+milestone/2.2.2) (2019-08-10)

* Fix battery widget with upower <= 0.99.7. #2221
* Fix BPM adjust in BpmControl. lp:1836480
* Disable track metadata export for .ogg files and TagLib 1.11.1. lp:1833190
* Fix interaction of hot cue buttons and looping. lp:1778246
* Fix detection of moved tracks. #2197
* Fix playlist import. lp:16878282
* Fix updating playlist labels. lp:1837315
* Fix potential segfault on exit. lp:1828360
* Fix parsing of invalid bpm values in MP3 files. lp:1832325
* Fix crash when removing rows from empty model. #2128
* Fix high DPI scaling of RGB overview waveforms. #2090
* Fix for OpenGL SL detection on macOS. lp:1828019
* Fix OpenGL ES detection. lp:1825461
* Fix FX1/2 buttons missing Mic unit in Deere (64 samplers). lp:1837716
* Tango64: Re-enable 64 samplers. #2223
* Numark DJ2Go re-enable note-off for deck A cue button. #2087
* Replace Flanger with QuickEffect in keyboard mapping. #2233

## [2.2.1](https://launchpad.net/mixxx/+milestone/2.2.1) (2019-04-22)

* Include all fixes from Mixxx 2.1.7 and 2.1.8
* Fix high CPU usage on MAC due to preview column lp:1812763
* Fix HID controller output on Windows with common-hid-packet-parser.js
* Fix rendering slow down by not using QStylePainter in WSpinny lp:1530720
* Fix broken Mic mute button lp:1782568
* added quick effect enable button to the control picker menu
* Fix Cover Window close issue with empty cover arts
* Fix Numark Mixtrack 3 mapping. #2057

## [2.2.0](https://launchpad.net/mixxx/+milestone/2.2.0) (2018-12-17)

General
* Update from Qt4 to Qt5.
* Use Qt5's automatic high DPI scaling (and remove the old
  scaling option from the preferences).
* Vectorize remaining raster graphics for better HiDPI support.

Effects
* Add mix mode switch (Dry/Wet vs Dry+Wet) for effect units.
* Add support for LV2 effects plugins (currently no way to show plugin GUIs).
* Add preference option for selecting which effects are shown in the
  list of available effects in the main window (all LV2 effects are
  hidden by default and must be explicitly enabled by users).

Skins
* Add 8 sampler and small sampler options to LateNight.
* Add key / BPM expansion indicators to Deere decks.
* Add skin settings menu to LateNight.

Controllers
* Add controller mapping for Numark Mixtrack Platinum.
* Update controller mapping for Numark N4.
* Add spinback and break for Vestax VCI-400 mapping.

Miscellaneous
* Add preference option to adjust the play position marker of
  scrolling waveforms.
* Add preference option to adjust opacity of beatgrid markers on
  scrolling waveforms.
* Support IRC/AIM/ICQ broadcast metadata.

## [2.1.8](https://launchpad.net/mixxx/+milestone/2.1.8) (2019-04-07)

* Fix a rare chance for a corrupt track file while writing metadata in out of disk situations. lp:1815305
* Fix export of BPM track file metadata. lp:1816490
* Fix export of BPM track file metadata. lp:1816490
* Fix sending of broadcast metadata with TLS enabled libshout 2.4.1. lp:1817395
* Fix resdicovering purged tracks in all cases. lp:1821514
* Fix dropping track from OSX Finder. lp:1822424

## [2.1.7](https://launchpad.net/mixxx/+milestone/2.1.7) (2019-01-15)

* Fix syncing to doublespeed [lp:1808697](https://bugs.launchpad.net/mixxx/+bug/1808697)
* Fix issues when changing beats of a synced track [lp:1808698](https://bugs.launchpad.net/mixxx/+bug/1808698)
* Fix direction of pitch bend buttons when inverting rate slider [lp:1770745](https://bugs.launchpad.net/mixxx/+bug/1770745)
* Use first loaded deck if no playing deck is found [lp:1784185](https://bugs.launchpad.net/mixxx/+bug/1784185)
* Encode file names correctly on macOS [lp:1776949](https://bugs.launchpad.net/mixxx/+bug/1776949)

## [2.1.6](https://launchpad.net/mixxx/+milestone/2.1.6) (2018-12-23)

* Fix crash when loading a Qt5 Soundsource / Vamp Plug-In. [lp:1774639](https://bugs.launchpad.net/mixxx/+bug/1774639)
* Validate effect parameter range. [lp:1795234](https://bugs.launchpad.net/mixxx/+bug/1795234)
* Fix crash using the bpm_tap button without a track loaded. [lp:1801844](https://bugs.launchpad.net/mixxx/+bug/1801844)
* Fix possible crash after ejecting a track. [lp:1801874](https://bugs.launchpad.net/mixxx/+bug/1801874)
* Fix wrong bitrate reported for faulty mp3 files. [lp:1782912](https://bugs.launchpad.net/mixxx/+bug/1782912)
* Fix Echo effect syncing [lp:1793232](https://bugs.launchpad.net/mixxx/+bug/1793232)
* Fix iTunes context menu [lp:1799932](https://bugs.launchpad.net/mixxx/+bug/1799932)
* Fix loading the wrong track after delete search and scroll. [lp:1803148](https://bugs.launchpad.net/mixxx/+bug/1803148)
* Improve search bar timing. [lp:1635087](https://bugs.launchpad.net/mixxx/+bug/1635087)
* Fix quoted search sentence. [lp:1784141](https://bugs.launchpad.net/mixxx/+bug/1784141)
* Fix loading a track formerly not existing. [lp:1800395](https://bugs.launchpad.net/mixxx/+bug/1800395)
* Fix importing m3u files with blank lines. [lp:1806271](https://bugs.launchpad.net/mixxx/+bug/1806271)
* Fix position in sampler overview waveforms. [lp:1744170](https://bugs.launchpad.net/mixxx/+bug/1744170)
* Don't reset rate slider, syncing a track without a beatgrid. [lp:1783020](https://bugs.launchpad.net/mixxx/+bug/1783020)
* Clean up iTunes track context menu. [lp:1800335](https://bugs.launchpad.net/mixxx/+bug/1800335)
* Collapsed sampler are not analyzed on startup. [lp:1801126](https://bugs.launchpad.net/mixxx/+bug/1801126)
* search for decoration characters like "˚". [lp:1802730](https://bugs.launchpad.net/mixxx/+bug/1802730)
* Fix cue button blinking after pressing eject on an empty deck. [lp:1808222](https://bugs.launchpad.net/mixxx/+bug/1808222)

## [2.1.5](https://launchpad.net/mixxx/+milestone/2.1.5) (2018-10-28)

* Code signing for Windows builds. [lp:1517823](https://bugs.launchpad.net/mixxx/+bug/1517823)
* Fix crash on exit when preferences is open. [lp:1793185](https://bugs.launchpad.net/mixxx/+bug/1793185)
* Fix crash when analyzing corrupt MP3s. [lp:1793387](https://bugs.launchpad.net/mixxx/+bug/1793387)
* Fix crash when importing metadata from MusicBrainz. [lp:1794993](https://bugs.launchpad.net/mixxx/+bug/1794993)
* Library search fixes when single quotes are used. [lp:1784090](https://bugs.launchpad.net/mixxx/+bug/1784090) [lp:1789728](https://bugs.launchpad.net/mixxx/+bug/1789728)
* Fix scrolling waveform on Windows with WDM-KS sound API. [lp:1729345](https://bugs.launchpad.net/mixxx/+bug/1729345)
* Fix right clicking on beatgrid alignment button in Tango and LateNight skins. [lp:1798237](https://bugs.launchpad.net/mixxx/+bug/1798237)
* Improve speed of importing iTunes library. [lp:1785545](https://bugs.launchpad.net/mixxx/+bug/1785545)
* Add 2 deck mapping for DJTechTools MIDI Fighter Twister.

## [2.1.4](https://launchpad.net/mixxx/+milestone/2.1.4) (2018-08-29)

Fix track selection not getting shown in the track
table on Windows. There are no changes to the
source code, but the Jenkins build configuration
was changed to delete the Jenkins workspace before
each build. [lp:1751482](https://bugs.launchpad.net/mixxx/+bug/1751482)

## [2.1.3](https://launchpad.net/mixxx/+milestone/2.1.3) (2018-08-20)

Fix a severe performance regression on Windows:
https://mixxx.org/forums/viewtopic.php?f=3&t=12082

## [2.1.2](https://launchpad.net/mixxx/+milestone/2.1.2) (2018-08-10)

Yet another bugfix release of Mixxx 2.1.
Here is a quick summary of what is new in Mixxx 2.1.2:

* Allow maximum deck speed of 4x normal.
* Don't always quantize hotcues, a 2.1.1 regression. [lp:1777429](https://bugs.launchpad.net/mixxx/+bug/1777429)
* Fix artifacts using more than 32 samplers. [lp:1779559](https://bugs.launchpad.net/mixxx/+bug/1779559)
* store No EQ and Filter persistently. [lp:1780479](https://bugs.launchpad.net/mixxx/+bug/1780479)
* Pad unreadable samples with silence on cache miss. [lp:1777480](https://bugs.launchpad.net/mixxx/+bug/1777480)
* Fixing painting of preview column for Qt5 builds. [lp:1776555](https://bugs.launchpad.net/mixxx/+bug/1776555)
* LateNight: Fix play button right click. [lp:1781829](https://bugs.launchpad.net/mixxx/+bug/1781829)
* LateNight: Added missing sort up/down buttons.
* Fix sampler play button tooltips. [lp:1779468](https://bugs.launchpad.net/mixxx/+bug/1779468)
* Shade: remove superfluid margins and padding in sampler.xml. [lp:1773588](https://bugs.launchpad.net/mixxx/+bug/1773588)
* Deere: Fix background-color code.
* ITunes: Don't stop import in case of duplicated Playlists. [lp:1783493](https://bugs.launchpad.net/mixxx/+bug/1783493)

## [2.1.1](https://launchpad.net/mixxx/+milestone/2.1.1) (2018-06-13)

After two months it is time to do a bugfix release of Mixxx 2.1.
Here is a quick summary of what is new in Mixxx 2.1.1:

* Require Soundtouch 2.0 to avoid segfault. [lp:1577042](https://bugs.launchpad.net/mixxx/+bug/1577042)
* Improved skins including library view fix. [lp:1773709](https://bugs.launchpad.net/mixxx/+bug/1773709) [lp:1772202](https://bugs.launchpad.net/mixxx/+bug/1772202) [lp:1763953](https://bugs.launchpad.net/mixxx/+bug/1763953)
* Fix crash when importing ID3v2 APIC frames. [lp:1774790](https://bugs.launchpad.net/mixxx/+bug/1774790)
* Synchronize execution of Vamp analyzers. [lp:1743256](https://bugs.launchpad.net/mixxx/+bug/1743256)
* DlgTrackInfo: Mismatching signal/slot connection.
* Detect M4A decoding errors on Windows. [lp:1766834](https://bugs.launchpad.net/mixxx/+bug/1766834)
* Fix spinback inertia effect.
* Fix decoding fixes and upgrade DB schema. [lp:1766042](https://bugs.launchpad.net/mixxx/+bug/1766042) [lp:1769717](https://bugs.launchpad.net/mixxx/+bug/1769717)
* Fix integration of external track libraries. [lp:1766360](https://bugs.launchpad.net/mixxx/+bug/1766360)
* Fix memory leak when loading cover art. [lp:1767068](https://bugs.launchpad.net/mixxx/+bug/1767068)
* Fix clearing of ReplayGain gain/ratio in file tags. [lp:1766094](https://bugs.launchpad.net/mixxx/+bug/1766094)
* Fix crash when removing a quick link. [lp:1510068](https://bugs.launchpad.net/mixxx/+bug/1510068)
* Fidlib: Thread-safe and reentrant generation of filters. [lp:1765210](https://bugs.launchpad.net/mixxx/+bug/1765210)
* Fix unresponsive scrolling through crates & playlists using encoder. [lp:1719474](https://bugs.launchpad.net/mixxx/+bug/1719474)
* Swap default values for temp/perm rate changes. [lp:1764254](https://bugs.launchpad.net/mixxx/+bug/1764254)

## [2.1.0](https://launchpad.net/mixxx/+milestone/2.1.0) (2018-04-15)

After two years of hard work, we are pleased to announce Mixxx 2.1. We
have overhauled the effects system, redesigned the skins, added and improved
lots of controller mappings, rewrote the audio file decoders twice, and of
course fixed a bunch of bugs. Download it!

Here is a quick summary of what is new in Mixxx 2.1.0:
  * Graphical interface scales for high resolution screens
  * Overhauled Deere and LateNight skins
  * New Tango skin
  * Effects are synchronized to the tempo
  * Effects are processed post-fader and post-crossfader and can be previewed
in headphones
  * One metaknob per effect with customizable parameter control for intuitive
use of effect chains
  * Nine new effects: Autopan, Biquad Equalizer, Biquad Full Kill Equalizer,
Loudness Contour, Metronome, Parametric Equalizer, Phaser, Stereo Balance,
Tremolo
  * Loaded effects and their parameters are saved and restored when Mixxx
restarts
  * More transparent sounding equalizers (Biquad Equalizer and Biquad Full Kill
Equalizer)
  * Improved scratching sounds with jog wheels, vinyl control, and dragging
waveforms with the mouse
  * Simplified looping and beatjump controls
  * Configurable rows of 8 samplers with up to 8 rows available for a total of
64 samplers
  * Files loaded to samplers are reloaded when Mixxx restarts
  * Improved volume normalization algorithm (EBU-R 128)
  * Filter library table by crates
  * Sort musical keys in library table by circle of fifths
  * Write metadata tags back to audio files
  * New JavaScript library for controller mapping
  * Configure multiple Internet broadcasting stations and use multiple stations
at the same time
  * Broadcast and record microphones with direct monitoring and latency
compensation
  * Broadcast and record from an external mixer
  * Booth output with independent gain knob for using sound cards with 6
output channels without an external mixer
  * Prevent screensaver from starting while Mixxx is running
  * CUP (Cue And Play) cue button mode
  * Time remaining and time elapsed now take into account the tempo fader
  * Clicking cover art now shows it full size in a separate window
  * and of course, lots and lots of bug fixes.

Here are controllers with mappings that have been added or updated since the 2.0
release. Mappings marked with an asterisk (*) have been updated for the new
effects interface:
  * American Audio VMS2
  * American Audio VMS4
  * Allen & Heath Xone K2/K1*
  * Behringer CMD Micro
  * Behringer CMD MM1*
  * Behringer CMD Studio 4a
  * Denon MC4000*
  * Denon MC6000 Mk2*
  * FaderFox DJ2
  * Hercules DJ Console 4-Mx*
  * Hercules DJ Control MP3 LE / Glow
  * Hercules DJ Control Compact
  * Hercules P32*
  * Ion Discover DJ
  * Korg Nanokontrol 2
  * Korg KAOSS DJ
  * M-Audio Xponent
  * Native Instruments Traktor Kontrol S4 Mk2*
  * Novation Launchpad Mk1 & Mk2
  * Novation Twitch
  * Numark Mixtrack Pro 3 & Numark Mixtrack 3*
  * Pioneer DDJ-SB2*
  * Pioneer DDJ-SX*
  * Reloop Beatmix 2
  * Reloop Beatmix 4
  * Reloop Digital Jockey 3 ME
  * Reloop Terminal Mix 2
  * Reloop Terminal Mix 4
  * Vestax VCI-100 Mk2
  * Vestax Typhoon

For users upgrading from older versions of Mixxx, we have a few important
announcements. First, if you are using Windows, you will have to uninstall any
old versions of Mixxx before you can install 2.1. How to uninstall Mixxx
varies on different versions of Windows:
  * Windows Vista, 7, and 8: Start > Control Panel > Programs > Uninstall a
Program [Guide](https://support.microsoft.com/en-us/help/2601726)
  * Windows 10: Start > Control Panel > Programs > Programs And Features > look
for Mixxx > Uninstall [Guide](https://support.microsoft.com/en-gb/help/4028054/windows-repair-or-remove-programs-in-windows-10)

If you are upgrading from an older version of Mixxx and have MP3 files in
your library, we have another important announcement. The good news is that we
fixed a bug where the waveforms and audio playback of MP3 files were
misaligned. The bad news is that we have no way of knowing which MP3 files were
affected or how much the offset was. That means that waveforms, beatgrids,
cues, and loops from older versions of Mixxx may be offset by an unknown amount
for any MP3 file. Only MP3 files were affected by this bug; other audio file
types are unaffected. You can either correct your beatgrids and cue points
manually for each track, or you can clear this information for all MP3s and
start fresh. Regardless, we recommend clearing the waveforms for all MP3
files. To clear these, type "location:mp3" into the library search bar, press
Control + A to select all tracks, right click, and select the information you
want to clear from the menu.

In the works for Mixxx 2.2, we have a big redesign of the library GUI. Along
with that will come saving & restoring search queries plus nested crates.
We are also planning on adding support for saving and loading custom effect
chain presets with the ability to import and export them to share online.

Want to help make Mixxx even more awesome? The biggest thing we need is more
people. You do not need to be a programmer to help out. Giving feedback on the
design of new features as they are being made is very valuable. Refer to the
Testing page on the wiki for more information on how to get involved with that.
Reporting bugs and telling us your ideas on the Launchpad bug tracker is a big
help too! We cannot fix problems we do not know about, so please let us know if
you find any issues with Mixxx. If you would like to help translate Mixxx into
another language, refer to the Internationalization wiki page.  Of course, more
programmers could always help. Read the Developer Documentation on the wiki for
tips on getting started contributing code to Mixxx.

We hope you have as much fun with Mixxx as we do!
