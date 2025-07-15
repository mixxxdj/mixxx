# Changelog

## [2.7.0](https://github.com/mixxxdj/mixxx/milestone/47) (Unreleased)

## [2.6.0](https://github.com/mixxxdj/mixxx/milestone/44) (Unreleased)

### STEM file support

* Add simple support for STEM files [#13044](https://github.com/mixxxdj/mixxx/pull/13044)
* Add stem controls [#13086](https://github.com/mixxxdj/mixxx/pull/13086)
* Add analyser support for stem [#13106](https://github.com/mixxxdj/mixxx/pull/13106)
* Add quick effect support on stem [#13123](https://github.com/mixxxdj/mixxx/pull/13123)
* Add advanced stem loading COs [#13268](https://github.com/mixxxdj/mixxx/pull/13268)
* Multithreaded Rubberband
  [#13143](https://github.com/mixxxdj/mixxx/pull/13143)
  [#13649](https://github.com/mixxxdj/mixxx/pull/13649)
* Add support for stem in the engine
  [#13070](https://github.com/mixxxdj/mixxx/pull/13070)
  [#14244](https://github.com/mixxxdj/mixxx/pull/14244)
* Add stem files to the taglib lookup table [#13612](https://github.com/mixxxdj/mixxx/pull/13612)
* Stem controls for LateNight
  [#13537](https://github.com/mixxxdj/mixxx/pull/13537)
  [#14745](https://github.com/mixxxdj/mixxx/pull/14745)
* Fix: make "stem_group,mute" a powerwindow button
  [#13751](https://github.com/mixxxdj/mixxx/pull/13751)
  [#13749](https://github.com/mixxxdj/mixxx/issues/13749)
* Stem control test fix [#13960](https://github.com/mixxxdj/mixxx/pull/13960)
* Solves problem with special characters in path to stems [#13784](https://github.com/mixxxdj/mixxx/pull/13784)
* Enable FFmpeg (free) on Windows. [#14695](https://github.com/mixxxdj/mixxx/pull/14695)
* FFmpeg: Use internal aac decoder. If not available give a hint. [#14645](https://github.com/mixxxdj/mixxx/pull/14645)
* Fix build with -DSTEM=OFF [#13948](https://github.com/mixxxdj/mixxx/pull/13948)
* Fix warning when building without STEM support [#14551](https://github.com/mixxxdj/mixxx/pull/14551)
* Fix: exclude stem samples for QML waveform [#13655](https://github.com/mixxxdj/mixxx/pull/13655)
* Fix: use generic way to get the group on stem waveform renderer [#14291](https://github.com/mixxxdj/mixxx/pull/14291)

### Library

* Add color coding for Key column [#13390](https://github.com/mixxxdj/mixxx/pull/13390)
* Add Key Color Palettes [#13497](https://github.com/mixxxdj/mixxx/pull/13497)
* Add overview column with small waveform [#14140](https://github.com/mixxxdj/mixxx/pull/14140)
* Add a rebindable keyboard shortcut for editing items as a replacement for F2 [#13148](https://github.com/mixxxdj/mixxx/pull/13148)
* DeveloperTools: restore searchbar shortcut Ctrl+F
  [#14761](https://github.com/mixxxdj/mixxx/pull/14761)
* CmdlineArgs: Add `--rescan-library` for rescanning on startup [#13661](https://github.com/mixxxdj/mixxx/pull/13661)
* Add case-insensitive parsing for Lancelot key notation [#14318](https://github.com/mixxxdj/mixxx/pull/14318)
* iTunes: Add iOS importer using the Media Player framework [#12690](https://github.com/mixxxdj/mixxx/pull/12690)
* Add Shuffle action to track table header menu [#13392](https://github.com/mixxxdj/mixxx/pull/13392)
* Library scan: log summary and show popup
  [#13427](https://github.com/mixxxdj/mixxx/pull/13427)
  [#10720](https://github.com/mixxxdj/mixxx/issues/10720)
* Search: add BPM lock filter `bpm:locked`
  [#14590](https://github.com/mixxxdj/mixxx/pull/14590)
  [#14583](https://github.com/mixxxdj/mixxx/issues/14583)
* Track menu, purge: allow to hide further success popups in the current session [#13807](https://github.com/mixxxdj/mixxx/pull/13807)
* Track Info dialogs: move metadata buttons below color picker [#13632](https://github.com/mixxxdj/mixxx/pull/13632)
* Track File Export: add 'Apply to all' checkbox, remove ".. All" buttons [#13614](https://github.com/mixxxdj/mixxx/pull/13614)
* Fix: restore BPM and Bitrate column width [#13571](https://github.com/mixxxdj/mixxx/pull/13571)
* Elide key text from the right [#13475](https://github.com/mixxxdj/mixxx/pull/13475)
* Playlists: add 'Unlock all' and 'Delete all unlocked' menu actions
  [#14091](https://github.com/mixxxdj/mixxx/pull/14091)
  [#8960](https://github.com/mixxxdj/mixxx/issues/8960)

### Effects

* Compressor effect: Adjust Makeup Time constant calculation [#13261](https://github.com/mixxxdj/mixxx/pull/13261)
[#13237](https://github.com/mixxxdj/mixxx/issues/13237)
* Fix: prevent quickFX model out of bound [#13668](https://github.com/mixxxdj/mixxx/pull/13668)

### Waveforms

* Simplify waveform combobox in preferences
  [#13220](https://github.com/mixxxdj/mixxx/issues/13220)
  [#6428](https://github.com/mixxxdj/mixxx/issues/6428)
  [#13226](https://github.com/mixxxdj/mixxx/issues/13226)
* Add minute markers on horizontal waveform overview
  [#13401](https://github.com/mixxxdj/mixxx/pull/13401)
  [#5843](https://github.com/mixxxdj/mixxx/issues/5843)
  [#13648](https://github.com/mixxxdj/mixxx/pull/13648)
  [#13489](https://github.com/mixxxdj/mixxx/pull/13489)
* Add slip waveform to Textured/'High details' type [#14039](https://github.com/mixxxdj/mixxx/pull/14039)
* Disable textured waveforms when using OpenGL ES
  [#13381](https://github.com/mixxxdj/mixxx/pull/13381)
  [#13380](https://github.com/mixxxdj/mixxx/issues/13380)
* Waveform Overview: Scale by ReplayGain
  [#14309](https://github.com/mixxxdj/mixxx/pull/14309)
  [#14331](https://github.com/mixxxdj/mixxx/pull/14331)
* feat: improve screen rendering framework [#13737](https://github.com/mixxxdj/mixxx/pull/13737)
* Rendergraph: Add rendergraph library and use if for waveform rendering
  [#14007](https://github.com/mixxxdj/mixxx/pull/14007)
  [#14021](https://github.com/mixxxdj/mixxx/pull/14021)
  [#14191](https://github.com/mixxxdj/mixxx/pull/14191)
  [#14185](https://github.com/mixxxdj/mixxx/pull/14185)
  [#14188](https://github.com/mixxxdj/mixxx/pull/14188)
  [#14192](https://github.com/mixxxdj/mixxx/pull/14192)
  [#14190](https://github.com/mixxxdj/mixxx/pull/14190)
  [#14186](https://github.com/mixxxdj/mixxx/pull/14186)
  [#14189](https://github.com/mixxxdj/mixxx/pull/14189)
  [#14187](https://github.com/mixxxdj/mixxx/pull/14187)
  [#13470](https://github.com/mixxxdj/mixxx/pull/13470)
  [#14461](https://github.com/mixxxdj/mixxx/pull/14461)
  [#14726](https://github.com/mixxxdj/mixxx/pull/14726)
  [#14706](https://github.com/mixxxdj/mixxx/issues/14706)
* Improve apperrance of marks on the waveforms [#13969](https://github.com/mixxxdj/mixxx/pull/13969)
* ControllerRenderingEngine: Patch out unavailable APIs when using GL ES [#13382](https://github.com/mixxxdj/mixxx/pull/13382)
* Fix high details waveforms wrapping around after visual index 65K [#13491](https://github.com/mixxxdj/mixxx/pull/13491)
* Fix: support for new WaveformData struct in shaders
  [#13474](https://github.com/mixxxdj/mixxx/pull/13474)
  [#13472](https://github.com/mixxxdj/mixxx/issues/13472)
* Fix: remove scaleSignal in waveform analyzer [#13416](https://github.com/mixxxdj/mixxx/pull/13416)
* Fix: prevent double free on DigitsRenderer [#13859](https://github.com/mixxxdj/mixxx/pull/13859)
* Fix: waveform overview seeking
  [#13947](https://github.com/mixxxdj/mixxx/pull/13947)
  [#13946](https://github.com/mixxxdj/mixxx/issues/13946)
* Fix invalid slip render marker [#13422](https://github.com/mixxxdj/mixxx/pull/13422)
* Fix waveform marker image alignment
  [#14656](https://github.com/mixxxdj/mixxx/pull/14656)
  [#14037](https://github.com/mixxxdj/mixxx/issues/14037)

### Auto-DJ

* Add transition mode 'Skip Silence, Start with Xfader centered' [#13628](https://github.com/mixxxdj/mixxx/pull/13628)
* Add crossafder recenter option when turning off (default off)
  [#13303](https://github.com/mixxxdj/mixxx/pull/13303)
  [#11571](https://github.com/mixxxdj/mixxx/issues/11571)
* Add context menu action for enabling/disabling the Auto DJ [#13593](https://github.com/mixxxdj/mixxx/pull/13593)

### Controller Mappings

* Behringer DDM4000 & BCR2000: Remove XML input declarations from mapping [#14285](https://github.com/mixxxdj/mixxx/pull/14285)
* Hercules DJ Control Starlight: Add EffectChain superknob control [#14126](https://github.com/mixxxdj/mixxx/pull/14126)
* Numark Mixtrack 3: Update scripts [#14193](https://github.com/mixxxdj/mixxx/pull/14193)
* Traktor S3: Small updates and fixes [#14340](https://github.com/mixxxdj/mixxx/pull/14340)

### Controller Backend

* Add screen renderer to support controllers with a screen
  [#11407](https://github.com/mixxxdj/mixxx/pull/11407)
  [#13334](https://github.com/mixxxdj/mixxx/pull/13334)
* Deprecate `lodash.mixxx.js`, and `script.deepMerge` [#13460](https://github.com/mixxxdj/mixxx/pull/13460)
* Add New CO "beats_translate_half" to move beatgrid a half beat
  [#14279](https://github.com/mixxxdj/mixxx/pull/14279)
  [#10811](https://github.com/mixxxdj/mixxx/issues/10811)
* Settings: Add a file and color controller setting types [#13669](https://github.com/mixxxdj/mixxx/pull/13669)
* Allow to enable MIDI Through Port in non-developer sessions [#13909](https://github.com/mixxxdj/mixxx/pull/13909)
* Refactor: modernize softtakeover code [#13553](https://github.com/mixxxdj/mixxx/pull/13553)
* document `ScriptConnection` readonly properties & slight cleanup [#13630](https://github.com/mixxxdj/mixxx/pull/13630)
* Modernize Hid/Bulk Lists [#13622](https://github.com/mixxxdj/mixxx/pull/13622)
* Prevent deadlock with BULK transfer and reduce log noise [#13735](https://github.com/mixxxdj/mixxx/pull/13735)
* Expose convertCharset convenience function to controllers
  [#13935](https://github.com/mixxxdj/mixxx/pull/13935)
  [#14108](https://github.com/mixxxdj/mixxx/pull/14108)
* Add HID error message upon failed open [#14184](https://github.com/mixxxdj/mixxx/pull/14184)
* Remove boilerplate and duplication in controller setting definition [#13920](https://github.com/mixxxdj/mixxx/pull/13920)
* Allow feedback on every release of a `powerWindow` button [#14335](https://github.com/mixxxdj/mixxx/pull/14335)
* Controller Settings: Improve click event filter [#14355](https://github.com/mixxxdj/mixxx/pull/14355)
* Controller Settings: Add a collapsible group box [#14324](https://github.com/mixxxdj/mixxx/pull/14324)
* Fix: Don't return in JogWheelBasic on deck absent in option
  [#13425](https://github.com/mixxxdj/mixxx/pull/13425)
  [#14106](https://github.com/mixxxdj/mixxx/pull/14106)

### Engine

* Fix: sync rate using the current BPM instead of the file one
  [#13671](https://github.com/mixxxdj/mixxx/pull/13671)
  [#12738](https://github.com/mixxxdj/mixxx/issues/12738)
* Sync: prefer playing inaudible decks over stopped non-sync decks [#14580](https://github.com/mixxxdj/mixxx/pull/14580)
* Fix: prevent null CO access when cloning sampler or preview [#13740](https://github.com/mixxxdj/mixxx/pull/13740)
* Use correct detected channel count on CoreAudio [#14372](https://github.com/mixxxdj/mixxx/pull/14372)
* Fix: revert to max. 36 hotcues, fix one-off issue [#14753](https://github.com/mixxxdj/mixxx/pull/14753)

### Preferences

* Waveforms: Group options, adjust tabstops, reorder ui file [#13615](https://github.com/mixxxdj/mixxx/pull/13615)
* Controllers: Make extended controller information available for device selection
* Controllers: Reorganize content into tabs
* Mixer: Show 'real' crossfader configuration [#14124](https://github.com/mixxxdj/mixxx/pull/14124)
* Mixer: Fix crossader graph [#13848](https://github.com/mixxxdj/mixxx/pull/13848)
  [#13896](https://github.com/mixxxdj/mixxx/pull/13896)
  [#14006](https://github.com/mixxxdj/mixxx/pull/14006)
  [#14354](https://github.com/mixxxdj/mixxx/pull/14354)
* Effects: Left/Right key in effect lists trigger hide/unhide [#14205](https://github.com/mixxxdj/mixxx/pull/14205)
* Sound Hardware: Open with sprecific I/O tab selected [#14346](https://github.com/mixxxdj/mixxx/pull/14346)
* Sound Hardware: Don't set m_settingsModified in update slots [#13450](https://github.com/mixxxdj/mixxx/pull/13450)
* Library, Track Search: Fix accidental use of wrong preference controls [#13592](https://github.com/mixxxdj/mixxx/pull/13592)

### Skins

* Allow swapping hotcues via dragging and dropping hotcue buttons
  [#13394](https://github.com/mixxxdj/mixxx/pull/13394)
  [#14367](https://github.com/mixxxdj/mixxx/pull/14367)
* Add controls to order Hotcues by position in the track
  [#13808](https://github.com/mixxxdj/mixxx/pull/13808)
  [#14423](https://github.com/mixxxdj/mixxx/pull/14423)
* Drop Hotcue or main Cue button onto Play button to latch `play`
  [#14179](https://github.com/mixxxdj/mixxx/pull/14179)
  [#14178](https://github.com/mixxxdj/mixxx/pull/14178)
  [#14753](https://github.com/mixxxdj/mixxx/pull/14753)
* Always show tooltips if Ctrl key is pressed [#14078](https://github.com/mixxxdj/mixxx/pull/14078)
* Update waveforms_container.xml [#13501](https://github.com/mixxxdj/mixxx/pull/13501)
* LegacySkinParser: Short-circuit if template fails to open [#13488](https://github.com/mixxxdj/mixxx/pull/13488)
* Tooltips: Fix cue mode setting location [#14045](https://github.com/mixxxdj/mixxx/pull/14045)

### Experimental Features

* SoundManagerIOS: Remove unsupported/redundant options [#13487](https://github.com/mixxxdj/mixxx/pull/13487)
* ControllerRenderingEngine: Disable BGRA when targeting Wasm [#13502](https://github.com/mixxxdj/mixxx/pull/13502)
* BaseTrackTableModel: Disable inline track editing on iOS [#13494](https://github.com/mixxxdj/mixxx/pull/13494)
* Set QQuickStyle to "basic" [#13696](https://github.com/mixxxdj/mixxx/pull/13696)
  [#13600](https://github.com/mixxxdj/mixxx/issues/13600)
* Fix: trigger QML waveform slot at init [#13736](https://github.com/mixxxdj/mixxx/pull/13736)
* CoreServices: Default to `~/Music` as a music directory on WASM and iOS [#13498](https://github.com/mixxxdj/mixxx/pull/13498)
* CMakeLists: Disable `QTlsBackendOpenSSLPlugin` on iOS [#14375](https://github.com/mixxxdj/mixxx/pull/14375)

### Target support

* Sound preferences: Add missing ifdefs for building without Rubberband [#13577](https://github.com/mixxxdj/mixxx/pull/13577)
* Update Linux-GitHub runner to Ubuntu 24.04.01 LTS
  [#13781](https://github.com/mixxxdj/mixxx/pull/13781)
  [#13880](https://github.com/mixxxdj/mixxx/pull/13880)
* Debiam: Add missing qt6-declarative-private-dev and qt6-base-private-dev package
  [#13904](https://github.com/mixxxdj/mixxx/pull/13904)
* RPM: Add missing deps [#14183](https://github.com/mixxxdj/mixxx/pull/14183)
* Show translator file path in debug message [#14209](https://github.com/mixxxdj/mixxx/pull/14209)
* Building without tests-tools [#14268](https://github.com/mixxxdj/mixxx/pull/14268)
* Remove unmaintained shell.nix [#14300](https://github.com/mixxxdj/mixxx/pull/14300)
* Add QGLES2 option for UNIX [#14489](https://github.com/mixxxdj/mixxx/pull/14489)
* Don't set GL_BGRA if QT_OPENGL_ES_2 [#14488](https://github.com/mixxxdj/mixxx/pull/14488)
* Windows and macOS: Update to Qt 6.8.3 (requires MSVC 2022) [#14655](https://github.com/mixxxdj/mixxx/pull/14655)

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
* Bump actions/upload-artifact from 4.3.3 to 4.6.0 [#14167](https://github.com/mixxxdj/mixxx/pull/14167)
* Bump azure/trusted-signing-action from 0.5.0 to 0.5.1 [#14168](https://github.com/mixxxdj/mixxx/pull/14168)
* Bump coverallsapp/github-action from 2.3.4 to 2.3.6 [#14246](https://github.com/mixxxdj/mixxx/pull/14246)
* Bump actions/upload-artifact from 4.6.0 to 4.6.1 [#14406](https://github.com/mixxxdj/mixxx/pull/14406)
* chore: update the donate button label [#13353](https://github.com/mixxxdj/mixxx/pull/13353)
* WPixmapStore: Change getPixmapNoCache to std::unique_ptr and further optimizations [#13369](https://github.com/mixxxdj/mixxx/pull/13369)
* Remove unused setSVG and hash functionality from pixmapsource [#13423](https://github.com/mixxxdj/mixxx/pull/13423)
* Remove FAQ from Readme.md [#13453](https://github.com/mixxxdj/mixxx/pull/13453)
* [#13452](https://github.com/mixxxdj/mixxx/pull/13452)
* Make Paintable::DrawMode an enum class [#13424](https://github.com/mixxxdj/mixxx/pull/13424)
* Paintable cleanup [#13435](https://github.com/mixxxdj/mixxx/pull/13435)
* hash clean up [#13458](https://github.com/mixxxdj/mixxx/pull/13458)
* BaseTrackTableModel: Fix `-Wimplicit-fallthrough` warning on GCC 14.1.1 [#13505](https://github.com/mixxxdj/mixxx/pull/13505)
* Refactor: fix trivial cpp coreguideline violations [#13552](https://github.com/mixxxdj/mixxx/pull/13552)
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
* Refactor `MovingInterquartileMean` [#13730](https://github.com/mixxxdj/mixxx/pull/13730)
* Improved comments in enginecontrol and use of std::size_t for bufferSize across the codebase [#13819](https://github.com/mixxxdj/mixxx/pull/13819)
* Refactor: use higher-level `std::span` based logic [#13654](https://github.com/mixxxdj/mixxx/pull/13654)
* VSyncThread: tsan fix pll vars data race [#13873](https://github.com/mixxxdj/mixxx/pull/13873)
* Control Indicator: Use atomic to fix tsan detected data race condition of blink value [#13875](https://github.com/mixxxdj/mixxx/pull/13875)
* Fix undefined behaviour of infinity() [#13884](https://github.com/mixxxdj/mixxx/pull/13884)
* Use atomic for m_bWakeScheduler, protect m_bQuit with mutex [#13898](https://github.com/mixxxdj/mixxx/pull/13898)
* Refactor `ValueTransformer` and `WBaseWidget` [#13853](https://github.com/mixxxdj/mixxx/pull/13853)
* Avoid data race on m_pStream [#13899](https://github.com/mixxxdj/mixxx/pull/13899)
* Cleanup and deprecate more `util/` classes
  [#13687](https://github.com/mixxxdj/mixxx/pull/13687)
  [#13968](https://github.com/mixxxdj/mixxx/pull/13968)
  [#13965](https://github.com/mixxxdj/mixxx/issues/13965)
  [#14107](https://github.com/mixxxdj/mixxx/pull/14107)
  [#14095](https://github.com/mixxxdj/mixxx/issues/14095)
  [#14087](https://github.com/mixxxdj/mixxx/pull/14087)
  [#14086](https://github.com/mixxxdj/mixxx/issues/14086)
* Github CI(clang-format): Indent Objective-C blocks with 4 spaces [#13503](https://github.com/mixxxdj/mixxx/pull/13503)
* Github CI(pre-commit): Add cmake-lint hook [#13932](https://github.com/mixxxdj/mixxx/pull/13932)
* Github CI(labeler): Add `developer experience` issue label [#14343](https://github.com/mixxxdj/mixxx/pull/14343)
* Github CI(labeler): add Dev Tools to `developer experience` [#14475](https://github.com/mixxxdj/mixxx/pull/14475)
* Refactor: remove samplew_autogen.h
  [#13988](https://github.com/mixxxdj/mixxx/pull/13988)
  [#14005](https://github.com/mixxxdj/mixxx/pull/14005)
* Fix clang-tidy complain [#14029](https://github.com/mixxxdj/mixxx/pull/14029)
* Github CI(dependabot): Open PRs against 2.5 branch instead of main [#14060](https://github.com/mixxxdj/mixxx/pull/14060)
* Happy New Year 2025! [#14098](https://github.com/mixxxdj/mixxx/pull/14098)
* Fix warning in Auto DJ test  [#14102](https://github.com/mixxxdj/mixxx/pull/14102)
* Fix: Add `QT_VERSION_CHECK`ed `QCheckBox::checkStateChanged` handlers [#14104](https://github.com/mixxxdj/mixxx/pull/14104)
* Remove warning introduced in [#13339](https://github.com/mixxxdj/mixxx/pull/13339) [#14109](https://github.com/mixxxdj/mixxx/pull/14109)
* Fix wrong access to ENV var MIXXX_VCPKG_ROOT instead of CMake setting MIXXX_VCPKG_ROOT [#14146](https://github.com/mixxxdj/mixxx/pull/14146)
* WOverview: remove unused coefficients [#14145](https://github.com/mixxxdj/mixxx/pull/14145)
* Fix missing initialization in Rotary() [#14176](https://github.com/mixxxdj/mixxx/pull/14176)
* WPushButton: remove obsolete focusOutEvent() [#14177](https://github.com/mixxxdj/mixxx/pull/14177)
* Fix memory leak and use parented_ptr in WTrackMenu [#14199](https://github.com/mixxxdj/mixxx/pull/14199)
* Fix Clazy warning in main [#14241](https://github.com/mixxxdj/mixxx/pull/14241)
* Fix recently introduced clazy warnings [#14336](https://github.com/mixxxdj/mixxx/pull/14336)
* Fix calculation of m_resourcePath in the testing case [#14110](https://github.com/mixxxdj/mixxx/pull/14110)
* Add borrowable_ptr, a threadsafe callback solution [#1713](https://github.com/mixxxdj/mixxx/pull/1713)
* Include QtConcurrentRun [#14303](https://github.com/mixxxdj/mixxx/pull/14303)
* Optimized tooltip generation in WBaseWidget [#13952](https://github.com/mixxxdj/mixxx/pull/13952)
* Github CI(pre-commit): qsscheck.py -> added utf-8 in open() [#14320](https://github.com/mixxxdj/mixxx/pull/14320)
* Small grammar fix for comment in `BpmControl::slotUpdateRateSlider` [#14344](https://github.com/mixxxdj/mixxx/pull/14344)
* Add links to important guidelines to CONTRIBUTING.md [#14342](https://github.com/mixxxdj/mixxx/pull/14342)
* Log Test fixes and refactoring.  [#14111](https://github.com/mixxxdj/mixxx/pull/14111)
* chore: lint CMakeLists.txt [#14369](https://github.com/mixxxdj/mixxx/pull/14369)
* RateControl/Position ScratchController: use std::unique_ptr, PollingControlProxy etc. [#14058](https://github.com/mixxxdj/mixxx/pull/14058)
* Controller preferences: Cond-compile out HID settings when building without HID [#14376](https://github.com/mixxxdj/mixxx/pull/14376)
* Add missing space to engine controller API documentation [#14384](https://github.com/mixxxdj/mixxx/pull/14384)
* Fix prettier pre commit [#14416](https://github.com/mixxxdj/mixxx/pull/14416)
* Use std::shared_ptr in  controller settings to fix memory leak [#14413](https://github.com/mixxxdj/mixxx/pull/14413)
* clean up README.md [#14471](https://github.com/mixxxdj/mixxx/pull/14471)
* Fix type safety warnings [#14613](https://github.com/mixxxdj/mixxx/pull/14613)
* CMake: Join project() with enable_language() [#14577](https://github.com/mixxxdj/mixxx/pull/14577)
* Scenegraph: condition to QML=ON  [#14487](https://github.com/mixxxdj/mixxx/pull/14487)
* Fix building with Qt 6.9 [#14678](https://github.com/mixxxdj/mixxx/pull/14678)
* Fix: import proper QtQml.Models module instead of qmllabs [#14675](https://github.com/mixxxdj/mixxx/pull/14675)
* qmlwaveform: Fix moc in Qt 6.9.0 [#14649](https://github.com/mixxxdj/mixxx/pull/14649)

## [2.5.3](https://github.com/mixxxdj/mixxx/milestone/50) (unreleased)

### Controller Mappings

* Traktor Kontrol S4 Mk3: tempo offset per deck [#14882](https://github.com/mixxxdj/mixxx/pull/14882)
* Traktor Kontrol S4 Mk3: don`t duplicate beatloop_activate behaviour [#14992](https://github.com/mixxxdj/mixxx/pull/14992)
* Traktor Kontrol S3: allow full library navigation [#14980](https://github.com/mixxxdj/mixxx/pull/14980)

### Misc

* Broadcast preferences: make setting string translatable [#15023](https://github.com/mixxxdj/mixxx/pull/15023)
* Sound Hardware preference: add (?) linking to Sound APIs in the manual [#14935](https://github.com/mixxxdj/mixxx/pull/14935)
* xwax: do not try to "correct" for drift in absolute mode. [#14960](https://github.com/mixxxdj/mixxx/pull/14960)
* Fix column header text assignment [#14944](https://github.com/mixxxdj/mixxx/pull/14944)
* Remove runtime assert to not risk crashes [#15000](https://github.com/mixxxdj/mixxx/pull/15000)
* Windows: Update build environment to Visual Studio 2022 [#15006](https://github.com/mixxxdj/mixxx/pull/15006)

## [2.5.2](https://github.com/mixxxdj/mixxx/milestone/49) (2025-06-13)

### Library

* Fix playlist export when name contains a dot [#14737](https://github.com/mixxxdj/mixxx/pull/14737)
* Fix loading the wrong track via drag and drop when using symlinks
  [#13708](https://github.com/mixxxdj/mixxx/pull/13708)
  [#13706](https://github.com/mixxxdj/mixxx/issues/13706)
* Fix: byte order in hotcue comments imported from rekordbox
  [#14808](https://github.com/mixxxdj/mixxx/pull/14808)
  [#14789](https://github.com/mixxxdj/mixxx/issues/14789)
* Tracks table: show ReplayGain with max. 2 decimals, full precision in tooltip
  [#14868](https://github.com/mixxxdj/mixxx/pull/14868)
  [#14867](https://github.com/mixxxdj/mixxx/issues/14867)
* Fix keyboard mappings with non-ASCII characters on Linux
  [#14843](https://github.com/mixxxdj/mixxx/pull/14843)
  [#14734](https://github.com/mixxxdj/mixxx/issues/14734)
* Computer feature: enable initial sorting during population [#14688](https://github.com/mixxxdj/mixxx/pull/14688)
* Computer feature: avoid false-positve 'has children' for non-directory links
  [#14907](https://github.com/mixxxdj/mixxx/pull/14907)
* Fix column header mapping when using external library [#13782](https://github.com/mixxxdj/mixxx/pull/13782)
* Fixed Single track cover reload on reload metadata from file
  [#14494](https://github.com/mixxxdj/mixxx/pull/14494)
  [#14409](https://github.com/mixxxdj/mixxx/issues/14409)

### Controller Mappings

* Arturia KeyLab Mk1: initial mapping [#14502](https://github.com/mixxxdj/mixxx/pull/14502)
* Denon MC7000: slicer mode TypeError [#14804](https://github.com/mixxxdj/mixxx/pull/14804)
* Denon MC7000: crossfader curve using wrong parameter [#14803](https://github.com/mixxxdj/mixxx/pull/14803)
* DJ TechTools MIDI Fighter Twister: support 4 decks [#14557](https://github.com/mixxxdj/mixxx/pull/14557)
* Hercules DJControl Inpulse 500: the crossfader was not reaching 100% to the right end
  [#14722](https://github.com/mixxxdj/mixxx/pull/14722)
* Icon Pro Audio iControls: initial mapping [#14591](https://github.com/mixxxdj/mixxx/pull/14591)
* Numark Mixtrack Platinium FX: Fix 4 steps browsing issue [#14778](https://github.com/mixxxdj/mixxx/pull/14778)
* Traktor Kontrol S3: Use GUI config for settings [#14904](https://github.com/mixxxdj/mixxx/pull/14904)
* Traktor S2 MK3: Fixed LED issue [#14717](https://github.com/mixxxdj/mixxx/pull/14717)
* Traktor S4 MK2: Use engine settings API for configuration [#14781](https://github.com/mixxxdj/mixxx/pull/14781)
* Traktor S4 MK3: prevent sync lockup, add setting for tempo center snap
  [#14735](https://github.com/mixxxdj/mixxx/pull/14735)
  [#14721](https://github.com/mixxxdj/mixxx/issues/14721)

### Controller Backend

* Control picker: Allow to learn MIDI Aux/Mic enable controls
  [#14720](https://github.com/mixxxdj/mixxx/pull/14720)
  [#14718](https://github.com/mixxxdj/mixxx/issues/14718)
* Make `[Main],headSplit` CO persistent across restart [#14817](https://github.com/mixxxdj/mixxx/pull/14817)
* Fix MIDI Controller button learning
  [#14816](https://github.com/mixxxdj/mixxx/pull/14816)
  [#14805](https://github.com/mixxxdj/mixxx/issues/14805)
* Fix learning with "No Mapping" selected [#14829](https://github.com/mixxxdj/mixxx/pull/14829)
* Unit tests for engine.beginTimer [#12437](https://github.com/mixxxdj/mixxx/pull/12437)
* engine-api.d.ts: brake()/spinback() documentation
  [#14929](https://github.com/mixxxdj/mixxx/pull/14929)

### Target support

* Fix building with a CMake multi-config setup [#14614](https://github.com/mixxxdj/mixxx/pull/14614)
* Fix building with gcc >= 14 with LTO and clang >= 19 (fpclassify)
  [#14749](https://github.com/mixxxdj/mixxx/pull/14749)
  [#14716](https://github.com/mixxxdj/mixxx/issues/14716)
* Fix: gcc `-Warray-bounds=` in fidlib by using a flexible member [#14798](https://github.com/mixxxdj/mixxx/pull/14798)
* Added Linux Mint Codenames to debian_buildenv.sh [#14709](https://github.com/mixxxdj/mixxx/pull/14709)
* Add hidden `[Config],notify_max_dbg_time` setting to reduce warnings in developer mode [#14015](https://github.com/mixxxdj/mixxx/pull/14015)
* Detect arch and fail early if not supported when installing buildenv [#14478](https://github.com/mixxxdj/mixxx/pull/14478)

### Misc

* Vinyl Control: Reduce sticker drift [#14435](https://github.com/mixxxdj/mixxx/pull/14435)
* Fix infinite number of pop ups of the "No Vinyl|Mic|Aux|Passthrough input configured" dialog
  [#14841](https://github.com/mixxxdj/mixxx/pull/14841)
  [#14837](https://github.com/mixxxdj/mixxx/issues/14837)
* Reduce CPU usage with Trace log messages
  [#14862](https://github.com/mixxxdj/mixxx/pull/14862)
  [#14791](https://github.com/mixxxdj/mixxx/issues/14791)
* Fix adjust Gain after adopting it as ReplayGain only in requesting player
  [#14812](https://github.com/mixxxdj/mixxx/pull/14812)
  [#14806](https://github.com/mixxxdj/mixxx/pull/14806)
* Skins: add loop anchor toggle to Deere, Shade, Tango
  [#14890](https://github.com/mixxxdj/mixxx/pull/14890)
  [#14173](https://github.com/mixxxdj/mixxx/issues/14173)
* Sound Hardware preferences: add manual link for Mic monitoring modes
  [#14889](https://github.com/mixxxdj/mixxx/pull/14889)
* Work around an Ubuntu, Ibus or Qt issue regarding detecting the current keyboard layout.
  [#14883](https://github.com/mixxxdj/mixxx/pull/14883)
  [#14838](https://github.com/mixxxdj/mixxx/issues/14838)
  [#14797](https://github.com/mixxxdj/mixxx/issues/14797)
* Fix BPM rounding for the 3/2 case [#14751](https://github.com/mixxxdj/mixxx/pull/14751)
* Update cue & play indicators on paused decks when switching cue mode
  [14930](https://github.com/mixxxdj/mixxx/pull/14930)
  [9928](https://github.com/mixxxdj/mixxx/issues/9928)

## [2.5.1](https://github.com/mixxxdj/mixxx/milestone/45) (2025-04-27)

### Controller Mappings

* Behringer DDM4000 & BCR2000: Update mappings to 2.5
  [#14232](https://github.com/mixxxdj/mixxx/pull/14232)
  [#14349](https://github.com/mixxxdj/mixxx/pull/14349)
* DJ TechTools MIDI Fighter Spectra: Add controller mapping
  [#14559](https://github.com/mixxxdj/mixxx/pull/14559)
* Hercules DJControl Inpulse 300: add toneplay, slicer, and beatmatch functionalities
  [#14051](https://github.com/mixxxdj/mixxx/pull/14051)
  [#14057](https://github.com/mixxxdj/mixxx/pull/14057)
* Hercules DJControl Inpulse 500: New mapping
  [#14491](https://github.com/mixxxdj/mixxx/pull/14491)
  [#14510](https://github.com/mixxxdj/mixxx/pull/14510)
* Hercules DJ Console Mk1: Fix pitch bend buttons [#14447](https://github.com/mixxxdj/mixxx/pull/14447)
* M-Vave SMC-Mixer: Add controller mapping
  [#14411](https://github.com/mixxxdj/mixxx/pull/14411)
  [#14448](https://github.com/mixxxdj/mixxx/pull/14448)
  [#14457](https://github.com/mixxxdj/mixxx/pull/14457)
  [#14458](https://github.com/mixxxdj/mixxx/pull/14458)
* M-Vave SMK-25 II: Piano keyboard mapping
  [#14412](https://github.com/mixxxdj/mixxx/pull/14412)
  [#14484](https://github.com/mixxxdj/mixxx/pull/14484)
* Numark Mixtrack Platinum: Fix VU Meters [#14575](https://github.com/mixxxdj/mixxx/pull/14575)
* Numark NS6II: New mapping [#11075](https://github.com/mixxxdj/mixxx/pull/11075)
* Numark Platinum FX: New mapping [#12872](https://github.com/mixxxdj/mixxx/pull/12872)
* Pioneer-DDJ-SB3: Fixes slip mode and adds missing knob controls [#11307](https://github.com/mixxxdj/mixxx/pull/11307)
* Reloop Digital Jockey 2 IE: New mapping
  [#4614](https://github.com/mixxxdj/mixxx/pull/4614)
  [#14328](https://github.com/mixxxdj/mixxx/pull/14328)
* Traktor S4mk3: Set 4 decks, avoid CO warnings for decks 3/4, eg. VU meter
  [#14249](https://github.com/mixxxdj/mixxx/pull/14249)
* Traktor S4mk3: Smooth xfader curve for Const Power mode
  [#14305](https://github.com/mixxxdj/mixxx/pull/14305)
  [#14329](https://github.com/mixxxdj/mixxx/pull/14329)
  [#14103](https://github.com/mixxxdj/mixxx/issues/14103)
* Traktor S4mk3: stop wheel led blinking when track is over/stopped
  [#14028](https://github.com/mixxxdj/mixxx/pull/14028)
  [#13995](https://github.com/mixxxdj/mixxx/issues/13995)
* Traktor Kontrol S3: Use pitch absolute mode as described in the manual [#14123](https://github.com/mixxxdj/mixxx/pull/14123)
* Stanton SCS.1m/d; Keith McMillen QuNeo; EKS Otus: use `playposition` instead of non-existent `visual_playposition`
  [#14609](https://github.com/mixxxdj/mixxx/pull/14609)
  [#14603](https://github.com/mixxxdj/mixxx/issues/14603)

### Controller Backend

* Controllers: Avoid timer warning on button release [#14323](https://github.com/mixxxdj/mixxx/pull/14323)
* Controller preferences: Fix notify of pending changes when closing preferences [#14234](https://github.com/mixxxdj/mixxx/pull/14234)
  [#14220](https://github.com/mixxxdj/mixxx/issues/14220)
* Controller preferences: Fix broken overwrite dialog ('Save as..' not working) [#14263](https://github.com/mixxxdj/mixxx/pull/14263)
* Controller preferences: Don't break support link texts [#14079](https://github.com/mixxxdj/mixxx/pull/14079)
* Controller preferences: Fix wrong mapping change confirmation request caused by MidiController::makeInputHandler()
  [#14281](https://github.com/mixxxdj/mixxx/pull/14281)
  [#14280](https://github.com/mixxxdj/mixxx/issues/14280)
  [#14292](https://github.com/mixxxdj/mixxx/pull/14292)
* Controller mapping info: Fix cropped description text
  [#14332](https://github.com/mixxxdj/mixxx/pull/14332)
  [#14117](https://github.com/mixxxdj/mixxx/issues/14117)
* MIDI controller learning: Make control box search usable [#14260](https://github.com/mixxxdj/mixxx/pull/14260)
* MIDI controller learning: Don't reload mapping after learn [#14253](https://github.com/mixxxdj/mixxx/pull/14253)
* MIDI controller learning: Correct skin control for mic/aux section [#14221](https://github.com/mixxxdj/mixxx/pull/14221)
* MIDI controller learning: Add more cue controls for samplers
  [#14419](https://github.com/mixxxdj/mixxx/pull/14419)
* MIDI controller learning: Continue after the maximum learning time is over [#14429](https://github.com/mixxxdj/mixxx/pull/14429)
* Allow `midino` 0 in `MidiController::makeInputHandler()
  [#14266](https://github.com/mixxxdj/mixxx/pull/14266)
  [#14265](https://github.com/mixxxdj/mixxx/issues/14265)
* Fix: provide `incomingData` to MIDI sysex mappings
  [#14368](https://github.com/mixxxdj/mixxx/pull/14368)
  [#13133](https://github.com/mixxxdj/mixxx/issues/13133)
* Fix log spam when using Midi for light mapping
  [#14326](https://github.com/mixxxdj/mixxx/issues/14326)
  [#14327](https://github.com/mixxxdj/mixxx/pull/14327)
  [#14333](https://github.com/mixxxdj/mixxx/pull/14333)
  [#14338](https://github.com/mixxxdj/mixxx/pull/14338)
  [#14371](https://github.com/mixxxdj/mixxx/pull/14371)
* Fix for `TypeError` in `midi-components-0.0.js`
  [#14203](https://github.com/mixxxdj/mixxx/pull/14203)
  [#14197](https://github.com/mixxxdj/mixxx/issues/14197)
* Fix crash due to concurrent access in MidiController [#14159](https://github.com/mixxxdj/mixxx/pull/14159)

### Skins

* Deere/LateNight (64 samplers): Bring back library in regular view
  [#14101](https://github.com/mixxxdj/mixxx/pull/14101)
  [#14097](https://github.com/mixxxdj/mixxx/issues/14097)
  [#14700](https://github.com/mixxxdj/mixxx/issues/14700)
* Fix crash when hiding waveforms in Deere
  [#14170](https://github.com/mixxxdj/mixxx/pull/14170)
* Waveform Overview: Abort play pos dragging if cursor is released outside the valid area
  [#13741](https://github.com/mixxxdj/mixxx/pull/13741)
  [#13732](https://github.com/mixxxdj/mixxx/issues/13732)
* Waveform Overview: Also render analysis progress when triggered by track menu or analysis feature [#14150](https://github.com/mixxxdj/mixxx/pull/14150)
* Don't show 'menubar hide' dialog when switching skins [#14254](https://github.com/mixxxdj/mixxx/pull/14254)
* Key Wheel: Move to View menu and make it a floating tool window
  [#14256](https://github.com/mixxxdj/mixxx/pull/14256)
  [#14239](https://github.com/mixxxdj/mixxx/pull/14239)
* Center effect parameter names [#14598](https://github.com/mixxxdj/mixxx/pull/14598)
* Track menu: highlight row when hovering checkbox
  [#14636](https://github.com/mixxxdj/mixxx/pull/14636)
  [#14680](https://github.com/mixxxdj/mixxx/pull/14680)

### Library

* Add Ctrl+Shift+C to copy the content of the selected cell(s) (The Mxxx 2.4 behaviour of Ctrl+C).
  [#14114](https://github.com/mixxxdj/mixxx/pull/14114)
  [#14065](https://github.com/mixxxdj/mixxx/issues/14065)
* Fix MusicBrainz lookup on Windows and macOS [#14216](https://github.com/mixxxdj/mixxx/pull/14216)
* Library scanner: Update cached 'missing' flag when file is redicovered
  [#14250](https://github.com/mixxxdj/mixxx/pull/14250)
* Hidden Tracks: Allow 'load to' via track context manu [#14077](https://github.com/mixxxdj/mixxx/pull/14077)
* Update to libdjinterop 0.24.3 - support for Engine 4.1/4.2
  [#14172](https://github.com/mixxxdj/mixxx/pull/14172)
  [#14289](https://github.com/mixxxdj/mixxx/pull/14289)
* Fix writing metadata via symlink [#13711](https://github.com/mixxxdj/mixxx/pull/13711)
* Library menu: change "Engine DJ Prime" to "Engine DJ"
  [#14248](https://github.com/mixxxdj/mixxx/pull/14248)
  [#14682](https://github.com/mixxxdj/mixxx/pull/14682)
* Fix file extension handling during playlist export [#14381](https://github.com/mixxxdj/mixxx/pull/14381)
* Fix manual key metadata editing in track properties dialog
  [#14022](https://github.com/mixxxdj/mixxx/pull/14022)
  [#14400](https://github.com/mixxxdj/mixxx/issues/14400)
  [#14295](https://github.com/mixxxdj/mixxx/pull/14295)
  [#14294](https://github.com/mixxxdj/mixxx/issues/14294)
* History: Don't allow joining with locked previous playlist
  [#14401](https://github.com/mixxxdj/mixxx/pull/14401)
  [#14399](https://github.com/mixxxdj/mixxx/issues/14399)
* Track info dialog: fixed cover label (max) size [#14418](https://github.com/mixxxdj/mixxx/pull/14418)
* Track Menu: Reset `eject` after moving track file to trash [#14402](https://github.com/mixxxdj/mixxx/pull/14402)
* Fix AutoDJ "Remove Crate" action
  [#14426](https://github.com/mixxxdj/mixxx/pull/14426)
  [#14425](https://github.com/mixxxdj/mixxx/issues/14425)
* Fix scrolling issue with coverart columns visible
  [#13719](https://github.com/mixxxdj/mixxx/pull/13719)
  [#14631](https://github.com/mixxxdj/mixxx/pull/14631)
* Developer Tools: multi-word search, no Tab navigation in controls table [#14474](https://github.com/mixxxdj/mixxx/pull/14474)
* Analyze feature: respect New / All selection when searching
  [#14660](https://github.com/mixxxdj/mixxx/pull/14660)
  [#14659](https://github.com/mixxxdj/mixxx/issues/14659)
* Stop populating Computer library feature when Mixxx should close [#14573](https://github.com/mixxxdj/mixxx/pull/14573)
* Tracks: apply played/missing text color also to selected tracks [#13583](https://github.com/mixxxdj/mixxx/pull/13583)
* Tracks: `show_track_menu` at index position [#14385](https://github.com/mixxxdj/mixxx/pull/14385)
* Search related menu: improve checkbox click UX [#14637](https://github.com/mixxxdj/mixxx/pull/14637)
* Avoid false missing tracks due to db inconsistency
  [#14615](https://github.com/mixxxdj/mixxx/pull/14615)
  [#14513](https://github.com/mixxxdj/mixxx/issues/14513)
* Fix automatic trimming of search bar text
  [#14497](https://github.com/mixxxdj/mixxx/pull/14497)
  [#14486](https://github.com/mixxxdj/mixxx/issues/14486)
* Avoid crash after removing Quick Link
  [#14556](https://github.com/mixxxdj/mixxx/pull/14556)
  [#8270](https://github.com/mixxxdj/mixxx/issues/8270)

### Other Fixes

* Enable R3 time-stretching with Rubberband 4.0.0 API version numbers [#14100](https://github.com/mixxxdj/mixxx/pull/14100)
* Preferences Effects: add Hide/Unhide (move) buttons to Effects tab [#13329](https://github.com/mixxxdj/mixxx/pull/13329)
* Preferences Effects: left/right key in effect lists trigger hide/unhide [#14205](https://github.com/mixxxdj/mixxx/pull/14205)
* Fix beat sync in Flanger effect [#14351](https://github.com/mixxxdj/mixxx/pull/14351)
* Apply talkover ducking after main effects to allow using a compressor effect
  [#13844](https://github.com/mixxxdj/mixxx/pull/13844)
  [#12451](https://github.com/mixxxdj/mixxx/issues/12451)
* Fix sporadic deadlocks when closing Mixxx or changing sound devices
  [#14208](https://github.com/mixxxdj/mixxx/pull/14208)
  [#14055](https://github.com/mixxxdj/mixxx/issues/14055)
* PositionScratchController: Fix loop wrap-around case [#14379](https://github.com/mixxxdj/mixxx/pull/14379)
* Allow seeking to a hotcue during waveform scratching
  [#14357](https://github.com/mixxxdj/mixxx/pull/14357)
  [#13981](https://github.com/mixxxdj/mixxx/issues/13981)
* Reset saved loop when toggling off after switching cue type
  [#14661](https://github.com/mixxxdj/mixxx/pull/14661)
  [#14657](https://github.com/mixxxdj/mixxx/issues/14657)
* Fix leaks from fid_design()
  [#14567](https://github.com/mixxxdj/mixxx/pull/14567)
  [#9470](https://github.com/mixxxdj/mixxx/issues/9470)

### Target support

* Allow to build with git "showSignature = true"
  [#14115](https://github.com/mixxxdj/mixxx/pull/14115)
  [#12997](https://github.com/mixxxdj/mixxx/issues/12997)
* Support building with Qt 6.8/6.9
  [#14080](https://github.com/mixxxdj/mixxx/pull/14080)
  [#14071](https://github.com/mixxxdj/mixxx/issues/14071)
  [#14200](https://github.com/mixxxdj/mixxx/pull/14200)
  [#14204](https://github.com/mixxxdj/mixxx/pull/14204)
  [#14440](https://github.com/mixxxdj/mixxx/pull/14440)
  [#14518](https://github.com/mixxxdj/mixxx/pull/14518)
* Welcome Ubuntu Plucky Puffin; Good bye Mantic Minotaur
  [#14148](https://github.com/mixxxdj/mixxx/pull/14148)
  [#14158](https://github.com/mixxxdj/mixxx/pull/14158)
* Add more translations to Linux desktop file
  [#14153](https://github.com/mixxxdj/mixxx/pull/14153)
  [#14169](https://github.com/mixxxdj/mixxx/pull/14169)
* Debian: recommend qt6-translations-l10n [#14147](https://github.com/mixxxdj/mixxx/pull/14147)
* Update FindFFTW3.cmake to not find version 2
  [#13937](https://github.com/mixxxdj/mixxx/pull/13937)
  [#13931](https://github.com/mixxxdj/mixxx/issues/13931)
* Allow building without tests-tools via new CMake options BUILD_TESTING and BUILD_BENCH
  [#14269](https://github.com/mixxxdj/mixxx/pull/14269)
* Fix and improve "missing env" error message [#14321](https://github.com/mixxxdj/mixxx/pull/14321)
* Qt 6.8: Ensure Mixxx uses "windowsvista" Qt style on Windows [#14228](https://github.com/mixxxdj/mixxx/pull/14228)
* Raise macOS target version to 11 (Qt 6.5 requirement). [#14440](https://github.com/mixxxdj/mixxx/pull/14440)
* Fail early when building on WSL [#14481](https://github.com/mixxxdj/mixxx/pull/14481)
* Remove useless udev rule [#14630](https://github.com/mixxxdj/mixxx/pull/14630)
* Handle new " / " from taglib 2.0
  [#12854](https://github.com/mixxxdj/mixxx/pull/12854)
  [#12790](https://github.com/mixxxdj/mixxx/issues/12790)

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
* Novation Dicer: Remove Flanger mapping with quickeffect toggle
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

## Older Changelog

Find older changelog entries [here](https://github.com/mixxxdj/mixxx/blob/2.5.0/CHANGELOG.md).
