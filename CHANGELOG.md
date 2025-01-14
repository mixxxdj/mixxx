# Changelog

## [2.6.0](https://github.com/mixxxdj/mixxx/milestone/44) (Unreleased)

### Engine

* fix: sync rate using the current BPM instead of the file one
  [#13671](https://github.com/mixxxdj/mixxx/pull/13671)
  [#12738](https://github.com/mixxxdj/mixxx/issues/12738)
* fix: prevent null CO access when cloning sampler or preview [#13740](https://github.com/mixxxdj/mixxx/pull/13740)
* Tooltips: fix cue mode setting location [#14045](https://github.com/mixxxdj/mixxx/pull/14045)

### Preferences

* (fix) Sound preferences: don't set m_settingsModified in update slots [#13450](https://github.com/mixxxdj/mixxx/pull/13450)
* Track Search Preferences: Fix accidental use of wrong preference controls [#13592](https://github.com/mixxxdj/mixxx/pull/13592)
* (fix) Pref Mixer: fix crossader graph [#13848](https://github.com/mixxxdj/mixxx/pull/13848)
* Make extended controller information available for device selection [#13896](https://github.com/mixxxdj/mixxx/pull/13896)

### Skins

* LegacySkinParser: Short-circuit if template fails to open [#13488](https://github.com/mixxxdj/mixxx/pull/13488)
* Update waveforms_container.xml [#13501](https://github.com/mixxxdj/mixxx/pull/13501)

### Library

* feat: static color coding for key column [#13390](https://github.com/mixxxdj/mixxx/pull/13390)
* fix: Key text is elided from left, should be right [#13475](https://github.com/mixxxdj/mixxx/pull/13475)
* Add Key Color Palettes [#13497](https://github.com/mixxxdj/mixxx/pull/13497)
* Fix BPM and Bitrate columns were wider than normal [#13571](https://github.com/mixxxdj/mixxx/pull/13571)
* Track Info dialogs: move metadata buttons below color picker [#13632](https://github.com/mixxxdj/mixxx/pull/13632)
* CmdlineArgs: Add `--rescan-library` for rescanning on startup [#13661](https://github.com/mixxxdj/mixxx/pull/13661)
* Track menu, purge: allow to hide further success popups in the current session [#13807](https://github.com/mixxxdj/mixxx/pull/13807)

### Effects

* Compressor effect: Adjust Makeup Time constant calculation [#13261](https://github.com/mixxxdj/mixxx/pull/13261)
[#13237](https://github.com/mixxxdj/mixxx/issues/13237)
* fix: prevent quickFX model out of bound [#13668](https://github.com/mixxxdj/mixxx/pull/13668)

### Waveforms

* Simplify waveform combobox in preferences
  [#13220](https://github.com/mixxxdj/mixxx/issues/13220)
  [#6428](https://github.com/mixxxdj/mixxx/issues/6428)
  [#13226](https://github.com/mixxxdj/mixxx/issues/13226)
* Disable textured waveforms when using OpenGL ES
  [#13381](https://github.com/mixxxdj/mixxx/pull/13381)
  [#13380](https://github.com/mixxxdj/mixxx/issues/13380)
* ControllerRenderingEngine: Patch out unavailable APIs when using GL ES [#13382](https://github.com/mixxxdj/mixxx/pull/13382)
* fix: invalid slip render marker [#13422](https://github.com/mixxxdj/mixxx/pull/13422)
* Add minute markers on horizontal waveform overview
  [#13401](https://github.com/mixxxdj/mixxx/pull/13401)
  [#5843](https://github.com/mixxxdj/mixxx/issues/5843)
  [#13648](https://github.com/mixxxdj/mixxx/pull/13648)
* Fix high details waveforms wrapping around after visual index 65K [#13491](https://github.com/mixxxdj/mixxx/pull/13491)
* Fix: support for new WaveformData struct in shaders
  [#13474](https://github.com/mixxxdj/mixxx/pull/13474)
  [#13472](https://github.com/mixxxdj/mixxx/issues/13472)
* Overview waveform: draw minute markers on top of played overlay [#13489](https://github.com/mixxxdj/mixxx/pull/13489)
* fix: remove scaleSignal in waveform analyzer [#13416](https://github.com/mixxxdj/mixxx/pull/13416)
* feat: improve screen rendering framework [#13737](https://github.com/mixxxdj/mixxx/pull/13737)
* fix: prevent double free on DigitsRenderer [#13859](https://github.com/mixxxdj/mixxx/pull/13859)
* fix: waveform overview seeking
  [#13947](https://github.com/mixxxdj/mixxx/pull/13947)
  [#13946](https://github.com/mixxxdj/mixxx/issues/13946)
* rendergraph: add rendergraph library [#14007](https://github.com/mixxxdj/mixxx/pull/14007)
* mark rendering improvements [#13969](https://github.com/mixxxdj/mixxx/pull/13969)

### STEM support

* Add simple support for STEM files [#13044](https://github.com/mixxxdj/mixxx/pull/13044)
* Multithreaded Rubberband
  [#13143](https://github.com/mixxxdj/mixxx/pull/13143)
  [#13649](https://github.com/mixxxdj/mixxx/pull/13649)
* Add support for stem in the engine [#13070](https://github.com/mixxxdj/mixxx/pull/13070)
* Add analyser support for stem [#13106](https://github.com/mixxxdj/mixxx/pull/13106)
* Add stem controls [#13086](https://github.com/mixxxdj/mixxx/pull/13086)
* Add quick effect support on stem [#13123](https://github.com/mixxxdj/mixxx/pull/13123)
* Add stem files to the taglib lookup table [#13612](https://github.com/mixxxdj/mixxx/pull/13612)
* fix: exclude stem samples for QML waveform [#13655](https://github.com/mixxxdj/mixxx/pull/13655)
* Non-floating stem controls for LateNight [#13537](https://github.com/mixxxdj/mixxx/pull/13537)
* (fix) make "stem_group,mute" a powerwindow button
  [#13751](https://github.com/mixxxdj/mixxx/pull/13751)
  [#13749](https://github.com/mixxxdj/mixxx/issues/13749)
* feat: add advanced stem loading COs [#13268](https://github.com/mixxxdj/mixxx/pull/13268)
* Fix build with -DSTEM=OFF [#13948](https://github.com/mixxxdj/mixxx/pull/13948)
* Stem control test fix [#13960](https://github.com/mixxxdj/mixxx/pull/13960)
* Solves problem with special characters in path to stems [#13784](https://github.com/mixxxdj/mixxx/pull/13784)

### Controller Backend

* Add screen renderer to support controllers with a screen
  [#11407](https://github.com/mixxxdj/mixxx/pull/11407)
  [#13334](https://github.com/mixxxdj/mixxx/pull/13334)
* Deprecate `lodash.mixxx.js`, and `script.deepMerge` [#13460](https://github.com/mixxxdj/mixxx/pull/13460)
* Don't return in JogWheelBasic on deck absent in option [#13425](https://github.com/mixxxdj/mixxx/pull/13425)
* Refactor: modernize softtakeover code [#13553](https://github.com/mixxxdj/mixxx/pull/13553)
* document `ScriptConnection` readonly properties & slight cleanup [#13630](https://github.com/mixxxdj/mixxx/pull/13630)
* Modernize Hid/Bulk Lists [#13622](https://github.com/mixxxdj/mixxx/pull/13622)
* Prevent deadlock with BULK transfer and reduce log noise [#13735](https://github.com/mixxxdj/mixxx/pull/13735)
* feat: add file and color controller setting types [#13669](https://github.com/mixxxdj/mixxx/pull/13669)
* Controllers: allow to enable MIDI Through Port in non-developer sessions [#13909](https://github.com/mixxxdj/mixxx/pull/13909)
* Expose convertCharset convenience function to controllers [#13935](https://github.com/mixxxdj/mixxx/pull/13935)

### Auto-DJ

* Add AutoDJ xfader recenter option (default off)
  [#13303](https://github.com/mixxxdj/mixxx/pull/13303)
  [#11571](https://github.com/mixxxdj/mixxx/issues/11571)
* Auto DJ: Add context menu action for enabling/disabling the Auto DJ [#13593](https://github.com/mixxxdj/mixxx/pull/13593)
* Auto DJ Cross fader center [#13628](https://github.com/mixxxdj/mixxx/pull/13628)

### Experimental Features

* SoundManagerIOS: Remove unsupported/redundant options [#13487](https://github.com/mixxxdj/mixxx/pull/13487)
* ControllerRenderingEngine: Disable BGRA when targeting Wasm [#13502](https://github.com/mixxxdj/mixxx/pull/13502)
* BaseTrackTableModel: Disable inline track editing on iOS [#13494](https://github.com/mixxxdj/mixxx/pull/13494)
* set QQuickStyle to "basic" [#13696](https://github.com/mixxxdj/mixxx/pull/13696)
  [#13600](https://github.com/mixxxdj/mixxx/issues/13600)
* fix: trigger QML waveform slot at init [#13736](https://github.com/mixxxdj/mixxx/pull/13736)

### Target support

* DlgPrefSound: Add missing ifdefs for building without Rubberband [#13577](https://github.com/mixxxdj/mixxx/pull/13577)
* Update Linux-GitHub runner to Ubuntu 24.04.01 LTS
  [#13781](https://github.com/mixxxdj/mixxx/pull/13781)
  [#13880](https://github.com/mixxxdj/mixxx/pull/13880)
* Add missing qt6-declarative-private-dev and qt6-base-private-dev package [#13904](https://github.com/mixxxdj/mixxx/pull/13904)

### Misc Refactorings

* Refactor/shrink modernize scopedtimer [#13258](https://github.com/mixxxdj/mixxx/pull/13258)
* Improve use of parented_ptr [#13411](https://github.com/mixxxdj/mixxx/pull/13411)
* Pre-allocate memory in basetrackcache to avoid multiple reallocations [#13368](https://github.com/mixxxdj/mixxx/pull/13368)
* Bump actions/checkout from 4.1.6 to 4.1.7 [#13386](https://github.com/mixxxdj/mixxx/pull/13386)
* Bump actions/checkout from 4.1.7 to 4.2.0 [#13713](https://github.com/mixxxdj/mixxx/pull/13713)
* Bump actions/checkout from 4.2.0 to 4.2.1 [#13726](https://github.com/mixxxdj/mixxx/pull/13726)
* Bump actions/checkout from 4.2.1 to 4.2.2 [#13810](https://github.com/mixxxdj/mixxx/pull/13810)
* Bump azure/trusted-signing-action from 0.3.20 to 0.4.0 [#13500](https://github.com/mixxxdj/mixxx/pull/13500)
* Bump azure/trusted-signing-action from 0.4.0 to 0.5.0 [#13809](https://github.com/mixxxdj/mixxx/pull/13809)
* Bump actions/upload-artifact from 4.3.4 to 4.3.5 [#13539](https://github.com/mixxxdj/mixxx/pull/13539)
* Bump actions/upload-artifact from 4.3.5 to 4.3.6 [#13562](https://github.com/mixxxdj/mixxx/pull/13562)
* Bump actions/upload-artifact from 4.3.6 to 4.4.0 [#13621](https://github.com/mixxxdj/mixxx/pull/13621)
* Bump actions/upload-artifact from 4.4.0 to 4.4.1 [#13725](https://github.com/mixxxdj/mixxx/pull/13725)
* Bump actions/upload-artifact from 4.4.1 to 4.4.3 [#13765](https://github.com/mixxxdj/mixxx/pull/13765)
* Bump coverallsapp/github-action from 2.3.0 to 2.3.1 [#13766](https://github.com/mixxxdj/mixxx/pull/13766)
* Bump coverallsapp/github-action from 2.3.1 to 2.3.3 [#13793](https://github.com/mixxxdj/mixxx/pull/13793)
* Bump coverallsapp/github-action from 2.3.3 to 2.3.4 [#13811](https://github.com/mixxxdj/mixxx/pull/13811)
* chore: update the donate button label [#13353](https://github.com/mixxxdj/mixxx/pull/13353)
* WPixmapStore: Change getPixmapNoCache to std::unique_ptr and further optimizations [#13369](https://github.com/mixxxdj/mixxx/pull/13369)
* Removed unused setSVG and hash functionality from pixmapsource [#13423](https://github.com/mixxxdj/mixxx/pull/13423)
* remove FAQ from Readme.md [#13453](https://github.com/mixxxdj/mixxx/pull/13453)
* [#13452](https://github.com/mixxxdj/mixxx/pull/13452)
* Paintable cleanup [#13435](https://github.com/mixxxdj/mixxx/pull/13435)
* Made Paintable::DrawMode an enum class [#13424](https://github.com/mixxxdj/mixxx/pull/13424)
* hash clean up [#13458](https://github.com/mixxxdj/mixxx/pull/13458)
* clang-format: Indent Objective-C blocks with 4 spaces [#13503](https://github.com/mixxxdj/mixxx/pull/13503)
* fix(basetracktablemodel): Fix `-Wimplicit-fallthrough` warning on GCC 14.1.1 [#13505](https://github.com/mixxxdj/mixxx/pull/13505)
* Refactor fix trivial cpp coreguideline violations [#13552](https://github.com/mixxxdj/mixxx/pull/13552)
* Refactor `EngineMixer`  [#13568](https://github.com/mixxxdj/mixxx/pull/13568)
* more `ControlDoublePrivate` optimization [#13581](https://github.com/mixxxdj/mixxx/pull/13581)
* Modernize `ControlValueAtomic`  [#13574](https://github.com/mixxxdj/mixxx/pull/13574)
* Optimize control code [#13354](https://github.com/mixxxdj/mixxx/pull/13354)
* Fix some minor code issue [#13586](https://github.com/mixxxdj/mixxx/pull/13586)
* Static initialization order fix [#13594](https://github.com/mixxxdj/mixxx/pull/13594)
* Remove referenceholder [#13240](https://github.com/mixxxdj/mixxx/pull/13240)
* chore: add note about ConfigKey naming convention [#13658](https://github.com/mixxxdj/mixxx/pull/13658)
* refactor: split out `AutoFileReloader` from `QmlAutoReload`
  [#13607](https://github.com/mixxxdj/mixxx/pull/13607)
  [#13756](https://github.com/mixxxdj/mixxx/pull/13756)
  [#13755](https://github.com/mixxxdj/mixxx/issues/13755)
* Fix Clazy v1.12 errors in main [#13770](https://github.com/mixxxdj/mixxx/pull/13770)
* Code cleanup in SidebarModel and WLibrarySidebar [#13816](https://github.com/mixxxdj/mixxx/pull/13816)
* Refactor: `MovingInterquartileMean` [#13730](https://github.com/mixxxdj/mixxx/pull/13730)
* Improved comments in enginecontrol and use of std::size_t for bufferSize across the codebase [#13819](https://github.com/mixxxdj/mixxx/pull/13819)
* refactor: use higher-level `std::span` based logic [#13654](https://github.com/mixxxdj/mixxx/pull/13654)
* tsan fix pll vars data race [#13873](https://github.com/mixxxdj/mixxx/pull/13873)
* use atomic to fix tsan detected data race condition of blink value in control indicator [#13875](https://github.com/mixxxdj/mixxx/pull/13875)
* Fix undefined behaviour of infinity() [#13884](https://github.com/mixxxdj/mixxx/pull/13884)
* use atomic for m_bWakeScheduler, protect m_bQuit with mutex [#13898](https://github.com/mixxxdj/mixxx/pull/13898)
* Refactor `ValueTransformer` and `WBaseWidget` [#13853](https://github.com/mixxxdj/mixxx/pull/13853)
* avoid data race on m_pStream [#13899](https://github.com/mixxxdj/mixxx/pull/13899)
* Cleanup and deprecate more `util/` classes
  [#13687](https://github.com/mixxxdj/mixxx/pull/13687)
  [#13968](https://github.com/mixxxdj/mixxx/pull/13968)
  [#13965](https://github.com/mixxxdj/mixxx/issues/13965)
  [#14107](https://github.com/mixxxdj/mixxx/pull/14107)
  [#14095](https://github.com/mixxxdj/mixxx/issues/14095)
  [#14087](https://github.com/mixxxdj/mixxx/pull/14087)
  [#14086](https://github.com/mixxxdj/mixxx/issues/14086)
* ci(pre-commit): Add cmake-lint hook [#13932](https://github.com/mixxxdj/mixxx/pull/13932)
* refactor: remove samplew_autogen.h
  [#13988](https://github.com/mixxxdj/mixxx/pull/13988)
  [#14005](https://github.com/mixxxdj/mixxx/pull/14005)
* fix clang-tidy complain [#14029](https://github.com/mixxxdj/mixxx/pull/14029)
* ci(dependabot): Open PRs against 2.5 branch instead of main [#14060](https://github.com/mixxxdj/mixxx/pull/14060)
* Happy New Year 2025! [#14098](https://github.com/mixxxdj/mixxx/pull/14098)

## [2.5.1](https://github.com/mixxxdj/mixxx/milestone/45) (unreleased)

### Controller Mappings

* Numark NS6II: Add new controller mapping [#11075](https://github.com/mixxxdj/mixxx/pull/11075)
* Hercules Inpulse 300: Updated mapping [#14051](https://github.com/mixxxdj/mixxx/pull/14051)

### Fixes

* Deere (64 samplers): Bring back library in regular view
  [#14101](https://github.com/mixxxdj/mixxx/pull/14101)
  [#14097](https://github.com/mixxxdj/mixxx/issues/14097)
* Enable R3 time-stretching with Rubberband 4.0.0 API version numbers [#14100](https://github.com/mixxxdj/mixxx/pull/14100)

## [2.5.0](https://github.com/mixxxdj/mixxx/issues?q=milestone%3A2.5.0) (2024-12-24)

### Modernized Platform: Update to Qt6

* Mixxx is now using Qt6, offering improved performance and enhanced compatibility with modern systems.
  [#11863](https://github.com/mixxxdj/mixxx/pull/11863)
  [#11892](https://github.com/mixxxdj/mixxx/pull/11892)
* Build system defaults to Qt6. Qt5 build support will be dropped with Mixxx 2.6
  [#11934](https://github.com/mixxxdj/mixxx/pull/11934)
* Drop support for macOS versions earlier than 11
* Drop support for Windows versions earlier than Windows 10 build 1809
* Drop support for Ubuntu versions earlier than 22.04
* Require a C++20 compiler
* Support GCC 14
  [#13504](https://github.com/mixxxdj/mixxx/pull/13504)
  [#13467](https://github.com/mixxxdj/mixxx/issues/13467)
* DlgAbout: Add Qt version to the dialog [#11862](https://github.com/mixxxdj/mixxx/pull/11862)
* WWidget: Disable touch events on macOS (fixing trackpad issues on Qt 6) [#11870](https://github.com/mixxxdj/mixxx/pull/11870)
* Various Skin adjustments
  [#11970](https://github.com/mixxxdj/mixxx/pull/11970)
  [#11957](https://github.com/mixxxdj/mixxx/issues/11957)
  [#12050](https://github.com/mixxxdj/mixxx/pull/12050)
  [#12939](https://github.com/mixxxdj/mixxx/pull/12939)
  [#13242](https://github.com/mixxxdj/mixxx/pull/13242)
  [#14014](https://github.com/mixxxdj/mixxx/pull/14014)
  [#13535](https://github.com/mixxxdj/mixxx/pull/13535)
  [#14013](https://github.com/mixxxdj/mixxx/pull/14013)
  [#13959](https://github.com/mixxxdj/mixxx/issues/13959)
  [#14034](https://github.com/mixxxdj/mixxx/pull/14034)
  [#12972](https://github.com/mixxxdj/mixxx/issues/12972)
  [#14035](https://github.com/mixxxdj/mixxx/pull/14035)
* Various Library adjustments
  [#12380](https://github.com/mixxxdj/mixxx/pull/12380)
  [#12478](https://github.com/mixxxdj/mixxx/pull/12478)
  [#13035](https://github.com/mixxxdj/mixxx/pull/13035)
  [#13033](https://github.com/mixxxdj/mixxx/issues/13033)
  [#12488](https://github.com/mixxxdj/mixxx/pull/12488)
  [#12216](https://github.com/mixxxdj/mixxx/pull/12216)
  [#13448](https://github.com/mixxxdj/mixxx/pull/13448)

### Engine

* Beats: allow undoing the last BPM/beats change [#12954](https://github.com/mixxxdj/mixxx/pull/12954)
  [#12774](https://github.com/mixxxdj/mixxx/issues/12774)
  [#10138](https://github.com/mixxxdj/mixxx/issues/10138)
  [#13339](https://github.com/mixxxdj/mixxx/pull/13339)
* Add beatloop anchor to set and adjust loop from either start or end
  [#12745](https://github.com/mixxxdj/mixxx/pull/12745)
  [#13241](https://github.com/mixxxdj/mixxx/pull/13241)
* Add Rate Tap button [#12104](https://github.com/mixxxdj/mixxx/pull/12104)
* Store/restore regular loop when toggling rolling loops
  [#12475](https://github.com/mixxxdj/mixxx/pull/12475)
  [#8947](https://github.com/mixxxdj/mixxx/issues/8947)
* Add `beats_translate_move` ControlEncoder [#12376](https://github.com/mixxxdj/mixxx/pull/12376)
* Looping/Beatjump: use seconds if track has no beats
  [#12961](https://github.com/mixxxdj/mixxx/pull/12961)
  [#11124](https://github.com/mixxxdj/mixxx/issues/11124)
* Add Track colour palette cycling controls `track_color_next` and `track_color_prev` to library, decks and samplers
  [#13066](https://github.com/mixxxdj/mixxx/pull/13066)
  [#12905](https://github.com/mixxxdj/mixxx/issues/12905)
* Add Tempo locking controls
  [#13041](https://github.com/mixxxdj/mixxx/pull/13041)
  [#13041](https://github.com/mixxxdj/mixxx/pull/13041)
  [#13038](https://github.com/mixxxdj/mixxx/issues/13038)
  [#13199](https://github.com/mixxxdj/mixxx/pull/13199)
* Recording: Fix bogus timestamp in CUE sheet after restarting a recording
  [#13966](https://github.com/mixxxdj/mixxx/pull/13966)
  [#13964](https://github.com/mixxxdj/mixxx/issues/13964)
* Improve Taglib/SoundSource logging [#13541](https://github.com/mixxxdj/mixxx/pull/13541)

### Skins / Interface

* Toggle the menubar with single Alt key press (auto hide)
  [#11526](https://github.com/mixxxdj/mixxx/pull/11526)
  [#13301](https://github.com/mixxxdj/mixxx/pull/13301)
* Fullscreen toggle rework
  [#11566](https://github.com/mixxxdj/mixxx/pull/11566)
  [#13189](https://github.com/mixxxdj/mixxx/pull/13189)
  [#13030](https://github.com/mixxxdj/mixxx/issues/13030)
* Allow to edit track title and artist directly within the decks via a delayed double-click
  [#11755](https://github.com/mixxxdj/mixxx/pull/11755)
  [#13930](https://github.com/mixxxdj/mixxx/pull/13930)
* Require a minimum movement before initiating the drag&drop of tracks [#12903](https://github.com/mixxxdj/mixxx/pull/12903)
* Add type toggle to cue popup [#13215](https://github.com/mixxxdj/mixxx/pull/13215)
* Handle not supported files when dragging to waveforms and spinnies
  [#13206](https://github.com/mixxxdj/mixxx/issues/13206)
* Tooltips: Improve `rate_up/down` description regarding pitch vs. speed [#12590](https://github.com/mixxxdj/mixxx/pull/12590)
* Tooltips: Add description for expand/collapse samplers buttons
  [#13005](https://github.com/mixxxdj/mixxx/pull/13005)
  [#12998](https://github.com/mixxxdj/mixxx/issues/12998)
* Track label widgets: Set `show_track_menu` only for main decks [#12978](https://github.com/mixxxdj/mixxx/pull/12978)
* MacOS: App proxy icon of the playing track to the window title [#12116](https://github.com/mixxxdj/mixxx/pull/12116)
* Auto DJ: Force-show decks 3/4 if we are going to use them [#13455](https://github.com/mixxxdj/mixxx/pull/13455)
* Auto DJ: Add new random tracks if one track does not exists [#13551](https://github.com/mixxxdj/mixxx/pull/13551)
* Allow to set LaunchImage style per color scheme [#13731](https://github.com/mixxxdj/mixxx/pull/13731)
* Show wait cursor when re/loading a skin (not during startup) [#13747](https://github.com/mixxxdj/mixxx/pull/13747)
* LateNight: Merge vinyl control toggle and status light
  [#12947](https://github.com/mixxxdj/mixxx/pull/12947)
  [#10192](https://github.com/mixxxdj/mixxx/issues/10192)
* LateNight, Deere, Tango: Deactivate beatgrid edit controls if BPM is locked
  [#13320](https://github.com/mixxxdj/mixxx/pull/13320)
  [#13323](https://github.com/mixxxdj/mixxx/pull/13323)
  [#13325](https://github.com/mixxxdj/mixxx/pull/13325)
* LateNight: Add/tweak CueDelete icons
  [#13495](https://github.com/mixxxdj/mixxx/pull/13495)
  [#13492](https://github.com/mixxxdj/mixxx/issues/13492)
* LateNight: Use Classic launch image style also for 64 samplers version [#13796](https://github.com/mixxxdj/mixxx/pull/13796)
* Adjust some skin controls, to allow point-and-click mapping
  [#13906](https://github.com/mixxxdj/mixxx/pull/13906)
* PreviewDeckN,LoadSelectedTrackAndPlay toggles play/pause if the track is already loaded
  [#12920](https://github.com/mixxxdj/mixxx/pull/12920)
  [#9819](https://github.com/mixxxdj/mixxx/issues/9819)
* Command line interface: Determine whether to color output based on `TERM` variable
  [#13486](https://github.com/mixxxdj/mixxx/pull/13486)
* Command line interface: Add option `--start-autodj` to start Auto DJ immediately after Mixxx start.
  [#13017](https://github.com/mixxxdj/mixxx/pull/13017)
  [#10189](https://github.com/mixxxdj/mixxx/issues/10189)
* Logging: Include timestamps in messages by default [#11861](https://github.com/mixxxdj/mixxx/pull/11861)
* Logging: Limit mixxx.log size to 100 MB or via --log-max-file-size
  [#13684](https://github.com/mixxxdj/mixxx/pull/13684)
  [#13660](https://github.com/mixxxdj/mixxx/issues/13660)
* Fix skin reload after changing color scheme [#13847](https://github.com/mixxxdj/mixxx/pull/13847)

### Effects

* Add Compressor effect [#12523](https://github.com/mixxxdj/mixxx/pull/12523)
* add Glitch effect [#11329](https://github.com/mixxxdj/mixxx/pull/11329)
* Add backend for Audio Unit (AU) plugins on macOS
  [#12112](https://github.com/mixxxdj/mixxx/pull/12112)
  [#13938](https://github.com/mixxxdj/mixxx/pull/13938)
  [#13887](https://github.com/mixxxdj/mixxx/pull/13887)
* Effect Meta knob: Draw arc from default meta position
  [#12638](https://github.com/mixxxdj/mixxx/pull/12638)
  [#12634](https://github.com/mixxxdj/mixxx/issues/12634)
* Show newly added effects, read/write HiddenEffects
  [#13326](https://github.com/mixxxdj/mixxx/pull/13326)
  [#11343](https://github.com/mixxxdj/mixxx/issues/11343)

### Library

* Shortkeys Cut, Copy, Paste for track list management
  [#12020](https://github.com/mixxxdj/mixxx/pull/12020)
  [#13361](https://github.com/mixxxdj/mixxx/issues/13361)
  [#13364](https://github.com/mixxxdj/mixxx/pull/13364)
  [#13958](https://github.com/mixxxdj/mixxx/pull/13958)
  [#13100](https://github.com/mixxxdj/mixxx/issues/13100)
* Playlists: move tracks with Alt + Up/Down/PageUp/PageDown/Home/End
  [#13092](https://github.com/mixxxdj/mixxx/pull/13092)
  [#10826](https://github.com/mixxxdj/mixxx/issues/10826)
  [#13098](https://github.com/mixxxdj/mixxx/pull/13098)
* Search: Add special BPM filters
  [#12072](https://github.com/mixxxdj/mixxx/pull/12072)
  [#8191](https://github.com/mixxxdj/mixxx/issues/8191)
* Search: Add "OR" search operator
  [#12061](https://github.com/mixxxdj/mixxx/pull/12061)
  [#8881](https://github.com/mixxxdj/mixxx/issues/8881)
* Search: Add 'type' filter
  [#13338](https://github.com/mixxxdj/mixxx/issues/13338)
* Search: Add 'id' filter [#13694](https://github.com/mixxxdj/mixxx/pull/13694)
* Search related Tracks menu: Allow to use multiple filters at once
  [#12213](https://github.com/mixxxdj/mixxx/pull/12213)
  [#12211](https://github.com/mixxxdj/mixxx/issues/12211)
* Track menu: Rephrase "Reset" to "Clear" [#12955](https://github.com/mixxxdj/mixxx/pull/12955)
* Track menu: Add support for scaling BPM by different ratios
  [#12934](https://github.com/mixxxdj/mixxx/pull/12934)
  [#9133](https://github.com/mixxxdj/mixxx/issues/9133)
* Track menu: Remove from disk: stop and eject all affected decks [#13214](https://github.com/mixxxdj/mixxx/pull/13214)
* Track menu: add star rating
  [#12700](https://github.com/mixxxdj/mixxx/pull/12700)
  [#10652](https://github.com/mixxxdj/mixxx/issues/10652)
* Track menu: Show Properties in Missing and Hidden view [#13426](https://github.com/mixxxdj/mixxx/pull/13426)
* Add multi-track property editor / batch tag editor
  [#12548](https://github.com/mixxxdj/mixxx/pull/12548)
  [#9023](https://github.com/mixxxdj/mixxx/issues/9023)
  [#13299](https://github.com/mixxxdj/mixxx/pull/13299)
  [#13609](https://github.com/mixxxdj/mixxx/pull/13609)
  [#13597](https://github.com/mixxxdj/mixxx/issues/13597)
  [#13631](https://github.com/mixxxdj/mixxx/pull/13631)
* Track property editor: focus the editing field in the track properties that corresponds to the focused column
  [#13841](https://github.com/mixxxdj/mixxx/pull/13841)
  [#14036](https://github.com/mixxxdj/mixxx/pull/14036)
* Computer feature: add sidebar action "Refresh directory tree" [#12908](https://github.com/mixxxdj/mixxx/pull/12908)
* Add feedback to directory operations (add, remove, relink)
  [#12436](https://github.com/mixxxdj/mixxx/pull/12436)
  [#10481](https://github.com/mixxxdj/mixxx/issues/10481)
* Add ability to import external playlists as crates [#11852](https://github.com/mixxxdj/mixxx/pull/11852)
* Add 'Shuffle playlist' sidebar action
  [#12498](https://github.com/mixxxdj/mixxx/pull/12498)
  [#6988](https://github.com/mixxxdj/mixxx/issues/6988)
* Playlists: Update of playlist labels after adding tracks [#12866](https://github.com/mixxxdj/mixxx/pull/12866) [#12761](https://github.com/mixxxdj/mixxx/issues/12761)
* Tracks: Custom color for missing tracks [#12895](https://github.com/mixxxdj/mixxx/pull/12895)
* Tracks: Custom text color for played tracks (qss)
  [#12744](https://github.com/mixxxdj/mixxx/pull/12744)
  [#5911](https://github.com/mixxxdj/mixxx/issues/5911)
  [#12912](https://github.com/mixxxdj/mixxx/pull/12912)
  [#13538](https://github.com/mixxxdj/mixxx/pull/13538)
* History: Show track count and duration in sidebar
  [#12811](https://github.com/mixxxdj/mixxx/pull/12811)
  [#12788](https://github.com/mixxxdj/mixxx/issues/12788)
* Don't allow pasting tracks into locked playlists/crates or History [#12926](https://github.com/mixxxdj/mixxx/pull/12926)
* Optimize Library scrolling [#13358](https://github.com/mixxxdj/mixxx/pull/13358)
* Keep the metadata key text unchanged, use it as the origin of information
  [#11096](https://github.com/mixxxdj/mixxx/pull/11096)
  [#11095](https://github.com/mixxxdj/mixxx/issues/11095)
  [#13650](https://github.com/mixxxdj/mixxx/pull/13650)
  [#14011](https://github.com/mixxxdj/mixxx/pull/14011)
  [#14008](https://github.com/mixxxdj/mixxx/pull/14008)
  [#14020](https://github.com/mixxxdj/mixxx/pull/14020)
* Center date values, right-align Track # [#13674](https://github.com/mixxxdj/mixxx/pull/13674)
* Analysis: Fix stop button when analyzing crate/playlist [#13902](https://github.com/mixxxdj/mixxx/pull/13902)
* Add a debug message, which appears when event loop processing in Mixxx application takes very long
  [#12094](https://github.com/mixxxdj/mixxx/pull/12094)
  [#13900](https://github.com/mixxxdj/mixxx/pull/13900)
  [#13889](https://github.com/mixxxdj/mixxx/pull/13889)
  [#13903](https://github.com/mixxxdj/mixxx/pull/13903)
  [#14012](https://github.com/mixxxdj/mixxx/pull/14012)

### Preferences

* Add load point option 'First hotcue'
  [#12869](https://github.com/mixxxdj/mixxx/pull/12869)
  [#12740](https://github.com/mixxxdj/mixxx/issues/12740)
* MIDI Input editor: allow selecting multiple Options [#12348](https://github.com/mixxxdj/mixxx/pull/12348)
* Apply changes only after pressing Apply in color preferences [#13302](https://github.com/mixxxdj/mixxx/pull/13302)
* Add/reorder tabstops in Library and Waveform preferences
  [#13846](https://github.com/mixxxdj/mixxx/pull/13846)
* Add missing spacer in interface preferences [#13094](https://github.com/mixxxdj/mixxx/pull/13094)
* Fix fetching of soundcard sample rate
  [#11951](https://github.com/mixxxdj/mixxx/pull/11951)
  [11949](https://github.com/mixxxdj/mixxx/issues/11949)

### Controller Mappings

* Denon MC7000: Add optional jog wheel acceleration to the controller mapping [#4684](https://github.com/mixxxdj/mixxx/pull/4684)
* Denon MC7000: Unify parameter button logic and add customizable modes [#13589](https://github.com/mixxxdj/mixxx/pull/13589)
* Denon MC7000: Add sampler options to mapping settings [#13950](https://github.com/mixxxdj/mixxx/pull/13950)
* MIDI for light: Implement new Active deck heuristic [#13513](https://github.com/mixxxdj/mixxx/pull/13513)
* MIDI for light: Add settings GUI [#13721](https://github.com/mixxxdj/mixxx/pull/13721)
* Numark Scratch: Add controller settings  [#13404](https://github.com/mixxxdj/mixxx/pull/13404)
* Pioneer DDJ-FLX4: Mapping improvements [#12842](https://github.com/mixxxdj/mixxx/pull/12842)
* Traktor Kontrol S4 MK3: Add setting definition for  [#12995](https://github.com/mixxxdj/mixxx/pull/12995)
* Traktor Kontrol S4 MK3: Software mixer support and default pad layout customisation [#13059](https://github.com/mixxxdj/mixxx/pull/13059)
* Traktor Kontrol S4 Mk3: Rework jogwheel speed compute and motorized platter [#13393](https://github.com/mixxxdj/mixxx/pull/13393)
* Traktor Kontrol S4 Mk3: Revert QuickEffect preset offset [#13997](https://github.com/mixxxdj/mixxx/pull/13997)
* Traktor Kontrol S4 Mk3: Correct wheel timestamp wrap-around [#14016](https://github.com/mixxxdj/mixxx/pull/14016)

### Controller Backend

* Send sysex to all handlers [#12827](https://github.com/mixxxdj/mixxx/pull/12827)
* Speed up midi sysex receive
  [#12843](https://github.com/mixxxdj/mixxx/pull/12843)
* Add control for showing a deck's track menu [#10825](https://github.com/mixxxdj/mixxx/pull/10825)
* Removed old examples HID keyboard and HID trackpad [#12977](https://github.com/mixxxdj/mixxx/pull/12977)
* Reduce log noise with HID device
  [#13010](https://github.com/mixxxdj/mixxx/pull/13010)
  [#13125](https://github.com/mixxxdj/mixxx/pull/13125)
* Allow controller mapping to discard polling [#12558](https://github.com/mixxxdj/mixxx/pull/12558)
* Add support for mapping user settings
  [#11300](https://github.com/mixxxdj/mixxx/pull/11300)
  [#13046](https://github.com/mixxxdj/mixxx/pull/13046)
  [#13057](https://github.com/mixxxdj/mixxx/pull/13057)
  [#13045](https://github.com/mixxxdj/mixxx/pull/13045)
  [#13656](https://github.com/mixxxdj/mixxx/pull/13656)
  [#13738](https://github.com/mixxxdj/mixxx/pull/13738)
  [#13979](https://github.com/mixxxdj/mixxx/pull/13979)
  [#13990](https://github.com/mixxxdj/mixxx/pull/13990)
* Registering MIDI Input Handlers From Javascript
  [#12781](https://github.com/mixxxdj/mixxx/pull/12781)
  [#13089](https://github.com/mixxxdj/mixxx/pull/13089)
* Controller IO table: Fix display text for Action/control delegate [#13188](https://github.com/mixxxdj/mixxx/pull/13188)
* Drop lodash dependency in ComponentJS [#12779](https://github.com/mixxxdj/mixxx/pull/12779)
* Support for bulk devices on Windows and Mac [#13008](https://github.com/mixxxdj/mixxx/pull/13008)
* Fix pending reference to the old mapping after selecting 'No mapping' [#13907](https://github.com/mixxxdj/mixxx/pull/13907)
* Fix crash with GoToItem when no app windows has the focus [#13657](https://github.com/mixxxdj/mixxx/pull/13657)

### Waveforms

* Visualize slip mode position by splitting waveform (RGB GLSL only)
  [#13002](https://github.com/mixxxdj/mixxx/pull/13002)
  [#13256](https://github.com/mixxxdj/mixxx/pull/13256)
  [#10063](https://github.com/mixxxdj/mixxx/issues/10063)
* Show beats and time until next marker in the waveform
  [#12994](https://github.com/mixxxdj/mixxx/pull/12994)
  [#13311](https://github.com/mixxxdj/mixxx/pull/13311)
  [#13953](https://github.com/mixxxdj/mixxx/pull/13953)
  [#13314](https://github.com/mixxxdj/mixxx/issues/13314)
* Don't elide hotcue labels
  [#13219](https://github.com/mixxxdj/mixxx/pull/13219)
  [#10722](https://github.com/mixxxdj/mixxx/issues/10722)
* Allshader RGB, Filtered and Stacked Waveforms using textures for waveform data
  [#13151](https://github.com/mixxxdj/mixxx/pull/13151)
  [#12641](https://github.com/mixxxdj/mixxx/issues/12641)
* Allow changing the waveform overview type without reloading the skin
  [#13273](https://github.com/mixxxdj/mixxx/pull/13273)
* Overview: Update immediately, when the normalize option or global gain changed
  [#13634](https://github.com/mixxxdj/mixxx/pull/13634)
* Overview: Clear pickup position display when opening cue menu
  [#13693](https://github.com/mixxxdj/mixxx/pull/13693)

### Experimental Features

* QML Skin: Can be tested via the --qml command line option
  [#13152](https://github.com/mixxxdj/mixxx/pull/13152)
  [#12139](https://github.com/mixxxdj/mixxx/pull/12139)
  [#13152](https://github.com/mixxxdj/mixxx/pull/13152)
* QML Skin related changes
  [#11423](https://github.com/mixxxdj/mixxx/pull/11423)
  [#12559](https://github.com/mixxxdj/mixxx/pull/12559)
  [#12549](https://github.com/mixxxdj/mixxx/pull/12549)
  [#12541](https://github.com/mixxxdj/mixxx/pull/12541)
  [#12795](https://github.com/mixxxdj/mixxx/pull/12795)
  [#12844](https://github.com/mixxxdj/mixxx/pull/12844)
  [#12546](https://github.com/mixxxdj/mixxx/pull/12546)
  [#12794](https://github.com/mixxxdj/mixxx/pull/12794)
  [#12536](https://github.com/mixxxdj/mixxx/issues/12536)
  [#13058](https://github.com/mixxxdj/mixxx/pull/13058)
  [#12604](https://github.com/mixxxdj/mixxx/pull/12604)
  [#3967](https://github.com/mixxxdj/mixxx/pull/3967)
  [#13009](https://github.com/mixxxdj/mixxx/pull/13009)
  [#13009](https://github.com/mixxxdj/mixxx/pull/13009)
  [#13011](https://github.com/mixxxdj/mixxx/pull/13011)
  [#13506](https://github.com/mixxxdj/mixxx/pull/13506)
* iOS support: Mixxx can be built for iOS
  [#12672](https://github.com/mixxxdj/mixxx/pull/12672)
* iOS support related changes
  [#12689](https://github.com/mixxxdj/mixxx/pull/12689)
  [#12714](https://github.com/mixxxdj/mixxx/pull/12714)
  [#12716](https://github.com/mixxxdj/mixxx/pull/12716)
  [#12698](https://github.com/mixxxdj/mixxx/pull/12698)
  [#12676](https://github.com/mixxxdj/mixxx/pull/12676)
  [#12688](https://github.com/mixxxdj/mixxx/pull/12688)
  [#13379](https://github.com/mixxxdj/mixxx/pull/13379)
  [#13378](https://github.com/mixxxdj/mixxx/issues/13378)
  [#13383](https://github.com/mixxxdj/mixxx/pull/13383)
* Emscripten/WebAssembly support, to run Mixxx hardware independent in a browser
  [#12918](https://github.com/mixxxdj/mixxx/pull/12918)
* Emscripten/WebAssembly related changes
  [#12910](https://github.com/mixxxdj/mixxx/pull/12910)
  [#12913](https://github.com/mixxxdj/mixxx/pull/12913)
  [#12916](https://github.com/mixxxdj/mixxx/pull/12916)
  [#12915](https://github.com/mixxxdj/mixxx/pull/12915)
  [#12921](https://github.com/mixxxdj/mixxx/pull/12921)
  [#12922](https://github.com/mixxxdj/mixxx/pull/12922)
  [#12931](https://github.com/mixxxdj/mixxx/pull/12931)
  [#12940](https://github.com/mixxxdj/mixxx/pull/12940)
  [#12945](https://github.com/mixxxdj/mixxx/pull/12945)
  [#12952](https://github.com/mixxxdj/mixxx/pull/12952)
  [#12930](https://github.com/mixxxdj/mixxx/pull/12930)
  [#12917](https://github.com/mixxxdj/mixxx/pull/12917)

### Target support

* Maintain GL ES support [#13485](https://github.com/mixxxdj/mixxx/pull/13485)
* Tools: Add `rpm_buildenv.sh` for building on Fedora [#13069](https://github.com/mixxxdj/mixxx/pull/13069)
* Lenient taglib 2.0 guard [#12793](https://github.com/mixxxdj/mixxx/pull/12793)
* MixxxApplication: Support linking Qt statically on Linux [#12284](https://github.com/mixxxdj/mixxx/pull/12284)
* FindSndFile: Link mpg123 in static builds [#13087](https://github.com/mixxxdj/mixxx/pull/13087)
* FindPortMidi: Link ALSA in static builds on Linux [#12292](https://github.com/mixxxdj/mixxx/pull/12292) [#12291](https://github.com/mixxxdj/mixxx/pull/12291)
* FindLibudev: Link hidapi and libusb with libudev in static builds on Linux [#12294](https://github.com/mixxxdj/mixxx/pull/12294)
* FindVorbis: Link ogg in static builds [#12297](https://github.com/mixxxdj/mixxx/pull/12297)
* FindSleef: Use OpenMP in static builds [#12295](https://github.com/mixxxdj/mixxx/pull/12295)
* macOS packaging: Enable app sandbox in ad-hoc-packaged (i.e. non-notarized) bundles too [#12101](https://github.com/mixxxdj/mixxx/pull/12101)
* CMakeLists: Match arbitrary `arm64-osx` triplets [#11933](https://github.com/mixxxdj/mixxx/pull/11933)
* Disable warning in lib/apple code [#13522](https://github.com/mixxxdj/mixxx/pull/13522)
* GitHub CI: Use retry loop for CPack to work around macOS issue [#13991](https://github.com/mixxxdj/mixxx/pull/13991)
* Github CI: Enable `WARNINGS_FATAL` on macOS, too [#11905](https://github.com/mixxxdj/mixxx/pull/11905)

## [2.4.2](https://github.com/mixxxdj/mixxx/milestone/43?closed=1) (2024-11-26)

### Controller Mappings

* Denon MC7000: Fix star up/down logic by only handling button down events [#13588](https://github.com/mixxxdj/mixxx/pull/13588)
* Intech TEK2: Add initial mapping [#13521](https://github.com/mixxxdj/mixxx/pull/13521)
* Korg Kaoss DJ: Update script [#12683](https://github.com/mixxxdj/mixxx/pull/12683)
* MIDI for light: Fix unsound timer handling [#13117](https://github.com/mixxxdj/mixxx/pull/13117)
* Novation Dicer: Remove flanger mapping with quickeffect toggle
  [#13196](https://github.com/mixxxdj/mixxx/pull/13196)
  [#13134](https://github.com/mixxxdj/mixxx/issues/13134)
* Novation Launchpad X: Fix detection on macOS
  [#13691](https://github.com/mixxxdj/mixxx/pull/13691)
  [#13633](https://github.com/mixxxdj/mixxx/issues/13633)
* Numark PartyMix: Fix EQ (script binding) display name [#13255](https://github.com/mixxxdj/mixxx/pull/13255)
* Numark Scratch: Add initial mapping
  [#4834](https://github.com/mixxxdj/mixxx/pull/4834)
  [#13375](https://github.com/mixxxdj/mixxx/pull/13375)
* Pioneer DDJ-400 and DDJ-FLX4: Remove tap beat mapping to resolve conflict with toggle quantize and fix shift + play
  [#13815](https://github.com/mixxxdj/mixxx/pull/13815)
  [#13813](https://github.com/mixxxdj/mixxx/issues/13813)
  [#13857](https://github.com/mixxxdj/mixxx/pull/13857)
* Reloop Beatmix 2/4: Fix eject button and jog LED being lit on track unload
  [#13601](https://github.com/mixxxdj/mixxx/pull/13601)
  [#13605](https://github.com/mixxxdj/mixxx/pull/13605)
* Reloop Mixage MK1, MK2, Controller Edition: Add initial mapping [#12296](https://github.com/mixxxdj/mixxx/pull/12296)
* Sony SIXAXIS: Fix mapping [#13319](https://github.com/mixxxdj/mixxx/pull/13319)

### Fixes

* Handle not supported files when dragging to waveforms and spinnies
  [#13208](https://github.com/mixxxdj/mixxx/pull/13208)
  [#13271](https://github.com/mixxxdj/mixxx/pull/13271)
  [#13275](https://github.com/mixxxdj/mixxx/pull/13275)
* Fix Sqlite 3.45 builds by using only single quotes for SQL strings
  [#13247](https://github.com/mixxxdj/mixxx/pull/13247)
  [#13257](https://github.com/mixxxdj/mixxx/pull/13257)
* LateNight: Use default colors for sampler overviews (like main decks) [#13274](https://github.com/mixxxdj/mixxx/pull/13274)
* Library: Allow to drop files to decks with unsupported or no file extensions
  [#13209](https://github.com/mixxxdj/mixxx/pull/13209)
  [#13204](https://github.com/mixxxdj/mixxx/issues/13204)
* Update build environment with libdjinterop 0.21.0 [#13288](https://github.com/mixxxdj/mixxx/pull/13288)
* Move to GitHub workflow runner macos-12
  [#13296](https://github.com/mixxxdj/mixxx/pull/13296)
  [#13248](https://github.com/mixxxdj/mixxx/issues/13248)
* Recording: with empty config, save default split size immediately
  [#13304](https://github.com/mixxxdj/mixxx/pull/13304)
* Add support for Ubuntu Oracular Oriole and remove Lunar Lobster
  [#13348](https://github.com/mixxxdj/mixxx/pull/13348)
* Recordbox: Fix string decoding issues
  [#13293](https://github.com/mixxxdj/mixxx/pull/13293)
  [#13291](https://github.com/mixxxdj/mixxx/issues/13291)
* Mixer preferences: Don't update EQs/QuickEffects while applying [#13333](https://github.com/mixxxdj/mixxx/pull/13333)
* Hardware preferences: Fix UX when applying config with missing/busy devices
  [#13312](https://github.com/mixxxdj/mixxx/pull/13312)
* Fix minor 64 bit CPU performance issue [#13355](https://github.com/mixxxdj/mixxx/pull/13355)
* Fix clicks at loop-out when looping into lead-in [#13294](https://github.com/mixxxdj/mixxx/pull/13294)
* Fix wrong pitch value on startup, caused by `components.Pot`
  [#11814](https://github.com/mixxxdj/mixxx/issues/11814)
  [#13463](https://github.com/mixxxdj/mixxx/pull/13463)
* Engine Prime: Fix build-failure [#13397](https://github.com/mixxxdj/mixxx/pull/13397)
* Engine Prime: Friendlier error message if export fails [#13524](https://github.com/mixxxdj/mixxx/pull/13524)
* macOs: Fix Keyboard shortcuts by not catching num key modifier
  [#13481](https://github.com/mixxxdj/mixxx/pull/13481)
  [#13305](https://github.com/mixxxdj/mixxx/issues/13305)
* Skins: fix time display to allow AM/PM
  [#13430](https://github.com/mixxxdj/mixxx/pull/13430)
  [#13421](https://github.com/mixxxdj/mixxx/issues/13421)
* Fix detection last sound if track does not end with silence.
  [#13545](https://github.com/mixxxdj/mixxx/pull/13545)
  [#13449](https://github.com/mixxxdj/mixxx/issues/13449)
* Remove false positive critical warning related to library columns
  [#13165](https://github.com/mixxxdj/mixxx/pull/13165)
  [#13164](https://github.com/mixxxdj/mixxx/issues/13164)
* Fix reading metadata for files with wrong extensions
  [#13218](https://github.com/mixxxdj/mixxx/pull/13218)
  [#13205](https://github.com/mixxxdj/mixxx/issues/13205)
* History: remove purged tracks, auto-remove empty playlists
  [#13579](https://github.com/mixxxdj/mixxx/pull/13579)
  [#13578](https://github.com/mixxxdj/mixxx/issues/13578)
* Synchronize AutoDJ next deck with top track in queue
  [#12909](https://github.com/mixxxdj/mixxx/pull/12909)
  [#8956](https://github.com/mixxxdj/mixxx/issues/8956)
* Playlists: Update play duration and bold state in sidebar when dragging tracks into the playlist table
  [#13591](https://github.com/mixxxdj/mixxx/pull/13591)
  [#13590](https://github.com/mixxxdj/mixxx/issues/13590)
  [#13575](https://github.com/mixxxdj/mixxx/pull/13575)
* Playlists: Keep correct track selection (# position) when sorting
  [#13103](https://github.com/mixxxdj/mixxx/pull/13103)
* Track file export: Various fixes
  [#13610](https://github.com/mixxxdj/mixxx/pull/13610)
* Controller engine: Unify/improve logging, expand error dialog's Details box
  [#13626](https://github.com/mixxxdj/mixxx/pull/13626)
* Fix quantization in the effect engine (metronome effect)
  [#13636](https://github.com/mixxxdj/mixxx/pull/13636)
  [#13733](https://github.com/mixxxdj/mixxx/pull/13733)
* Musicbrainz: Improved messages
  [#13672](https://github.com/mixxxdj/mixxx/pull/13672)
  [#13673](https://github.com/mixxxdj/mixxx/pull/13673)
* Fix ReplayGain detection in case of short tracks
  [#13680](https://github.com/mixxxdj/mixxx/pull/13680)
  [#13676](https://github.com/mixxxdj/mixxx/issues/13676)
  [#13702](https://github.com/mixxxdj/mixxx/issues/13702)
  [#13703](https://github.com/mixxxdj/mixxx/pull/13703)
* Track menu: Avoid crash and UX issues with track nullptr
  [#13685](https://github.com/mixxxdj/mixxx/pull/13685)
* Disable Properties shortcut in Computer feature views
  [#13698](https://github.com/mixxxdj/mixxx/pull/13698)
* Overview waveform: Add tooltip info about left-click dragging
  [#13739](https://github.com/mixxxdj/mixxx/pull/13739)
* Make `hotcue_focus_color_next`/`_prev` COs `ControlPushButton`s to allow direct mappings
  [#13764](https://github.com/mixxxdj/mixxx/pull/13764)
* Scaled svg cache to speed up drawing in hidpi mode [#13679](https://github.com/mixxxdj/mixxx/pull/13679)
* Update to libdjinterop 0.22.1 for Enigine Prime 4.0.1 support [#13790](https://github.com/mixxxdj/mixxx/pull/13790)
* HID: Avoid repeated error messages from hid_write()/hid_read() in case of errors
  [#13692](https://github.com/mixxxdj/mixxx/pull/13692)
  [#13660](https://github.com/mixxxdj/mixxx/issues/13660)
* Fix unnecessary painting with covers in library [#13715](https://github.com/mixxxdj/mixxx/pull/13715)
* Fix check for unrelated decks playing when starting Auto DJ
  [#13762](https://github.com/mixxxdj/mixxx/pull/13762)
  [#13734](https://github.com/mixxxdj/mixxx/issues/13734)
* Fix read before m_bufferInt during scratching
  [#13917](https://github.com/mixxxdj/mixxx/pull/13917)
  [#13916](https://github.com/mixxxdj/mixxx/issues/13916)
* Fix waveform EQ High&Mid visualization
  [#13923](https://github.com/mixxxdj/mixxx/pull/13923)
  [#13922](https://github.com/mixxxdj/mixxx/issues/13922)

## [2.4.1](https://github.com/mixxxdj/mixxx/milestone/41?closed=1) (2024-05-08)

### Controller Mappings

* Behringer DDM4000 & BCR2000: Fix exception in JS code [#12969](https://github.com/mixxxdj/mixxx/pull/12969)
* Denon DJ MC6000MK2: Fix mapping of filter knob/button [#13166](https://github.com/mixxxdj/mixxx/pull/13166)
* Denon DJ MC7000: Fix redundant argument and migrate to `hotcue_x_status`
  [#13113](https://github.com/mixxxdj/mixxx/pull/13113)
  [#13121](https://github.com/mixxxdj/mixxx/pull/13121)
* Hercules Inpulse 200: Configure shift-browser knob to scroll the library (quick) [#12932](https://github.com/mixxxdj/mixxx/pull/12932)
* Nintendo Wii Remote: Fix hid script regarding addOutput [#12973](https://github.com/mixxxdj/mixxx/pull/12973)
* Pioneer CDJ: Fix hid script regarding addOutput [#12973](https://github.com/mixxxdj/mixxx/pull/12973)
* Pioneer DDJ-FLX4: Add waveform zoom and other mapping improvements
  [#12896](https://github.com/mixxxdj/mixxx/pull/12896)
  [#12842](https://github.com/mixxxdj/mixxx/pull/12842)
* Traktor Kontrol F1: Fixes for hid-parser and related script [#12876](https://github.com/mixxxdj/mixxx/pull/12876)
* Traktor S2 Mk1: fix warnings [#13145](https://github.com/mixxxdj/mixxx/pull/13145)
* Traktor S3: Fix mapping crash on macOS [#12840](https://github.com/mixxxdj/mixxx/pull/12840)
* Controller I/O table: sort action column by display string [#13039](https://github.com/mixxxdj/mixxx/pull/13039)

### Target Support

* Fix various minor build issues
  [#12853](https://github.com/mixxxdj/mixxx/pull/12853)
  [#12847](https://github.com/mixxxdj/mixxx/pull/12847)
  [#12822](https://github.com/mixxxdj/mixxx/pull/12822)
  [#12892](https://github.com/mixxxdj/mixxx/pull/12892)
  [#13079](https://github.com/mixxxdj/mixxx/pull/13079)
  [#12989](https://github.com/mixxxdj/mixxx/pull/12989)
* CMakeLists: Always prefer OpenGL framework on macOS
  [#13080](https://github.com/mixxxdj/mixxx/pull/13080)
* Use capitalized Mixxx in Windows installer and start menu
  [#13178](https://github.com/mixxxdj/mixxx/pull/13178)

### Skins

* Deere: make sampler rows persist [#12928](https://github.com/mixxxdj/mixxx/pull/12928)
* Tango: Remove unneeded waveform Singleton [#12938](https://github.com/mixxxdj/mixxx/pull/12938)
* Tango 64: fix Main VU meter
* Prevent possible crash in customs skins using parallel waveforms
  [#13043](https://github.com/mixxxdj/mixxx/pull/13043)
  [#12580](https://github.com/mixxxdj/mixxx/issues/12580)
  [#13136](https://github.com/mixxxdj/mixxx/pull/13136)
* Slider tooltip: consider orientation for up/down shortcut tooltips + add support for WKnobComposed [#13088](https://github.com/mixxxdj/mixxx/pull/13088)
* Tooltips: update 'hotcue' with saved loop features [#12875](https://github.com/mixxxdj/mixxx/pull/12875)
* Animate long press latching of sync button
  [#12990](https://github.com/mixxxdj/mixxx/pull/12990)
  [#13212](https://github.com/mixxxdj/mixxx/pull/13212)
* Polish fx chain controls [#12805](https://github.com/mixxxdj/mixxx/pull/12805)
* Waveforms: draw loop gradient at the correct position
  [#13061](https://github.com/mixxxdj/mixxx/pull/13061)
  [#13060](https://github.com/mixxxdj/mixxx/issues/13060)
* Waveform / spinnies: don't take keyboard focus on click
  [#13174](https://github.com/mixxxdj/mixxx/pull/13174)
  [#13211](https://github.com/mixxxdj/mixxx/pull/13211)

### Library

* Sidebar: show track count and duration of History playlists
  [#13020](https://github.com/mixxxdj/mixxx/pull/13020)
  [#13019](https://github.com/mixxxdj/mixxx/issues/13019)
  [#12788](https://github.com/mixxxdj/mixxx/issues/12788)
  [#12880](https://github.com/mixxxdj/mixxx/issues/12880)
  [#12882](https://github.com/mixxxdj/mixxx/pull/12882)
* Computer feature: update removable devices on Linux [#12893](https://github.com/mixxxdj/mixxx/pull/12893) [#12891](https://github.com/mixxxdj/mixxx/issues/12891)
* Playlists: Prevent removing tracks from locked playlists [#12927](https://github.com/mixxxdj/mixxx/pull/12927)
* History feature: Fix removing deleted tracks after export
  [#13016](https://github.com/mixxxdj/mixxx/pull/13016)
  [#13000](https://github.com/mixxxdj/mixxx/issues/13000)
* BPM display uses decimal separator of selected locale [#13067](https://github.com/mixxxdj/mixxx/pull/13067) [#13051](https://github.com/mixxxdj/mixxx/issues/13051)
* Fix relink directory when migrate between Linux/macOS and Windows [#12878](https://github.com/mixxxdj/mixxx/pull/12878)
* Allow adding new directories while watched directories are missing
  [#12937](https://github.com/mixxxdj/mixxx/pull/12937)
  [#10481](https://github.com/mixxxdj/mixxx/issues/10481)
* Require a minimum movement before initiating the drag&drop of tracks
  [#13135](https://github.com/mixxxdj/mixxx/pull/13135)
  [#12902](https://github.com/mixxxdj/mixxx/issues/12902)
  [#12979](https://github.com/mixxxdj/mixxx/pull/12979)
* iTunes/Serato/Traktor/Rhythmbox: Print error if library file could not be opened
  [#13012](https://github.com/mixxxdj/mixxx/pull/13012)
* Playlists: improve table update after deleting (purging) track files
  [#13127](https://github.com/mixxxdj/mixxx/pull/13127)
* Fix Color column width issue [#12852](https://github.com/mixxxdj/mixxx/pull/12852)
* Tracks: select track row when clicking the preview button (only when starting preview)
  [#12791](https://github.com/mixxxdj/mixxx/pull/12791)
* Library track menu: show Hide action also in Playlist & Crates [#11901](https://github.com/mixxxdj/mixxx/pull/11901)
* iTunes: Obtain FileAccess before accessing iTunes XML [#13013](https://github.com/mixxxdj/mixxx/pull/13013)

### Miscellaneous

* Remove unnecessary unpolish operation of the style, before polish the new style [#12445](https://github.com/mixxxdj/mixxx/pull/12445)
* Developer Tools: Initially sort controls by group name, ascending [#12884](https://github.com/mixxxdj/mixxx/pull/12884)
* Waveforms: Fix scratching crossing loop boundaries [#13007](https://github.com/mixxxdj/mixxx/pull/13007)
* Prohibit un-replace when deck is playing [#13023](https://github.com/mixxxdj/mixxx/pull/13023) [#12906](https://github.com/mixxxdj/mixxx/issues/12906)
* Track Properties dialog: Prevent wiping metadata when applying twice quickly
  [#12965](https://github.com/mixxxdj/mixxx/pull/12965)
  [#12963](https://github.com/mixxxdj/mixxx/issues/12963)
* AutoDJ: Fix button state after error message about playing deck 3/4
  [#12976](https://github.com/mixxxdj/mixxx/pull/12976)
  [#12975](https://github.com/mixxxdj/mixxx/issues/12975)
* Tagfetcher: Cache fetched covers
  [#12301](https://github.com/mixxxdj/mixxx/pull/12301)
  [#11084](https://github.com/mixxxdj/mixxx/issues/11084)
* Avoid beats iterator being one off and DEBUG_ASSERT in Beats::iteratorFrom
  [#13150](https://github.com/mixxxdj/mixxx/pull/13150)
  [#13149](https://github.com/mixxxdj/mixxx/issues/13149)
* Show hint if resource path in CMakeCache.txt does not exist
  [#12929](https://github.com/mixxxdj/mixxx/pull/12929)
* Always calculate the auto value for colorful console output [#13153](https://github.com/mixxxdj/mixxx/pull/13153)
* Fix FLAC recording on macOS and Windows
  [#10880](https://github.com/mixxxdj/mixxx/issues/10880)
  [#13154](https://github.com/mixxxdj/mixxx/pull/13154)
* LV Mix EQ: Fix pops when enabling in effect rack
  [#13055](https://github.com/mixxxdj/mixxx/issues/13055)
  [#13073](https://github.com/mixxxdj/mixxx/pull/13073)
* Fix hid addOutput

## [2.4.0](https://github.com/mixxxdj/mixxx/milestone/15?closed=1) (2024-02-16)

### Music Library: Tracks Table & Track Menu

* Remember track selection when switching library features, fix initial selection etc.
  [#4177](https://github.com/mixxxdj/mixxx/pull/4177)
  [#4536](https://github.com/mixxxdj/mixxx/pull/4536)
  [#12321](https://github.com/mixxxdj/mixxx/pull/12321)
  [#12064](https://github.com/mixxxdj/mixxx/issues/12064)
  [#11196](https://github.com/mixxxdj/mixxx/pull/11196)
  [#11130](https://github.com/mixxxdj/mixxx/pull/11130)
* Add new library column that shows the last time a track was played
  [#3140](https://github.com/mixxxdj/mixxx/pull/3140)
  [#3457](https://github.com/mixxxdj/mixxx/pull/3457)
  [#3494](https://github.com/mixxxdj/mixxx/pull/3494)
  [#3596](https://github.com/mixxxdj/mixxx/pull/3596)
  [#3740](https://github.com/mixxxdj/mixxx/pull/3740)
* Add keyboard shortcut Ctrl+Enter to open track properties [#4347](https://github.com/mixxxdj/mixxx/pull/4347)
* Home/End keys jump to first/last row [#4850](https://github.com/mixxxdj/mixxx/pull/4850)
* Wrap selection around at the bottom/top, only if Shift is not pressed
  [#11090](https://github.com/mixxxdj/mixxx/pull/11090)
  [#11100](https://github.com/mixxxdj/mixxx/pull/11100)
  [#12391](https://github.com/mixxxdj/mixxx/pull/12391)
* Allow to hide/remove tracks from the library by pressing the Delete key
  [#4330](https://github.com/mixxxdj/mixxx/pull/4330)
  [#7176](https://github.com/mixxxdj/mixxx/issues/7176)
  [#9793](https://github.com/mixxxdj/mixxx/issues/9793)
  [#9837](https://github.com/mixxxdj/mixxx/issues/9837)
  [#10537](https://github.com/mixxxdj/mixxx/issues/10537)
  [#11239](https://github.com/mixxxdj/mixxx/pull/11239)
  [#4577](https://github.com/mixxxdj/mixxx/pull/4577)
  [#10577](https://github.com/mixxxdj/mixxx/issues/10577)
  [#11171](https://github.com/mixxxdj/mixxx/pull/11171)
  [#10761](https://github.com/mixxxdj/mixxx/issues/10761)
* Fix Recording table refresh issues [#4648](https://github.com/mixxxdj/mixxx/pull/4648)
* Show time in addition to the date in the timestamp column
  [#4900](https://github.com/mixxxdj/mixxx/pull/4900)
  [#10726](https://github.com/mixxxdj/mixxx/issues/10726)
  [#11020](https://github.com/mixxxdj/mixxx/pull/11020)
* Show only the date in Date Added / Last Played columns. Move the time of day to tooltips [#3945](https://github.com/mixxxdj/mixxx/pull/3945)
* Right-align BPM, duration & bitrate values [#11634](https://github.com/mixxxdj/mixxx/pull/11634) [#11668](https://github.com/mixxxdj/mixxx/pull/11668) [#11657](https://github.com/mixxxdj/mixxx/issues/11657)
* Remove parenthesis from play counter display [#11357](https://github.com/mixxxdj/mixxx/pull/11357)
* Refocus library, after editing skin controls [#11767](https://github.com/mixxxdj/mixxx/pull/11767)
* Fix performance with large playlists [#11851](https://github.com/mixxxdj/mixxx/pull/11851) [#11724](https://github.com/mixxxdj/mixxx/issues/11724)
* Add multi-line editor delegate for comment column [#11752](https://github.com/mixxxdj/mixxx/pull/11752)
* Keep current item visible when the view shrinks vertically [#11273](https://github.com/mixxxdj/mixxx/pull/11273)
* macOS scrollbar: Make sure last track is shown in library [#11669](https://github.com/mixxxdj/mixxx/pull/11669) [#9495](https://github.com/mixxxdj/mixxx/issues/9495)
* Add action to select loaded track in library [#4740](https://github.com/mixxxdj/mixxx/pull/4740)
* Add menu for Analyze and Reanalyze
  [#4806](https://github.com/mixxxdj/mixxx/pull/4806)
  [#11873](https://github.com/mixxxdj/mixxx/pull/11873)
  [#11872](https://github.com/mixxxdj/mixxx/issues/11872)
* Add support for overriding analysis settings about variable/constant BPM on a per-track basis [#10931](https://github.com/mixxxdj/mixxx/pull/10931)
* Add menu for looking up track metadata at Discogs, SoundCloud and LastFM [#4772](https://github.com/mixxxdj/mixxx/pull/4772) [#4836](https://github.com/mixxxdj/mixxx/pull/4836)
* Add "Delete Track Files" action, does "Move to Trash" with Qt >= 5.15
  [#4560](https://github.com/mixxxdj/mixxx/pull/4560)
  [#4831](https://github.com/mixxxdj/mixxx/pull/4831)
  [#10763](https://github.com/mixxxdj/mixxx/issues/10763)
  [#11580](https://github.com/mixxxdj/mixxx/pull/11580)
  [#11577](https://github.com/mixxxdj/mixxx/issues/11577)
  [#11583](https://github.com/mixxxdj/mixxx/pull/11583)
  [#3212](https://github.com/mixxxdj/mixxx/pull/3212)
  [#11842](https://github.com/mixxxdj/mixxx/pull/11842)
* Allow to clear the comment field
  [#4722](https://github.com/mixxxdj/mixxx/pull/4722)
  [#10615](https://github.com/mixxxdj/mixxx/issues/10615)
* Allow to reset loops and also via "[ChannelN], loop_remove" control object
  [#4802](https://github.com/mixxxdj/mixxx/pull/4802)
  [#10748](https://github.com/mixxxdj/mixxx/issues/10748)
  [#12392](https://github.com/mixxxdj/mixxx/pull/12392)
  [#12521](https://github.com/mixxxdj/mixxx/pull/12521)
* Add 'Update ReplayGain' decks' to track menus [#4031](https://github.com/mixxxdj/mixxx/pull/4031) [#4719](https://github.com/mixxxdj/mixxx/pull/4719)
* Restore "Remove from playlist" in History [#11591](https://github.com/mixxxdj/mixxx/pull/11591) [#10974](https://github.com/mixxxdj/mixxx/issues/10974)
* Enable Lock BPM action if any selected track BPM is unlocked [#12385](https://github.com/mixxxdj/mixxx/pull/12385)
* Order BPM action by factor, show peview (for single track) [#12701](https://github.com/mixxxdj/mixxx/pull/12701) [#10128](https://github.com/mixxxdj/mixxx/issues/10128)
* Provide the same features in all deck track menus [#12214](https://github.com/mixxxdj/mixxx/pull/12214)
* Track table header: Keep menu open after toggling a checkbox [#12218](https://github.com/mixxxdj/mixxx/pull/12218)

### Music Library: Sidebar & Searchbar

* Add F2 and Del/Backspace shortcuts for renaming & deleting playlists and crates
  [#11172](https://github.com/mixxxdj/mixxx/pull/11172)
  [#11235](https://github.com/mixxxdj/mixxx/pull/11235)
  [#4697](https://github.com/mixxxdj/mixxx/pull/4697)
  [#4700](https://github.com/mixxxdj/mixxx/pull/4700)
  [#10294](https://github.com/mixxxdj/mixxx/issues/10294)
* Improve presentation of the History library tree
  [#2996](https://github.com/mixxxdj/mixxx/pull/2996)
  [#4298](https://github.com/mixxxdj/mixxx/pull/4298)
  [#10533](https://github.com/mixxxdj/mixxx/issues/10533)
* History: Fix sidebar context menu actions
  [#4384](https://github.com/mixxxdj/mixxx/pull/4384)
  [#4297](https://github.com/mixxxdj/mixxx/pull/4297)
  [#10529](https://github.com/mixxxdj/mixxx/issues/10529)
* History: Add cleanup options
  [#4726](https://github.com/mixxxdj/mixxx/pull/4726)
  [#9259](https://github.com/mixxxdj/mixxx/issues/9259)
  [#10714](https://github.com/mixxxdj/mixxx/issues/10714)
* History: Fix update of play count after removing tracks
  [#12258](https://github.com/mixxxdj/mixxx/pull/12258)
  [#12046](https://github.com/mixxxdj/mixxx/issues/12046)
  [#12256](https://github.com/mixxxdj/mixxx/issues/12256)
* Improve UX with right-click and selection after add, rename, delete, duplicate etc.
  [#11208](https://github.com/mixxxdj/mixxx/pull/11208)
  [#4193](https://github.com/mixxxdj/mixxx/pull/4193)
  [#10488](https://github.com/mixxxdj/mixxx/issues/10488)
  [#11574](https://github.com/mixxxdj/mixxx/pull/11574)
  [#11208](https://github.com/mixxxdj/mixxx/pull/11208)
  [#11712](https://github.com/mixxxdj/mixxx/pull/11712)
* Map Left Arrow Key to jump to parent node and activates it
  [#4253](https://github.com/mixxxdj/mixxx/pull/4253)
* Crates: only store or activate sibling crate if it's valid
  [#11770](https://github.com/mixxxdj/mixxx/pull/11770)
  [#11769](https://github.com/mixxxdj/mixxx/issues/11769)
* Add recent searches to a drop down menu of the search box
  [#3171](https://github.com/mixxxdj/mixxx/pull/3171)
  [#3262](https://github.com/mixxxdj/mixxx/pull/3262)
  [#4505](https://github.com/mixxxdj/mixxx/pull/4505)
* Save search queries across restarts
  [#4458](https://github.com/mixxxdj/mixxx/pull/4458)
  [#10517](https://github.com/mixxxdj/mixxx/issues/10517)
  [#10561](https://github.com/mixxxdj/mixxx/issues/10561)
  [#4571](https://github.com/mixxxdj/mixxx/pull/4571)
* Enable search in Browse & Recording views [#11014](https://github.com/mixxxdj/mixxx/pull/11014) [#11012](https://github.com/mixxxdj/mixxx/issues/11012) [#4382](https://github.com/mixxxdj/mixxx/pull/4382)
* Update Clear button when search is disabled [#4447](https://github.com/mixxxdj/mixxx/pull/4447)
* Fix reset to default of search timeout in preferences [#4504](https://github.com/mixxxdj/mixxx/pull/4504) [#10589](https://github.com/mixxxdj/mixxx/issues/10589)
* Ctrl+F in focused search box selects the entire search string [#4515](https://github.com/mixxxdj/mixxx/pull/4515)
* Improve keypress handling, fix glitch in popup, strip whitespaces [#4658](https://github.com/mixxxdj/mixxx/pull/4658)
* Enter jumps to track table if search query was transmitted [#4844](https://github.com/mixxxdj/mixxx/pull/4844)
  Push completion entry to top, to make up/down behave naturally
* Remove ESC shortcut in favour of new `[Library],focused_widget` [#4571](https://github.com/mixxxdj/mixxx/pull/4571)
  [#11030](https://github.com/mixxxdj/mixxx/pull/11030)
  [#10975](https://github.com/mixxxdj/mixxx/issues/10975)
* Restore previous search term when switching between playlists and crates
  [#11129](https://github.com/mixxxdj/mixxx/pull/11129)
  [#11015](https://github.com/mixxxdj/mixxx/issues/11015)
  [#11477](https://github.com/mixxxdj/mixxx/pull/11477)
  [#11476](https://github.com/mixxxdj/mixxx/issues/11476)
* Add options to disable auto-completion and history [#10942](https://github.com/mixxxdj/mixxx/pull/10942) [#10634](https://github.com/mixxxdj/mixxx/issues/10634)
* Require Enter or Right key to search for auto completed strings
  [#11207](https://github.com/mixxxdj/mixxx/pull/11207)
  [#11289](https://github.com/mixxxdj/mixxx/pull/11289)
  [#11287](https://github.com/mixxxdj/mixxx/issues/11287)
* Allow to use := and quotes to find exact matches [#12063](https://github.com/mixxxdj/mixxx/pull/12063) [#10699](https://github.com/mixxxdj/mixxx/issues/10699)

### Music Library: Backend & Database

* Add new "[AutoDJ],add_random_track" to make this feature accessible from controllers [#3076](https://github.com/mixxxdj/mixxx/pull/3076)
* Don't store or update metadata of missing tracks in the Mixxx database to prevent inconsistencies with file tags [#3811](https://github.com/mixxxdj/mixxx/pull/3811)
* Update library schema to 37 for synchronizing file modified time with track source on metadata import/export
  [#3978](https://github.com/mixxxdj/mixxx/pull/3978)
  [#4012](https://github.com/mixxxdj/mixxx/pull/4012)
* Track Metadata: Fix synchronization (import/export) of file tags
  [#4628](https://github.com/mixxxdj/mixxx/pull/4628)
  [#4631](https://github.com/mixxxdj/mixxx/pull/4631)
  [#4847](https://github.com/mixxxdj/mixxx/pull/4847) [#10782](https://bugs.launchpad.net/bugs/1981106)
* Track Metadata: Do not overwrite unchanged multi-valued fields [#12613](https://github.com/mixxxdj/mixxx/pull/12613) [#12587](https://github.com/mixxxdj/mixxx/issues/12587)
* Optionally reset metadata on reimport if file tags are missing, enabled by "[Library] ResetMissingTagMetadataOnImport 1"). [#4873](https://github.com/mixxxdj/mixxx/pull/4873)
* Logging: Suppress expected and harmless schema migration errors [#4248](https://github.com/mixxxdj/mixxx/pull/4248)
* Fix handling of undefined BPM values
  [#4062](https://github.com/mixxxdj/mixxx/pull/4062)
  [#4063](https://github.com/mixxxdj/mixxx/pull/4063)
  [#4100](https://github.com/mixxxdj/mixxx/pull/4100)
  [#4154](https://github.com/mixxxdj/mixxx/pull/4154)
  [#4165](https://github.com/mixxxdj/mixxx/pull/4165)
  [#4168](https://github.com/mixxxdj/mixxx/pull/4168)
* Automatic analyze and optimize database [#4199](https://github.com/mixxxdj/mixxx/pull/4199)
* Re-import and update metadata after files have been modified when loading tracks [#4218](https://github.com/mixxxdj/mixxx/pull/4218)
* Re-enable shortcuts after editing controls
  [#4360](https://github.com/mixxxdj/mixxx/pull/4360)
  [#10184](https://github.com/mixxxdj/mixxx/issues/10184)
  [#10523](https://github.com/mixxxdj/mixxx/issues/10523)
* Allow to remove a track form the disk [#3212](https://github.com/mixxxdj/mixxx/pull/3212) [#4639](https://github.com/mixxxdj/mixxx/pull/4639)
* Fix accasional resetting of played counter in database [#4578](https://github.com/mixxxdj/mixxx/pull/4578) [#10617](https://github.com/mixxxdj/mixxx/issues/10617)
* Experimental: Fix writing of undefined MusicBrainz Recording ID [#4694](https://github.com/mixxxdj/mixxx/pull/4694)
* Traktor library: fix importing track key [#4701](https://github.com/mixxxdj/mixxx/pull/4701)
* Fix exporting m3u files with tracks and special characters by using the URL format [#4752](https://github.com/mixxxdj/mixxx/pull/4752)
* Library Scanner: Sort files before adding them [#10919](https://github.com/mixxxdj/mixxx/pull/10919)
* Library Scanner: Fix track relocation query [#12462](https://github.com/mixxxdj/mixxx/pull/12462)
* MenuBar: Add shortcut for rescanning library [#11136](https://github.com/mixxxdj/mixxx/pull/11136)
* Playlists: simplify import function, add whitespace before the # suffix [#12246](https://github.com/mixxxdj/mixxx/pull/12246)
* Destroy PlayerInfo after EngineRecord is stopped to fix a debug assertion [#12341](https://github.com/mixxxdj/mixxx/pull/12341) [#12242](https://github.com/mixxxdj/mixxx/issues/12242)
* iTunes: Modularize importer and use `iTunesLibrary` on macOS for compatibility with `Music.app`
  [#11353](https://github.com/mixxxdj/mixxx/pull/11353)
  [#11256](https://github.com/mixxxdj/mixxx/issues/11256)
  [#11446](https://github.com/mixxxdj/mixxx/pull/11446)
  [#11444](https://github.com/mixxxdj/mixxx/pull/11444)
  [#11503](https://github.com/mixxxdj/mixxx/pull/11503)
  [#11500](https://github.com/mixxxdj/mixxx/pull/11500)
  [#11509](https://github.com/mixxxdj/mixxx/pull/11509)
* iTunes: Fix sporadic crash during unit tests due to a not initialized reference. [#11666](https://github.com/mixxxdj/mixxx/pull/11666)
* iTunes: Permit duplicate playlist names by identifying playlists by id (rather than name) [#11794](https://github.com/mixxxdj/mixxx/pull/11794)
* iTunes: Re-enable test and add `composer`, `playCount`, `lastPlayedAt` and `dateAdded` to model [#11948](https://github.com/mixxxdj/mixxx/pull/11948)
* Fix setting the wrong default cue color [#11554](https://github.com/mixxxdj/mixxx/pull/11554) [#11260](https://github.com/mixxxdj/mixxx/issues/11260)
* Ensure that tracks with an invalid BPM are re-analyzed [#2776](https://github.com/mixxxdj/mixxx/pull/2776)
* Add support for exporting crates, playlists and the library to Engine Prime and Denon standalone controllers
  [#2753](https://github.com/mixxxdj/mixxx/pull/2753)
  [#2932](https://github.com/mixxxdj/mixxx/pull/2932)
  [#3102](https://github.com/mixxxdj/mixxx/pull/3102)
  [#3155](https://github.com/mixxxdj/mixxx/pull/3155)
  [#3621](https://github.com/mixxxdj/mixxx/pull/3621)
  [#3776](https://github.com/mixxxdj/mixxx/pull/3776)
  [#3787](https://github.com/mixxxdj/mixxx/pull/3787)
  [#3797](https://github.com/mixxxdj/mixxx/pull/3797)
  [#3798](https://github.com/mixxxdj/mixxx/pull/3798)
  [#4025](https://github.com/mixxxdj/mixxx/pull/4025)
  [#4087](https://github.com/mixxxdj/mixxx/pull/4087)
  [#4102](https://github.com/mixxxdj/mixxx/pull/4102)
  [#4143](https://github.com/mixxxdj/mixxx/pull/4143)
  [#4463](https://github.com/mixxxdj/mixxx/pull/4463)
  [#11815](https://github.com/mixxxdj/mixxx/pull/11815)
  [#12309](https://github.com/mixxxdj/mixxx/pull/12309)
  [#12005](https://github.com/mixxxdj/mixxx/pull/12005)
  [#11816](https://github.com/mixxxdj/mixxx/pull/11816)
  [#11720](https://github.com/mixxxdj/mixxx/pull/11720)
  [#11834](https://github.com/mixxxdj/mixxx/pull/11834)
  [#12452](https://github.com/mixxxdj/mixxx/pull/12452)
  [#11979](https://github.com/mixxxdj/mixxx/pull/11979)
* Rekordbox: Save all loops and correct AAC timing offset for CoreAudio [#2779](https://github.com/mixxxdj/mixxx/pull/2779)
* Rekordbox: Fix missing playlists due to invalid child ID [#10955](https://github.com/mixxxdj/mixxx/pull/10955)
* Rekordbox: Fix unhandled exception when parsing corrupt PDB files
  [#10452](https://github.com/mixxxdj/mixxx/issues/10452)
  [#4040](https://github.com/mixxxdj/mixxx/pull/4040)
* Improve log messages during schema migration [#2979](https://github.com/mixxxdj/mixxx/pull/2979)
* Search related tracks in collection
  [#3181](https://github.com/mixxxdj/mixxx/pull/3181)
  [#3213](https://github.com/mixxxdj/mixxx/pull/3213)
  [#2796](https://github.com/mixxxdj/mixxx/pull/2796)
  [#4207](https://github.com/mixxxdj/mixxx/pull/4207)

### Sync

* Add support for setting an explicit leader for sync lock
  [#2768](https://github.com/mixxxdj/mixxx/pull/2768)
  [#3099](https://github.com/mixxxdj/mixxx/pull/3099)
  [#3695](https://github.com/mixxxdj/mixxx/pull/3695)
  [#3734](https://github.com/mixxxdj/mixxx/pull/3734)
  [#3698](https://github.com/mixxxdj/mixxx/pull/3698)
  [#3864](https://github.com/mixxxdj/mixxx/pull/3864)
  [#3867](https://github.com/mixxxdj/mixxx/pull/3867)
  [#3921](https://github.com/mixxxdj/mixxx/pull/3921)
  [#4119](https://github.com/mixxxdj/mixxx/pull/4119)
  [#4135](https://github.com/mixxxdj/mixxx/pull/4135)
  [#4149](https://github.com/mixxxdj/mixxx/pull/4149)
  [#4276](https://github.com/mixxxdj/mixxx/pull/4276)
  [#3944](https://github.com/mixxxdj/mixxx/pull/3944)
  [#11828](https://github.com/mixxxdj/mixxx/pull/11828)
  [#11831](https://github.com/mixxxdj/mixxx/pull/11831)
  [#11829](https://github.com/mixxxdj/mixxx/issues/11829)
  [#12431](https://github.com/mixxxdj/mixxx/pull/12431)
  [#11788](https://github.com/mixxxdj/mixxx/issues/11788)
  [#12234](https://github.com/mixxxdj/mixxx/pull/12234)
  [#12499](https://github.com/mixxxdj/mixxx/pull/12499)
* Fix pitch issue with dynamic tracks and sync while cloning tracks
  [#12515](https://github.com/mixxxdj/mixxx/pull/12515)
* Fix issue with half/double BPM calculation when using sync
  [#3899](https://github.com/mixxxdj/mixxx/pull/3899)
  [#3706](https://github.com/mixxxdj/mixxx/pull/3706)
* Sync Lock: Don't seek phase when disabling sync [#4169](https://github.com/mixxxdj/mixxx/pull/4169)
* Sync Lock: Fix issues with single-playing syncables
  [#4155](https://github.com/mixxxdj/mixxx/pull/4155)
  [#4389](https://github.com/mixxxdj/mixxx/pull/4389)
* Re-sync to leader after scratching [#4005](https://github.com/mixxxdj/mixxx/pull/4005)
* Fix audio artifacts when fading from or to zero [#4363](https://github.com/mixxxdj/mixxx/pull/4363)
* EngineBuffer: Fix assert when new track is loaded during playback with sync [#4682](https://github.com/mixxxdj/mixxx/pull/4682)

### Audio Codecs

* Add support for m4v files [#4088](https://github.com/mixxxdj/mixxx/pull/4088)
* Fix recovering from FAAD2 decoding issues [#2850](https://github.com/mixxxdj/mixxx/pull/2850)
* MP3: Log recoverable errors as info instead of warning [#4365](https://github.com/mixxxdj/mixxx/pull/4365)
* MP3: Garbage detection fix [#12464](https://github.com/mixxxdj/mixxx/pull/12464)
* MP3: Improve decoding precision on Windows [#11911](https://github.com/mixxxdj/mixxx/pull/11911) [#11888](https://github.com/mixxxdj/mixxx/issues/11888)
* AAC encoder: Fix a memory leak [#4386](https://github.com/mixxxdj/mixxx/pull/4386) [#4408](https://github.com/mixxxdj/mixxx/pull/4408)
* Improve robustness of file type detection by considering the actual MIME type of the content. [#7970](https://github.com/mixxxdj/mixxx/issues/7970) [#4356](https://github.com/mixxxdj/mixxx/pull/4356) [#4357](https://github.com/mixxxdj/mixxx/pull/4357)
* Fix file type detection when file has wrong file extension by determining the MIME type from content
  [#4602](https://github.com/mixxxdj/mixxx/pull/4602)
  [#4600](https://github.com/mixxxdj/mixxx/pull/4600)
  [#4615](https://github.com/mixxxdj/mixxx/pull/4615)
  [#7970](https://github.com/mixxxdj/mixxx/issues/7970)
  [#10624](https://github.com/mixxxdj/mixxx/issues/10624)
  [#4683](https://github.com/mixxxdj/mixxx/pull/4683)
  [#10669](https://github.com/mixxxdj/mixxx/issues/10669)
* Fix type detection of AIFF files [#4364](https://github.com/mixxxdj/mixxx/pull/4364)
* Fix synchronization time stamps of ModPlug files [#4826](https://github.com/mixxxdj/mixxx/pull/4826) [#10758](https://github.com/mixxxdj/mixxx/issues/10758)
* ID3v2 parsing: Improve log warnings [#4610](https://github.com/mixxxdj/mixxx/pull/4610)
* ID3v2 parsing: Fix inconsistent import of comment field [#11249](https://github.com/mixxxdj/mixxx/pull/11249)
* Enable Modpug and Wavpack Support on macOS  [#11182](https://github.com/mixxxdj/mixxx/pull/11182) [#11119](https://github.com/mixxxdj/mixxx/issues/11119)
* Fix missing file name in file metadata error message [#11965](https://github.com/mixxxdj/mixxx/pull/11965) [#11964](https://github.com/mixxxdj/mixxx/issues/11964)
* Verify the "first sound" of as an analysis sanity check
  [#4773](https://github.com/mixxxdj/mixxx/pull/4773)
  [#11887](https://github.com/mixxxdj/mixxx/pull/11887)
  [#11946](https://github.com/mixxxdj/mixxx/pull/11946)
  [#11940](https://github.com/mixxxdj/mixxx/issues/11940)
* Fix zeros in the first m4a chunk on Linux [#11879](https://github.com/mixxxdj/mixxx/pull/11879)
* Fix overlapping buffers when decoding m4a files using ffmpeg [#11760](https://github.com/mixxxdj/mixxx/pull/11760) [#11545](https://github.com/mixxxdj/mixxx/issues/11545)
* Fix possible crash with opus files with embedded cover arts and require TagLib 1.11 or newer
  [#4251](https://github.com/mixxxdj/mixxx/pull/4251)
  [#4252](https://github.com/mixxxdj/mixxx/pull/4252) [#10500](https://github.com/mixxxdj/mixxx/issues/10500)

### Audio Engine

* Add support for Saved loops
  [#2194](https://github.com/mixxxdj/mixxx/pull/2194)
  [#3267](https://github.com/mixxxdj/mixxx/pull/3267)
  [#3202](https://github.com/mixxxdj/mixxx/pull/3202)
  [#4265](https://github.com/mixxxdj/mixxx/pull/4265)
  [#7574](https://github.com/mixxxdj/mixxx/issues/7574)
  [#11006](https://github.com/mixxxdj/mixxx/pull/11006)
  [#11003](https://github.com/mixxxdj/mixxx/issues/11003)
  [#12637](https://github.com/mixxxdj/mixxx/pull/12637)
  [#12632](https://github.com/mixxxdj/mixxx/pull/12632)
  [#12623](https://github.com/mixxxdj/mixxx/pull/12623)
  [#12618](https://github.com/mixxxdj/mixxx/issues/12618)
* Fix an issue when pressing multiple cue buttons at the same time [#3382](https://github.com/mixxxdj/mixxx/pull/3382)
* Fix synchronization of main cue point/position
  [#4137](https://github.com/mixxxdj/mixxx/pull/4137)
  [#10478](https://github.com/mixxxdj/mixxx/issues/10478)
  [#4153](https://github.com/mixxxdj/mixxx/pull/4153)
* Adjust ReplayGain: Allow user to update the replaygain value based on a deck pregain value [#4031](https://github.com/mixxxdj/mixxx/pull/4031)
* Add halve/double controls for beatjump size [#4269](https://github.com/mixxxdj/mixxx/pull/4269)
* Implement Un-eject by pressing eject again
  [#4668](https://github.com/mixxxdj/mixxx/pull/4668)
  [#11246](https://github.com/mixxxdj/mixxx/pull/11246)
* Implement Un-replace by double-clicking eject
  [#11246](https://github.com/mixxxdj/mixxx/pull/11246)
* Allow to cancel active loops via beatloop_activate [#4328](https://github.com/mixxxdj/mixxx/pull/4328) [#9950](https://github.com/mixxxdj/mixxx/issues/9950)
* Slip Mode: Preserve active (regular) loop when leaving Slip Mode [#11435](https://github.com/mixxxdj/mixxx/pull/11435) [#6993](https://github.com/mixxxdj/mixxx/issues/6993)
* Fix possible segfault when ejecting track [#4362](https://github.com/mixxxdj/mixxx/pull/4362) [#10497](https://github.com/mixxxdj/mixxx/issues/10497)
* Fix possible crash when ejecting track from a controller [#11884](https://github.com/mixxxdj/mixxx/pull/11884) [#11819](https://github.com/mixxxdj/mixxx/issues/11819)
* Fix an assertion when loop is before track start [#4383](https://github.com/mixxxdj/mixxx/pull/4383) [#10556](https://github.com/mixxxdj/mixxx/issues/10556)
* Fix and improve snapping to beats in various situations [#4366](https://github.com/mixxxdj/mixxx/pull/4366) [#10541](https://github.com/mixxxdj/mixxx/issues/10541)
* Don't wipe inapplicable sound config immediately [#4544](https://github.com/mixxxdj/mixxx/pull/4544)
* Rubberband: Support Version 3 "finer" (near-hi-fi quality) setting, on Windows and MacOs and when available on Linux
  [#4853](https://github.com/mixxxdj/mixxx/pull/4853)
  [#4855](https://github.com/mixxxdj/mixxx/pull/4855)
  [#11047](https://github.com/mixxxdj/mixxx/pull/11047)
* Rubberband: Add missing padding, preventing it from eating the initial transient [#11120](https://github.com/mixxxdj/mixxx/pull/11120)
* Rubberband: Improve mono-compatibility for R3 "finer" [#11418](https://github.com/mixxxdj/mixxx/pull/11418)
* Fix a possible crash when ejecting a track [#11334](https://github.com/mixxxdj/mixxx/pull/11334) [#11257](https://github.com/mixxxdj/mixxx/issues/11257)
* Add a range limits for beatjump_size of 512 [#11248](https://github.com/mixxxdj/mixxx/pull/11248) [#11203](https://github.com/mixxxdj/mixxx/issues/11203)
* Auto DJ: Fix sharp cut transition after cueing a track without a defined intro [#11629](https://github.com/mixxxdj/mixxx/pull/11629) [#11621](https://github.com/mixxxdj/mixxx/issues/11621)
* Auto DJ: Don't use removed Intro end and outro start makers, use transition time instead [#11830](https://github.com/mixxxdj/mixxx/pull/11830)
* Auto DJ: Fix GUI freeze when updating duration for many selected tracks
  [#12530](https://github.com/mixxxdj/mixxx/pull/12530)
  [#12520](https://github.com/mixxxdj/mixxx/issues/12520)
  [#12537](https://github.com/mixxxdj/mixxx/pull/12537)
* KeyControl: fix keylock/unlock bugs, reset pitch_adjust [4710](https://github.com/mixxxdj/mixxx/pull/4710)
* Looping: fix asserts for loop move [#11735](https://github.com/mixxxdj/mixxx/pull/11735)
* Looping: reset loop_end_pos on eject [#12224](https://github.com/mixxxdj/mixxx/pull/12224) [#12223](https://github.com/mixxxdj/mixxx/issues/12223)
* Fix Loop_out not seeking back [#12739](https://github.com/mixxxdj/mixxx/pull/12739) [#12742](https://github.com/mixxxdj/mixxx/pull/12742)
* ReadAheadManager: fix loop wraparound reader condition [#11717](https://github.com/mixxxdj/mixxx/pull/11717)
* Slip mode: consider loop for background position only if it was enabled  before slip [#11848](https://github.com/mixxxdj/mixxx/pull/11848) [#11844](https://github.com/mixxxdj/mixxx/issues/11844)
* Make decks' xfader assignment persistent [#12074](https://github.com/mixxxdj/mixxx/pull/12074) [#10122](https://github.com/mixxxdj/mixxx/issues/10122)
* Fix gain issue with cloned tracks [#12435](https://github.com/mixxxdj/mixxx/pull/12435) [#10550](https://github.com/mixxxdj/mixxx/issues/10550)

### Controller Mappings

* new: Hercules DJControl MIX controller mapping [#11279](https://github.com/mixxxdj/mixxx/pull/11279)
* new: Pioneer DDJ-FLX4 controller mapping based on DDJ-400 [#11245](https://github.com/mixxxdj/mixxx/pull/11245)
* new: Traktor Kontrol S4 Mk3 controller mapping [#11284](https://github.com/mixxxdj/mixxx/pull/11284)
* new: Traktor Kontrol Z1 HID controller mapping [#12366](https://github.com/mixxxdj/mixxx/pull/12366) [#12426](https://github.com/mixxxdj/mixxx/pull/12426)
* new: Yaeltex MiniMixxx controller mapping [#4350](https://github.com/mixxxdj/mixxx/pull/4350)
* Behringer DDM4000 mixer: Update controller mapping [#4262](https://github.com/mixxxdj/mixxx/pull/4262) [#4799](https://github.com/mixxxdj/mixxx/pull/4799)
* Hercules DJ Console RMX: Replace not defined CO name pitch_reset by pitch_set_default [#12441](https://github.com/mixxxdj/mixxx/pull/12441)
* Korg nanoKONTROL2: Don't try to configure more than 4 main decks [#12322](https://github.com/mixxxdj/mixxx/pull/12322) [#12317](https://github.com/mixxxdj/mixxx/issues/12317)
* Korg nanoKONTROL2: Removed along with Mixco scripts [#2682](https://github.com/mixxxdj/mixxx/pull/2682)
* MAudio Xponent: Removed along with Mixco scripts [#2682](https://github.com/mixxxdj/mixxx/pull/2682)
* MIDI4lights: Give beginTimer callbacks the anonymous function expression form [#12048](https://github.com/mixxxdj/mixxx/pull/12048)
* Novation Twitch: Removed along with Mixco scripts [#2682](https://github.com/mixxxdj/mixxx/pull/2682)
* Novation Launchpad: Update controller scripts [#2600](https://github.com/mixxxdj/mixxx/pull/2600) [#11914](https://github.com/mixxxdj/mixxx/pull/11914)
* Numark DJ2GO2 Touch: Fix sampler, hotcue, beatloop buttons [#4287](https://github.com/mixxxdj/mixxx/pull/4287) [#11595](https://github.com/mixxxdj/mixxx/pull/11595)
* Numark MixTrack Pro 3: Fix beginTimer callback syntax [#12401](https://github.com/mixxxdj/mixxx/pull/12401) [#12369](https://github.com/mixxxdj/mixxx/issues/12369)
* Roland DJ-505: Make blinking lights blink in sync and other improvements [#4159](https://github.com/mixxxdj/mixxx/pull/4159) [#4517](https://github.com/mixxxdj/mixxx/pull/4517)
* Traktor Kontrol S2 MK1: Add calibration and refactor [#11237](https://github.com/mixxxdj/mixxx/pull/11237)
* Traktor Kontrol S2 MK2 fix loaded chain preset CO [#11823](https://github.com/mixxxdj/mixxx/pull/11823) [#10667](https://github.com/mixxxdj/mixxx/issues/10667)
* Traktor Kontrol S2 MK3: Use FX select buttons to set quick effect presets
  [#11702](https://github.com/mixxxdj/mixxx/pull/11702)
* Traktor Kontrol S3: script improvements, vanilla-like FX behavior, control initialization, better scratching, and more
  [#11199](https://github.com/mixxxdj/mixxx/pull/11199)
  [#10645](https://github.com/mixxxdj/mixxx/issues/10645)
  [#12409](https://github.com/mixxxdj/mixxx/pull/12409)
  [#12510](https://github.com/mixxxdj/mixxx/pull/12510)
* Various mappings: Fix `waveform_zoom` ranges [#12393](https://github.com/mixxxdj/mixxx/pull/12393)
* Various mappings: Ensure required samplers are created [#12769](https://github.com/mixxxdj/mixxx/pull/12769)

### Controller Backend

* Never raise a fatal error if a controller mapping tries access a non-existent control object [#2947](https://github.com/mixxxdj/mixxx/pull/2947)
* Add support to access HID FeatureReports
  [#11326](https://github.com/mixxxdj/mixxx/pull/11326)
  [#10828](https://github.com/mixxxdj/mixxx/issues/10828)
  [#11664](https://github.com/mixxxdj/mixxx/pull/11664)
* Add function to request HID InputReports, to determine controller state at startup [#3317](https://github.com/mixxxdj/mixxx/pull/3317)
* Exclude HID device: ELAN touch screen [#11324](https://github.com/mixxxdj/mixxx/pull/11324) [#11323](https://github.com/mixxxdj/mixxx/issues/11323)
* Show otherwise hidden HID devices in developer mode  [#11317](https://github.com/mixxxdj/mixxx/pull/11317)
* Use hidapi's hidraw backend instead of libusb on Linux [#4054](https://github.com/mixxxdj/mixxx/pull/4054)
* Fix broken HID controller mappings Traktor Kontrol S2 MK3 and others [#11470](https://github.com/mixxxdj/mixxx/pull/11470) [#11461](https://github.com/mixxxdj/mixxx/issues/11461)
* HID mappings: Modernize and document common-hid-packet-parser.js [#4718](https://github.com/mixxxdj/mixxx/pull/4718) [#4894](https://github.com/mixxxdj/mixxx/pull/4894)
* HID mappings: Small fixes for common-hid-packet-parser.js [#11925](https://github.com/mixxxdj/mixxx/pull/11925)
* HID mappings: Add [Main] to the list of valid groups [#12102](https://github.com/mixxxdj/mixxx/pull/12102) [#12406](https://github.com/mixxxdj/mixxx/pull/12406)
* Consistently use "mapping" instead of "preset" to refer to controller mappings [#3472](https://github.com/mixxxdj/mixxx/pull/3472)
* Introduce new control object `[Library],show_track_menu` to open/close the track menu [#4465](https://github.com/mixxxdj/mixxx/pull/4465)
* Introduce new control object `[Library],sort_focused_column` [#4749](https://github.com/mixxxdj/mixxx/pull/4749) [#4763](https://github.com/mixxxdj/mixxx/pull/4763) [#10719](https://github.com/mixxxdj/mixxx/issues/10719)
* Introduce new control objects `[Master],indicator_250millis` and `[Master],indicator_500millis` [#4157](https://github.com/mixxxdj/mixxx/pull/4157)
* Introduce new control object `[Library],clear_search` [#4331](https://github.com/mixxxdj/mixxx/pull/4331)
* Introduce new control object `[Library],focused_widget` to focus library directly [#4369](https://github.com/mixxxdj/mixxx/pull/4369) [#4490](https://github.com/mixxxdj/mixxx/pull/4490)
* Introduce new control object `LoadTrackFromDeck` and `LoadTrackFromSampler` [#11244](https://github.com/mixxxdj/mixxx/pull/11244)
* Don't automatically enable controller if it was disabled before [#4244](https://github.com/mixxxdj/mixxx/pull/4244) [#10503](https://github.com/mixxxdj/mixxx/issues/10503)
* Enable Qt logging categories for controller logging [#4523](https://github.com/mixxxdj/mixxx/pull/4523)
* Fix segfault during Mixxx shutdown due to a stale controller connection [#4476](https://github.com/mixxxdj/mixxx/pull/4476) [#10553](https://github.com/mixxxdj/mixxx/issues/10553)
* Components JS: Fix syncbutton [#4329](https://github.com/mixxxdj/mixxx/pull/4329)
* Components JS: Add script.posMod for euclidean modulo [#11415](https://github.com/mixxxdj/mixxx/pull/11415)
* Components JS: make JogWheelBasic correctly switch which deck it controls [#11913](https://github.com/mixxxdj/mixxx/pull/11913) [#11867](https://github.com/mixxxdj/mixxx/issues/11867)
* Add Trace for the mapping connections, to allow JS profiling [#4766](https://github.com/mixxxdj/mixxx/pull/4766)
* Controller preferences: Allow creating a new mapping with 'No Mapping' selected
  [#4905](https://github.com/mixxxdj/mixxx/pull/4905)
  [#10540](https://github.com/mixxxdj/mixxx/issues/10540)
  [#10539](https://github.com/mixxxdj/mixxx/issues/10539)
* Add TypeScript declarations for engine and controller scripting API to improve IDE code completion during mapping developent [#4759](https://github.com/mixxxdj/mixxx/pull/4759)
* Retire Mixco Scripts [#2682](https://github.com/mixxxdj/mixxx/pull/2682)
* Relax strictness of `ControllerScriptInterfaceLegacy` methods.  [#11474](https://github.com/mixxxdj/mixxx/pull/11474) [#11473](https://github.com/mixxxdj/mixxx/issues/11473)
* Do not show ControlObject aliases in developer tools window [#12265](https://github.com/mixxxdj/mixxx/pull/12265)
* Do not use deprecated COs in C++ code/Keyboard Mapping/Skins [#11990](https://github.com/mixxxdj/mixxx/pull/11990)
* Fix creation of Sampler `end_of_track` ControlObjects [#12305](https://github.com/mixxxdj/mixxx/pull/12305) [#12304](https://github.com/mixxxdj/mixxx/issues/12304)
* Add a test SoftTakeoverTest.CatchOutOfBounds [#12114](https://github.com/mixxxdj/mixxx/pull/12114) [#12011](https://github.com/mixxxdj/mixxx/issues/12011)
* Make WHotcueButton learnable with the MIDI Wizard [#12252](https://github.com/mixxxdj/mixxx/pull/12252)
* Control picker menu: add `waveform_zoom_set_default` [#12247](https://github.com/mixxxdj/mixxx/pull/12247)
* CO Renaming
  [#12022](https://github.com/mixxxdj/mixxx/pull/12022)
  [#12021](https://github.com/mixxxdj/mixxx/pull/12021)
  [#11998](https://github.com/mixxxdj/mixxx/pull/11998)
  [#11996](https://github.com/mixxxdj/mixxx/pull/11996)
  [#11980](https://github.com/mixxxdj/mixxx/pull/11980)
  [#12007](https://github.com/mixxxdj/mixxx/pull/12007)
* Remove deprecated ControlObjects from Skins [#12030](https://github.com/mixxxdj/mixxx/pull/12030)
* Log warning if deprecated control is used [#11972](https://github.com/mixxxdj/mixxx/pull/11972)
* ControlObject alias improvements [#11973](https://github.com/mixxxdj/mixxx/pull/11973)
* Keyboard mapping: Repeat certain control actions if key is held [#12474](https://github.com/mixxxdj/mixxx/pull/12474)
* Keyboard mapping: Return triggers double-click, move Preview functions to P / Shift+P [#12639](https://github.com/mixxxdj/mixxx/pull/12639)
* Keyboard mapping: Various fixes [#12730](https://github.com/mixxxdj/mixxx/pull/12730)
* Update keyboard sheet [#12578](https://github.com/mixxxdj/mixxx/pull/12578)
* Logging: Add support for `QT_MESSAGE_PATTERN` environment variable
  [#3204](https://github.com/mixxxdj/mixxx/pull/3204)
  [#3518](https://github.com/mixxxdj/mixxx/pull/3518)
* Avoid issue with `stars_up/_down` ControlObjects [#12591](https://github.com/mixxxdj/mixxx/pull/12591)
* hotcue_X_color control: Fix color not stored in cue [#12733](https://github.com/mixxxdj/mixxx/pull/12733)

### Skins

* Add harmonic keywheel window
  [#1695](https://github.com/mixxxdj/mixxx/pull/1695)
  [#3622](https://github.com/mixxxdj/mixxx/pull/3622)
  [#3624](https://github.com/mixxxdj/mixxx/pull/3624)
* Allow skin scaling from preferences
  [#3960](https://github.com/mixxxdj/mixxx/pull/3960)
  [#11588](https://github.com/mixxxdj/mixxx/pull/11588)
  [#11586](https://github.com/mixxxdj/mixxx/issues/11586)
* Fix icon rendering on HiDPI/Retina screens [#12407](https://github.com/mixxxdj/mixxx/pull/12407) [#12361](https://github.com/mixxxdj/mixxx/issues/12361)
* Increase pixmapCache size limit and made it dependent on devicePixelRatio (for HiDPI/Retina displays) [#12416](https://github.com/mixxxdj/mixxx/pull/12416)
* Make beat indicator control behaviour more natural [#3608](https://github.com/mixxxdj/mixxx/pull/3608)
* Fix crash if no skin is available
  [#3918](https://github.com/mixxxdj/mixxx/pull/3918)
  [#3939](https://github.com/mixxxdj/mixxx/pull/3939)
* Fix crash when starting without a valid skin directory [#4575](https://github.com/mixxxdj/mixxx/pull/4575) [#10461](https://github.com/mixxxdj/mixxx/issues/10461)
* Fix leaked controls [#4213](https://github.com/mixxxdj/mixxx/pull/4213) [#10293](https://github.com/mixxxdj/mixxx/issues/10293)
* Fix switching from Shade to other skins [#4421](https://github.com/mixxxdj/mixxx/pull/4421) [#10558](https://github.com/mixxxdj/mixxx/issues/10558)
* Use double click to reset knobs and sliders [#4509](https://github.com/mixxxdj/mixxx/pull/4509) [#9947](https://github.com/mixxxdj/mixxx/issues/9947)
* Use info not warning for skin COs [#4525](https://github.com/mixxxdj/mixxx/pull/4525)
* Spinny: Allow to toggle cover art at runtime [#4565](https://github.com/mixxxdj/mixxx/pull/4565) [#10015](https://github.com/mixxxdj/mixxx/issues/10015)
* Passthrough: improve UI / UX [#4794](https://github.com/mixxxdj/mixxx/pull/4794)
* Knob: Hide cursor on wheel event for .8s [#11077](https://github.com/mixxxdj/mixxx/pull/11077)
* Move skin control hack to c++ (spinny/cover controls, mic/ducking controls) [#11183](https://github.com/mixxxdj/mixxx/pull/11183)
* LateNight: Move logo to the right [#4677](https://github.com/mixxxdj/mixxx/pull/4677)
* LateNight: Use correct tooltip for key control toggle [#4696](https://github.com/mixxxdj/mixxx/pull/4696)
* LateNight: Add toggles to show loop and beatjump controls [#4713](https://github.com/mixxxdj/mixxx/pull/4713)
* LateNight: Remove blinking play indicator from mini samplers [#4807](https://github.com/mixxxdj/mixxx/pull/4807)
* LateNight: Add buffer underflow indicator [#4906](https://github.com/mixxxdj/mixxx/pull/4906) [#10978](https://github.com/mixxxdj/mixxx/pull/10978)
* LateNight: Fix xfader icons in samplers and aux units [#12477](https://github.com/mixxxdj/mixxx/pull/12477)
* LateNight: use default RGB waveform colors [#12712](https://github.com/mixxxdj/mixxx/pull/12712)
* Add LateNight (64 Samplers) [#11715](https://github.com/mixxxdj/mixxx/pull/11715)
* Deere: fix skin/library layout (library missing in default view with Qt6) [#11912](https://github.com/mixxxdj/mixxx/pull/11912)
* Deere: use decks' waveform colors for sliders (Vol + pitch) [#12129](https://github.com/mixxxdj/mixxx/pull/12129) [#10240](https://github.com/mixxxdj/mixxx/issues/10240)
* Shade: Remove initial setting of now accessible effect controls [#4398](https://github.com/mixxxdj/mixxx/pull/4398) [#10557](https://github.com/mixxxdj/mixxx/issues/10557)
* Shade: Audio Latency meter fix [#11601](https://github.com/mixxxdj/mixxx/pull/11601)
* Tango: allow to toggle crossfader independently from mixer [#12703](https://github.com/mixxxdj/mixxx/pull/12703) [#12654](https://github.com/mixxxdj/mixxx/issues/12654)
* Fix outdated tooltips
  [#11387](https://github.com/mixxxdj/mixxx/pull/11387)
  [#11384](https://github.com/mixxxdj/mixxx/issues/11384)
  [#11860](https://github.com/mixxxdj/mixxx/pull/11860)
* Add settings directory link to Help menu [#11670](https://github.com/mixxxdj/mixxx/pull/11670) [#11667](https://github.com/mixxxdj/mixxx/issues/11667)
* Fix sidebar item styling
  [#11975](https://github.com/mixxxdj/mixxx/pull/11975)
  [#11957](https://github.com/mixxxdj/mixxx/issues/11957)
* Fix 500ms blocking of the whole event loop, when holding mouse down on title bar on Windows [#12359](https://github.com/mixxxdj/mixxx/pull/12359) [#12358](https://github.com/mixxxdj/mixxx/issues/12358) [#12433](https://github.com/mixxxdj/mixxx/pull/12433) [#12458](https://github.com/mixxxdj/mixxx/pull/12458)
* Change SKIN_WARNING to show the skin file and line first, then c++ context [#12253](https://github.com/mixxxdj/mixxx/pull/12253)
* Fix style of selected QComboBox items on Windows [#12339](https://github.com/mixxxdj/mixxx/pull/12339) [#12323](https://github.com/mixxxdj/mixxx/issues/12323)
* Fix reading the Spinny cover on Windows  [#12103](https://github.com/mixxxdj/mixxx/pull/12103) [#11131](https://github.com/mixxxdj/mixxx/issues/11131)
* Fix inconsistent/wrong musical keys in the UI [#12051](https://github.com/mixxxdj/mixxx/pull/12051) [#12044](https://github.com/mixxxdj/mixxx/issues/12044)
* Add `skins:` path alias [#12463](https://github.com/mixxxdj/mixxx/pull/12463)
* Remove `Text`, use `TrackProperty` or `Label` [#12004](https://github.com/mixxxdj/mixxx/pull/12004)
* Beat spinBox/AutoDJ spinbox: Enter & Esc also move focus to library [#4617](https://github.com/mixxxdj/mixxx/pull/4617) [#4845](https://github.com/mixxxdj/mixxx/pull/4845)
* Add effect chain menu button to Deere, polish in Tango [#12735](https://github.com/mixxxdj/mixxx/pull/12735)
* Skins: reload default.qss when (re)loading a skin [#12219](https://github.com/mixxxdj/mixxx/pull/12219)

### Waveforms and GL Widgets

* Waveform overhaul based on QOpenGlWindow and introduce full GLSL shader based waveforms, vumeters and spinnies. This fixes a couple of performance issues mainly on macOS.
  [#10989](https://github.com/mixxxdj/mixxx/pull/10989)
  [#10416](https://github.com/mixxxdj/mixxx/issues/10416)
  [#11460](https://github.com/mixxxdj/mixxx/issues/11460)
  [#11556](https://github.com/mixxxdj/mixxx/issues/11556)
  [#11450](https://github.com/mixxxdj/mixxx/issues/11450)
  [#10416](https://github.com/mixxxdj/mixxx/issues/10416)
  [#11734](https://github.com/mixxxdj/mixxx/issues/11734)
  [#12466](https://github.com/mixxxdj/mixxx/pull/12466)
  [#12678](https://github.com/mixxxdj/mixxx/pull/12678)
  [#12731](https://github.com/mixxxdj/mixxx/pull/12731)
* Default to 60 Hz waveform refresh rate [#11918](https://github.com/mixxxdj/mixxx/pull/11918)
* Introduce a VSsync mode driven by a phase locked loop [#12469](https://github.com/mixxxdj/mixxx/pull/12469)
* Make VSync mode 0 refer to the default mode and make ST_PLL the default on macOS, ST_TIMER otherwise [#12489](https://github.com/mixxxdj/mixxx/pull/12489)
* Use WaveformWidgetType::AllShaderRGBWaveform as autoChooseWidgetType [#11822](https://github.com/mixxxdj/mixxx/pull/11822)
* Add new "RGB Stacked" waveform [#3153](https://github.com/mixxxdj/mixxx/pull/3153)
* Fix micro jitter from clamping position offset to vsync interval [#12470](https://github.com/mixxxdj/mixxx/pull/12470)
* Avoid flickering when resizing [#12487](https://github.com/mixxxdj/mixxx/pull/12487)
* Invert scroll wheel waveform zoom direction to mach other applications [#4195](https://github.com/mixxxdj/mixxx/pull/4195)
* Waveform scrolling: Use set interval setting to fix performance degradation for AMD graphics adapters [#11681](https://github.com/mixxxdj/mixxx/pull/11681) [#11617](https://github.com/mixxxdj/mixxx/issues/11617)
* Fix waveform zooming [#11650](https://github.com/mixxxdj/mixxx/pull/11650) [#11626](https://github.com/mixxxdj/mixxx/issues/11626)
* Fix OpenGL version detection [#11673](https://github.com/mixxxdj/mixxx/pull/11673)
* Fix crash when no GL context is available [#11963](https://github.com/mixxxdj/mixxx/pull/11963) [#11929](https://github.com/mixxxdj/mixxx/issues/11929)
* Fix stopped waveform rendering in case of vinyl control [#11977](https://github.com/mixxxdj/mixxx/pull/11977) [#10764](https://github.com/mixxxdj/mixxx/issues/10764)
* Fix visual play position related to looping
  [#11840](https://github.com/mixxxdj/mixxx/pull/11840)
  [#11836](https://github.com/mixxxdj/mixxx/issues/11836)
  [#12538](https://github.com/mixxxdj/mixxx/pull/12538)
  [#12506](https://github.com/mixxxdj/mixxx/issues/12506)
  [#12513](https://github.com/mixxxdj/mixxx/issues/12513)
* Fix for visual position while scratching outside of an activated loop [#12281](https://github.com/mixxxdj/mixxx/pull/12281) [#12274](https://github.com/mixxxdj/mixxx/issues/12274)
* Spinny: Fix drawing of non-square cover arts [#11971](https://github.com/mixxxdj/mixxx/pull/11971) [#11967](https://github.com/mixxxdj/mixxx/issues/11967)
* Spinny/VU-Meter: Fix drawing [#12010](https://github.com/mixxxdj/mixxx/pull/12010) [#11930](https://github.com/mixxxdj/mixxx/issues/11930)
* VU-Meter: Don't use OpenGL by default [#11722](https://github.com/mixxxdj/mixxx/pull/11722)
* Improve GLSL pre-roll triangles [#12100](https://github.com/mixxxdj/mixxx/pull/12100) [#12015](https://github.com/mixxxdj/mixxx/issues/12015)
* Make scaling of GLSL RGB and RGB L/R waveform amplitudes consistent with simple waveform [#12205](https://github.com/mixxxdj/mixxx/pull/12205) [#12356](https://github.com/mixxxdj/mixxx/pull/12356)
* Improve rendering of waveform marks [#12203](https://github.com/mixxxdj/mixxx/pull/12203) [#12237](https://github.com/mixxxdj/mixxx/pull/12237)
* avoid overlapping marks [#12273](https://github.com/mixxxdj/mixxx/pull/12273)
* gradually "compact" the markers if the waveform height is reduced [#12501](https://github.com/mixxxdj/mixxx/pull/12501)
* Fix clamping of the index for drawing the waveform left of zero position [#12411](https://github.com/mixxxdj/mixxx/pull/12411)
* Fix possible crash when closing Mixxx [#12314](https://github.com/mixxxdj/mixxx/pull/12314) [#11737](https://github.com/mixxxdj/mixxx/issues/11737)
* Fix EGL support
  [#11982](https://github.com/mixxxdj/mixxx/pull/11982)
  [#11641](https://github.com/mixxxdj/mixxx/issues/11641)
  [#11935](https://github.com/mixxxdj/mixxx/pull/11935)
  [#11985](https://github.com/mixxxdj/mixxx/pull/11985)
  [#11982](https://github.com/mixxxdj/mixxx/pull/11982)
  [#11995](https://github.com/mixxxdj/mixxx/pull/11995)
  [#11994](https://github.com/mixxxdj/mixxx/pull/11994)
  [#12607](https://github.com/mixxxdj/mixxx/pull/12607)
* Preferences: recall correct waveform type when selecting an overview type
  [#12231](https://github.com/mixxxdj/mixxx/pull/12231)
  [#12226](https://github.com/mixxxdj/mixxx/issues/12226)

### Cover Art

* Prevent wrong cover art display due to hash conflicts [#2524](https://github.com/mixxxdj/mixxx/pull/2524) [#4904](https://github.com/mixxxdj/mixxx/pull/4904)
* Add background color for quick cover art preview [#2524](https://github.com/mixxxdj/mixxx/pull/2524)
* Fix coverart tooltip if cover is not cached [#12087](https://github.com/mixxxdj/mixxx/pull/12087)
* Add cover art fetcher to the Musicbrainz dialog
  [#10908](https://github.com/mixxxdj/mixxx/pull/10908)
  [#4871](https://github.com/mixxxdj/mixxx/pull/4871)
  [#10795](https://github.com/mixxxdj/mixxx/issues/10795)
  [#10796](https://github.com/mixxxdj/mixxx/issues/10796)
  [#10902](https://github.com/mixxxdj/mixxx/pull/10902)
  [#4851](https://github.com/mixxxdj/mixxx/pull/4851)
  [#11938](https://github.com/mixxxdj/mixxx/pull/11938)
  [#11086](https://github.com/mixxxdj/mixxx/issues/11086)
  [#12041](https://github.com/mixxxdj/mixxx/pull/12041)
  [#12300](https://github.com/mixxxdj/mixxx/pull/12300)
  [#12543](https://github.com/mixxxdj/mixxx/pull/12543)
  [#12532](https://github.com/mixxxdj/mixxx/issues/12532)
* CoverArtCache refactoring + Fix scrolling lag after updating Mixxx  [#12009](https://github.com/mixxxdj/mixxx/pull/12009)

### Effects

* Effect refactoring: Effect chain saving/loading, parameter hiding/rearrangement, effect preferences overhaul
  [#4467](https://github.com/mixxxdj/mixxx/pull/4467)
  [#4431](https://github.com/mixxxdj/mixxx/pull/4431)
  [#4426](https://github.com/mixxxdj/mixxx/pull/4426)
  [#4457](https://github.com/mixxxdj/mixxx/pull/4457)
  [#4456](https://github.com/mixxxdj/mixxx/pull/4456)
  [#4459](https://github.com/mixxxdj/mixxx/pull/4459)
  [#4462](https://github.com/mixxxdj/mixxx/pull/4462)
  [#4466](https://github.com/mixxxdj/mixxx/pull/4466)
  [#4468](https://github.com/mixxxdj/mixxx/pull/4468)
  [#4472](https://github.com/mixxxdj/mixxx/pull/4472)
  [#4470](https://github.com/mixxxdj/mixxx/pull/4470)
  [#4471](https://github.com/mixxxdj/mixxx/pull/4471)
  [#4483](https://github.com/mixxxdj/mixxx/pull/4483)
  [#4482](https://github.com/mixxxdj/mixxx/pull/4482)
  [#4484](https://github.com/mixxxdj/mixxx/pull/4484)
  [#4486](https://github.com/mixxxdj/mixxx/pull/4486)
  [#4502](https://github.com/mixxxdj/mixxx/pull/4502)
  [#4501](https://github.com/mixxxdj/mixxx/pull/4501)
  [#4518](https://github.com/mixxxdj/mixxx/pull/4518)
  [#4532](https://github.com/mixxxdj/mixxx/pull/4532)
  [#4461](https://github.com/mixxxdj/mixxx/pull/4461)
  [#4548](https://github.com/mixxxdj/mixxx/pull/4548)
  [#4503](https://github.com/mixxxdj/mixxx/pull/4503)
  [#4686](https://github.com/mixxxdj/mixxx/pull/4686)
  [#4691](https://github.com/mixxxdj/mixxx/pull/4691)
  [#4704](https://github.com/mixxxdj/mixxx/pull/4704)
  [#4748](https://github.com/mixxxdj/mixxx/pull/4748)
  [#4833](https://github.com/mixxxdj/mixxx/pull/4833)
  [#10762](https://github.com/mixxxdj/mixxx/issues/10762)
  [#4884](https://github.com/mixxxdj/mixxx/pull/4884)
  [#10802](https://github.com/mixxxdj/mixxx/issues/10802)
  [#10801](https://github.com/mixxxdj/mixxx/issues/10801)
  [#4899](https://github.com/mixxxdj/mixxx/pull/4899)
  [#8817](https://github.com/mixxxdj/mixxx/pull/8817)
  [#10868](https://github.com/mixxxdj/mixxx/pull/10868)
  [#11055](https://github.com/mixxxdj/mixxx/pull/11055)
  [#11135](https://github.com/mixxxdj/mixxx/pull/11135)
  [#11185](https://github.com/mixxxdj/mixxx/pull/11185)
  [#11242](https://github.com/mixxxdj/mixxx/pull/11242)
  [#10837](https://github.com/mixxxdj/mixxx/pull/10837)
  [#10834](https://github.com/mixxxdj/mixxx/issues/10834)
  [#11424](https://github.com/mixxxdj/mixxx/pull/11424)
  [#11376](https://github.com/mixxxdj/mixxx/pull/11376)
  [#11456](https://github.com/mixxxdj/mixxx/pull/11456)
  [#11454](https://github.com/mixxxdj/mixxx/issues/11454)
  [#11695](https://github.com/mixxxdj/mixxx/pull/11695)
  [#12633](https://github.com/mixxxdj/mixxx/pull/12633)
  [#12561](https://github.com/mixxxdj/mixxx/pull/12561)
  [#10859](https://github.com/mixxxdj/mixxx/pull/10859)
  [#10777](https://github.com/mixxxdj/mixxx/issues/10777)
  [#11886](https://github.com/mixxxdj/mixxx/pull/11886)
  [#12282](https://github.com/mixxxdj/mixxx/pull/12282)
  [#12277](https://github.com/mixxxdj/mixxx/issues/12277)
  [#11705](https://github.com/mixxxdj/mixxx/pull/11705)
  [#4469](https://github.com/mixxxdj/mixxx/pull/4469)
  [#11902](https://github.com/mixxxdj/mixxx/pull/11902)
  [#10605](https://github.com/mixxxdj/mixxx/issues/10605)
  [#4702](https://github.com/mixxxdj/mixxx/pull/4702)
  [#10579](https://github.com/mixxxdj/mixxx/issues/10579)
  [#4501](https://github.com/mixxxdj/mixxx/pull/4501)
  [#4502](https://github.com/mixxxdj/mixxx/pull/4502)
  [#4503](https://github.com/mixxxdj/mixxx/pull/4503)
  [#4590](https://github.com/mixxxdj/mixxx/pull/4590)
  [#4593](https://github.com/mixxxdj/mixxx/pull/4593)
  [#11062](https://github.com/mixxxdj/mixxx/pull/11062)
* Add Noise effect [#2921](https://github.com/mixxxdj/mixxx/pull/2921)
* Add Pitch Shift effect
  [#4775](https://github.com/mixxxdj/mixxx/pull/4775)
  [#7389](https://github.com/mixxxdj/mixxx/issues/7389)
  [#4810](https://github.com/mixxxdj/mixxx/pull/4810)
  [#4901](https://github.com/mixxxdj/mixxx/pull/4901)
  [#10858](https://github.com/mixxxdj/mixxx/pull/10858)
  [#12481](https://github.com/mixxxdj/mixxx/pull/12481)
* Add Distortion effect [#10932](https://github.com/mixxxdj/mixxx/pull/10932)
* Effect parameter knobs: Briefly show parameter value in parameter name widget
  [#11032](https://github.com/mixxxdj/mixxx/pull/11032)
  [#9022](https://github.com/mixxxdj/mixxx/issues/9022)
  [#11034](https://github.com/mixxxdj/mixxx/pull/11034)
* Effect parameter knobs: Implement ValueScaler::Integral, snap value to int [#11061](https://github.com/mixxxdj/mixxx/pull/11061)
* Show effect parameter units in parameter name label [#11041](https://github.com/mixxxdj/mixxx/pull/11041) [#11194](https://github.com/mixxxdj/mixxx/pull/11194)
* Fix gain compensation for the Moog filter [#11177](https://github.com/mixxxdj/mixxx/pull/11177)
* Fix memory leak in AutoPan [#11346](https://github.com/mixxxdj/mixxx/pull/11346)
* EngineFilterDelay: clamp wrong delay values [#4869](https://github.com/mixxxdj/mixxx/pull/4869)
* Fix crash when changing effect unit routing [#4707](https://github.com/mixxxdj/mixxx/pull/4707) [#9331](https://github.com/mixxxdj/mixxx/issues/9331)
* Clear effect buffer after ejecting a track  [#10692](https://github.com/mixxxdj/mixxx/issues/10692)
* Center Super knob when loading empty (QuickEffect) chain preset [#12320](https://github.com/mixxxdj/mixxx/pull/12320)
* Don't reset "super" and "mix" knob on startup [#11781](https://github.com/mixxxdj/mixxx/pull/11781) [#11773](https://github.com/mixxxdj/mixxx/issues/11773)
* Add a missing early return [#11809](https://github.com/mixxxdj/mixxx/pull/11809) [#111808](https://github.com/mixxxdj/mixxx/issues/11808)
* Update EffectSlot meta default value according to loaded effect [#12480](https://github.com/mixxxdj/mixxx/pull/12480) [#12479](https://github.com/mixxxdj/mixxx/issues/12479)

### Target Support

* Added support for macOS ARM builds on M1/M2 Apple silicon [#11398](https://github.com/mixxxdj/mixxx/pull/11398)
* Set app_id to fix Mixxx window icon on Wayland [#12635](https://github.com/mixxxdj/mixxx/pull/12635)
* Require C++20 but keep Ubuntu Focal support
  [#4889](https://github.com/mixxxdj/mixxx/pull/4889)
  [#4895](https://github.com/mixxxdj/mixxx/pull/4895)
  [#11204](https://github.com/mixxxdj/mixxx/pull/11204)
  [#4832](https://github.com/mixxxdj/mixxx/pull/4832)
  [#4803](https://github.com/mixxxdj/mixxx/pull/4803)
  [#11551](https://github.com/mixxxdj/mixxx/issues/11551)
  [#11573](https://github.com/mixxxdj/mixxx/pull/11573)
* Drop Ubuntu Bionic support, require Qt 5.12
  [#3687](https://github.com/mixxxdj/mixxx/pull/3687)
  [#3735](https://github.com/mixxxdj/mixxx/pull/3735)
  [#3736](https://github.com/mixxxdj/mixxx/pull/3736)
  [#3985](https://github.com/mixxxdj/mixxx/pull/3985)
* Drop Ubuntu Groovy and Impish support because of EOL
  [#4283](https://github.com/mixxxdj/mixxx/pull/4283)
  [#4849](https://github.com/mixxxdj/mixxx/pull/4849)
  [#12353](https://github.com/mixxxdj/mixxx/pull/12353)
* Support Ubuntu Noble and Jammy
  [#4780](https://github.com/mixxxdj/mixxx/pull/4780)
  [#4857](https://github.com/mixxxdj/mixxx/pull/4857)
  [#12353](https://github.com/mixxxdj/mixxx/pull/12353)
* Add NixOS support
  [#2820](https://github.com/mixxxdj/mixxx/pull/2820)
  [#2828](https://github.com/mixxxdj/mixxx/pull/2828)
  [#2836](https://github.com/mixxxdj/mixxx/pull/2836)
  [#2827](https://github.com/mixxxdj/mixxx/pull/2827)
  [#2827](https://github.com/mixxxdj/mixxx/pull/2827)
  [#2828](https://github.com/mixxxdj/mixxx/pull/2828)
  [#3113](https://github.com/mixxxdj/mixxx/pull/3113)
  [#3089](https://github.com/mixxxdj/mixxx/pull/3089)
  [#3545](https://github.com/mixxxdj/mixxx/pull/3545)
* Windows packaging: Use Azure for signing exe, msi and all dlls with timestamp and sha256
  [#12465](https://github.com/mixxxdj/mixxx/pull/12465)
  [#4824](https://github.com/mixxxdj/mixxx/pull/4824)
  [#4825](https://github.com/mixxxdj/mixxx/pull/4825)
* macOS packaging: Fix signing and migrate script to `notarytool`
  [#12123](https://github.com/mixxxdj/mixxx/pull/12123)
  [#12089](https://github.com/mixxxdj/mixxx/issues/12089)
  [#12095](https://github.com/mixxxdj/mixxx/pull/12095)
* macOS packaging: Enable app sandbox and fix related issues
  [#12138](https://github.com/mixxxdj/mixxx/pull/12138)
  [#12457](https://github.com/mixxxdj/mixxx/pull/12457)
  [#12137](https://github.com/mixxxdj/mixxx/issues/12137)
  [#11552](https://github.com/mixxxdj/mixxx/issues/11552)
  [#4018](https://github.com/mixxxdj/mixxx/pull/4018)
  [#10373](https://github.com/mixxxdj/mixxx/issues/10373)
* macOS: Use rounded Mixxx Icon to follow Apples style guide
  [#4545](https://github.com/mixxxdj/mixxx/pull/4545)
  [#10958](https://github.com/mixxxdj/mixxx/pull/10958)
* macOS packaging: Capitalize bundle and executable name (Mixxx.app)
  [#12656](https://github.com/mixxxdj/mixxx/pull/12656)
* OpenBSD: Allow building Mixxx [#11083](https://github.com/mixxxdj/mixxx/pull/11083)
* Improve Linux launcher
  [#11826](https://github.com/mixxxdj/mixxx/pull/11826)
  [#11820](https://github.com/mixxxdj/mixxx/issues/11820)
  [#11805](https://github.com/mixxxdj/mixxx/pull/11805)
  [#12424](https://github.com/mixxxdj/mixxx/pull/12424)
* Experimental iOS support
  [#12665](https://github.com/mixxxdj/mixxx/pull/12665)
  [#12666](https://github.com/mixxxdj/mixxx/pull/12666)
  [#12662](https://github.com/mixxxdj/mixxx/pull/12662)
  [#12663](https://github.com/mixxxdj/mixxx/pull/12663)
  [#12661](https://github.com/mixxxdj/mixxx/pull/12661)
  [#12650](https://github.com/mixxxdj/mixxx/pull/12650)
* Fail early in case Taglib 2.0 is found [#12709](https://github.com/mixxxdj/mixxx/pull/12709)

### Track properties

* Fix a SIGSEGV after a debug assertion [#4316](https://github.com/mixxxdj/mixxx/pull/4316)
* Apply pending changes also when saving via hotkey [#4562](https://github.com/mixxxdj/mixxx/pull/4562) [#10612](https://github.com/mixxxdj/mixxx/issues/10612)
* Fix crash when trying to scale 0.0 BPM [#4587](https://github.com/mixxxdj/mixxx/pull/4587) [#1955853](https://github.com/mixxxdj/mixxx/issues/10625)
* Add track color selector [#11436](https://github.com/mixxxdj/mixxx/pull/11436) [#10324](https://github.com/mixxxdj/mixxx/issues/10324)
* Don't clear unsaved properties when updating star rating [#11565](https://github.com/mixxxdj/mixxx/pull/11565) [#11540](https://github.com/mixxxdj/mixxx/issues/11540)
* Fix glitch in Star rating [#12582](https://github.com/mixxxdj/mixxx/pull/12582) [#12576](https://github.com/mixxxdj/mixxx/issues/12576)
* Focus Double-clicked property field for edit
  [#11764](https://github.com/mixxxdj/mixxx/pull/11764)
  [#11804](https://github.com/mixxxdj/mixxx/pull/11804)
  [#11802](https://github.com/mixxxdj/mixxx/issues/11802)
* Display the samplerate [#12418](https://github.com/mixxxdj/mixxx/pull/12418)

### Preferences

* Always show tooltips [#4198](https://github.com/mixxxdj/mixxx/pull/4198) [#9716](https://github.com/mixxxdj/mixxx/issues/9716)
* Add option to keep deck playing on track load [#10944](https://github.com/mixxxdj/mixxx/pull/10944) [#10548](https://github.com/mixxxdj/mixxx/issues/10548)
* Always enable Alt shortcut keys [#11145](https://github.com/mixxxdj/mixxx/pull/11145) [#10413](https://github.com/mixxxdj/mixxx/issues/10413)
* Sound Hardware: auto select free device channels [#11859](https://github.com/mixxxdj/mixxx/pull/11859) [#10163](https://github.com/mixxxdj/mixxx/issues/10163)
* Various layout and UX fixes
  [#12429](https://github.com/mixxxdj/mixxx/pull/12429)
  [#12399](https://github.com/mixxxdj/mixxx/pull/12399)
  [#11663](https://github.com/mixxxdj/mixxx/pull/11663)
  [#11926](https://github.com/mixxxdj/mixxx/pull/11926)
  [#12057](https://github.com/mixxxdj/mixxx/pull/12057)
* macOS: set preferences dialog title to the selected page title [#11696](https://github.com/mixxxdj/mixxx/pull/11696)
* macOS: fix the preferences menu and opening the settings directory [#11679](https://github.com/mixxxdj/mixxx/pull/11679)
* macOS: fix slider styling in preferences dialog [#11647](https://github.com/mixxxdj/mixxx/pull/11647)
* Vinyl control: Improve quality indicator [#3279](https://github.com/mixxxdj/mixxx/pull/3279)
* Mixer: apply & save settings only in slotApply(), fix bugs, improve UX [#11527](https://github.com/mixxxdj/mixxx/pull/11527)
* Mixer: fix reset of EQ auto-reset checkbox [#11818](https://github.com/mixxxdj/mixxx/pull/11818) [#11817](https://github.com/mixxxdj/mixxx/issues/11817)
* Interface: avoid unneeded skin reload, clean up [#11853](https://github.com/mixxxdj/mixxx/pull/11853)
* Library: Add link to settings files info in the manual [#4367](https://github.com/mixxxdj/mixxx/pull/4367)
* Controllers: add search bars to mapping tables [#11165](https://github.com/mixxxdj/mixxx/pull/11165)
* Add 13 new translation languages [#4785](https://github.com/mixxxdj/mixxx/pull/4785) [#9702](https://github.com/mixxxdj/mixxx/issues/9702)
* Join Franch translations to "fr" and remove all untranslated English strings.  [#12699](https://github.com/mixxxdj/mixxx/pull/12699)
* Apply changes from all pages when pressing Apply (like when pressing Okay) [#12194](https://github.com/mixxxdj/mixxx/pull/12194)

### Known issues

* Volume / Loudness spikes on Windows with M4A/AAC files.
  Last known working version is Windows 10 build 17763.
  Affected versions are Windows 10 build 19041 and Windows 11 build 22000.
  [#12289](https://github.com/mixxxdj/mixxx/issues/12289)
  [#11094](https://github.com/mixxxdj/mixxx/issues/11094)
* macOS: Library entries are now sorted using the language depending Unicode Collation Algorithm (UCA).
  [#12517](https://github.com/mixxxdj/mixxx/issues/12517)
* macOS: Visual glitches with the main EQ sliders
  [#12517](https://github.com/mixxxdj/mixxx/issues/12630)
* Linux: possible crash when enabling a MIDI controller [#12001](https://github.com/mixxxdj/mixxx/issues/12001)
  Introduce with Qt 5.15.5, fixed in Qt 5.15.17 and Qt 6.6.3
* Extra Samplers are created during startup, when found in a saved Sampler Bank [#12657](https://github.com/mixxxdj/mixxx/pull/12657) [#12809](https://github.com/mixxxdj/mixxx/pull/12809)

## [2.3.6](https://github.com/mixxxdj/mixxx/milestone/40) (2023-08-15)

* Fixed possible crash when closing Mixxx while browsing the file system
  [#11593](https://github.com/mixxxdj/mixxx/pull/11593)
  [#11589](https://github.com/mixxxdj/mixxx/issues/11589)
* No longer stop a track with an active loop at the very end
  [#11558](https://github.com/mixxxdj/mixxx/pull/11558)
  [#11557](https://github.com/mixxxdj/mixxx/issues/11557)
* Fixed resyncing when moving an active loop
  [#11152](https://github.com/mixxxdj/mixxx/pull/11152)
  [#11381](https://github.com/mixxxdj/mixxx/issues/11381)
* Allow true gapless playback when repeating full tracks
  [#11532](https://github.com/mixxxdj/mixxx/pull/11532)
  [#9842](https://github.com/mixxxdj/mixxx/issues/9842)
  [#11704](https://github.com/mixxxdj/mixxx/pull/11704)
* Rhythmbox: Fixed bulk track imports from playlists
  [#11661](https://github.com/mixxxdj/mixxx/pull/11661)
* Console log spam reduced
  [#11690](https://github.com/mixxxdj/mixxx/pull/11690)
  [#11691](https://github.com/mixxxdj/mixxx/issues/11691)
* Numark DJ2GO2 Touch: Add missing loop_out mapping for the right deck
  [#11595](https://github.com/mixxxdj/mixxx/pull/11595)
  [#11659](https://github.com/mixxxdj/mixxx/pull/11659)
* Shade: Fixed VU-Meter and other minor issues
  [#11598](https://github.com/mixxxdj/mixxx/pull/11598)
* Fixed a rare crash when disabling quantize form a controller
  [#11744](https://github.com/mixxxdj/mixxx/pull/11744)
  [#11709](https://github.com/mixxxdj/mixxx/issues/11709)
* Controller Preferences: Avoid scrollbars in I/O tabs if Info tab exceeds page height
  [#11756](https://github.com/mixxxdj/mixxx/pull/11756)
* Broadcast: Improved error message in case of timeout
  [#11775](https://github.com/mixxxdj/mixxx/pull/11775)
* Handle setting `loop_in` and `loop_out` to the same position
  [#11771](https://github.com/mixxxdj/mixxx/pull/11771)
  [#10600](https://github.com/mixxxdj/mixxx/issues/10600)
* Fix build issues with Protobuf v23.4 and with clang 32
  [#11751](https://github.com/mixxxdj/mixxx/pull/11751)
  [#11765](https://github.com/mixxxdj/mixxx/pull/11765)
  [#11762](https://github.com/mixxxdj/mixxx/issues/11762)
* Disable GL VU-Meters on Windows by default. They can be re-enabled via the command line option `--enableVuMeterGL`.
  [#11787](https://github.com/mixxxdj/mixxx/pull/11787)
  [#11785](https://github.com/mixxxdj/mixxx/issues/11785)
  [#11789](https://github.com/mixxxdj/mixxx/issues/11789)
* Library preferences: Uncheck Serato metadata export when file metadata export is unchecked
  [#11782](https://github.com/mixxxdj/mixxx/pull/11782)
  [#11226](https://github.com/mixxxdj/mixxx/issues/11226)
* Denon MC6000MK2: Delete mapping for main gain
  [#11792](https://github.com/mixxxdj/mixxx/pull/11792)
* Improve output in case of some failed file system operations
  [#11783](https://github.com/mixxxdj/mixxx/pull/11783)
* Fix overlapping buffers when decoding M4A files using FFmpeg before 4.4
  [#11760](https://github.com/mixxxdj/mixxx/pull/11760)
  [#11545](https://github.com/mixxxdj/mixxx/issues/11545)
* Don't reject key values from file metadata with non-minor/-major scales.
  [#11001](https://github.com/mixxxdj/mixxx/pull/11001)
  [#10995](https://github.com/mixxxdj/mixxx/issues/10995)
* Allow playing tracks with durations of more than 6 hours
  [#11511](https://github.com/mixxxdj/mixxx/pull/11511)
  [#11504](https://github.com/mixxxdj/mixxx/issues/11504)
* Update latency compensation for Soundtouch version 2.1.1 to 2.3
  [#11154](https://github.com/mixxxdj/mixxx/pull/11154)

## [2.3.5](https://github.com/mixxxdj/mixxx/milestone/39) (2023-05-10)

* Fix empty waveform overview after loading a track (Mixxx 2.3.4 regression)
  Fixed by [#11333](https://github.com/mixxxdj/mixxx/pull/11333)
  [#11359](https://github.com/mixxxdj/mixxx/pull/11359)
  [#11344](https://github.com/mixxxdj/mixxx/issues/11344)
* Fullscreen: Fix a crash that occurs on Linux after enabling fullsceen and using menu
  shortcuts e.g. Alt-F.
  [#11328](https://github.com/mixxxdj/mixxx/pull/11328)
  [#11320](https://github.com/mixxxdj/mixxx/issues/11320)
* Fullscreen: Rebuild & reconnect menu only on desktops with global menu
  [#11350](https://github.com/mixxxdj/mixxx/pull/11350)
* macOS: Request Microphone and line-in access permission.
  [#11367](https://github.com/mixxxdj/mixxx/pull/11367)
  [#11365](https://github.com/mixxxdj/mixxx/issues/11365)
* JACK API: Allow to explicit select buffers of 2048 and 4096 frames/period. They are not
  supported by the automatic buffer setting of the used PortAudio library.
  [#11366](https://github.com/mixxxdj/mixxx/pull/11366)
  [#11341](https://github.com/mixxxdj/mixxx/issues/11341)
* Pioneer DDJ-400: Make Beat FX section more intuitive
  [#10912](https://github.com/mixxxdj/mixxx/pull/10912)
* Playlist export: Adopt new extension after changing the playlist type
  [#11332](https://github.com/mixxxdj/mixxx/pull/11332)
  [#11327](https://github.com/mixxxdj/mixxx/issues/11327)
* LateNight: brighter fx parameter buttons
  [#11397](https://github.com/mixxxdj/mixxx/pull/11397)
* Fix drift in analysis data after exporting metadata to MP3 files with ID3v1.1 tags
  [#11168](https://github.com/mixxxdj/mixxx/pull/11168)
  [#11159](https://github.com/mixxxdj/mixxx/issues/11159)
* Fix broadcasting using Opus encoding
  [#11349](https://github.com/mixxxdj/mixxx/pull/11349)
  [#10666](https://github.com/mixxxdj/mixxx/issues/10666)
* Tango: Remove VU peak indicators from stacked layout. This fixes a visual regression in Mixxx 2.3.4.
  [#11430](https://github.com/mixxxdj/mixxx/pull/11430)
  [#11362](https://github.com/mixxxdj/mixxx/issues/11362)
* Hercules P32: Allow optional using pregain instead of dry/wet knob
  [#3538](https://github.com/mixxxdj/mixxx/pull/3538)
* Improve Color Picker dialog
  [#11439](https://github.com/mixxxdj/mixxx/pull/11439)
* Fix blank Waveform overview after changing Skin with a track loaded
  [#11453](https://github.com/mixxxdj/mixxx/pull/11453)
* Linux: Log a warning when the audio thread is not scheduled with real-time policy
  [#11472](https://github.com/mixxxdj/mixxx/pull/11472)
  [#11465](https://github.com/mixxxdj/mixxx/issues/11465)
* Auto DJ: Fixes stop due to tracks with changed length
  [#11479](https://github.com/mixxxdj/mixxx/pull/11479)
  [#11492](https://github.com/mixxxdj/mixxx/pull/11492)
  [#11448](https://github.com/mixxxdj/mixxx/issues/11448)
  * Auto DJ: Fix Auto DJ indicator state when controlling it via  shortcut (SHIFT+F12)
  [#11494](https://github.com/mixxxdj/mixxx/issues/11494)
  [#11495](https://github.com/mixxxdj/mixxx/pull/11495)
* Fix building with Clang 15/16
  [#11490](https://github.com/mixxxdj/mixxx/pull/11490)
  [#11485](https://github.com/mixxxdj/mixxx/pull/11485)
* Fix EQ and waveforms analysis when compiling with GCC 13
  [#11501](https://github.com/mixxxdj/mixxx/pull/11501)
  [#11483](https://github.com/mixxxdj/mixxx/issues/11483)
  [#11502](https://github.com/mixxxdj/mixxx/pull/11502)
  [#11480](https://github.com/mixxxdj/mixxx/pull/11480)
  [#11488](https://github.com/mixxxdj/mixxx/pull/11488)
* Numark Mixtrack Pro FX: Fix sound output via WDM-KS on Windows
  [#11393](https://github.com/mixxxdj/mixxx/issues/11393)
* Fix crash on startup caused by faulty ASIO driver like FlexASIO 1.4 or Music Maker
  [#11426](https://github.com/mixxxdj/mixxx/issues/11426)
  [#10081](https://github.com/mixxxdj/mixxx/issues/10081)
* Windows: Show a loopback device that allows to mix in system sound
  [#11427](https://github.com/mixxxdj/mixxx/issues/11427)
  [#11451](https://github.com/mixxxdj/mixxx/issues/11451)
* Fix sorting via column header in external library features
  [#11491](https://github.com/mixxxdj/mixxx/issues/11491)
  [#11499](https://github.com/mixxxdj/mixxx/pull/11499)
  [#11498](https://github.com/mixxxdj/mixxx/pull/11498)
* Fix wrong waveform background color on recent Linux distributions like Fedora 37
  [#11164](https://github.com/mixxxdj/mixxx/issues/11164)
  [#11523](https://github.com/mixxxdj/mixxx/pull/11523)
* Serato Metadata: Don't import empty (black) cue points
  [#11534](https://github.com/mixxxdj/mixxx/pull/11534)
  [#11530](https://github.com/mixxxdj/mixxx/issues/11530)
  [#11467](https://github.com/mixxxdj/mixxx/pull/11467)
  [#11466](https://github.com/mixxxdj/mixxx/pull/11466)
  [#11283](https://github.com/mixxxdj/mixxx/issues/11283)
* Track context menu: Immediately adopt new position when resetting cues
  [#11482](https://github.com/mixxxdj/mixxx/pull/11482)
* Windows: Fix possible crash with faulty mp3 files
  [#11535](https://github.com/mixxxdj/mixxx/pull/11535)
  [#11531](https://github.com/mixxxdj/mixxx/issues/11531)
  [#11528](https://github.com/mixxxdj/mixxx/pull/11528)
  [#11521](https://github.com/mixxxdj/mixxx/issues/11521)

## [2.3.4](https://launchpad.net/mixxx/+milestone/2.3.4) (2023-03-03)

* Track Properties: Show 'date added' as local time [#4838](https://github.com/mixxxdj/mixxx/pull/4838) [#10776](https://github.com/mixxxdj/mixxx/issues/10776)
* Shade: Fix library sidebar splitter glitch [#4828](https://github.com/mixxxdj/mixxx/pull/4828) [#10757](https://github.com/mixxxdj/mixxx/issues/10757)
* LateNight: Add a border to the crossfader when Auto DJ is active. [#10913](https://github.com/mixxxdj/mixxx/pull/10913)
* LateNight: Isolate searchbar so maximize button is attached to tracks view. [#11132](https://github.com/mixxxdj/mixxx/pull/11132)
* macOS builds: Perform ad-hoc signing of macOS bundle in Pull request and personal repositories [#4774](https://github.com/mixxxdj/mixxx/pull/4774)
* Waveform: Avoid visual glitch with ranges < 1 px [#4804](https://github.com/mixxxdj/mixxx/pull/4804)
* Build Mixxx on macOS 11, replacing deprecated macOS 10.15 [#4863](https://github.com/mixxxdj/mixxx/pull/4863)
* Add macOS 13.0 (Ventura) support, by using portaudio 19.7.0  [#11046](https://github.com/mixxxdj/mixxx/pull/11046)
* EQ preferences: Properly restore 'One EQ for all decks' setting [#4886](https://github.com/mixxxdj/mixxx/pull/4886)
* Cover Art: Fix picking wrong cover file, when track file name contains extra dots [#4909](https://github.com/mixxxdj/mixxx/pull/4909)
* MusicBrainz: Respect rate limits [#10874](https://github.com/mixxxdj/mixxx/pull/10874) [#10795](https://github.com/mixxxdj/mixxx/issues/10795)
* MusicBrainz: Stop fetching after closing the dialog [#10878](https://github.com/mixxxdj/mixxx/pull/10878) [#10877](https://github.com/mixxxdj/mixxx/issues/10877)
* MusicBrainz: Fixed stalled GUI after client timeout [#10875](https://github.com/mixxxdj/mixxx/pull/10875) [#10883](https://github.com/mixxxdj/mixxx/issues/10883)
* macOs: Fix frozen skin control after Ctrl-Click [#10869](https://github.com/mixxxdj/mixxx/pull/10869) [#10831](https://github.com/mixxxdj/mixxx/issues/10831)
* Avoid redundant messages boxes after track loading fails [#10889](https://github.com/mixxxdj/mixxx/pull/10889)
* Use OpenGL VU meter widgets. This aims to improve performance with macOS.
  [#10893](https://github.com/mixxxdj/mixxx/pull/10893)
  [#11052](https://github.com/mixxxdj/mixxx/pull/11052)
  [#10979](https://github.com/mixxxdj/mixxx/pull/10979)
  [#10973](https://github.com/mixxxdj/mixxx/pull/10973)
  [#10983](https://github.com/mixxxdj/mixxx/pull/10983)
  [#11288](https://github.com/mixxxdj/mixxx/pull/11288)
* Prevent wild numbers from appearing during scratching under vinyl control. [#10916](https://github.com/mixxxdj/mixxx/pull/10916)
* Fixed a possible crash due to a race condition when editing cue points. [#10976](https://github.com/mixxxdj/mixxx/pull/10976) [#10689](https://github.com/mixxxdj/mixxx/issues/10689)
* Fixed a possible crash when overing cue point via mouse in the waveforms. [#10960](https://github.com/mixxxdj/mixxx/pull/10960) [#10956](https://github.com/mixxxdj/mixxx/issues/10956)
* History: Disallow dropping tracks. [#10969](https://github.com/mixxxdj/mixxx/pull/10969) [#10250](https://github.com/mixxxdj/mixxx/issues/10250)
* WTrackMenu: Sort crates and playlists like in sidebar. [#11023](https://github.com/mixxxdj/mixxx/pull/11023)
* WCoverArtLabel: Don't open full-size cover if no cover is loaded, to avoid an issue when closing. [#11022](https://github.com/mixxxdj/mixxx/pull/11022) [#11021](https://github.com/mixxxdj/mixxx/issues/11021)
* Removed integer truncation of the position when reading cue points from the database. [#10998](https://github.com/mixxxdj/mixxx/pull/10998)
* Fix cue points being assigned invalid value of -1.0 [#11000](https://github.com/mixxxdj/mixxx/pull/11000) [#10993](https://github.com/mixxxdj/mixxx/issues/10993)
* Auto DJ: Added a warning in a message box when it is started without decks with left and a right crossfader orientation [#11018](https://github.com/mixxxdj/mixxx/pull/11018)
* Fixed crash with FFmpeg decoder [#11044](https://github.com/mixxxdj/mixxx/pull/11044)
* Fixed issue with finding moved library tracks. [#11051](https://github.com/mixxxdj/mixxx/pull/11051)
* Preserve and synchronize ID3v1 tags (TagLib v1.12) [#11163](https://github.com/mixxxdj/mixxx/pull/11163) [#11123](https://github.com/mixxxdj/mixxx/issues/11123)
* Replay Gain Preferences: Fix the "adjust by" text in case of negative adjustments [#11176](https://github.com/mixxxdj/mixxx/pull/11176)
* macOs: Install Qt translation [#11134](https://github.com/mixxxdj/mixxx/pull/11134) [#11110](https://github.com/mixxxdj/mixxx/issues/11110)
* macOs: Fix assuming wrong system language [#11218](https://github.com/mixxxdj/mixxx/pull/11218) [#11195](https://github.com/mixxxdj/mixxx/issues/11195)
* Fix resetting track colors on metadata reimport (Serato metadata): [#11217](https://github.com/mixxxdj/mixxx/pull/11217) [#11213](https://github.com/mixxxdj/mixxx/issues/11213)
* Preferences: Fix incomplete version check in 2.3 during upgrade [#11229](https://github.com/mixxxdj/mixxx/pull/11229) [#9709](https://github.com/mixxxdj/mixxx/issues/9709)
* Allow search in external libraries [#11221](https://github.com/mixxxdj/mixxx/pull/11221) [#11216](https://github.com/mixxxdj/mixxx/issues/11216)
* JACK buffer size fix [#11121](https://github.com/mixxxdj/mixxx/pull/11121)
* Don't discard file tags with tuning information like "A#m +50" [#10992](https://github.com/mixxxdj/mixxx/pull/10992)
* Year search: Find also full date entries [#11251](https://github.com/mixxxdj/mixxx/pull/11251) [#11113](https://github.com/mixxxdj/mixxx/issues/11113)
* Fix visual alignment of beats and waveform in case of decoding issues [#11162](https://github.com/mixxxdj/mixxx/pull/11162)
* Avoid "active key-value observers" messages during skin parsing on macOS [#11265](https://github.com/mixxxdj/mixxx/pull/11265)
* Fullscreen on Linux: Fix issues caused by Ubuntu Unity workaround [#11295](https://github.com/mixxxdj/mixxx/pull/11295) [#11281](https://github.com/mixxxdj/mixxx/issues/11281) [#11294](https://github.com/mixxxdj/mixxx/issues/11294)

### New Controller Mappings

* Traktor Kontrol S2 Mk1: Add controller mapping [#3905](https://github.com/mixxxdj/mixxx/pull/3905)
* Numark Party Mix: Mapping added [#4720](https://github.com/mixxxdj/mixxx/pull/4720)

### Controller Fixes

* Traktor S3: Fix issues with sampler and hotcue buttons [#4676](https://github.com/mixxxdj/mixxx/pull/4676)
* Numark DJ2GO2: Fix sliders and knobs [#4835](https://github.com/mixxxdj/mixxx/pull/4835) [#10586](https://github.com/mixxxdj/mixxx/issues/10586)
* Numark DJ2Go2: Support HotCue clear with pad [#10841](https://github.com/mixxxdj/mixxx/pull/10841)
* Numark DJ2Go2: Fix inverted tempo fader [#10852](https://github.com/mixxxdj/mixxx/pull/10852) [#10586](https://github.com/mixxxdj/mixxx/issues/10586)
* Numark N4: Inverted pitch slider, to match the GUI orientation [#11057](https://github.com/mixxxdj/mixxx/pull/11046)
* Ableton Push: Show as one device [#10905](https://github.com/mixxxdj/mixxx/pull/10905)
* Denon DJ MC7000: off-by-one fix, soft-start/break effect, pitch play, 32 velocity samplers
  [#4902](https://github.com/mixxxdj/mixxx/pull/4902)
  [#4729](https://github.com/mixxxdj/mixxx/pull/4729)
* Potmeters: Add support for arbitrary maximums in 7-/14-bit handlers from controller scripts [#4495](https://github.com/mixxxdj/mixxx/pull/4495)
* Controller Preferences: Fix some usability issues [#10821](https://github.com/mixxxdj/mixxx/pull/10821)
* Controller mapping table: show readable/translated strings for script bindings [#11139](https://github.com/mixxxdj/mixxx/pull/11139)
* Control picker menu: Added loop_in/out_goto to list [#11133](https://github.com/mixxxdj/mixxx/pull/11133)

### Packaging

* Fix compatibility with FFmpeg 5.1 and require FFmpeg v4.1.9 [#10862](https://github.com/mixxxdj/mixxx/pull/10862) [#10866](https://github.com/mixxxdj/mixxx/pull/10866)
* Fix GCC 12.2.0 compatibility [#10863](https://github.com/mixxxdj/mixxx/pull/10863)
* Improve CMake 3.24 compatibility [#10864](https://github.com/mixxxdj/mixxx/pull/10864)
* Use MIXXX_VCPKG_ROOT cmake and environment variable to find the vcpkg environment [#10904](https://github.com/mixxxdj/mixxx/pull/10904)
* Fix `-Wswitch` when building with FLAC >= 1.4.0 [#10921](https://github.com/mixxxdj/mixxx/pull/10921)

## [2.3.3](https://launchpad.net/mixxx/+milestone/2.3.3) (2022-06-21)

* Pioneer DDJ-SB3: Fix controller breaking when releasing the shift button [#4659](https://github.com/mixxxdj/mixxx/pull/4659)
* Traktor S3: Push two deck switches to explicitly clone decks [#4665](https://github.com/mixxxdj/mixxx/pull/4665) [#4671](https://github.com/mixxxdj/mixxx/pull/4671) [#10660](https://github.com/mixxxdj/mixxx/issues/10660)
* Behringer DDM4000: Improve stability and add soft-takeover for encoder knobs [#4318](https://github.com/mixxxdj/mixxx/pull/4318) [#4799](https://github.com/mixxxdj/mixxx/pull/4799)
* Denon MC7000: Fix 'inverted shift' bug in the controller mapping [#4755](https://github.com/mixxxdj/mixxx/pull/4755)
* Fix spinback and break effect in the controller engine [#4708](https://github.com/mixxxdj/mixxx/pull/4708)
* Fix scratch on first wheel touch [#4761](https://github.com/mixxxdj/mixxx/pull/4761) [#9489](https://github.com/mixxxdj/mixxx/issues/9489)
* Preferences: Prevent controller settings being treated as changed even though they were not [#4721](https://github.com/mixxxdj/mixxx/pull/4721) [#10365](https://github.com/mixxxdj/mixxx/issues/10365)
* Fix rare crash when closing the progress dialog [#4695](https://github.com/mixxxdj/mixxx/pull/4695)
* Prevent preferences dialog from going out of screen [#4613](https://github.com/mixxxdj/mixxx/pull/4613)
* Fix undesired jump-cuts in Auto DJ [#4693](https://github.com/mixxxdj/mixxx/pull/4693) [#10592](https://github.com/mixxxdj/mixxx/issues/10592) [#10093](https://github.com/mixxxdj/mixxx/issues/10093)
* Fix bug that caused Auto DJ to stop playback after some time [#4698](https://github.com/mixxxdj/mixxx/pull/4698) [#10093](https://github.com/mixxxdj/mixxx/issues/10093) [#10670](https://github.com/mixxxdj/mixxx/issues/10670)
* Do not reset crossfader when Auto DJ is deactivated [#4714](https://github.com/mixxxdj/mixxx/pull/4714) [#10683](https://bugs.launchpad.net/bugs/1965298)
* Change the minimum Auto DJ transition time to -99 [#4768](https://github.com/mixxxdj/mixxx/pull/4768) [#10739](https://github.com/mixxxdj/mixxx/issues/10739)
* Samplers, crates, playlists: fix storing import/export paths [#4699](https://github.com/mixxxdj/mixxx/pull/4699) [#10679](https://bugs.launchpad.net/bugs/1964508)
* Library: keep hidden tracks in history [#4725](https://github.com/mixxxdj/mixxx/pull/4725)
* Broadcasting: allow multiple connections to same mount if only one is enabled [#4750](https://github.com/mixxxdj/mixxx/pull/4750) [#10727](https://github.com/mixxxdj/mixxx/issues/10727)
* Fix a rare mouse vanish bug when controlling knobs [#4744](https://github.com/mixxxdj/mixxx/pull/4744) [#6922](https://github.com/mixxxdj/mixxx/issues/6922) [#10715](https://github.com/mixxxdj/mixxx/issues/10715)
* Restore keylock from configuration and fix pitch ratio rounding issue [#4756](https://github.com/mixxxdj/mixxx/pull/4756) [#10518](https://github.com/mixxxdj/mixxx/issues/10518)
* Improve CSV export of playlists and crates and fix empty rating column [#4762](https://github.com/mixxxdj/mixxx/pull/4762)
* Fix passthrough-related crash in waveform code [#4789](https://github.com/mixxxdj/mixxx/pull/4789) [#4791](https://github.com/mixxxdj/mixxx/pull/4791) [#10650](https://github.com/mixxxdj/mixxx/issues/10650) [#10743](https://github.com/mixxxdj/mixxx/issues/10743)
* Passthrough: stop rendering waveforms and disable Cue/Play indicators [4793](https://github.com/mixxxdj/mixxx/pull/4793)

## [2.3.2](https://launchpad.net/mixxx/+milestone/2.3.2) (2022-01-31)

* Playlist: Enable sorting by color [#4352](https://github.com/mixxxdj/mixxx/pull/4352) [#10546](https://github.com/mixxxdj/mixxx/issues/10546)
* Fix crash when using Doubling/Halving/etc. BPM from track's Properties window on tracks without BPM [#4587](https://github.com/mixxxdj/mixxx/pull/4587) [#10625](https://github.com/mixxxdj/mixxx/issues/10625)
* Fix writing metadata on Windows for files that have never been played [#4586](https://github.com/mixxxdj/mixxx/pull/4586) [#10620](https://github.com/mixxxdj/mixxx/issues/10620)
* Preserve file creation time when writing metadata on Windows [#4586](https://github.com/mixxxdj/mixxx/pull/4586) [#10619](https://github.com/mixxxdj/mixxx/issues/10619)
* Fix handling of file extension when importing and exporting sampler settings [#4539](https://github.com/mixxxdj/mixxx/pull/4539)
* Fix crash when using an empty directory as resource path using the `--resource-path` command line option [#4575](https://github.com/mixxxdj/mixxx/pull/4575) [#10461](https://github.com/mixxxdj/mixxx/issues/10461)
* Pioneer DDJ-SB3: Add controller mapping [#3821](https://github.com/mixxxdj/mixxx/pull/3821)
* Don't wipe sound config during startup if configured devices are unavailable [#4544](https://github.com/mixxxdj/mixxx/pull/4544)
* Append selected file extension when exporting to playlist files [#4531](https://github.com/mixxxdj/mixxx/pull/4531) [#10066](https://github.com/mixxxdj/mixxx/issues/10066)
* Fix crash when using midi.sendShortMsg and platform vnc [#4635](https://github.com/mixxxdj/mixxx/pull/4635) [#10632](https://github.com/mixxxdj/mixxx/issues/10632)
* Traktor S3: Fix timedelta calculation bugs [#4646](https://github.com/mixxxdj/mixxx/pull/4646) [#10645](https://github.com/mixxxdj/mixxx/issues/10645)

### Packaging

* Downloads of external dependencies are placed in build/downloads
* The sources for libkeyfinder are now expected in build/downloads/libkeyfinder-2.2.6.zip instead of build/download/libkeyfinder/v2.2.6.zip
* CMake: Adjust the download directory and name of external dependencies [#4511](https://github.com/mixxxdj/mixxx/pull/4511)
* Fix/Improve Appstream metainfo
  [#4344](https://github.com/mixxxdj/mixxx/pull/4344)
  [#4346](https://github.com/mixxxdj/mixxx/pull/4346)
  [#4349](https://github.com/mixxxdj/mixxx/pull/4349)

## [2.3.1](https://launchpad.net/mixxx/+milestone/2.3.1) (2021-09-29)

* Added mapping for the Numark DJ2GO2 Touch controller [#4108](https://github.com/mixxxdj/mixxx/pull/4108) [#4287](https://github.com/mixxxdj/mixxx/pull/4287)
* Added mapping for the Numark Mixtrack Pro FX controller [#4160](https://github.com/mixxxdj/mixxx/pull/4160)
* Updated mapping for Behringer DDM4000 mixer [#4262](https://github.com/mixxxdj/mixxx/pull/4262)
* Updated mapping for Denon MC7000 controller [#4021](https://github.com/mixxxdj/mixxx/pull/4021)
* Hercules Inpulse 300: Add better FX controls and other minor improvements [#4246](https://github.com/mixxxdj/mixxx/pull/4246)
* Denon MC7000: Improve slip mode and jog wheel handling [#4021](https://github.com/mixxxdj/mixxx/pull/4021) [#4324](https://github.com/mixxxdj/mixxx/pull/4324)
* Disabled detection of keyboards and mice as HID controllers [#4243](https://github.com/mixxxdj/mixxx/pull/4243)
* Disabled detection of all HID controllers with Apple's vendor ID. Apple doesn't build actual controllers. [#4260](https://github.com/mixxxdj/mixxx/pull/4260) [#4273](https://github.com/mixxxdj/mixxx/pull/4273)
* Add support for HiDPI scale factors of 125% and 175% (only with Qt 5.14+) [#10485](https://github.com/mixxxdj/mixxx/issues/10485) [#4161](https://github.com/mixxxdj/mixxx/pull/4161)
* Fix Echo effect adding left channel samples to right channel [#4141](https://github.com/mixxxdj/mixxx/pull/4141)
* Fix bad phase seek when starting from preroll [#10423](https://github.com/mixxxdj/mixxx/issues/10423) [#4093](https://github.com/mixxxdj/mixxx/pull/4093)
* Fix bad phase seek when a channel's audible status changes [#4156](https://github.com/mixxxdj/mixxx/pull/4156)
* Tango skin: Show crossfader assign buttons by default [#4046](https://github.com/mixxxdj/mixxx/pull/4046)
* Fix keyfinder library in arm64 builds [#4047](https://github.com/mixxxdj/mixxx/pull/4047)
* Fix wrong track being recorded in History [#10454](https://github.com/mixxxdj/mixxx/issues/10454) [#4041](https://github.com/mixxxdj/mixxx/pull/4041) [#4059](https://github.com/mixxxdj/mixxx/pull/4059) [#4107](https://github.com/mixxxdj/mixxx/pull/4107) [#4296](https://github.com/mixxxdj/mixxx/pull/4296)
* Fix support for relative paths in the skin system which caused missing images in third-party skins [#4151](https://github.com/mixxxdj/mixxx/pull/4151)
* Fix relocation of directories with special/reserved characters in path name [#4146](https://github.com/mixxxdj/mixxx/pull/4146)
* Update keyboard shortcuts sheet [#4042](https://github.com/mixxxdj/mixxx/pull/4042)
* Library: resize the Played checkbox and BPM lock with the library font [#4050](https://github.com/mixxxdj/mixxx/pull/4050)
* Don't allow Input focus on waveforms [#4134](https://github.com/mixxxdj/mixxx/pull/4134)
* Fix performance issue on AArch64 by enabling flush-to-zero for floating-point arithmetic [#4144](https://github.com/mixxxdj/mixxx/pull/4144)
* Fix custom key notation not restored correctly after restart [#4136](https://github.com/mixxxdj/mixxx/pull/4136)
* Traktor S3: Disable scratch when switching decks to prevent locked scratch issue [#4073](https://github.com/mixxxdj/mixxx/pull/4073)
* FFmpeg: Ignore inaudible samples before start of stream [#4245](https://github.com/mixxxdj/mixxx/pull/4245)
* Controller Preferences: Don't automatically enable checkbox if controller is disabled [#4244](https://github.com/mixxxdj/mixxx/pull/4244) [#10503](https://github.com/mixxxdj/mixxx/issues/10503)
* Tooltips: Always show tooltips in preferences [#4198](https://github.com/mixxxdj/mixxx/pull/4198) [#9716](https://github.com/mixxxdj/mixxx/issues/9716)
* Tooltips: Use item label for tooltips in library side bar and show ID when debugging. [#4247](https://github.com/mixxxdj/mixxx/pull/4247)
* Library sidebar: Also activate items on PageUp/Down events. [#4237](https://github.com/mixxxdj/mixxx/pull/4237)
* Fix handling of preview button cell events in developer mode. [#4264](https://github.com/mixxxdj/mixxx/pull/4264) [#10418](https://github.com/mixxxdj/mixxx/issues/10418)
* Auto DJ: Fix bug which could make an empty track stop Auto DJ. [#4267](https://github.com/mixxxdj/mixxx/pull/4267) [#10504](https://github.com/mixxxdj/mixxx/issues/10504)
* Fix Auto DJ skipping tracks randomly [#4319](https://github.com/mixxxdj/mixxx/pull/4319) [#10505](https://github.com/mixxxdj/mixxx/issues/10505)
* Fix high CPU load due to extremely high internal sync clock values [#4312](https://github.com/mixxxdj/mixxx/pull/4312) [#10520](https://github.com/mixxxdj/mixxx/issues/10520)
* Fix preference option for re-analyzing beatgrids imported from other software [#4288](https://github.com/mixxxdj/mixxx/pull/4288)
* Fix wrong base tag used for deployment and displayed in About dialog [#4070](https://github.com/mixxxdj/mixxx/pull/4070)

### Packaging

* It is no longer necessary to manually copy the udev rule file in packaging scripts. Now pkg-config is used to determine the udevdir used to install the rules file in the CMake install step when CMAKE_INSTALL_PREFIX is `/` or `/usr`.  [#4126](https://github.com/mixxxdj/mixxx/pull/4126)
* Various build issues on FreeBSD are fixed [#4122](https://github.com/mixxxdj/mixxx/pull/4122) [#4123](https://github.com/mixxxdj/mixxx/pull/4123) [#4124](https://github.com/mixxxdj/mixxx/pull/4124)
* .desktop file has be renamed to org.mixxx.Mixxx.desktop according to Freedesktop standards [#4206](https://github.com/mixxxdj/mixxx/pull/4206)
* Uses system provided hidapi library if version >= 0.10.1 [#4215](https://github.com/mixxxdj/mixxx/pull/4215)
* Please update PortAudio to [19.7](https://github.com/PortAudio/portaudio/releases/tag/v19.7.0) if you have not done so already. This is required for Mixxx to work with PipeWire via the JACK API for many devices.
* Install multiple sizes of rasterized icons [#4204](https://github.com/mixxxdj/mixxx/pull/4204) [#4315](https://github.com/mixxxdj/mixxx/pull/4315) [#4254](https://github.com/mixxxdj/mixxx/pull/4254)
* CMake: Fixed detection of SoundTouch pkgconfig file and version [#4209](https://github.com/mixxxdj/mixxx/pull/4209)
* Fix AppStream metainfo [#4205](https://github.com/mixxxdj/mixxx/pull/4205) [#4317](https://github.com/mixxxdj/mixxx/pull/4317)

## [2.3.0](https://launchpad.net/mixxx/+milestone/2.3.0) (2021-06-28)

### Hotcues

* Add hotcue colors and custom labels by right clicking hotcue buttons or right clicking hotcues on overview waveforms [#2016](https://github.com/mixxxdj/mixxx/pull/2016) [#2520](https://github.com/mixxxdj/mixxx/pull/2520) [#2238](https://github.com/mixxxdj/mixxx/pull/2238) [#2560](https://github.com/mixxxdj/mixxx/pull/2560) [#2557](https://github.com/mixxxdj/mixxx/pull/2557) [#2362](https://github.com/mixxxdj/mixxx/pull/2362)
* Mouse hover cues on overview waveform to show time remaining until the cue [#2238](https://github.com/mixxxdj/mixxx/pull/2238)

### Hotcue & Track Colors

* Add configurable color per track [#2470](https://github.com/mixxxdj/mixxx/pull/2470) [#2539](https://github.com/mixxxdj/mixxx/pull/2539) [#2545](https://github.com/mixxxdj/mixxx/pull/2545) [#2630](https://github.com/mixxxdj/mixxx/pull/2630) [#6852](https://github.com/mixxxdj/mixxx/issues/6852)
* Add customizable color palettes for hotcue and track colors [#2530](https://github.com/mixxxdj/mixxx/pull/2530) [#2589](https://github.com/mixxxdj/mixxx/pull/2589) [#3749](https://github.com/mixxxdj/mixxx/pull/3749) [#2902](https://github.com/mixxxdj/mixxx/pull/2902)
* Add hotcue color find-and-replace tool [#2547](https://github.com/mixxxdj/mixxx/pull/2547)

### Importing From Other DJ Software

* Import cue points, track colors, and playlists from Serato file tags & database [#2480](https://github.com/mixxxdj/mixxx/pull/2480) [#2526](https://github.com/mixxxdj/mixxx/pull/2526) [#2499](https://github.com/mixxxdj/mixxx/pull/2499) [#2495](https://github.com/mixxxdj/mixxx/pull/2495) [#2673](https://github.com/mixxxdj/mixxx/pull/2673) [#3885](https://github.com/mixxxdj/mixxx/pull/3885)
  * Note: Mixxx does not yet support multiple loops per track. We are [working on this for Mixxx 2.4](https://github.com/mixxxdj/mixxx/pull/2194). In Mixxx 2.3, if you import a track with multiple loops from Serato, Mixxx will use the first loop cue as the single loop Mixxx currently supports. The imported loops are still stored in Mixxx's database and are treated as hotcues in Mixxx 2.3. If you do not delete these hotcues, they will be usable as loops in Mixxx 2.4. Serato keeps loops and hotcues in separate lists, but Mixxx does not, so loops from Serato are imported starting as hotcue 9.
* Import cue points, track colors, and playlists from Rekordbox USB drives [#2119](https://github.com/mixxxdj/mixxx/pull/2119) [#2555](https://github.com/mixxxdj/mixxx/pull/2555) [#2543](https://github.com/mixxxdj/mixxx/pull/2543) [#2779](https://github.com/mixxxdj/mixxx/pull/2779)
  * Note: The first Rekordbox memory cue is imported for the main cue button in Mixxx and the remaining Rekordbox memory cues are imported as Mixxx hotcues, starting with the next hotcue number after the last hotcue from Rekordbox.
  * Note: Mixxx does not yet support multiple loops per track. Imported loops from Rekordbox are treated like imported loops from Serato, so refer to the note above for details.

### Intro & Outro Cues

* Add intro & outro range cues with automatic silence detection [#1242](https://github.com/mixxxdj/mixxx/pull/1242)
* Show duration of intro & outro ranges on overview waveform [#2089](https://github.com/mixxxdj/mixxx/pull/2089)
* Use intro & outro cues in AutoDJ transitions [#2103](https://github.com/mixxxdj/mixxx/pull/2103)

### Deck cloning

* Add deck cloning (also known as "instant doubles" in other DJ software) by dragging and dropping between decks [#1892](https://github.com/mixxxdj/mixxx/pull/1892) and samplers [#3200](https://github.com/mixxxdj/mixxx/pull/3200)
* Clone decks by double pressing the load button on a controller (with option to disable this) [#2024](https://github.com/mixxxdj/mixxx/pull/2024) [#2042](https://github.com/mixxxdj/mixxx/pull/2042)

### Skins & GUI

* Aesthetically revamped LateNight skin [#2298](https://github.com/mixxxdj/mixxx/pull/2298) [#2342](https://github.com/mixxxdj/mixxx/pull/2342)
* Right click overview waveform to show time remaining until that point [#2238](https://github.com/mixxxdj/mixxx/pull/2238)
* Show track info dialog when double clicking track labels in decks [#2990](https://github.com/mixxxdj/mixxx/pull/2990)
* Show track context menu when right clicking text in decks [#2612](https://github.com/mixxxdj/mixxx/pull/2612) [#2675](https://github.com/mixxxdj/mixxx/pull/2675) [#2684](https://github.com/mixxxdj/mixxx/pull/2684) [#2696](https://github.com/mixxxdj/mixxx/pull/2696)
* Add laptop battery widget to skins [#2283](https://github.com/mixxxdj/mixxx/pull/2283) [#2277](https://github.com/mixxxdj/mixxx/pull/2277) [#2250](https://github.com/mixxxdj/mixxx/pull/2250) [#2228](https://github.com/mixxxdj/mixxx/pull/2228) [#2221](https://github.com/mixxxdj/mixxx/pull/2221) [#2163](https://github.com/mixxxdj/mixxx/pull/2163) [#2160](https://github.com/mixxxdj/mixxx/pull/2160) [#2147](https://github.com/mixxxdj/mixxx/pull/2147) [#2281](https://github.com/mixxxdj/mixxx/pull/2281) [#2319](https://github.com/mixxxdj/mixxx/pull/2319) [#2287](https://github.com/mixxxdj/mixxx/pull/2287)
* Show when passthrough mode is active on overview waveforms [#2575](https://github.com/mixxxdj/mixxx/pull/2575) [#2616](https://github.com/mixxxdj/mixxx/pull/2616)
* Changed format of currently playing track in window title from "artist, title" to "artist - title" [#2807](https://github.com/mixxxdj/mixxx/pull/2807)
* Workaround Linux skin change crash [#3144](https://github.com/mixxxdj/mixxx/pull/3144) [#10030](https://github.com/mixxxdj/mixxx/issues/10030)
* Fix touch control [#10108](https://github.com/mixxxdj/mixxx/issues/10108)
* Fix broken knob interaction on touchscreens [#3512](https://github.com/mixxxdj/mixxx/pull/3512)
* AutoDJ: Make "enable" shortcut work after startup [#3242](https://github.com/mixxxdj/mixxx/pull/3242)
* Add rate range indicator [#3693](https://github.com/mixxxdj/mixxx/pull/3693)
* Allow menubar to be styled [#3372](https://github.com/mixxxdj/mixxx/pull/3372) [#3788](https://github.com/mixxxdj/mixxx/pull/3788)
* Add Donate button to About dialog [#3838](https://github.com/mixxxdj/mixxx/pull/3838) [#3846](https://github.com/mixxxdj/mixxx/pull/3846)
* Add Scrollable Skin Widget [#3890](https://github.com/mixxxdj/mixxx/pull/3890)
* Fix minor visual issues in Skins [#3958](https://github.com/mixxxdj/mixxx/pull/3958/) [#3954](https://github.com/mixxxdj/mixxx/pull/3954/) [#3941](https://github.com/mixxxdj/mixxx/pull/3941/) [#3938](https://github.com/mixxxdj/mixxx/pull/3938/) [#3936](https://github.com/mixxxdj/mixxx/pull/3936/) [#3886](https://github.com/mixxxdj/mixxx/pull/3886/) [#3927](https://github.com/mixxxdj/mixxx/pull/3927/) [#3844](https://github.com/mixxxdj/mixxx/pull/3844/) [#3933](https://github.com/mixxxdj/mixxx/pull/3933/) [#3835](https://github.com/mixxxdj/mixxx/pull/3835/) [#3902](https://github.com/mixxxdj/mixxx/pull/3902) [#3931](https://github.com/mixxxdj/mixxx/pull/3931)

### Music Feature Analysis

* Multithreaded analysis for much faster batch analysis on multicore CPUs [#1624](https://github.com/mixxxdj/mixxx/pull/1624) [#2142](https://github.com/mixxxdj/mixxx/pull/2142) [#8686](https://github.com/mixxxdj/mixxx/issues/8686)
* Fix bugs affecting key detection accuracy [#2137](https://github.com/mixxxdj/mixxx/pull/2137) [#2152](https://github.com/mixxxdj/mixxx/pull/2152) [#2112](https://github.com/mixxxdj/mixxx/pull/2112) [#2136](https://github.com/mixxxdj/mixxx/pull/2136)
  * Note: Users who have not manually corrected keys are advised to clear all keys in their library by pressing Ctrl + A in the library, right clicking, going to Reset -> Key, then reanalyzing their library. This will freeze the GUI while Mixxx clears the keys; this is a known problem that we will not be able to fix for 2.3. Wait until it is finished and you will be able to reanalyze tracks for better key detection results.
* Remove VAMP plugin support and use Queen Mary DSP library directly. vamp-plugin-sdk and vamp-hostsdk are no longer required dependencies. [#926](https://github.com/mixxxdj/mixxx/pull/926)
* Improvements BPM detection on non-const beatgrids [#3626](https://github.com/mixxxdj/mixxx/pull/3626)
* Fix const beatgrid placement [#3965](https://github.com/mixxxdj/mixxx/pull/3965) [#3973](https://github.com/mixxxdj/mixxx/pull/3973)

### Music Library

* Add support for searching for empty fields (for example `crate:""`) [#9411](https://github.com/mixxxdj/mixxx/issues/9411)
* Improve synchronization of track metadata and file tags [#2406](https://github.com/mixxxdj/mixxx/pull/2406)
* Library Scanner: Improve hashing of directory contents [#2497](https://github.com/mixxxdj/mixxx/pull/2497)
* Rework of Cover Image Hashing [#8618](https://github.com/mixxxdj/mixxx/issues/8618) [#2507](https://github.com/mixxxdj/mixxx/pull/2507) [#2508](https://github.com/mixxxdj/mixxx/pull/2508)
* MusicBrainz: Handle 301 status response [#2510](https://github.com/mixxxdj/mixxx/pull/2510)
* MusicBrainz: Add extended metadata support [#8549](https://github.com/mixxxdj/mixxx/issues/8549) [#2522](https://github.com/mixxxdj/mixxx/pull/2522)
* TagLib: Fix detection of empty or missing file tags [#9891](https://github.com/mixxxdj/mixxx/issues/9891) [#2535](https://github.com/mixxxdj/mixxx/pull/2535)
* Fix caching of duplicate tracks that reference the same file [#3027](https://github.com/mixxxdj/mixxx/pull/3027)
* Use 6 instead of only 4 compatible musical keys (major/minor) [#3205](https://github.com/mixxxdj/mixxx/pull/3205)
* Fix possible crash when trying to refocus the tracks table while another Mixxx window has focus [#3201](https://github.com/mixxxdj/mixxx/pull/3201)
* Don't create new tags in file when exporting metadata to it [#3898](https://github.com/mixxxdj/mixxx/pull/3898)
* Fix playlist files beginning with non-english characters not being loaded [#3916](https://github.com/mixxxdj/mixxx/pull/3916)
* Enable sorting in "Hidden Tracks" and "Missing Tracks" views [#3828](https://github.com/mixxxdj/mixxx/pull/3828) [#9658](https://github.com/mixxxdj/mixxx/issues/9658/) [#10397](https://github.com/mixxxdj/mixxx/issues/10397/)
* Fix track table being empty after start [#3935](https://github.com/mixxxdj/mixxx/pull/3935/) [#10426](https://github.com/mixxxdj/mixxx/issues/10426/) [#10402](https://github.com/mixxxdj/mixxx/issues/10402/)

### Audio Codecs

* Add FFmpeg audio decoder, bringing support for ALAC files [#1356](https://github.com/mixxxdj/mixxx/pull/1356)
* Include LAME MP3 encoder with Mixxx now that the MP3 patent has expired [#7341](https://github.com/mixxxdj/mixxx/issues/7341) [buildserver:#37](https://github.com/mixxxdj/buildserver/pull/37) [buildserver:9e8bcee](https://github.com/mixxxdj/buildserver/commit/9e8bcee771731920ae82f3e076d43f0fb51e5027)
* Add Opus streaming and recording support. [#7530](https://github.com/mixxxdj/mixxx/issues/7530)
* Remove support for SoundSource plugins because the code was not well-maintained and could lead to crashes [#9435](https://github.com/mixxxdj/mixxx/issues/9435)
* Add HE-AAC encoding capabilities for recording and broadcasting [#3615](https://github.com/mixxxdj/mixxx/pull/3615)

### Audio Engine

* Fix loss of precision when dealing with floating-point sample positions while setting loop out position and seeking using vinyl control [#3126](https://github.com/mixxxdj/mixxx/pull/3126) [#3127](https://github.com/mixxxdj/mixxx/pull/3127)
* Prevent moving a loop beyond track end [#3117](https://github.com/mixxxdj/mixxx/pull/3117) [#9478](https://github.com/mixxxdj/mixxx/issues/9478)
* Fix possible memory corruption using JACK on Linux [#3160](https://github.com/mixxxdj/mixxx/pull/3160)
* Fix changing of vinyl lead-in time [#10319](https://github.com/mixxxdj/mixxx/issues/10319) [#3781](https://github.com/mixxxdj/mixxx/pull/3781)
* Fix tempo change of non-const beatgrid track on audible deck when cueing another track [#3772](https://github.com/mixxxdj/mixxx/pull/3772)
* Fix crash when changing effect unit routing [#3882](https://github.com/mixxxdj/mixxx/pull/3882) [#9331](https://github.com/mixxxdj/mixxx/issues/9331)
* Make microphone ducking use strength knob the same way in automatic & manual mode [#2750](https://github.com/mixxxdj/mixxx/pull/2750)

### Controllers

* Improve workflow for configuring controller mappings and editing mappings [#2569](https://github.com/mixxxdj/mixxx/pull/2569) [#3278](https://github.com/mixxxdj/mixxx/pull/3278) [#3667](https://github.com/mixxxdj/mixxx/pull/3667)
* Improve error reporting from controller scripts [#2588](https://github.com/mixxxdj/mixxx/pull/2588)
* Make hotcue and track colors mappable on controllers [#2030](https://github.com/mixxxdj/mixxx/pull/2030) [#2541](https://github.com/mixxxdj/mixxx/pull/2541) [#2665](https://github.com/mixxxdj/mixxx/pull/2665) [#2520](https://github.com/mixxxdj/mixxx/pull/2520)
* Add way to change library table sorting from controllers [#2118](https://github.com/mixxxdj/mixxx/pull/2118)
* Add support for velocity sensitive sampler buttons in Components JS library [#2032](https://github.com/mixxxdj/mixxx/pull/2032)
* Add logging when script ControlObject callback is disconnected successfully [#2054](https://github.com/mixxxdj/mixxx/pull/2054)
* Add controller mapping for Roland DJ-505 [#2111](https://github.com/mixxxdj/mixxx/pull/2111)
* Add controller mapping for Numark iDJ Live II [#2818](https://github.com/mixxxdj/mixxx/pull/2818)
* Add controller mapping for Hercules DJControl Inpulse 200 [#2542](https://github.com/mixxxdj/mixxx/pull/2542)
* Add controller mapping for Hercules DJControl Jogvision [#2370](https://github.com/mixxxdj/mixxx/pull/2370)
* Add controller mapping for Pioneer DDJ-200 [#3185](https://github.com/mixxxdj/mixxx/pull/3185) [#3193](https://github.com/mixxxdj/mixxx/pull/3193) [#3742](https://github.com/mixxxdj/mixxx/pull/3742) [#3793](https://github.com/mixxxdj/mixxx/pull/3793) [#3949](https://github.com/mixxxdj/mixxx/pull/3949)
* Add controller mapping for Pioneer DDJ-400 [#3479](https://github.com/mixxxdj/mixxx/pull/3479)
* Add controller mapping for ION Discover DJ Pro [#2893](https://github.com/mixxxdj/mixxx/pull/2893)
* Add controller mapping for Native Instrument Traktor Kontrol S3 [#3031](https://github.com/mixxxdj/mixxx/pull/3031)
* Add controller mapping for Behringer BCR2000 [#3342](https://github.com/mixxxdj/mixxx/pull/3342) [#3943](https://github.com/mixxxdj/mixxx/pull/3943)
* Add controller mapping for Behringer DDM4000 [#3542](https://github.com/mixxxdj/mixxx/pull/3542)
* Add controller mapping for Native Instruments Traktor Kontrol S4MK3 [#11284](https://github.com/mixxxdj/mixxx/pull/11284)
* Update controller mapping for Allen & Heath Xone K2 to add intro/outro cues [#2236](https://github.com/mixxxdj/mixxx/pull/2236)
* Update controller mapping for Hercules P32 for more accurate headmix control [#3537](https://github.com/mixxxdj/mixxx/pull/3537)
* Update controller mapping for Native Instruments Traktor Kontrol S4MK2 to add auto-slip mode and pitch fader range [#3331](https://github.com/mixxxdj/mixxx/pull/3331)
* Fix Pioneer DDJ-SB2 controller mapping auto tempo going to infinity bug [#2559](https://github.com/mixxxdj/mixxx/pull/2559) [#9838](https://github.com/mixxxdj/mixxx/issues/9838)
* Fix Numark Mixtrack Pro 3 controller mapping inverted FX on/off control [#3758](https://github.com/mixxxdj/mixxx/pull/3758)
* Gracefully handle MIDI overflow [#825](https://github.com/mixxxdj/mixxx/pull/825)

### Other

* Add CMake build system with `ccache` and `sccache` support for faster compilation times and remove SCons [#2280](https://github.com/mixxxdj/mixxx/pull/2280) [#3618](https://github.com/mixxxdj/mixxx/pull/3618)
* Make Mixxx compile even though `QT_NO_OPENGL` or `QT_OPENGL_ES_2` is defined (fixes build on Raspberry Pi) [#9887](https://github.com/mixxxdj/mixxx/issues/9887) [#2504](https://github.com/mixxxdj/mixxx/pull/2504)
* Fix ARM build issues [#3602](https://github.com/mixxxdj/mixxx/pull/3602)
* Fix missing manual in DEB package [#10070](https://github.com/mixxxdj/mixxx/issues/10070) [#2985](https://github.com/mixxxdj/mixxx/pull/2985)
* Add macOS codesigning and notarization to fix startup warnings [#3281](https://github.com/mixxxdj/mixxx/pull/3281)
* Don't trash user configuration if an error occurs when writing [#3192](https://github.com/mixxxdj/mixxx/pull/3192)
* Enable CUE sheet recording by default [#3374](https://github.com/mixxxdj/mixxx/pull/3374)
* Fix crash when double clicking GLSL waveforms with right mouse button [#3904](https://github.com/mixxxdj/mixxx/pull/3904)
* Derive Mixxx version from `git describe` [#3824](https://github.com/mixxxdj/mixxx/pull/3824) [#3841](https://github.com/mixxxdj/mixxx/pull/3841) [#3848](https://github.com/mixxxdj/mixxx/pull/3848)
* Improve tapping the BPM of a deck [#3790](https://github.com/mixxxdj/mixxx/pull/3790) [#10010](https://github.com/mixxxdj/mixxx/issues/10010)
* And countless other small fixes and improvements (too many to list them all!)

## [2.2.4](https://launchpad.net/mixxx/+milestone/2.2.4) (2020-06-27)

* Store default recording format after "Restore Defaults" [#9853](https://github.com/mixxxdj/mixxx/issues/9853) [#2414](https://github.com/mixxxdj/mixxx/pull/2414)
* Prevent infinite loop when decoding corrupt MP3 files [#2417](https://github.com/mixxxdj/mixxx/pull/2417)
* Add workaround for broken libshout versions [#2040](https://github.com/mixxxdj/mixxx/pull/2040) [#2438](https://github.com/mixxxdj/mixxx/pull/2438)
* Speed up purging of tracks [#9762](https://github.com/mixxxdj/mixxx/issues/9762) [#2393](https://github.com/mixxxdj/mixxx/pull/2393)
* Don't stop playback if vinyl passthrough input is configured and PASS button is pressed [#2474](https://github.com/mixxxdj/mixxx/pull/2474)
* Fix debug assertion for invalid crate names [#9871](https://github.com/mixxxdj/mixxx/issues/9871) [#2477](https://github.com/mixxxdj/mixxx/pull/2477)
* Fix crashes when executing actions on tracks that already disappeared from the DB [#2527](https://github.com/mixxxdj/mixxx/pull/2527)
* AutoDJ: Skip next track when both deck are playing [#7712](https://github.com/mixxxdj/mixxx/issues/7712) [#2531](https://github.com/mixxxdj/mixxx/pull/2531)
* Tweak scratch parameters for Mixtrack Platinum [#2028](https://github.com/mixxxdj/mixxx/pull/2028)
* Fix auto tempo going to infinity on Pioneer DDJ-SB2 [#2559](https://github.com/mixxxdj/mixxx/pull/2559)
* Fix bpm.tapButton logic and reject missed & double taps [#2594](https://github.com/mixxxdj/mixxx/pull/2594)
* Add controller mapping for Native Instruments Traktor Kontrol S2 MK3 [#2348](https://github.com/mixxxdj/mixxx/pull/2348)
* Add controller mapping for Soundless joyMIDI [#2425](https://github.com/mixxxdj/mixxx/pull/2425)
* Add controller mapping for Hercules DJControl Inpulse 300 [#2465](https://github.com/mixxxdj/mixxx/pull/2465)
* Add controller mapping for Denon MC7000 [#2546](https://github.com/mixxxdj/mixxx/pull/2546)
* Add controller mapping for Stanton DJC.4 [#2607](https://github.com/mixxxdj/mixxx/pull/2607)
* Fix broadcasting via broadcast/recording input [#9959](https://github.com/mixxxdj/mixxx/issues/9959) [#2743](https://github.com/mixxxdj/mixxx/pull/2743)
* Only apply ducking gain in manual ducking mode when talkover is enabled [#7668](https://github.com/mixxxdj/mixxx/issues/7668) [#8995](https://github.com/mixxxdj/mixxx/issues/8995) [#8795](https://github.com/mixxxdj/mixxx/issues/8795) [#2759](https://github.com/mixxxdj/mixxx/pull/2759)
* Ignore MIDI Clock Messages (0xF8) because they are not usable in Mixxx and inhibited the screensaver [#2786](https://github.com/mixxxdj/mixxx/pull/2786)

## [2.2.3](https://launchpad.net/mixxx/+milestone/2.2.3) (2019-11-24)

* Don't make users reconfigure sound hardware when it has not changed [#2253](https://github.com/mixxxdj/mixxx/pull/2253)
* Fix MusicBrainz metadata lookup [#9780](https://github.com/mixxxdj/mixxx/issues/9780) [#2328](https://github.com/mixxxdj/mixxx/pull/2328)
* Fix high DPI scaling of cover art [#2247](https://github.com/mixxxdj/mixxx/pull/2247)
* Fix high DPI scaling of cue point labels on scrolling waveforms [#2331](https://github.com/mixxxdj/mixxx/pull/2331)
* Fix high DPI scaling of sliders in Tango skin [#2318](https://github.com/mixxxdj/mixxx/pull/2318)
* Fix sound dropping out during recording [#9732](https://github.com/mixxxdj/mixxx/issues/9732) [#2265](https://github.com/mixxxdj/mixxx/pull/2265) [#2305](https://github.com/mixxxdj/mixxx/pull/2305) [#2308](https://github.com/mixxxdj/mixxx/pull/2308) [#2309](https://github.com/mixxxdj/mixxx/pull/2309)
* Fix rare crash on application shutdown [#2293](https://github.com/mixxxdj/mixxx/pull/2293)
* Workaround various rare bugs caused by database inconsistencies [#9773](https://github.com/mixxxdj/mixxx/issues/9773) [#2321](https://github.com/mixxxdj/mixxx/pull/2321)
* Improve handling of corrupt FLAC files [#2315](https://github.com/mixxxdj/mixxx/pull/2315)
* Don't immediately jump to loop start when loop_out is pressed in quantized mode [#9694](https://github.com/mixxxdj/mixxx/issues/9694) [#2269](https://github.com/mixxxdj/mixxx/pull/2269)
* Preserve order of tracks when dragging and dropping from AutoDJ to playlist [#9661](https://github.com/mixxxdj/mixxx/issues/9661) [#2237](https://github.com/mixxxdj/mixxx/pull/2237)
* Explicitly use X11 Qt platform plugin instead of Wayland in .desktop launcher [#9787](https://github.com/mixxxdj/mixxx/issues/9787) [#2340](https://github.com/mixxxdj/mixxx/pull/2340)
* Pioneer DDJ-SX: fix delayed sending of MIDI messages with low audio buffer sizes [#2326](https://github.com/mixxxdj/mixxx/pull/2326)
* Enable modplug support on Linux by default [#9719](https://github.com/mixxxdj/mixxx/issues/9719) [#2244](https://github.com/mixxxdj/mixxx/pull/2244) [#2272](https://github.com/mixxxdj/mixxx/pull/2272)
* Fix keyboard shortcut for View > Skin Preferences [#9796](https://github.com/mixxxdj/mixxx/issues/9796) [#2358](https://github.com/mixxxdj/mixxx/pull/2358) [#2372](https://github.com/mixxxdj/mixxx/pull/2372)
* Reloop Terminal Mix: Fix mapping of sampler buttons 5-8 [#9772](https://github.com/mixxxdj/mixxx/issues/9772) [#2330](https://github.com/mixxxdj/mixxx/pull/2330)

## [2.2.2](https://launchpad.net/mixxx/+milestone/2.2.2) (2019-08-10)

* Fix battery widget with upower <= 0.99.7. [#2221](https://github.com/mixxxdj/mixxx/pull/2221)
* Fix BPM adjust in BpmControl. [#9690](https://github.com/mixxxdj/mixxx/issues/9690)
* Disable track metadata export for .ogg files and TagLib 1.11.1. [#9680](https://github.com/mixxxdj/mixxx/issues/9680)
* Fix interaction of hot cue buttons and looping. [#9350](https://github.com/mixxxdj/mixxx/issues/9350)
* Fix detection of moved tracks. [#2197](https://github.com/mixxxdj/mixxx/pull/2197)
* Fix playlist import. [#2200](https://github.com/mixxxdj/mixxx/pull/2200) [#8852](https://github.com/mixxxdj/mixxx/issues/8852)
* Fix updating playlist labels. [#9697](https://github.com/mixxxdj/mixxx/issues/9697)
* Fix potential segfault on exit. [#9656](https://github.com/mixxxdj/mixxx/issues/9656)
* Fix parsing of invalid BPM values in MP3 files. [#9671](https://github.com/mixxxdj/mixxx/issues/9671)
* Fix crash when removing rows from empty model. [#2128](https://github.com/mixxxdj/mixxx/pull/2128)
* Fix high DPI scaling of RGB overview waveforms. [#2090](https://github.com/mixxxdj/mixxx/pull/2090)
* Fix for OpenGL SL detection on macOS. [#9653](https://github.com/mixxxdj/mixxx/issues/9653)
* Fix OpenGL ES detection. [#9636](https://github.com/mixxxdj/mixxx/issues/9636)
* Fix FX1/2 buttons missing Mic unit in Deere (64 samplers). [#9703](https://github.com/mixxxdj/mixxx/issues/9703)
* Tango64: Re-enable 64 samplers. [#2223](https://github.com/mixxxdj/mixxx/pull/2223)
* Numark DJ2Go re-enable note-off for deck A cue button. [#2087](https://github.com/mixxxdj/mixxx/pull/2087)
* Replace Flanger with QuickEffect in keyboard mapping. [#2233](https://github.com/mixxxdj/mixxx/pull/2233)

## [2.2.1](https://launchpad.net/mixxx/+milestone/2.2.1) (2019-04-22)

* Include all fixes from Mixxx 2.1.7 and 2.1.8
* Fix high CPU usage on MAC due to preview column [#9574](https://github.com/mixxxdj/mixxx/issues/9574)
* Fix HID controller output on Windows with common-hid-packet-parser.js
* Fix rendering slow down by not using QStylePainter in WSpinny [#8419](https://github.com/mixxxdj/mixxx/issues/8419)
* Fix broken Mic mute button [#9387](https://github.com/mixxxdj/mixxx/issues/9387)
* added quick effect enable button to the control picker menu
* Fix Cover Window close issue with empty cover arts
* Fix Numark Mixtrack 3 mapping. [#2057](https://github.com/mixxxdj/mixxx/pull/2057)

## [2.2.0](https://launchpad.net/mixxx/+milestone/2.2.0) (2018-12-17)

### General

* Update from Qt4 to Qt5.
* Use Qt5's automatic high DPI scaling (and remove the old
  scaling option from the preferences).
* Vectorize remaining raster graphics for better HiDPI support.

### Effects

* Add mix mode switch (Dry/Wet vs Dry+Wet) for effect units.
* Add support for LV2 effects plugins (currently no way to show plugin GUIs).
* Add preference option for selecting which effects are shown in the
  list of available effects in the main window (all LV2 effects are
  hidden by default and must be explicitly enabled by users).

### Skins

* Add 8 sampler and small sampler options to LateNight.
* Add key / BPM expansion indicators to Deere decks.
* Add skin settings menu to LateNight.

### Controllers

* Add controller mapping for Numark Mixtrack Platinum.
* Update controller mapping for Numark N4.
* Add spinback and break for Vestax VCI-400 mapping.

### Miscellaneous

* Add preference option to adjust the play position marker of
  scrolling waveforms.
* Add preference option to adjust opacity of beatgrid markers on
  scrolling waveforms.
* Support IRC/AIM/ICQ broadcast metadata.

## [2.1.8](https://launchpad.net/mixxx/+milestone/2.1.8) (2019-04-07)

* Fix a rare chance for a corrupt track file while writing metadata in out of disk situations. [lp:1815305](https://bugs.launchpad.net/mixxx/+bug/1815305)
* Fix export of BPM track file metadata. [#9593](https://github.com/mixxxdj/mixxx/issues/9593)
* Fix sending of broadcast metadata with TLS enabled libshout 2.4.1. [#9599](https://github.com/mixxxdj/mixxx/issues/9599)
* Fix resdicovering purged tracks in all cases. [#9616](https://github.com/mixxxdj/mixxx/issues/9616)
* Fix dropping track from OSX Finder. [#9620](https://github.com/mixxxdj/mixxx/issues/9620)

## [2.1.7](https://launchpad.net/mixxx/+milestone/2.1.7) (2019-01-15)

* Fix syncing to doublespeed [#9549](https://github.com/mixxxdj/mixxx/issues/9549)
* Fix issues when changing beats of a synced track [#9550](https://github.com/mixxxdj/mixxx/issues/9550)
* Fix direction of pitch bend buttons when inverting rate slider [#9284](https://github.com/mixxxdj/mixxx/issues/9284)
* Use first loaded deck if no playing deck is found [#9397](https://github.com/mixxxdj/mixxx/issues/9397)
* Encode file names correctly on macOS [lp:1776949](https://bugs.launchpad.net/mixxx/+bug/1776949)

## [2.1.6](https://launchpad.net/mixxx/+milestone/2.1.6) (2018-12-23)

* Fix crash when loading a Qt5 Soundsource / Vamp Plug-In. [#9324](https://github.com/mixxxdj/mixxx/issues/9324)
* Validate effect parameter range. [lp:1795234](https://bugs.launchpad.net/mixxx/+bug/1795234)
* Fix crash using the bpm_tap button without a track loaded. [#9512](https://github.com/mixxxdj/mixxx/issues/9512)
* Fix possible crash after ejecting a track. [#9513](https://github.com/mixxxdj/mixxx/issues/9513)
* Fix wrong bitrate reported for faulty mp3 files. [#9389](https://github.com/mixxxdj/mixxx/issues/9389)
* Fix Echo effect syncing [#9442](https://github.com/mixxxdj/mixxx/issues/9442)
* Fix iTunes context menu [#9484](https://github.com/mixxxdj/mixxx/issues/9484)
* Fix loading the wrong track after delete search and scroll. [#9519](https://github.com/mixxxdj/mixxx/issues/9519)
* Improve search bar timing. [#8665](https://github.com/mixxxdj/mixxx/issues/8665)
* Fix quoted search sentence. [#9396](https://github.com/mixxxdj/mixxx/issues/9396)
* Fix loading a track formerly not existing. [#9492](https://github.com/mixxxdj/mixxx/issues/9492)
* Fix importing m3u files with blank lines. [#9535](https://github.com/mixxxdj/mixxx/issues/9535)
* Fix position in sampler overview waveforms. [#9096](https://github.com/mixxxdj/mixxx/issues/9096)
* Don't reset rate slider, syncing a track without a beatgrid. [#9391](https://github.com/mixxxdj/mixxx/issues/9391)
* Clean up iTunes track context menu. [#9488](https://github.com/mixxxdj/mixxx/issues/9488)
* Collapsed sampler are not analyzed on startup. [#9502](https://github.com/mixxxdj/mixxx/issues/9502)
* search for decoration characters like "˚". [#9517](https://github.com/mixxxdj/mixxx/issues/9517)
* Fix cue button blinking after pressing eject on an empty deck. [#9543](https://github.com/mixxxdj/mixxx/issues/9543)

## [2.1.5](https://launchpad.net/mixxx/+milestone/2.1.5) (2018-10-28)

* Code signing for Windows builds. [#8309](https://github.com/mixxxdj/mixxx/issues/8309)
* Fix crash on exit when preferences is open. [#9438](https://github.com/mixxxdj/mixxx/issues/9438)
* Fix crash when analyzing corrupt MP3s. [#9443](https://github.com/mixxxdj/mixxx/issues/9443)
* Fix crash when importing metadata from MusicBrainz. [#9456](https://github.com/mixxxdj/mixxx/issues/9456)
* Library search fixes when single quotes are used. [#9395](https://github.com/mixxxdj/mixxx/issues/9395) [#9419](https://github.com/mixxxdj/mixxx/issues/9419)
* Fix scrolling waveform on Windows with WDM-KS sound API. [lp:1729345](https://bugs.launchpad.net/mixxx/+bug/1729345)
* Fix right clicking on beatgrid alignment button in Tango and LateNight skins. [#9471](https://github.com/mixxxdj/mixxx/issues/9471)
* Improve speed of importing iTunes library. [#9400](https://github.com/mixxxdj/mixxx/issues/9400)
* Add 2 deck mapping for DJTechTools MIDI Fighter Twister.

## [2.1.4](https://launchpad.net/mixxx/+milestone/2.1.4) (2018-08-29)

Fix track selection not getting shown in the track
table on Windows. There are no changes to the
source code, but the Jenkins build configuration
was changed to delete the Jenkins workspace before
each build. [lp:1751482](https://bugs.launchpad.net/mixxx/+bug/1751482)

## [2.1.3](https://launchpad.net/mixxx/+milestone/2.1.3) (2018-08-20)

Fix a severe performance regression on Windows:
[Mixxx 2.1.2 running much slower than 2.1.1](https://mixxx.org/forums/viewtopic.php?f=3&t=12082)

## [2.1.2](https://launchpad.net/mixxx/+milestone/2.1.2) (2018-08-10)

Yet another bugfix release of Mixxx 2.1.
Here is a quick summary of what is new in Mixxx 2.1.2:

* Allow maximum deck speed of 4x normal.
* Don't always quantize hotcues, a 2.1.1 regression. [#9345](https://github.com/mixxxdj/mixxx/issues/9345)
* Fix artifacts using more than 32 samplers. [#9363](https://github.com/mixxxdj/mixxx/issues/9363)
* store No EQ and Filter persistently. [#9376](https://github.com/mixxxdj/mixxx/issues/9376)
* Pad unreadable samples with silence on cache miss. [#9346](https://github.com/mixxxdj/mixxx/issues/9346)
* Fixing painting of preview column for Qt5 builds. [#9337](https://github.com/mixxxdj/mixxx/issues/9337)
* LateNight: Fix play button right click. [#9384](https://github.com/mixxxdj/mixxx/issues/9384)
* LateNight: Added missing sort up/down buttons.
* Fix sampler play button tooltips. [#9358](https://github.com/mixxxdj/mixxx/issues/9358)
* Shade: remove superfluid margins and padding in sampler.xml. [#9310](https://github.com/mixxxdj/mixxx/issues/9310)
* Deere: Fix background-color code.
* ITunes: Don't stop import in case of duplicated Playlists. [#9394](https://github.com/mixxxdj/mixxx/issues/9394)

## [2.1.1](https://launchpad.net/mixxx/+milestone/2.1.1) (2018-06-13)

After two months it is time to do a bugfix release of Mixxx 2.1.
Here is a quick summary of what is new in Mixxx 2.1.1:

* Require Soundtouch 2.0 to avoid segfault. [#8534](https://github.com/mixxxdj/mixxx/issues/8534)
* Improved skins including library view fix. [#9317](https://github.com/mixxxdj/mixxx/issues/9317) [#9297](https://github.com/mixxxdj/mixxx/issues/9297) [#9239](https://github.com/mixxxdj/mixxx/issues/9239)
* Fix crash when importing ID3v2 APIC frames. [#9325](https://github.com/mixxxdj/mixxx/issues/9325)
* Synchronize execution of Vamp analyzers. [#9085](https://github.com/mixxxdj/mixxx/issues/9085)
* DlgTrackInfo: Mismatching signal/slot connection.
* Detect M4A decoding errors on Windows. [#9266](https://github.com/mixxxdj/mixxx/issues/9266)
* Fix spinback inertia effect.
* Fix decoding fixes and upgrade DB schema. [#9255](https://github.com/mixxxdj/mixxx/issues/9255) [#9275](https://github.com/mixxxdj/mixxx/issues/9275)
* Fix integration of external track libraries. [#9264](https://github.com/mixxxdj/mixxx/issues/9264)
* Fix memory leak when loading cover art. [#9267](https://github.com/mixxxdj/mixxx/issues/9267)
* Fix clearing of ReplayGain gain/ratio in file tags. [#9256](https://github.com/mixxxdj/mixxx/issues/9256)
* Fix crash when removing a quick link. [#8270](https://github.com/mixxxdj/mixxx/issues/8270)
* Fidlib: Thread-safe and reentrant generation of filters. [#9247](https://github.com/mixxxdj/mixxx/issues/9247)
* Fix unresponsive scrolling through crates & playlists using encoder. [#8941](https://github.com/mixxxdj/mixxx/issues/8941)
* Swap default values for temp/perm rate changes. [#9243](https://github.com/mixxxdj/mixxx/issues/9243)

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
* Effects are processed post-fader and post-crossfader and can be previewed in headphones
* One metaknob per effect with customizable parameter control for intuitive use of effect chains
* Nine new effects: Autopan, Biquad Equalizer, Biquad Full Kill Equalizer, Loudness Contour, Metronome, Parametric Equalizer, Phaser, Stereo Balance, Tremolo
* Loaded effects and their parameters are saved and restored when Mixxx restarts
* More transparent sounding equalizers (Biquad Equalizer and Biquad Full Kill Equalizer)
* Improved scratching sounds with jog wheels, vinyl control, and dragging waveforms with the mouse
* Simplified looping and beatjump controls
* Configurable rows of 8 samplers with up to 8 rows available for a total of 64 samplers
* Files loaded to samplers are reloaded when Mixxx restarts
* Improved volume normalization algorithm (EBU-R 128)
* Filter library table by crates
* Sort musical keys in library table by circle of fifths
* Write metadata tags back to audio files
* New JavaScript library for controller mapping
* Configure multiple Internet broadcasting stations and use multiple stations at the same time
* Broadcast and record microphones with direct monitoring and latency compensation
* Broadcast and record from an external mixer
* Booth output with independent gain knob for using sound cards with 6 output channels without an external mixer
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

* Windows Vista, 7, and 8: [Start > Control Panel > Programs > Uninstall a
  Program](https://support.microsoft.com/en-us/help/2601726)
* Windows 10: [Start > Control Panel > Programs > Programs And Features >
  look for Mixxx > Uninstall](https://support.microsoft.com/en-gb/help/4028054/windows-repair-or-remove-programs-in-windows-10)

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

For a full list of new features and bugfixes, check out the
[2.1.0 milestone on Launchpad](https://launchpad.net/mixxx/+milestone/2.1.0).

## [2.0.0](https://launchpad.net/mixxx/+milestone/2.0.0) (2015-12-31)

* 4 Decks with Master Sync
* New Effects Framework with 4 Effect Units and 5 Built-in Effects:
  * Flanger, Bit Crusher, Reverb, Echo, Filter
  * More to come!
* Configurable, Resizable User Interface with 3 Brand New Skins
* Cover Art Display
* Music Key Detection and Shifting
* Vinyl Audio Pass-Through
* 4 Microphone inputs and 4 Auxiliary inputs
* MIDI Mapping GUI and Improved Learning Wizard
* MusicBrainz metadata fetching
* RGB Musical Waveforms
* Hundreds of Bug Fixes and Improvements
* New Pitch-Independent Algorithm for Better-Sounding Key-lock

For a full list of new features and bugfixes, check out the
[2.0.0 milestone on Launchpad](https://launchpad.net/mixxx/+milestone/2.0.0).
