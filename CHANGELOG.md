# Changelog

## [2.4.0](https://launchpad.net/mixxx/+milestone/2.4.0) (Unreleased)

### Cover Art

* Prevent wrong cover art display due to hash conflicts [#2524](https://github.com/mixxxdj/mixxx/pull/2524)
* Add background color for quick cover art preview [#2524](https://github.com/mixxxdj/mixxx/pull/2524)

### Music Library

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
* Rekordbox: Save all loops and correct AAC timing offset for CoreAudio [#2779](https://github.com/mixxxdj/mixxx/pull/2779)
* Improve log messages during schema migration [#2979](https://github.com/mixxxdj/mixxx/pull/2979)
* Search related tracks in collection
  [#3181](https://github.com/mixxxdj/mixxx/pull/3181)
  [#3213](https://github.com/mixxxdj/mixxx/pull/3213)
  [#2796](https://github.com/mixxxdj/mixxx/pull/2796)
  [#4207](https://github.com/mixxxdj/mixxx/pull/4207)
* Add recent searches to a drop down menu of the search box
  [#3171](https://github.com/mixxxdj/mixxx/pull/3171)
  [#3262](https://github.com/mixxxdj/mixxx/pull/3262)
* Search: Save queries across restarts
  [#4458](https://github.com/mixxxdj/mixxx/pull/4458)
  [lp:1943084](https://bugs.launchpad.net/mixxx/+bug/1943084)
  [lp:1947479](https://bugs.launchpad.net/mixxx/+bug/1947479)
  [#4571](https://github.com/mixxxdj/mixxx/pull/4571)
* Browse/Recording: Disable the non-working search box [#4382](https://github.com/mixxxdj/mixxx/pull/4382)
* Search: Update Clear button when search is disabled [#4447](https://github.com/mixxxdj/mixxx/pull/4447)
* Fix reset to default of search timeout in preferences [#4504](https://github.com/mixxxdj/mixxx/pull/4504) [lp:1948690](https://bugs.launchpad.net/mixxx/+bug/1948690)
* Search: Ctrl+F in focused search box selects the entire search string [#4515](https://github.com/mixxxdj/mixxx/pull/4515)
* Search: Improve keypress handling, fix glitch in popup, strip whitespaces [#4658](https://github.com/mixxxdj/mixxx/pull/4658)
* Search: Enter jumps to track table if search query was transmitted [#4844](https://github.com/mixxxdj/mixxx/pull/4844)
* Add new "[AutoDJ],add_random_track" to make this feature accessible from controllers [#3076](https://github.com/mixxxdj/mixxx/pull/3076)
* Add new library column that shows the last time a track was played
  [#3140](https://github.com/mixxxdj/mixxx/pull/3140)
  [#3457](https://github.com/mixxxdj/mixxx/pull/3457)
  [#3494](https://github.com/mixxxdj/mixxx/pull/3494)
  [#3596](https://github.com/mixxxdj/mixxx/pull/3596)
  [#3740](https://github.com/mixxxdj/mixxx/pull/3740)
* Improve presentation of the history library tree
  [#2996](https://github.com/mixxxdj/mixxx/pull/2996)
  [#4298](https://github.com/mixxxdj/mixxx/pull/4298) [lp:1944029](https://bugs.launchpad.net/mixxx/+bug/1944029)
* Don't store or update metadata of missing tracks in the Mixxx database to prevent inconsistencies with file tags [#3811](https://github.com/mixxxdj/mixxx/pull/3811)
* Code improvements and minor bug fixes when importing track metadata
  [#3851](https://github.com/mixxxdj/mixxx/pull/3851)
  [#3858](https://github.com/mixxxdj/mixxx/pull/3858)
  [#3860](https://github.com/mixxxdj/mixxx/pull/3860)
  [#3866](https://github.com/mixxxdj/mixxx/pull/3866)
  [#3871](https://github.com/mixxxdj/mixxx/pull/3871)
  [#3870](https://github.com/mixxxdj/mixxx/pull/3870)
  [#3924](https://github.com/mixxxdj/mixxx/pull/3924)
  [#4368](https://github.com/mixxxdj/mixxx/pull/4368)
  [#3906](https://github.com/mixxxdj/mixxx/pull/3906)
  [#3998](https://github.com/mixxxdj/mixxx/pull/3998)
* Update library schema to 37 for synchronizing file modified time with track source on metadata import/export
  [#3978](https://github.com/mixxxdj/mixxx/pull/3978)
  [#4012](https://github.com/mixxxdj/mixxx/pull/4012)
* Track Metadata: Fix synchronization (import/export) of file tags
  [#4628](https://github.com/mixxxdj/mixxx/pull/4628)
  [#4631](https://github.com/mixxxdj/mixxx/pull/4631)
  [#4847](https://github.com/mixxxdj/mixxx/pull/4847) [lp:1981106](https://bugs.launchpad.net/bugs/1981106)
* Optionally reset metadata on reimport if file tags are missing, enabled by "[Library] ResetMissingTagMetadataOnImport 1"). [#4873](https://github.com/mixxxdj/mixxx/pull/4873)
* Logging: Suppress expected and harmless schema migration errors [#4248](https://github.com/mixxxdj/mixxx/pull/4248)
* Only show the date in Date Added / Last Played columns. Move the time of day to tooltips [#3945](https://github.com/mixxxdj/mixxx/pull/3945)
* Fix handling of undefined BPM values
  [#4062](https://github.com/mixxxdj/mixxx/pull/4062)
  [#4063](https://github.com/mixxxdj/mixxx/pull/4063)
  [#4100](https://github.com/mixxxdj/mixxx/pull/4100)
  [#4154](https://github.com/mixxxdj/mixxx/pull/4154)
  [#4165](https://github.com/mixxxdj/mixxx/pull/4165)
  [#4168](https://github.com/mixxxdj/mixxx/pull/4168)
* Add support for m4v files [#4088](https://github.com/mixxxdj/mixxx/pull/4088)
* Adjust ReplayGain: Allow user to update the replaygain value based on a deck pregain value [#4031](https://github.com/mixxxdj/mixxx/pull/4031)
* Automatic analyze and optimize database [#4199](https://github.com/mixxxdj/mixxx/pull/4199)
* Re-import and update metadata after files have been modified when loading tracks [#4218](https://github.com/mixxxdj/mixxx/pull/4218)
* Fix playlists sidebar navigation/activation [#4193](https://github.com/mixxxdj/mixxx/pull/4193) [lp:1939082](https://bugs.launchpad.net/mixxx/+bug/1939082)
* Sidebar: Map Left Arrow Key to jump to parent node and activates it [#4253](https://github.com/mixxxdj/mixxx/pull/4253)
* Fix assertion when navigating history playlists [#4297](https://github.com/mixxxdj/mixxx/pull/4297) [lp:1943643](https://bugs.launchpad.net/mixxx/+bug/1943643)
* Track table: Add keyboard shortcut Ctrl+Enter to open track properties [#4347](https://github.com/mixxxdj/mixxx/pull/4347)
* Track table: Home/End keys jump to first/last row [#4850](https://github.com/mixxxdj/mixxx/pull/4850)
* Re-enable shortcuts after editing controls
  [#4360](https://github.com/mixxxdj/mixxx/pull/4360)
  [lp:1902125](https://bugs.launchpad.net/mixxx/+bug/1902125)
  [lp:1943325](https://bugs.launchpad.net/mixxx/+bug/1943325)
* History: Fix sidebar context menu actions [#4384](https://github.com/mixxxdj/mixxx/pull/4384)
* Allow to remove tracks from the library by pressing the Delete key
  [#4330](https://github.com/mixxxdj/mixxx/pull/4330)
  [lp:1227676](https://bugs.launchpad.net/mixxx/+bug/1227676)
  [lp:1851457](https://bugs.launchpad.net/mixxx/+bug/1851457)
  [lp:1856402](https://bugs.launchpad.net/mixxx/+bug/1856402)
  [lp:1944565](https://bugs.launchpad.net/mixxx/+bug/1944565)
* Remember track selection when switching library features, save model state [#4177](https://github.com/mixxxdj/mixxx/pull/4177) [#4536](https://github.com/mixxxdj/mixxx/pull/4536)
* Allow to remove a track form the disk [#3212](https://github.com/mixxxdj/mixxx/pull/3212) [#4639](https://github.com/mixxxdj/mixxx/pull/4639)
* Fix accasional resetting of played counter in database [#4578](https://github.com/mixxxdj/mixxx/pull/4578) [lp:1955159](https://bugs.launchpad.net/mixxx/+bug/1955159)
* Recording: Fix table refresh issues [#4648](https://github.com/mixxxdj/mixxx/pull/4648)
* Add popup before deletion playlist and crates [#4697](https://github.com/mixxxdj/mixxx/pull/4697) [#4700](https://github.com/mixxxdj/mixxx/pull/4700) [lp:1912475](https://bugs.launchpad.net/mixxx/+bug/1912475)
* Experimental: Fix writing of undefined MusicBrainz Recording ID [#4694](https://github.com/mixxxdj/mixxx/pull/4694)
* Traktor library: fix importing track key [#4701](https://github.com/mixxxdj/mixxx/pull/4701)
* Add History cleanup options [#4726](https://github.com/mixxxdj/mixxx/pull/4726) [lp:1766163](https://bugs.launchpad.net/mixxx/+bug/1766163) [lp:1969217](https://bugs.launchpad.net/mixxx/+bug/1969217)
* Track Menu: Add action to select loaded track in library [#4740](https://github.com/mixxxdj/mixxx/pull/4740)
* Track Menu: Add menu for Analyze and Reanlyze [#4806](https://github.com/mixxxdj/mixxx/pull/4806)
* Track Menu: Add menu for looking up track metadata at Discogs, Soundcoud and LastFM [#4772](https://github.com/mixxxdj/mixxx/pull/4772) [#4836](https://github.com/mixxxdj/mixxx/pull/4836)
* Track Menu: Add Remove from disk and fix undeleted message and progress dialog  [#4560](https://github.com/mixxxdj/mixxx/pull/4560) [#4831](https://github.com/mixxxdj/mixxx/pull/4831) [lp:1980091](https://bugs.launchpad.net/mixxx/+bug/1980091)
* Track Menu: Allow to clear the comment field [#4722](https://github.com/mixxxdj/mixxx/pull/4722) [lp:1954589](https://bugs.launchpad.net/mixxx/+bug/1954589)
* Track Menu: Allow to reset loops and also via "[ChannelN], loop_remove" control object [#4802](https://github.com/mixxxdj/mixxx/pull/4802) [lp:1978507](https://bugs.launchpad.net/mixxx/+bug/1978507)
* Track Menu: Show 'Update ReplayGain' only in decks' menus [#4719](https://github.com/mixxxdj/mixxx/pull/4719)
* Fix exporting m3u files with tracks and special characters by using the URL format [#4752](https://github.com/mixxxdj/mixxx/pull/4752)
* Refactoring of library code
  [#2756](https://github.com/mixxxdj/mixxx/pull/2756)
  [#2717](https://github.com/mixxxdj/mixxx/pull/2717)
  [#2715](https://github.com/mixxxdj/mixxx/pull/2715)
  [#2810](https://github.com/mixxxdj/mixxx/pull/2756)
  [#2900](https://github.com/mixxxdj/mixxx/pull/2900)
  [#2906](https://github.com/mixxxdj/mixxx/pull/2906)
  [#2925](https://github.com/mixxxdj/mixxx/pull/2925)
  [#3017](https://github.com/mixxxdj/mixxx/pull/3017)
  [#3475](https://github.com/mixxxdj/mixxx/pull/3475)
  [#4164](https://github.com/mixxxdj/mixxx/pull/4164)
  [#4152](https://github.com/mixxxdj/mixxx/pull/4152)
  [#4162](https://github.com/mixxxdj/mixxx/pull/4162)
  [#4101](https://github.com/mixxxdj/mixxx/pull/4101)
  [#4214](https://github.com/mixxxdj/mixxx/pull/4214)
  [#4280](https://github.com/mixxxdj/mixxx/pull/4280)
  [#4429](https://github.com/mixxxdj/mixxx/pull/4429)
  [#4477](https://github.com/mixxxdj/mixxx/pull/4477)
  [#4475](https://github.com/mixxxdj/mixxx/pull/4475)
  [#4480](https://github.com/mixxxdj/mixxx/pull/4480)
  [#4633](https://github.com/mixxxdj/mixxx/pull/4633)
  [#4837](https://github.com/mixxxdj/mixxx/pull/4837)
  [#4864](https://github.com/mixxxdj/mixxx/pull/4864)

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

* Fix recovering from FAAD2 decoding issues [#2850](https://github.com/mixxxdj/mixxx/pull/2850)
* SoundSourceMP3: Log recoverable errors as info instead of warning [#4365](https://github.com/mixxxdj/mixxx/pull/4365)
* Fix type detection of AIFF files [#4364](https://github.com/mixxxdj/mixxx/pull/4364)
* AAC encoder: Fixed a memory leak [#4386](https://github.com/mixxxdj/mixxx/pull/4386) [#4408](https://github.com/mixxxdj/mixxx/pull/4408)
* Improve robustness of file type detection by considering the actual MIME type of the content. [lp:1445885](https://bugs.launchpad.net/mixxx/+bug/1445885) [#4356](https://github.com/mixxxdj/mixxx/pull/4356) [#4357](https://github.com/mixxxdj/mixxx/pull/4357)
* Fix file type detection when file has wrong file extension by determining the MIME type from content [#4602](https://github.com/mixxxdj/mixxx/pull/4602) [#4600](https://github.com/mixxxdj/mixxx/pull/4600)
* Fix handling of files with wrong suffix
  [#4615](https://github.com/mixxxdj/mixxx/pull/4615)
  [lp:1445885](https://bugs.launchpad.net/mixxx/+bug/1445885)
  [lp:1955840](https://bugs.launchpad.net/mixxx/+bug/1955840)
  [#4683](https://github.com/mixxxdj/mixxx/pull/4683)
  [lp:1961623](https://bugs.launchpad.net/mixxx/+bug/1961623)
* Fix synchronization time stamps of ModPlug files [#4826](https://github.com/mixxxdj/mixxx/pull/4826) [lp:1979864](https://bugs.launchpad.net/mixxx/+bug/1979864)
* ID3v2 parsing: Improve log warnings [#4610](https://github.com/mixxxdj/mixxx/pull/4610)

### Audio Engine

* Add support for Saved loops
  [#2194](https://github.com/mixxxdj/mixxx/pull/2194)
  [#3267](https://github.com/mixxxdj/mixxx/pull/3267)
  [#3202](https://github.com/mixxxdj/mixxx/pull/3202)
  [#4265](https://github.com/mixxxdj/mixxx/pull/4265)
  [lp:1367159](https://bugs.launchpad.net/mixxx/+bug/1367159)
* Fix an issue when pressing multiple cue buttons at the same time [#3382](https://github.com/mixxxdj/mixxx/pull/3382)
* Fix synchronization of main cue point/position
  [#4137](https://github.com/mixxxdj/mixxx/pull/4137)
  [lp1937074](https://bugs.launchpad.net/mixxx/+bug/1937074)
  [#4153](https://github.com/mixxxdj/mixxx/pull/4153)
* Add halve/double controls for beatjump size [#4269](https://github.com/mixxxdj/mixxx/pull/4269)
* Fix possible segfault when ejecting track [#4362](https://github.com/mixxxdj/mixxx/pull/4362) [lp:1940589](https://bugs.launchpad.net/mixxx/+bug/1940589)
* Fixed an assertion when loop is before track start [#4383](https://github.com/mixxxdj/mixxx/pull/4383) [lp:1946759](https://bugs.launchpad.net/mixxx/+bug/1946759)
* Fix/Improve snapping to beats in various situations [#4366](https://github.com/mixxxdj/mixxx/pull/4366) [lp:1945238](https://bugs.launchpad.net/mixxx/+bug/1945238)
* Allow to cancel active loops via beatloop_activate [#4328](https://github.com/mixxxdj/mixxx/pull/4328) [lp:1876003](https://bugs.launchpad.net/mixxx/+bug/1876003)
* Don't wipe inapplicable sound config immediately [#4544](https://github.com/mixxxdj/mixxx/pull/4544)
* Rubberband: Support Version 3 "finer" (near-hi-fi quality) setting, on Windows and MacOs and when available on Linux
  [#4853](https://github.com/mixxxdj/mixxx/pull/4853)
  [#4855](https://github.com/mixxxdj/mixxx/pull/4855)
  [#4855](https://github.com/mixxxdj/mixxx/pull/4855)
* Refactoring of beatgrid/beatmap code
  [#4044](https://github.com/mixxxdj/mixxx/pull/4044)
  [#4048](https://github.com/mixxxdj/mixxx/pull/4048)
  [#4045](https://github.com/mixxxdj/mixxx/pull/4045)
  [#4049](https://github.com/mixxxdj/mixxx/pull/4049)
  [#4092](https://github.com/mixxxdj/mixxx/pull/4092)
  [#4094](https://github.com/mixxxdj/mixxx/pull/4094)
  [#4104](https://github.com/mixxxdj/mixxx/pull/4104)
  [#4103](https://github.com/mixxxdj/mixxx/pull/4103)
  [#4127](https://github.com/mixxxdj/mixxx/pull/4127)
  [#4099](https://github.com/mixxxdj/mixxx/pull/4099)
  [#4071](https://github.com/mixxxdj/mixxx/pull/4071)
  [#4184](https://github.com/mixxxdj/mixxx/pull/4184)
  [#4234](https://github.com/mixxxdj/mixxx/pull/4234)
  [#4233](https://github.com/mixxxdj/mixxx/pull/4233)
  [#4258](https://github.com/mixxxdj/mixxx/pull/4258)
  [#4259](https://github.com/mixxxdj/mixxx/pull/4259)
  [#4263](https://github.com/mixxxdj/mixxx/pull/4263)
  [#4272](https://github.com/mixxxdj/mixxx/pull/4272)
  [#4268](https://github.com/mixxxdj/mixxx/pull/4268)
  [#4270](https://github.com/mixxxdj/mixxx/pull/4270)
  [#4342](https://github.com/mixxxdj/mixxx/pull/4342)
  [#4336](https://github.com/mixxxdj/mixxx/pull/4336)
  [#4409](https://github.com/mixxxdj/mixxx/pull/4409)
  [#4361](https://github.com/mixxxdj/mixxx/pull/4361)
  [#4255](https://github.com/mixxxdj/mixxx/pull/4455)
  [#4488](https://github.com/mixxxdj/mixxx/pull/4488)
  [#4411](https://github.com/mixxxdj/mixxx/pull/4411)
  [#4498](https://github.com/mixxxdj/mixxx/pull/4498)
  [#4500](https://github.com/mixxxdj/mixxx/pull/4500)
  [#4499](https://github.com/mixxxdj/mixxx/pull/4499)
  [#4510](https://github.com/mixxxdj/mixxx/pull/4510)
  [#4607](https://github.com/mixxxdj/mixxx/pull/4607)
* Refactoring of audio engine code
  [#2762](https://github.com/mixxxdj/mixxx/pull/2762)
  [#2801](https://github.com/mixxxdj/mixxx/pull/2801)
  [#2885](https://github.com/mixxxdj/mixxx/pull/2885)
  [#2997](https://github.com/mixxxdj/mixxx/pull/2997)
  [#3266](https://github.com/mixxxdj/mixxx/pull/3266)
  [#4064](https://github.com/mixxxdj/mixxx/pull/4064)
  [#4065](https://github.com/mixxxdj/mixxx/pull/4065)
  [#4066](https://github.com/mixxxdj/mixxx/pull/4066)
  [#4069](https://github.com/mixxxdj/mixxx/pull/4069)
  [#4074](https://github.com/mixxxdj/mixxx/pull/4074)
  [#4075](https://github.com/mixxxdj/mixxx/pull/4075)
  [#4076](https://github.com/mixxxdj/mixxx/pull/4076)
  [#4078](https://github.com/mixxxdj/mixxx/pull/4078)
  [#4082](https://github.com/mixxxdj/mixxx/pull/4082)
  [#4077](https://github.com/mixxxdj/mixxx/pull/4077)
  [#4080](https://github.com/mixxxdj/mixxx/pull/4080)
  [#4086](https://github.com/mixxxdj/mixxx/pull/4086)
  [#4089](https://github.com/mixxxdj/mixxx/pull/4089)
  [#4090](https://github.com/mixxxdj/mixxx/pull/4090)
  [#4079](https://github.com/mixxxdj/mixxx/pull/4079)
  [#4091](https://github.com/mixxxdj/mixxx/pull/4091)
  [#4083](https://github.com/mixxxdj/mixxx/pull/4083)
  [#4095](https://github.com/mixxxdj/mixxx/pull/4095)
  [#4081](https://github.com/mixxxdj/mixxx/pull/4081)
  [#4061](https://github.com/mixxxdj/mixxx/pull/4061)
  [#4105](https://github.com/mixxxdj/mixxx/pull/4105)
  [#4183](https://github.com/mixxxdj/mixxx/pull/4183)
  [#4186](https://github.com/mixxxdj/mixxx/pull/4186)
  [#4189](https://github.com/mixxxdj/mixxx/pull/4189)
  [#4216](https://github.com/mixxxdj/mixxx/pull/4216)
  [#4221](https://github.com/mixxxdj/mixxx/pull/4221)
  [#4219](https://github.com/mixxxdj/mixxx/pull/4219)
  [#4191](https://github.com/mixxxdj/mixxx/pull/4191)
  [#4232](https://github.com/mixxxdj/mixxx/pull/4232)
  [#4231](https://github.com/mixxxdj/mixxx/pull/4231)
  [#4229](https://github.com/mixxxdj/mixxx/pull/4229)
  [#4257](https://github.com/mixxxdj/mixxx/pull/4257)
  [#4266](https://github.com/mixxxdj/mixxx/pull/4266)
  [#4217](https://github.com/mixxxdj/mixxx/pull/4217)
  [#1966](https://github.com/mixxxdj/mixxx/pull/1966)
  [#4535](https://github.com/mixxxdj/mixxx/pull/4535)
  [#4770](https://github.com/mixxxdj/mixxx/pull/4770)
  [#4771](https://github.com/mixxxdj/mixxx/pull/4771)
  [#4589](https://github.com/mixxxdj/mixxx/pull/4589)
  [#1717](https://github.com/mixxxdj/mixxx/pull/1717)

### Controllers

* Never raise a fatal error if a controller mapping tries access a non-existent control object [#2947](https://github.com/mixxxdj/mixxx/pull/2947)
* Update Novation Launchpad controller scripts [#2600](https://github.com/mixxxdj/mixxx/pull/2600)
* Add generic USB HID "Set Reports (Feature)" functionality [#3051](https://github.com/mixxxdj/mixxx/pull/3051)
* Add support for reading the status of an HID controller (like MIDI SYSEX) [#3317](https://github.com/mixxxdj/mixxx/pull/3317)
* Use hidapi's hidraw backend instead of libusb on Linux [#4054](https://github.com/mixxxdj/mixxx/pull/4054)
* Consistently use "mapping" instead of "preset" to refer to controller mappings [#3472](https://github.com/mixxxdj/mixxx/pull/3472)
* Library: Add control object `[Library],show_track_menu` to open/close the track menu [#4465](https://github.com/mixxxdj/mixxx/pull/4465)
* Library: Add control object `[Library],sort_focused_column` [#4749](https://github.com/mixxxdj/mixxx/pull/4749) [#4763](https://github.com/mixxxdj/mixxx/pull/4763) [lp:1969634](https://bugs.launchpad.net/mixxx/+bug/1969634)
* Introduce new control objects `[Master],indicator_250millis` and `[Master],indicator_500millis` [#4157](https://github.com/mixxxdj/mixxx/pull/4157)
* Introduce new control object `[Library],clear_search` [#4331](https://github.com/mixxxdj/mixxx/pull/4331)
* Introduce new control object `[Library],focused_widget` to focus library directly [#4369](https://github.com/mixxxdj/mixxx/pull/4369) [#4490](https://github.com/mixxxdj/mixxx/pull/4490)
* Serch bar: Remove ESC shortcut in favour of new `[Library],focused_widget` [#4571](https://github.com/mixxxdj/mixxx/pull/4571)
* Beat spinBox/AutoDJ spinbox: Enter & Esc also move focus to library [#4617](https://github.com/mixxxdj/mixxx/pull/4617) [#4845](https://github.com/mixxxdj/mixxx/pull/4845)
* Don't automatically enable controller if it was disabled before [#4244](https://github.com/mixxxdj/mixxx/pull/4244) [lp:1941042](https://bugs.launchpad.net/mixxx/+bug/1941042)
* Enable Qt logging categories for controller logging [#4523](https://github.com/mixxxdj/mixxx/pull/4523)
* Fix segfault during Mixxx shutdown due to a stale controller connection [#4476](https://github.com/mixxxdj/mixxx/pull/4476) [lp:1946581](https://bugs.launchpad.net/mixxx/+bug/1946581)
* Components JS: Fix syncbutton [#4329](https://github.com/mixxxdj/mixxx/pull/4329)
* Roland DJ-505: Make blinking lights blink in sync and other improvements [#4159](https://github.com/mixxxdj/mixxx/pull/4159) [#4517](https://github.com/mixxxdj/mixxx/pull/4517)
* Behringer DDM4000 mixer: Update controller mapping [#4262](https://github.com/mixxxdj/mixxx/pull/4262) [#4799](https://github.com/mixxxdj/mixxx/pull/4799)
* Numark DJ2GO2 Touch: Fix sampler, hotcue, beatloop buttons [#4287](https://github.com/mixxxdj/mixxx/pull/4287)
* Denon MC6000MK2: Improve mapping code [#4385](https://github.com/mixxxdj/mixxx/pull/4385)
* Yaeltex MiniMixxx: Add controller mapping [#4350](https://github.com/mixxxdj/mixxx/pull/4350)
* Added Trace for the mapping connections, to allow JS profiling [#4766](https://github.com/mixxxdj/mixxx/pull/4766)
* Fix crash when using `midi.sendShortMsg` and platform vnc [#4635](https://github.com/mixxxdj/mixxx/pull/4635)
* Traktor S3: Fix timedelta calculation bugs [#4646](https://github.com/mixxxdj/mixxx/pull/4646) [lp:1958925](https://bugs.launchpad.net/mixxx/+bug/1958925)
* Prepare code for upcoming ES6 based controller mapping system with module support
  [#2682](https://github.com/mixxxdj/mixxx/pull/2682)
  [#2868](https://github.com/mixxxdj/mixxx/pull/2868)
  [#2875](https://github.com/mixxxdj/mixxx/pull/2875)
  [#2936](https://github.com/mixxxdj/mixxx/pull/2936)
  [#2946](https://github.com/mixxxdj/mixxx/pull/2946)
* Other refactorings of controller code
  [#2904](https://github.com/mixxxdj/mixxx/pull/2904)
  [#3308](https://github.com/mixxxdj/mixxx/pull/3308)
  [#3463](https://github.com/mixxxdj/mixxx/pull/3463)
  [#3634](https://github.com/mixxxdj/mixxx/pull/3634)
  [#3635](https://github.com/mixxxdj/mixxx/pull/3635)
  [#3636](https://github.com/mixxxdj/mixxx/pull/3636)
  [#3676](https://github.com/mixxxdj/mixxx/pull/3676)
  [#3880](https://github.com/mixxxdj/mixxx/pull/3880)
  [#4085](https://github.com/mixxxdj/mixxx/pull/4085)
  [#4524](https://github.com/mixxxdj/mixxx/pull/4524)
  [#4533](https://github.com/mixxxdj/mixxx/pull/4533)
  [#4521](https://github.com/mixxxdj/mixxx/pull/4521)
  [#4585](https://github.com/mixxxdj/mixxx/pull/4585)
  [#4692](https://github.com/mixxxdj/mixxx/pull/4692)
  [#4566](https://github.com/mixxxdj/mixxx/pull/4566)
  [#4599](https://github.com/mixxxdj/mixxx/pull/4599)
  [#4624](https://github.com/mixxxdj/mixxx/pull/4624)
  [#4627](https://github.com/mixxxdj/mixxx/pull/4627)
  [#4738](https://github.com/mixxxdj/mixxx/pull/4738)

### Skins

* Add experimental QML user interface
  [#3345](https://github.com/mixxxdj/mixxx/pull/3345)
  [#3446](https://github.com/mixxxdj/mixxx/pull/3446)
  [#3854](https://github.com/mixxxdj/mixxx/pull/3854)
  [#3891](https://github.com/mixxxdj/mixxx/pull/3891)
  [#2874](https://github.com/mixxxdj/mixxx/pull/2874)
  [#3915](https://github.com/mixxxdj/mixxx/pull/3915)
  [#3894](https://github.com/mixxxdj/mixxx/pull/3894)
  [#3920](https://github.com/mixxxdj/mixxx/pull/3920)
  [#3907](https://github.com/mixxxdj/mixxx/pull/3907)
  [#3925](https://github.com/mixxxdj/mixxx/pull/3925)
  [#3928](https://github.com/mixxxdj/mixxx/pull/3928)
  [#3932](https://github.com/mixxxdj/mixxx/pull/3932)
  [#3911](https://github.com/mixxxdj/mixxx/pull/3911)
  [#3937](https://github.com/mixxxdj/mixxx/pull/3937)
  [#3940](https://github.com/mixxxdj/mixxx/pull/3940)
  [#3913](https://github.com/mixxxdj/mixxx/pull/3913)
  [#3950](https://github.com/mixxxdj/mixxx/pull/3950)
  [#3919](https://github.com/mixxxdj/mixxx/pull/3919)
  [#3955](https://github.com/mixxxdj/mixxx/pull/3955)
  [#3957](https://github.com/mixxxdj/mixxx/pull/3957)
  [#3961](https://github.com/mixxxdj/mixxx/pull/3961)
  [#3952](https://github.com/mixxxdj/mixxx/pull/3952)
  [#3963](https://github.com/mixxxdj/mixxx/pull/3963)
  [#3971](https://github.com/mixxxdj/mixxx/pull/3971)
  [#3959](https://github.com/mixxxdj/mixxx/pull/3959)
  [#3972](https://github.com/mixxxdj/mixxx/pull/3972)
  [#3992](https://github.com/mixxxdj/mixxx/pull/3992)
  [#4003](https://github.com/mixxxdj/mixxx/pull/4003)
  [#4004](https://github.com/mixxxdj/mixxx/pull/4004)
  [#3999](https://github.com/mixxxdj/mixxx/pull/3999)
  [#4000](https://github.com/mixxxdj/mixxx/pull/4000)
  [#4067](https://github.com/mixxxdj/mixxx/pull/4067)
  [#4068](https://github.com/mixxxdj/mixxx/pull/4068)
  [#4060](https://github.com/mixxxdj/mixxx/pull/4060)
  [#4037](https://github.com/mixxxdj/mixxx/pull/4037)
  [#4414](https://github.com/mixxxdj/mixxx/pull/4414)
  [#3934](https://github.com/mixxxdj/mixxx/pull/3934)
  [#4117](https://github.com/mixxxdj/mixxx/pull/4117)
  [#4327](https://github.com/mixxxdj/mixxx/pull/4327)
  [#4339](https://github.com/mixxxdj/mixxx/pull/4339)
  [#4568](https://github.com/mixxxdj/mixxx/pull/4568)
  [#4576](https://github.com/mixxxdj/mixxx/pull/4576)
  [#4591](https://github.com/mixxxdj/mixxx/pull/4591)
  [#4616](https://github.com/mixxxdj/mixxx/pull/4616)
  [#4567](https://github.com/mixxxdj/mixxx/pull/4567)
  [#4629](https://github.com/mixxxdj/mixxx/pull/4629)
  [#4643](https://github.com/mixxxdj/mixxx/pull/4643)
  [#4650](https://github.com/mixxxdj/mixxx/pull/4650)
  [#4654](https://github.com/mixxxdj/mixxx/pull/4654)
  [#4673](https://github.com/mixxxdj/mixxx/pull/4673)
  [#4680](https://github.com/mixxxdj/mixxx/pull/4680)
  [#4681](https://github.com/mixxxdj/mixxx/pull/4681)
  [#4736](https://github.com/mixxxdj/mixxx/pull/4736)
* Add new "RGB Stacked" waveform [#3153](https://github.com/mixxxdj/mixxx/pull/3153)
* Add harmonic keywheel window
  [#1695](https://github.com/mixxxdj/mixxx/pull/1695)
  [#3622](https://github.com/mixxxdj/mixxx/pull/3622)
  [#3624](https://github.com/mixxxdj/mixxx/pull/3624)
* Make beat indicator control behaviour more natural [#3608](https://github.com/mixxxdj/mixxx/pull/3608)
* Allow skin scaling from preferences [#3960](https://github.com/mixxxdj/mixxx/pull/3960)
* Invert scroll wheel waveform zoom direction to mach other applications [#4195](https://github.com/mixxxdj/mixxx/pull/4195)
* Fix crash if no skin is available
  [#3918](https://github.com/mixxxdj/mixxx/pull/3918)
  [#3939](https://github.com/mixxxdj/mixxx/pull/3939)
* Fix leaked controls [#4213](https://github.com/mixxxdj/mixxx/pull/4213) [lp:1912129](https://bugs.launchpad.net/mixxx/+bug/1912129)
* Shade: Remove initial setting of now accessible effect controls [#4398](https://github.com/mixxxdj/mixxx/pull/4398) [lp:1946811](https://bugs.launchpad.net/mixxx/+bug/1946811)
* Fix switching from Shade to other skins [#4421](https://github.com/mixxxdj/mixxx/pull/4421) [lp:1946812](https://bugs.launchpad.net/mixxx/+bug/1946812)
* Use double click to reset knobs and sliders [#4509](https://github.com/mixxxdj/mixxx/pull/4509) [lp:1875999](https://bugs.launchpad.net/mixxx/+bug/1875999)
* Use info not warning for skin COs [#4525](https://github.com/mixxxdj/mixxx/pull/4525)
* LateNight: Move logo to the right [#4677](https://github.com/mixxxdj/mixxx/pull/4677)
* Spinny: Allow to toggle cover art at runtime [#4565](https://github.com/mixxxdj/mixxx/pull/4565) [lp:1883362](https://bugs.launchpad.net/mixxx/+bug/1883362)
* Add new searchbox styles [#4505](https://github.com/mixxxdj/mixxx/pull/4505)
* Add new Quick Effect selector and effect chain buttons [#4501](https://github.com/mixxxdj/mixxx/pull/4501) [#4502](https://github.com/mixxxdj/mixxx/pull/4502) [#4503](https://github.com/mixxxdj/mixxx/pull/4503) [#4590](https://github.com/mixxxdj/mixxx/pull/4590)  [#4593](https://github.com/mixxxdj/mixxx/pull/4593)
* Fix crash when starting without a valid skin directory [#4575](https://github.com/mixxxdj/mixxx/pull/4575) [lp:1934560](https://bugs.launchpad.net/mixxx/+bug/1934560)
* LateNight: Use correct tooltip for key control toggle [#4696](https://github.com/mixxxdj/mixxx/pull/4696)
* LateNight: Add toggles to show loop and beatjump controls [#4713](https://github.com/mixxxdj/mixxx/pull/[#4713](https://github.com/mixxxdj/mixxx/pull/4713))
* LateNight: Remove blinking play indicator from mini samplers [#4807](https://github.com/mixxxdj/mixxx/pull/4807)

### Effects

* Add a Noise effect [#2921](https://github.com/mixxxdj/mixxx/pull/2921)
* Add a pitch shift effect [#4775](https://github.com/mixxxdj/mixxx/pull/4775) [lp:1299035](https://bugs.launchpad.net/mixxx/+bug/1299035)
* Use '---' instead of 'None' for empty slots to spot them easier [#4469](https://github.com/mixxxdj/mixxx/pull/4469)
* EngineFilterDelay: clamp wrong delay values [#4869](https://github.com/mixxxdj/mixxx/pull/4869)
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
  [#4833](https://github.com/mixxxdj/mixxx/pull/4833) [lp:1980078](https://bugs.launchpad.net/mixxx/+bug/1980078)
  [#4884](https://github.com/mixxxdj/mixxx/pull/4884) [lp:1983789](https://bugs.launchpad.net/mixxx/+bug/1983789) [lp1983764](https://bugs.launchpad.net/mixxx/+bug/1983764)

### Other

* Improve/fix the CMake build system
  [#2943](https://github.com/mixxxdj/mixxx/pull/2943)
  [#3046](https://github.com/mixxxdj/mixxx/pull/3046)
  [#3114](https://github.com/mixxxdj/mixxx/pull/3114)
  [#3471](https://github.com/mixxxdj/mixxx/pull/3471)
  [#3765](https://github.com/mixxxdj/mixxx/pull/3765)
  [#3849](https://github.com/mixxxdj/mixxx/pull/3849)
  [#3876](https://github.com/mixxxdj/mixxx/pull/3876)
  [#4098](https://github.com/mixxxdj/mixxx/pull/4098)
  [#4113](https://github.com/mixxxdj/mixxx/pull/4113)
  [#4166](https://github.com/mixxxdj/mixxx/pull/4166)
  [#4185](https://github.com/mixxxdj/mixxx/pull/4185)
  [#4187](https://github.com/mixxxdj/mixxx/pull/4187)
  [#4351](https://github.com/mixxxdj/mixxx/pull/4351)
  [#4423](https://github.com/mixxxdj/mixxx/pull/4423)
  [#4422](https://github.com/mixxxdj/mixxx/pull/4422)
  [#4497](https://github.com/mixxxdj/mixxx/pull/4497)
  [#4514](https://github.com/mixxxdj/mixxx/pull/4514)
  [#3550](https://github.com/mixxxdj/mixxx/pull/3550)
* Improve GitHub workflow continuous integration
  [#2937](https://github.com/mixxxdj/mixxx/pull/2937)
  [#3041](https://github.com/mixxxdj/mixxx/pull/3041)
  [#3300](https://github.com/mixxxdj/mixxx/pull/3300)
  [#4007](https://github.com/mixxxdj/mixxx/pull/4007)
  [#4084](https://github.com/mixxxdj/mixxx/pull/4084)
  [#4250](https://github.com/mixxxdj/mixxx/pull/4250)
  [#4274](https://github.com/mixxxdj/mixxx/pull/4274)
  [#4313](https://github.com/mixxxdj/mixxx/pull/4313)
  [#4226](https://github.com/mixxxdj/mixxx/pull/4226)
  [#4452](https://github.com/mixxxdj/mixxx/pull/4452)
  [#4742](https://github.com/mixxxdj/mixxx/pull/4742)
  [#4741](https://github.com/mixxxdj/mixxx/pull/4741)
  [#4795](https://github.com/mixxxdj/mixxx/pull/4795)
  [#4798](https://github.com/mixxxdj/mixxx/pull/4798)
  [#4792](https://github.com/mixxxdj/mixxx/pull/4792)
  [#4829](https://github.com/mixxxdj/mixxx/pull/4829)
  [#4822](https://github.com/mixxxdj/mixxx/pull/4822)
  [#4820](https://github.com/mixxxdj/mixxx/pull/4820)
  [#4821](https://github.com/mixxxdj/mixxx/pull/4821)
  [#4818](https://github.com/mixxxdj/mixxx/pull/4818)
  [#4819](https://github.com/mixxxdj/mixxx/pull/4819)
  [#4817](https://github.com/mixxxdj/mixxx/pull/4817)
  [#4815](https://github.com/mixxxdj/mixxx/pull/4815)
* Improve pre-commit hook
  [#2796](https://github.com/mixxxdj/mixxx/pull/2796)
  [#3923](https://github.com/mixxxdj/mixxx/pull/3923)
  [#3948](https://github.com/mixxxdj/mixxx/pull/3948)
  [#3929](https://github.com/mixxxdj/mixxx/pull/3929)
  [#4192](https://github.com/mixxxdj/mixxx/pull/4192)
  [#4282](https://github.com/mixxxdj/mixxx/pull/4282)
  [#4278](https://github.com/mixxxdj/mixxx/pull/4278)
  [#4314](https://github.com/mixxxdj/mixxx/pull/4314)
  [#4321](https://github.com/mixxxdj/mixxx/pull/4321)
  [#4374](https://github.com/mixxxdj/mixxx/pull/4374)
  [#4494](https://github.com/mixxxdj/mixxx/pull/4494)
  [#4512](https://github.com/mixxxdj/mixxx/pull/4512)
  [#4558](https://github.com/mixxxdj/mixxx/pull/4558)
  [#4580](https://github.com/mixxxdj/mixxx/pull/4580)
  [#4743](https://github.com/mixxxdj/mixxx/pull/4743)
* Improve Lauchpad PPA builds
  [#4277](https://github.com/mixxxdj/mixxx/pull/4277)
  [#4285](https://github.com/mixxxdj/mixxx/pull/4285)
  [#4425](https://github.com/mixxxdj/mixxx/pull/4425)
* Drop Ubuntu Bionic support, require Qt 5.12
  [#3687](https://github.com/mixxxdj/mixxx/pull/3687)
  [#3735](https://github.com/mixxxdj/mixxx/pull/3735)
  [#3736](https://github.com/mixxxdj/mixxx/pull/3736)
  [#3985](https://github.com/mixxxdj/mixxx/pull/3985)
* Remove Ubuntu Focal CI
  [#4889](https://github.com/mixxxdj/mixxx/pull/4889)
* Drop Ubuntu Groovy and Impish support because of EOL
  [#4283](https://github.com/mixxxdj/mixxx/pull/4283) [#4849](https://github.com/mixxxdj/mixxx/pull/4849)
* Support Ubuntu Kinetic Kudo [#4780](https://github.com/mixxxdj/mixxx/pull/4780)
* Provide Ubuntu Jammy CI builds [#4857](https://github.com/mixxxdj/mixxx/pull/4857)
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
* Update vcpkg build environment for Windows and macOS
  [#4163](https://github.com/mixxxdj/mixxx/pull/4163)
  [#4225](https://github.com/mixxxdj/mixxx/pull/4225)
  [#4338](https://github.com/mixxxdj/mixxx/pull/4338)
  [#4709](https://github.com/mixxxdj/mixxx/pull/4709)
  [#4717](https://github.com/mixxxdj/mixxx/pull/4717)
* Devendor libraries from the mixxx lib directory
  [#4201](https://github.com/mixxxdj/mixxx/pull/4201)
  [#4202](https://github.com/mixxxdj/mixxx/pull/4202)
* Update Google Benchmark library to v1.6.0 [#4540](https://github.com/mixxxdj/mixxx/pull/4540)
* Update hidapi to 0.11.2 [#4783](https://github.com/mixxxdj/mixxx/pull/4783)
* Update xwax to Version 1.8 [#4597](https://github.com/mixxxdj/mixxx/pull/4597) [lp:1956018](https://bugs.launchpad.net/mixxx/+bug/1956018)
* Update rigtorp/SPSCQueue [#4678](https://github.com/mixxxdj/mixxx/pull/4678)
* Migration to Qt6 (work in progress)
  [#4052](https://github.com/mixxxdj/mixxx/pull/4052)
  [#4295](https://github.com/mixxxdj/mixxx/pull/4295)
  [#4293](https://github.com/mixxxdj/mixxx/pull/4293)
  [#4294](https://github.com/mixxxdj/mixxx/pull/4294)
  [#4291](https://github.com/mixxxdj/mixxx/pull/4291)
  [#4290](https://github.com/mixxxdj/mixxx/pull/4290)
  [#4300](https://github.com/mixxxdj/mixxx/pull/4300)
  [#4302](https://github.com/mixxxdj/mixxx/pull/4302)
  [#4289](https://github.com/mixxxdj/mixxx/pull/4289)
  [#4292](https://github.com/mixxxdj/mixxx/pull/4292)
  [#4299](https://github.com/mixxxdj/mixxx/pull/4299)
  [#4051](https://github.com/mixxxdj/mixxx/pull/4051)
  [#4303](https://github.com/mixxxdj/mixxx/pull/4303)
  [#4305](https://github.com/mixxxdj/mixxx/pull/4305)
  [#4304](https://github.com/mixxxdj/mixxx/pull/4304)
  [#4306](https://github.com/mixxxdj/mixxx/pull/4306)
  [#4308](https://github.com/mixxxdj/mixxx/pull/4308)
  [#4309](https://github.com/mixxxdj/mixxx/pull/4309)
  [#4322](https://github.com/mixxxdj/mixxx/pull/4322)
  [#4373](https://github.com/mixxxdj/mixxx/pull/4373)
  [#4371](https://github.com/mixxxdj/mixxx/pull/4371)
  [#4375](https://github.com/mixxxdj/mixxx/pull/4375)
  [#4378](https://github.com/mixxxdj/mixxx/pull/4378)
  [#4381](https://github.com/mixxxdj/mixxx/pull/4381)
  [#4380](https://github.com/mixxxdj/mixxx/pull/4380)
  [#4376](https://github.com/mixxxdj/mixxx/pull/4376)
  [#4379](https://github.com/mixxxdj/mixxx/pull/4379)
  [#4372](https://github.com/mixxxdj/mixxx/pull/4372)
  [#4377](https://github.com/mixxxdj/mixxx/pull/4377)
  [#4387](https://github.com/mixxxdj/mixxx/pull/4387)
  [#4391](https://github.com/mixxxdj/mixxx/pull/4391)
  [#4392](https://github.com/mixxxdj/mixxx/pull/4392)
  [#4395](https://github.com/mixxxdj/mixxx/pull/4395)
  [#4397](https://github.com/mixxxdj/mixxx/pull/4397)
  [#4396](https://github.com/mixxxdj/mixxx/pull/4396)
  [#4402](https://github.com/mixxxdj/mixxx/pull/4402)
  [#4405](https://github.com/mixxxdj/mixxx/pull/4405)
  [#4394](https://github.com/mixxxdj/mixxx/pull/4394)
  [#4404](https://github.com/mixxxdj/mixxx/pull/4404)
  [#4401](https://github.com/mixxxdj/mixxx/pull/4401)
  [#4400](https://github.com/mixxxdj/mixxx/pull/4400)
  [#4403](https://github.com/mixxxdj/mixxx/pull/4403)
  [#4407](https://github.com/mixxxdj/mixxx/pull/4407)
  [#4399](https://github.com/mixxxdj/mixxx/pull/4399)
  [#4406](https://github.com/mixxxdj/mixxx/pull/4406)
  [#4420](https://github.com/mixxxdj/mixxx/pull/4420)
  [#4415](https://github.com/mixxxdj/mixxx/pull/4415)
  [#4417](https://github.com/mixxxdj/mixxx/pull/4417)
  [#4419](https://github.com/mixxxdj/mixxx/pull/4419)
  [#4416](https://github.com/mixxxdj/mixxx/pull/4416)
  [#4418](https://github.com/mixxxdj/mixxx/pull/4418)
  [#4547](https://github.com/mixxxdj/mixxx/pull/4547)
  [#4433](https://github.com/mixxxdj/mixxx/pull/4433)
  [#4434](https://github.com/mixxxdj/mixxx/pull/4434)
  [#4441](https://github.com/mixxxdj/mixxx/pull/4441)
  [#4445](https://github.com/mixxxdj/mixxx/pull/4445)
  [#4446](https://github.com/mixxxdj/mixxx/pull/4446)
  [#4444](https://github.com/mixxxdj/mixxx/pull/4444)
  [#4436](https://github.com/mixxxdj/mixxx/pull/4436)
  [#4437](https://github.com/mixxxdj/mixxx/pull/4437)
  [#4440](https://github.com/mixxxdj/mixxx/pull/4440)
  [#4430](https://github.com/mixxxdj/mixxx/pull/4430)
  [#4435](https://github.com/mixxxdj/mixxx/pull/4435)
  [#4443](https://github.com/mixxxdj/mixxx/pull/4443)
  [#4439](https://github.com/mixxxdj/mixxx/pull/4439)
  [#4442](https://github.com/mixxxdj/mixxx/pull/4442)
  [#4438](https://github.com/mixxxdj/mixxx/pull/4438)
  [#4449](https://github.com/mixxxdj/mixxx/pull/4449)
  [#4451](https://github.com/mixxxdj/mixxx/pull/4451)
  [#4453](https://github.com/mixxxdj/mixxx/pull/4453)
  [#4478](https://github.com/mixxxdj/mixxx/pull/4478)
  [#4479](https://github.com/mixxxdj/mixxx/pull/4479)
  [#4506](https://github.com/mixxxdj/mixxx/pull/4506)
  [#4556](https://github.com/mixxxdj/mixxx/pull/4556)
  [#4554](https://github.com/mixxxdj/mixxx/pull/4554)
  [#4555](https://github.com/mixxxdj/mixxx/pull/4555)
  [#4552](https://github.com/mixxxdj/mixxx/pull/4552)
  [#4549](https://github.com/mixxxdj/mixxx/pull/4549)
  [#4788](https://github.com/mixxxdj/mixxx/pull/4788)
  [#4620](https://github.com/mixxxdj/mixxx/pull/4620)
  [#4641](https://github.com/mixxxdj/mixxx/pull/4641)
  [#4649](https://github.com/mixxxdj/mixxx/pull/4649)
  [#4679](https://github.com/mixxxdj/mixxx/pull/4679)
* Disable QWidget based library with Qt6, support only QML skins
  [#4393](https://github.com/mixxxdj/mixxx/pull/4393)
* Made use of inclusive language
  [#2894](https://github.com/mixxxdj/mixxx/pull/2894)
  [#3868](https://github.com/mixxxdj/mixxx/pull/3868)
* Improve the unit tests
  [#2938](https://github.com/mixxxdj/mixxx/pull/2938)
  [#2980](https://github.com/mixxxdj/mixxx/pull/2980)
  [#3006](https://github.com/mixxxdj/mixxx/pull/3006)
  [#4345](https://github.com/mixxxdj/mixxx/pull/4345)
* Logging: Add support for `QT_MESSAGE_PATTERN` environment variable
  [#3204](https://github.com/mixxxdj/mixxx/pull/3204)
  [#3518](https://github.com/mixxxdj/mixxx/pull/3518)
* Colored logging console output
  [#3197](https://github.com/mixxxdj/mixxx/pull/3197)
* Improve command line argument parser
  [#3640](https://github.com/mixxxdj/mixxx/pull/3640)
  [#3962](https://github.com/mixxxdj/mixxx/pull/3962)
  [#4022](https://github.com/mixxxdj/mixxx/pull/4022)
  [#4036](https://github.com/mixxxdj/mixxx/pull/4036)
  [#4170](https://github.com/mixxxdj/mixxx/pull/4170)
  [#4057](https://github.com/mixxxdj/mixxx/pull/4057)
* Improve message when dealing with macOS sandbox [#4018](https://github.com/mixxxdj/mixxx/pull/4018) [lp:1921541](https://bugs.launchpad.net/mixxx/+bug/1921541)
* Moved contribution guidelines into our git repository [#2699](https://github.com/mixxxdj/mixxx/pull/2699)
* Automate deployment of CHANGELOG to the manual
  [#4180](https://github.com/mixxxdj/mixxx/pull/4180)
  [#4256](https://github.com/mixxxdj/mixxx/pull/4256)
  [#4208](https://github.com/mixxxdj/mixxx/pull/4208)
  [#4228](https://github.com/mixxxdj/mixxx/pull/4228)
  [#4222](https://github.com/mixxxdj/mixxx/pull/4222)
* Always show tooltips in preferences [#4198](https://github.com/mixxxdj/mixxx/pull/4198) [lp:1840493](https://bugs.launchpad.net/mixxx/+bug/1840493)
* Allow to build Mixxx on Linux without ALSA, working around a Pipewire bug [#4242](https://github.com/mixxxdj/mixxx/pull/4242)
* Fix possible crash with opus files with embedded cover arts and require TagLib 1.11 or newer
  [#4251](https://github.com/mixxxdj/mixxx/pull/4251)
  [#4252](https://github.com/mixxxdj/mixxx/pull/4252) [lp:1940777](https://bugs.launchpad.net/mixxx/+bug/1940777)
* DlgTrackInfo: Fixed a SIGSEGV after a debug assertion [#4316](https://github.com/mixxxdj/mixxx/pull/4316)
* Library Preferences: Added link to settings files info in the manual [#4367](https://github.com/mixxxdj/mixxx/pull/4367)
* Use rounded Mixxx Icon for MacOS to follow Apples style guide [#4545](https://github.com/mixxxdj/mixxx/pull/4545)
* Show native path separators in command line argument --help [#4657](https://github.com/mixxxdj/mixxx/pull/4657)
* Add 13 new translation languages [#4785](https://github.com/mixxxdj/mixxx/pull/4785) [lp:1837663](https://bugs.launchpad.net/mixxx/+bug/1837663)
* Improve Vinyl control quality indicator [#3279](https://github.com/mixxxdj/mixxx/pull/3279)
* Track info dialog: Apply pending changes also when saving via hotkey [#4562](https://github.com/mixxxdj/mixxx/pull/4562) [lp:1954346](https://bugs.launchpad.net/mixxx/+bug/1954346)
* Track info dialog: Fix crash when trying to scale 0.0 BPM [#4587](https://github.com/mixxxdj/mixxx/pull/4587) [1955853](https://bugs.launchpad.net/mixxx/+bug/1955853)
* Tweak remove/hide track confirm message to match confirm exit message [#4577](https://github.com/mixxxdj/mixxx/pull/4577) [1947992](https://bugs.launchpad.net/mixxx/+bug/1947992)
* Implement Un-eject by pressing eject again [#4668](https://github.com/mixxxdj/mixxx/pull/4668)
* Preferences > Effects: Fix Tab key navigation / UX [#4702](https://github.com/mixxxdj/mixxx/pull/4702) [lp:1948535](https://bugs.launchpad.net/mixxx/+bug/1948535)
* Switch to C++20 [#4832](https://github.com/mixxxdj/mixxx/pull/4832) [#4803](https://github.com/mixxxdj/mixxx/pull/4803)
* Windows builds: Sign binary and installer with timestamp and sha256 [#4824](https://github.com/mixxxdj/mixxx/pull/4824) [#4825](https://github.com/mixxxdj/mixxx/pull/4825)
* Misc. refactorings
  [#3154](https://github.com/mixxxdj/mixxx/pull/3154)
  [#2870](https://github.com/mixxxdj/mixxx/pull/2870)
  [#2872](https://github.com/mixxxdj/mixxx/pull/2872)
  [#2978](https://github.com/mixxxdj/mixxx/pull/2978)
  [#2969](https://github.com/mixxxdj/mixxx/pull/2969)
  [#3016](https://github.com/mixxxdj/mixxx/pull/3016)
  [#3320](https://github.com/mixxxdj/mixxx/pull/3320)
  [#3356](https://github.com/mixxxdj/mixxx/pull/3356)
  [#3453](https://github.com/mixxxdj/mixxx/pull/3453)
  [#3487](https://github.com/mixxxdj/mixxx/pull/3487)
  [#3558](https://github.com/mixxxdj/mixxx/pull/3558)
  [#3685](https://github.com/mixxxdj/mixxx/pull/3685)
  [#3741](https://github.com/mixxxdj/mixxx/pull/3741)
  [#3744](https://github.com/mixxxdj/mixxx/pull/3744)
  [#3753](https://github.com/mixxxdj/mixxx/pull/3753)
  [#3761](https://github.com/mixxxdj/mixxx/pull/3761)
  [#3834](https://github.com/mixxxdj/mixxx/pull/3834)
  [#3842](https://github.com/mixxxdj/mixxx/pull/3842)
  [#3853](https://github.com/mixxxdj/mixxx/pull/3853)
  [#3874](https://github.com/mixxxdj/mixxx/pull/3874)
  [#3883](https://github.com/mixxxdj/mixxx/pull/3883)
  [#3922](https://github.com/mixxxdj/mixxx/pull/3922)
  [#3947](https://github.com/mixxxdj/mixxx/pull/3947)
  [#3974](https://github.com/mixxxdj/mixxx/pull/3974)
  [#4024](https://github.com/mixxxdj/mixxx/pull/4024)
  [#4026](https://github.com/mixxxdj/mixxx/pull/4026)
  [#4034](https://github.com/mixxxdj/mixxx/pull/4034)
  [#4038](https://github.com/mixxxdj/mixxx/pull/4038)
  [#4039](https://github.com/mixxxdj/mixxx/pull/4039)
  [#4043](https://github.com/mixxxdj/mixxx/pull/4043)
  [#4053](https://github.com/mixxxdj/mixxx/pull/4053)
  [#4072](https://github.com/mixxxdj/mixxx/pull/4072)
  [#4097](https://github.com/mixxxdj/mixxx/pull/4097)
  [#4096](https://github.com/mixxxdj/mixxx/pull/4096)
  [#4118](https://github.com/mixxxdj/mixxx/pull/4118)
  [#4130](https://github.com/mixxxdj/mixxx/pull/4130)
  [#4129](https://github.com/mixxxdj/mixxx/pull/4129)
  [#4109](https://github.com/mixxxdj/mixxx/pull/4109)
  [#4106](https://github.com/mixxxdj/mixxx/pull/4106)
  [#4131](https://github.com/mixxxdj/mixxx/pull/4131)
  [#4140](https://github.com/mixxxdj/mixxx/pull/4140)
  [#3032](https://github.com/mixxxdj/mixxx/pull/3032)
  [#4110](https://github.com/mixxxdj/mixxx/pull/4110)
  [#4173](https://github.com/mixxxdj/mixxx/pull/4173)
  [#4178](https://github.com/mixxxdj/mixxx/pull/4178)
  [#4194](https://github.com/mixxxdj/mixxx/pull/4194)
  [#4197](https://github.com/mixxxdj/mixxx/pull/4197)
  [#4190](https://github.com/mixxxdj/mixxx/pull/4190)
  [#4212](https://github.com/mixxxdj/mixxx/pull/4212)
  [#4223](https://github.com/mixxxdj/mixxx/pull/4223)
  [#4238](https://github.com/mixxxdj/mixxx/pull/4238)
  [#4236](https://github.com/mixxxdj/mixxx/pull/4236)
  [#4320](https://github.com/mixxxdj/mixxx/pull/4320)
  [#4325](https://github.com/mixxxdj/mixxx/pull/4325)
  [#4203](https://github.com/mixxxdj/mixxx/pull/4203)
  [#3861](https://github.com/mixxxdj/mixxx/pull/3861)
  [#3514](https://github.com/mixxxdj/mixxx/pull/3514)
  [#3274](https://github.com/mixxxdj/mixxx/pull/3274)
  [#3182](https://github.com/mixxxdj/mixxx/pull/3182)
  [#4343](https://github.com/mixxxdj/mixxx/pull/4343)
  [#4358](https://github.com/mixxxdj/mixxx/pull/4358)
  [#4388](https://github.com/mixxxdj/mixxx/pull/4388)
  [#4427](https://github.com/mixxxdj/mixxx/pull/4427)
  [#4341](https://github.com/mixxxdj/mixxx/pull/4341)
  [#4473](https://github.com/mixxxdj/mixxx/pull/4473)
  [#4464](https://github.com/mixxxdj/mixxx/pull/4464)
  [#4481](https://github.com/mixxxdj/mixxx/pull/4481)
  [#4527](https://github.com/mixxxdj/mixxx/pull/4527)
  [#4534](https://github.com/mixxxdj/mixxx/pull/4534)
  [#4537](https://github.com/mixxxdj/mixxx/pull/4537)
  [#4539](https://github.com/mixxxdj/mixxx/pull/4539)
  [#4541](https://github.com/mixxxdj/mixxx/pull/4541)
  [#4543](https://github.com/mixxxdj/mixxx/pull/4543)
  [#4546](https://github.com/mixxxdj/mixxx/pull/4546)
  [#4542](https://github.com/mixxxdj/mixxx/pull/4542)
  [#4559](https://github.com/mixxxdj/mixxx/pull/4559)
  [#4574](https://github.com/mixxxdj/mixxx/pull/4574)
  [#4581](https://github.com/mixxxdj/mixxx/pull/4581)
  [#4582](https://github.com/mixxxdj/mixxx/pull/4582)
  [#4598](https://github.com/mixxxdj/mixxx/pull/4598)
  [#4596](https://github.com/mixxxdj/mixxx/pull/4596)
  [#4592](https://github.com/mixxxdj/mixxx/pull/4592)
  [#4784](https://github.com/mixxxdj/mixxx/pull/4784)
  [#4604](https://github.com/mixxxdj/mixxx/pull/4604)
  [#4603](https://github.com/mixxxdj/mixxx/pull/4603)
  [#4611](https://github.com/mixxxdj/mixxx/pull/4611)
  [#4605](https://github.com/mixxxdj/mixxx/pull/4605)
  [#4583](https://github.com/mixxxdj/mixxx/pull/4583)
  [#4636](https://github.com/mixxxdj/mixxx/pull/4636)
  [#4644](https://github.com/mixxxdj/mixxx/pull/4644)
  [#4653](https://github.com/mixxxdj/mixxx/pull/4653)
  [#4655](https://github.com/mixxxdj/mixxx/pull/4655)
  [#4609](https://github.com/mixxxdj/mixxx/pull/4609)
  [#4647](https://github.com/mixxxdj/mixxx/pull/4647)
  [#4660](https://github.com/mixxxdj/mixxx/pull/4660)
  [#4663](https://github.com/mixxxdj/mixxx/pull/4663)
  [#4661](https://github.com/mixxxdj/mixxx/pull/4661)
  [#4670](https://github.com/mixxxdj/mixxx/pull/4670)
  [#4672](https://github.com/mixxxdj/mixxx/pull/4672)
  [#4674](https://github.com/mixxxdj/mixxx/pull/4674)
  [#4618](https://github.com/mixxxdj/mixxx/pull/4618)
  [#4690](https://github.com/mixxxdj/mixxx/pull/4690)
  [#4724](https://github.com/mixxxdj/mixxx/pull/4724)
  [#4730](https://github.com/mixxxdj/mixxx/pull/4730)
  [#4731](https://github.com/mixxxdj/mixxx/pull/4731)
  [#4732](https://github.com/mixxxdj/mixxx/pull/4732)
  [#4728](https://github.com/mixxxdj/mixxx/pull/4728)
  [#4737](https://github.com/mixxxdj/mixxx/pull/4737)
  [#4739](https://github.com/mixxxdj/mixxx/pull/4739)
  [#4745](https://github.com/mixxxdj/mixxx/pull/4745)
  [#4765](https://github.com/mixxxdj/mixxx/pull/4765)
  [#4767](https://github.com/mixxxdj/mixxx/pull/4767)
  [#4797](https://github.com/mixxxdj/mixxx/pull/4797)
  [#4782](https://github.com/mixxxdj/mixxx/pull/4782)
  [#4780](https://github.com/mixxxdj/mixxx/pull/4800)
  [#4791](https://github.com/mixxxdj/mixxx/pull/4791)
  [#4848](https://github.com/mixxxdj/mixxx/pull/4848)
  [#4856](https://github.com/mixxxdj/mixxx/pull/4856)
  [#4860](https://github.com/mixxxdj/mixxx/pull/4860)
  [#4867](https://github.com/mixxxdj/mixxx/pull/4867)
  [#4888](https://github.com/mixxxdj/mixxx/pull/4888)

## [2.3.4](https://launchpad.net/mixxx/+milestone/2.3.4) (unreleased)

* Track Properties: show 'date added' as local time [#4838](https://github.com/mixxxdj/mixxx/pull/4838) [lp:1980658](https://bugs.launchpad.net/mixxx/+bug/1980658)
* Numark DJ2GO2 Touch: Fix  sliders and knobs [#4835](https://github.com/mixxxdj/mixxx/pull/4835) [lp:1948596](https://bugs.launchpad.net/mixxx/+bug/1948596)
* Potmeters: Add support for arbitrary maximums in 7-/14-bit handlers from controller scripts [#4495](https://github.com/mixxxdj/mixxx/pull/4495)
* Shade: Fix library sidebar splitter glitch [#4828](https://github.com/mixxxdj/mixxx/pull/4828) [lp:1979823](https://bugs.launchpad.net/mixxx/+bug/1979823)
* MacOS builds: Perform ad-hoc signing of macOS bundle in Pull request and personal repositories [#4774](https://github.com/mixxxdj/mixxx/pull/4774)
* Waveform: Avoid visual glitch with ranges < 1 px [#4804](https://github.com/mixxxdj/mixxx/pull/4804)
* Traktor S3: Fix issues with sampler and hotcue buttons [#4676](https://github.com/mixxxdj/mixxx/pull/4676)
* Build Mixxx on MacOS 11, replacing deprecated MacOS 10.15 [#4863](https://github.com/mixxxdj/mixxx/pull/4863)
* EQ preferences: properly restore 'One EQ for all decks' setting [#4886](https://github.com/mixxxdj/mixxx/pull/4886)
* MC7000: Fix off-by-one indexing in the controller script [#4902](https://github.com/mixxxdj/mixxx/pull/4902)
* Cover Art: Fix picking wrong cover file, when track file name contains extra dots [#4909](https://github.com/mixxxdj/mixxx/pull/4909)

## [2.3.3](https://launchpad.net/mixxx/+milestone/2.3.3) (2022-06-21)

* Pioneer DDJ-SB3: Fix controller breaking when releasing the shift button [#4659](https://github.com/mixxxdj/mixxx/pull/4659)
* Traktor S3: Push two deck switches to explicitly clone decks [#4665](https://github.com/mixxxdj/mixxx/pull/4665) [#4671](https://github.com/mixxxdj/mixxx/pull/4671) [lp:1960680](https://bugs.launchpad.net/mixxx/+bug/1960680)
* Behringer DDM4000: Improve stability and add soft-takeover for encoder knobs [#4318](https://github.com/mixxxdj/mixxx/pull/4318) [#4799](https://github.com/mixxxdj/mixxx/pull/4799)
* Denon MC7000: Fix 'inverted shift' bug in the controller mapping [#4755](https://github.com/mixxxdj/mixxx/pull/4755)
* Fix spinback and break effect in the controller engine [#4708](https://github.com/mixxxdj/mixxx/pull/4708)
* Fix scratch on first wheel touch [#4761](https://github.com/mixxxdj/mixxx/pull/4761) [lp:1800343](https://bugs.launchpad.net/mixxx/+bug/1800343)
* Preferences: Prevent controller settings being treated as changed even though they were not [#4721](https://github.com/mixxxdj/mixxx/pull/4721) [lp:1920844](https://bugs.launchpad.net/mixxx/+bug/1920844)
* Fix rare crash when closing the progress dialog [#4695](https://github.com/mixxxdj/mixxx/pull/4695)
* Prevent preferences dialog from going out of screen [#4613](https://github.com/mixxxdj/mixxx/pull/4613)
* Fix undesired jump-cuts in Auto DJ [#4693](https://github.com/mixxxdj/mixxx/pull/4693) [lp:1948975](https://bugs.launchpad.net/mixxx/+bug/1948975) [lp:1893197](https://bugs.launchpad.net/mixxx/+bug/1893197)
* Fix bug that caused Auto DJ to stop playback after some time [#4698](https://github.com/mixxxdj/mixxx/pull/4698) [lp:1893197](https://bugs.launchpad.net/mixxx/+bug/1893197) [lp:1961970](https://bugs.launchpad.net/mixxx/+bug/1961970)
* Do not reset crossfader when Auto DJ is deactivated [#4714](https://github.com/mixxxdj/mixxx/pull/4714) [lp:1965298](https://bugs.launchpad.net/bugs/1965298)
* Change the minimum Auto DJ transition time to -99 [#4768](https://github.com/mixxxdj/mixxx/pull/4768) [lp:1975552](https://bugs.launchpad.net/mixxx/+bug/1975552)
* Samplers, crates, playlists: fix storing import/export paths [#4699](https://github.com/mixxxdj/mixxx/pull/4699) [lp:1964508](https://bugs.launchpad.net/bugs/1964508)
* Library: keep hidden tracks in history [#4725](https://github.com/mixxxdj/mixxx/pull/4725)
* Broadcasting: allow multiple connections to same mount if only one is enabled [#4750](https://github.com/mixxxdj/mixxx/pull/4750) [lp:1972813](https://bugs.launchpad.net/mixxx/+bug/1972813)
* Fix a rare mouse vanish bug when controlling knobs [#4744](https://github.com/mixxxdj/mixxx/pull/4744) [lp:1130794](https://bugs.launchpad.net/mixxx/+bug/1130794) [lp:1969278](https://bugs.launchpad.net/mixxx/+bug/1969278)
* Restore keylock from configuration and fix pitch ratio rounding issue [#4756](https://github.com/mixxxdj/mixxx/pull/4756) [lp:1943180](https://bugs.launchpad.net/mixxx/+bug/1943180)
* Improve CSV export of playlists and crates and fix empty rating column [#4762](https://github.com/mixxxdj/mixxx/pull/4762)
* Fix passthrough-related crash in waveform code [#4789](https://github.com/mixxxdj/mixxx/pull/4789) [#4791](https://github.com/mixxxdj/mixxx/pull/4791) [lp:1959489](https://bugs.launchpad.net/mixxx/+bug/1959489) [lp:1977662](https://bugs.launchpad.net/mixxx/+bug/1977662)
* Passthrough: stop rendering waveforms and disable Cue/Play indicators [4793](https://github.com/mixxxdj/mixxx/pull/4793)

## [2.3.2](https://launchpad.net/mixxx/+milestone/2.3.2) (2022-01-31)

* Playlist: Enable sorting by color [#4352](https://github.com/mixxxdj/mixxx/pull/4352) [lp:1945976](https://bugs.launchpad.net/mixxx/+bug/1945976)
* Fix crash when using Doubling/Halving/etc. BPM from track's Properties window on tracks without BPM [#4587](https://github.com/mixxxdj/mixxx/pull/4587) [lp:1955853](https://bugs.launchpad.net/mixxx/+bug/1955853)
* Fix writing metadata on Windows for files that have never been played [#4586](https://github.com/mixxxdj/mixxx/pull/4586) [lp:1955331](https://bugs.launchpad.net/mixxx/+bug/1955331)
* Preserve file creation time when writing metadata on Windows [#4586](https://github.com/mixxxdj/mixxx/pull/4586) [lp1955314](https://bugs.launchpad.net/mixxx/+bug/1955314)
* Fix handling of file extension when importing and exporting sampler settings [#4539](https://github.com/mixxxdj/mixxx/pull/4539)
* Fix crash when using an empty directory as resource path using the `--resource-path` command line option [#4575](https://github.com/mixxxdj/mixxx/pull/4575) [lp:1934560](https://bugs.launchpad.net/mixxx/+bug/1934560)
* Pioneer DDJ-SB3: Add controller mapping [#3821](https://github.com/mixxxdj/mixxx/pull/3821)
* Don't wipe sound config during startup if configured devices are unavailable [#4544](https://github.com/mixxxdj/mixxx/pull/4544)
* Append selected file extension when exporting to playlist files [#4531](https://github.com/mixxxdj/mixxx/pull/4531) [lp:1889352](https://bugs.launchpad.net/mixxx/+bug/1889352)
* Fix crash when using midi.sendShortMsg and platform vnc [#4635](https://github.com/mixxxdj/mixxx/pull/4635) [lp:1956144](https://bugs.launchpad.net/mixxx/+bug/1956144)
* Traktor S3: Fix timedelta calculation bugs [#4646](https://github.com/mixxxdj/mixxx/pull/4646) [lp:1958925](https://bugs.launchpad.net/mixxx/+bug/1958925)

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
* Add support for HiDPI scale factors of 125% and 175% (only with Qt 5.14+) [lp1938102](https://bugs.launchpad.net/mixxx/+bug/1938102) [#4161](https://github.com/mixxxdj/mixxx/pull/4161)
* Fix unhandled exception when parsing corrupt Rekordbox PDB files [lp1933853](https://bugs.launchpad.net/mixxx/+bug/1933853) [#4040](https://github.com/mixxxdj/mixxx/pull/4040)
* Fix Echo effect adding left channel samples to right channel [#4141](https://github.com/mixxxdj/mixxx/pull/4141)
* Fix bad phase seek when starting from preroll [lp1930143](https://bugs.launchpad.net/mixxx/+bug/1930143) [#4093](https://github.com/mixxxdj/mixxx/pull/4093)
* Fix bad phase seek when a channel's audible status changes [#4156](https://github.com/mixxxdj/mixxx/pull/4156)
* Tango skin: Show crossfader assign buttons by default [#4046](https://github.com/mixxxdj/mixxx/pull/4046)
* Fix keyfinder library in arm64 builds [#4047](https://github.com/mixxxdj/mixxx/pull/4047)
* Fix wrong track being recorded in History [lp1933991](https://bugs.launchpad.net/mixxx/+bug/1933991) [#4041](https://github.com/mixxxdj/mixxx/pull/4041) [#4059](https://github.com/mixxxdj/mixxx/pull/4059) [#4107](https://github.com/mixxxdj/mixxx/pull/4107) [#4296](https://github.com/mixxxdj/mixxx/pull/4296)
* Fix support for relative paths in the skin system which caused missing images in third-party skins [#4151](https://github.com/mixxxdj/mixxx/pull/4151)
* Fix relocation of directories with special/reserved characters in path name [#4146](https://github.com/mixxxdj/mixxx/pull/4146)
* Update keyboard shortcuts sheet [#4042](https://github.com/mixxxdj/mixxx/pull/4042)
* Library: resize the Played checkbox and BPM lock with the library font [#4050](https://github.com/mixxxdj/mixxx/pull/4050)
* Don't allow Input focus on waveforms [#4134](https://github.com/mixxxdj/mixxx/pull/4134)
* Fix performance issue on AArch64 by enabling flush-to-zero for floating-point arithmetic [#4144](https://github.com/mixxxdj/mixxx/pull/4144)
* Fix custom key notation not restored correctly after restart [#4136](https://github.com/mixxxdj/mixxx/pull/4136)
* Traktor S3: Disable scratch when switching decks to prevent locked scratch issue [#4073](https://github.com/mixxxdj/mixxx/pull/4073)
* FFmpeg: Ignore inaudible samples before start of stream [#4245](https://github.com/mixxxdj/mixxx/pull/4245)
* Controller Preferences: Don't automatically enable checkbox if controller is disabled [#4244](https://github.com/mixxxdj/mixxx/pull/4244) [lp:1941042](https://bugs.launchpad.net/mixxx/+bug/1941042)
* Tooltips: Always show tooltips in preferences [#4198](https://github.com/mixxxdj/mixxx/pull/4198) [lp:1840493](https://bugs.launchpad.net/mixxx/+bug/1840493)
* Tooltips: Use item label for tooltips in library side bar and show ID when debugging. [#4247](https://github.com/mixxxdj/mixxx/pull/4247)
* Library sidebar: Also activate items on PageUp/Down events. [#4237](https://github.com/mixxxdj/mixxx/pull/4237)
* Fix handling of preview button cell events in developer mode. [#4264](https://github.com/mixxxdj/mixxx/pull/4264) [lp:1929141](https://bugs.launchpad.net/mixxx/+bug/1929141)
* Auto DJ: Fix bug which could make an empty track stop Auto DJ. [#4267](https://github.com/mixxxdj/mixxx/pull/4267) [lp:1941743](https://bugs.launchpad.net/mixxx/+bug/1941743)
* Fix Auto DJ skipping tracks randomly [#4319](https://github.com/mixxxdj/mixxx/pull/4319) [lp1941989](https://bugs.launchpad.net/mixxx/+bug/1941989)
* Fix high CPU load due to extremely high internal sync clock values [#4312](https://github.com/mixxxdj/mixxx/pull/4312) [lp1943320](https://bugs.launchpad.net/mixxx/+bug/1943320)
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

* Add configurable color per track [#2470](https://github.com/mixxxdj/mixxx/pull/2470) [#2539](https://github.com/mixxxdj/mixxx/pull/2539) [#2545](https://github.com/mixxxdj/mixxx/pull/2545) [#2630](https://github.com/mixxxdj/mixxx/pull/2630) [lp:1100882](https://bugs.launchpad.net/mixxx/+bug/1100882)
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
* Workaround Linux skin change crash [#3144](https://github.com/mixxxdj/mixxx/pull/3144) [lp:1885009](https://bugs.launchpad.net/mixxx/+bug/1885009)
* Fix touch control [lp:1895431](https://bugs.launchpad.net/mixxx/+bug/1895431)
* Fix broken knob interaction on touchscreens [#3512](https://github.com/mixxxdj/mixxx/pull/3512)
* AutoDJ: Make "enable" shortcut work after startup [#3242](https://github.com/mixxxdj/mixxx/pull/3242)
* Add rate range indicator [#3693](https://github.com/mixxxdj/mixxx/pull/3693)
* Allow menubar to be styled [#3372](https://github.com/mixxxdj/mixxx/pull/3372) [#3788](https://github.com/mixxxdj/mixxx/pull/3788)
* Add Donate button to About dialog [#3838](https://github.com/mixxxdj/mixxx/pull/3838) [#3846](https://github.com/mixxxdj/mixxx/pull/3846)
* Add Scrollable Skin Widget [#3890](https://github.com/mixxxdj/mixxx/pull/3890)
* Fix minor visual issues in Skins [#3958](https://github.com/mixxxdj/mixxx/pull/3958/) [#3954](https://github.com/mixxxdj/mixxx/pull/3954/) [#3941](https://github.com/mixxxdj/mixxx/pull/3941/) [#3938](https://github.com/mixxxdj/mixxx/pull/3938/) [#3936](https://github.com/mixxxdj/mixxx/pull/3936/) [#3886](https://github.com/mixxxdj/mixxx/pull/3886/) [#3927](https://github.com/mixxxdj/mixxx/pull/3927/) [#3844](https://github.com/mixxxdj/mixxx/pull/3844/) [#3933](https://github.com/mixxxdj/mixxx/pull/3933/) [#3835](https://github.com/mixxxdj/mixxx/pull/3835/) [#3902](https://github.com/mixxxdj/mixxx/pull/3902) [#3931](https://github.com/mixxxdj/mixxx/pull/3931)

### Music Feature Analysis

* Multithreaded analysis for much faster batch analysis on multicore CPUs [#1624](https://github.com/mixxxdj/mixxx/pull/1624) [#2142](https://github.com/mixxxdj/mixxx/pull/2142) [lp:1641153](https://bugs.launchpad.net/mixxx/+bug/1641153)
* Fix bugs affecting key detection accuracy [#2137](https://github.com/mixxxdj/mixxx/pull/2137) [#2152](https://github.com/mixxxdj/mixxx/pull/2152) [#2112](https://github.com/mixxxdj/mixxx/pull/2112) [#2136](https://github.com/mixxxdj/mixxx/pull/2136)
  * Note: Users who have not manually corrected keys are advised to clear all keys in their library by pressing Ctrl + A in the library, right clicking, going to Reset -> Key, then reanalyzing their library. This will freeze the GUI while Mixxx clears the keys; this is a known problem that we will not be able to fix for 2.3. Wait until it is finished and you will be able to reanalyze tracks for better key detection results.
* Remove VAMP plugin support and use Queen Mary DSP library directly. vamp-plugin-sdk and vamp-hostsdk are no longer required dependencies. [#926](https://github.com/mixxxdj/mixxx/pull/926)
* Improvements BPM detection on non-const beatgrids [#3626](https://github.com/mixxxdj/mixxx/pull/3626)
* Fix const beatgrid placement [#3965](https://github.com/mixxxdj/mixxx/pull/3965) [#3973](https://github.com/mixxxdj/mixxx/pull/3973)

### Music Library

* Add support for searching for empty fields (for example `crate:""`) [lp:1788086](https://bugs.launchpad.net/mixxx/+bug/1788086)
* Improve synchronization of track metadata and file tags [#2406](https://github.com/mixxxdj/mixxx/pull/2406)
* Library Scanner: Improve hashing of directory contents [#2497](https://github.com/mixxxdj/mixxx/pull/2497)
* Rework of Cover Image Hashing [lp:1607097](https://bugs.launchpad.net/mixxx/+bug/1607097) [#2507](https://github.com/mixxxdj/mixxx/pull/2507) [#2508](https://github.com/mixxxdj/mixxx/pull/2508)
* MusicBrainz: Handle 301 status response [#2510](https://github.com/mixxxdj/mixxx/pull/2510)
* MusicBrainz: Add extended metadata support [lp:1581256](https://bugs.launchpad.net/mixxx/+bug/1581256) [#2522](https://github.com/mixxxdj/mixxx/pull/2522)
* TagLib: Fix detection of empty or missing file tags [lp:1865957](https://bugs.launchpad.net/mixxx/+bug/1865957) [#2535](https://github.com/mixxxdj/mixxx/pull/2535)
* Fix caching of duplicate tracks that reference the same file [#3027](https://github.com/mixxxdj/mixxx/pull/3027)
* Use 6 instead of only 4 compatible musical keys (major/minor) [#3205](https://github.com/mixxxdj/mixxx/pull/3205)
* Fix possible crash when trying to refocus the tracks table while another Mixxx window has focus [#3201](https://github.com/mixxxdj/mixxx/pull/3201)
* Don't create new tags in file when exporting metadata to it [#3898](https://github.com/mixxxdj/mixxx/pull/3898)
* Fix playlist files beginning with non-english characters not being loaded [#3916](https://github.com/mixxxdj/mixxx/pull/3916)
* Enable sorting in "Hidden Tracks" and "Missing Tracks" views [#3828](https://github.com/mixxxdj/mixxx/pull/3828) [lp:1828555](https://bugs.launchpad.net/mixxx/+bug/1828555/) [lp:1924616](https://bugs.launchpad.net/mixxx/+bug/1924616/)
* Fix track table being empty after start [#3935](https://github.com/mixxxdj/mixxx/pull/3935/) [lp:1930546](https://bugs.launchpad.net/mixxx/+bug/1930546/) [lp:1924843](https://bugs.launchpad.net/mixxx/+bug/1924843/)

### Audio Codecs

* Add FFmpeg audio decoder, bringing support for ALAC files [#1356](https://github.com/mixxxdj/mixxx/pull/1356)
* Include LAME MP3 encoder with Mixxx now that the MP3 patent has expired [lp:1294128](https://bugs.launchpad.net/mixxx/+bug/1294128) [buildserver:#37](https://github.com/mixxxdj/buildserver/pull/37) [buildserver:9e8bcee](https://github.com/mixxxdj/buildserver/commit/9e8bcee771731920ae82f3e076d43f0fb51e5027)
* Add Opus streaming and recording support. [lp:1338413](https://bugs.launchpad.net/mixxx/+bug/1338413)
* Remove support for SoundSource plugins because the code was not well-maintained and could lead to crashes [lp:1792747](https://bugs.launchpad.net/mixxx/+bug/1792747)
* Add HE-AAC encoding capabilities for recording and broadcasting [#3615](https://github.com/mixxxdj/mixxx/pull/3615)

### Audio Engine

* Fix loss of precision when dealing with floating-point sample positions while setting loop out position and seeking using vinyl control [#3126](https://github.com/mixxxdj/mixxx/pull/3126) [#3127](https://github.com/mixxxdj/mixxx/pull/3127)
* Prevent moving a loop beyond track end [#3117](https://github.com/mixxxdj/mixxx/pull/3117) [lp:1799574](https://bugs.launchpad.net/mixxx/+bug/1799574)
* Fix possible memory corruption using JACK on Linux [#3160](https://github.com/mixxxdj/mixxx/pull/3160)
* Fix changing of vinyl lead-in time [lp:1915483](https://bugs.launchpad.net/mixxx/+bug/1915483) [#3781](https://github.com/mixxxdj/mixxx/pull/3781)
* Fix tempo change of non-const beatgrid track on audible deck when cueing another track [#3772](https://github.com/mixxxdj/mixxx/pull/3772)
* Fix crash when changing effect unit routing [#3882](https://github.com/mixxxdj/mixxx/pull/3882) [lp:1775497](https://bugs.launchpad.net/mixxx/+bug/1775497)
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
* Update controller mapping for Allen & Heath Xone K2 to add intro/outro cues [#2236](https://github.com/mixxxdj/mixxx/pull/2236)
* Update controller mapping for Hercules P32 for more accurate headmix control [#3537](https://github.com/mixxxdj/mixxx/pull/3537)
* Update controller mapping for Native Instruments Traktor Kontrol S4MK2 to add auto-slip mode and pitch fader range [#3331](https://github.com/mixxxdj/mixxx/pull/3331)
* Fix Pioneer DDJ-SB2 controller mapping auto tempo going to infinity bug [#2559](https://github.com/mixxxdj/mixxx/pull/2559) [lp:1846403](https://bugs.launchpad.net/mixxx/+bug/1846403)
* Fix Numark Mixtrack Pro 3 controller mapping inverted FX on/off control [#3758](https://github.com/mixxxdj/mixxx/pull/3758)
* Gracefully handle MIDI overflow [#825](https://github.com/mixxxdj/mixxx/pull/825)

### Other

* Add CMake build system with `ccache` and `sccache` support for faster compilation times and remove SCons [#2280](https://github.com/mixxxdj/mixxx/pull/2280) [#3618](https://github.com/mixxxdj/mixxx/pull/3618)
* Make Mixxx compile even though `QT_NO_OPENGL` or `QT_OPENGL_ES_2` is defined (fixes build on Raspberry Pi) [lp:1863440](https://bugs.launchpad.net/mixxx/+bug/1863440) [#2504](https://github.com/mixxxdj/mixxx/pull/2504)
* Fix ARM build issues [#3602](https://github.com/mixxxdj/mixxx/pull/3602)
* Fix missing manual in DEB package [lp:1889776](https://bugs.launchpad.net/mixxx/+bug/1889776) [#2985](https://github.com/mixxxdj/mixxx/pull/2985)
* Add macOS codesigning and notarization to fix startup warnings [#3281](https://github.com/mixxxdj/mixxx/pull/3281)
* Don't trash user configuration if an error occurs when writing [#3192](https://github.com/mixxxdj/mixxx/pull/3192)
* Enable CUE sheet recording by default [#3374](https://github.com/mixxxdj/mixxx/pull/3374)
* Fix crash when double clicking GLSL waveforms with right mouse button [#3904](https://github.com/mixxxdj/mixxx/pull/3904)
* Derive Mixxx version from `git describe` [#3824](https://github.com/mixxxdj/mixxx/pull/3824) [#3841](https://github.com/mixxxdj/mixxx/pull/3841) [#3848](https://github.com/mixxxdj/mixxx/pull/3848)
* Improve tapping the BPM of a deck [#3790](https://github.com/mixxxdj/mixxx/pull/3790) [lp:1882776](https://bugs.launchpad.net/mixxx/+bug/1882776)
* And countless other small fixes and improvements (too many to list them all!)

## [2.2.4](https://launchpad.net/mixxx/+milestone/2.2.4) (2020-06-27)

* Store default recording format after "Restore Defaults" [lp:1857806](https://bugs.launchpad.net/mixxx/+bug/1857806) [#2414](https://github.com/mixxxdj/mixxx/pull/2414)
* Prevent infinite loop when decoding corrupt MP3 files [#2417](https://github.com/mixxxdj/mixxx/pull/2417)
* Add workaround for broken libshout versions [#2040](https://github.com/mixxxdj/mixxx/pull/2040) [#2438](https://github.com/mixxxdj/mixxx/pull/2438)
* Speed up purging of tracks [lp:1845837](https://bugs.launchpad.net/mixxx/+bug/1845837) [#2393](https://github.com/mixxxdj/mixxx/pull/2393)
* Don't stop playback if vinyl passthrough input is configured and PASS button is pressed [#2474](https://github.com/mixxxdj/mixxx/pull/2474)
* Fix debug assertion for invalid crate names [lp:1861431](https://bugs.launchpad.net/mixxx/+bug/1861431) [#2477](https://github.com/mixxxdj/mixxx/pull/2477)
* Fix crashes when executing actions on tracks that already disappeared from the DB [#2527](https://github.com/mixxxdj/mixxx/pull/2527)
* AutoDJ: Skip next track when both deck are playing [lp:1399974](https://bugs.launchpad.net/mixxx/+bug/1399974) [#2531](https://github.com/mixxxdj/mixxx/pull/2531)
* Tweak scratch parameters for Mixtrack Platinum [#2028](https://github.com/mixxxdj/mixxx/pull/2028)
* Fix auto tempo going to infinity on Pioneer DDJ-SB2 [#2559](https://github.com/mixxxdj/mixxx/pull/2559)
* Fix bpm.tapButton logic and reject missed & double taps [#2594](https://github.com/mixxxdj/mixxx/pull/2594)
* Add controller mapping for Native Instruments Traktor Kontrol S2 MK3 [#2348](https://github.com/mixxxdj/mixxx/pull/2348)
* Add controller mapping for Soundless joyMIDI [#2425](https://github.com/mixxxdj/mixxx/pull/2425)
* Add controller mapping for Hercules DJControl Inpulse 300 [#2465](https://github.com/mixxxdj/mixxx/pull/2465)
* Add controller mapping for Denon MC7000 [#2546](https://github.com/mixxxdj/mixxx/pull/2546)
* Add controller mapping for Stanton DJC.4 [#2607](https://github.com/mixxxdj/mixxx/pull/2607)
* Fix broadcasting via broadcast/recording input [lp:1876222](https://bugs.launchpad.net/mixxx/+bug/1876222) [#2743](https://github.com/mixxxdj/mixxx/pull/2743)
* Only apply ducking gain in manual ducking mode when talkover is enabed [lp:1394968](https://bugs.launchpad.net/mixxx/+bug/1394968) [lp:1737113](https://bugs.launchpad.net/mixxx/+bug/1737113) [lp:1662536](https://bugs.launchpad.net/mixxx/+bug/1662536) [#2759](https://github.com/mixxxdj/mixxx/pull/2759)
* Ignore MIDI Clock Messages (0xF8) because they are not usable in Mixxx and inhibited the screensaver [#2786](https://github.com/mixxxdj/mixxx/pull/2786)

## [2.2.3](https://launchpad.net/mixxx/+milestone/2.2.3) (2019-11-24)

* Don't make users reconfigure sound hardware when it has not changed [#2253](https://github.com/mixxxdj/mixxx/pull/2253)
* Fix MusicBrainz metadata lookup [lp:1848887](https://bugs.launchpad.net/mixxx/+bug/1848887) [#2328](https://github.com/mixxxdj/mixxx/pull/2328)
* Fix high DPI scaling of cover art [#2247](https://github.com/mixxxdj/mixxx/pull/2247)
* Fix high DPI scaling of cue point labels on scrolling waveforms [#2331](https://github.com/mixxxdj/mixxx/pull/2331)
* Fix high DPI scaling of sliders in Tango skin [#2318](https://github.com/mixxxdj/mixxx/pull/2318)
* Fix sound dropping out during recording [lp:1842679](https://bugs.launchpad.net/mixxx/+bug/1842679) [#2265](https://github.com/mixxxdj/mixxx/pull/2265) [#2305](https://github.com/mixxxdj/mixxx/pull/2305) [#2308](https://github.com/mixxxdj/mixxx/pull/2308) [#2309](https://github.com/mixxxdj/mixxx/pull/2309)
* Fix rare crash on application shutdown [#2293](https://github.com/mixxxdj/mixxx/pull/2293)
* Workaround various rare bugs caused by database inconsistencies [lp:1846971](https://bugs.launchpad.net/mixxx/+bug/1846971) [#2321](https://github.com/mixxxdj/mixxx/pull/2321)
* Improve handling of corrupt FLAC files [#2315](https://github.com/mixxxdj/mixxx/pull/2315)
* Don't immediately jump to loop start when loop_out is pressed in quantized mode [lp:1837077](https://bugs.launchpad.net/mixxx/+bug/1837077) [#2269](https://github.com/mixxxdj/mixxx/pull/2269)
* Preserve order of tracks when dragging and dropping from AutoDJ to playlist [lp:1829601](https://bugs.launchpad.net/mixxx/+bug/1829601) [#2237](https://github.com/mixxxdj/mixxx/pull/2237)
* Explicitly use X11 Qt platform plugin instead of Wayland in .desktop launcher [lp:1850729](https://bugs.launchpad.net/mixxx/+bug/1850729) [#2340](https://github.com/mixxxdj/mixxx/pull/2340)
* Pioneer DDJ-SX: fix delayed sending of MIDI messages with low audio buffer sizes [#2326](https://github.com/mixxxdj/mixxx/pull/2326)
* Enable modplug support on Linux by default [lp:1840537](https://bugs.launchpad.net/mixxx/+bug/1840537) [#2244](https://github.com/mixxxdj/mixxx/pull/2244) [#2272](https://github.com/mixxxdj/mixxx/pull/2272)
* Fix keyboard shortcut for View > Skin Preferences [lp:1851993](https://bugs.launchpad.net/mixxx/+bug/1851993) [#2358](https://github.com/mixxxdj/mixxx/pull/2358) [#2372](https://github.com/mixxxdj/mixxx/pull/2372)
* Reloop Terminal Mix: Fix mapping of sampler buttons 5-8 [lp:1846966](https://bugs.launchpad.net/mixxx/+bug/1846966) [#2330](https://github.com/mixxxdj/mixxx/pull/2330)

## [2.2.2](https://launchpad.net/mixxx/+milestone/2.2.2) (2019-08-10)

* Fix battery widget with upower <= 0.99.7. [#2221](https://github.com/mixxxdj/mixxx/pull/2221)
* Fix BPM adjust in BpmControl. [lp:1836480](https://bugs.launchpad.net/mixxx/+bug/1836480)
* Disable track metadata export for .ogg files and TagLib 1.11.1. [lp:1833190](https://bugs.launchpad.net/mixxx/+bug/1833190)
* Fix interaction of hot cue buttons and looping. [lp:1778246](https://bugs.launchpad.net/mixxx/+bug/1778246)
* Fix detection of moved tracks. [#2197](https://github.com/mixxxdj/mixxx/pull/2197)
* Fix playlist import. [#2200](https://github.com/mixxxdj/mixxx/pull/2200) [lp:1687828](https://bugs.launchpad.net/mixxx/+bug/1687828)
* Fix updating playlist labels. [lp:1837315](https://bugs.launchpad.net/mixxx/+bug/1837315)
* Fix potential segfault on exit. [lp:1828360](https://bugs.launchpad.net/mixxx/+bug/1828360)
* Fix parsing of invalid BPM values in MP3 files. [lp:1832325](https://bugs.launchpad.net/mixxx/+bug/1832325)
* Fix crash when removing rows from empty model. [#2128](https://github.com/mixxxdj/mixxx/pull/2128)
* Fix high DPI scaling of RGB overview waveforms. [#2090](https://github.com/mixxxdj/mixxx/pull/2090)
* Fix for OpenGL SL detection on macOS. [lp:1828019](https://bugs.launchpad.net/mixxx/+bug/1828019)
* Fix OpenGL ES detection. [lp:1825461](https://bugs.launchpad.net/mixxx/+bug/1825461)
* Fix FX1/2 buttons missing Mic unit in Deere (64 samplers). [lp:1837716](https://bugs.launchpad.net/mixxx/+bug/1837716)
* Tango64: Re-enable 64 samplers. [#2223](https://github.com/mixxxdj/mixxx/pull/2223)
* Numark DJ2Go re-enable note-off for deck A cue button. [#2087](https://github.com/mixxxdj/mixxx/pull/2087)
* Replace Flanger with QuickEffect in keyboard mapping. [#2233](https://github.com/mixxxdj/mixxx/pull/2233)

## [2.2.1](https://launchpad.net/mixxx/+milestone/2.2.1) (2019-04-22)

* Include all fixes from Mixxx 2.1.7 and 2.1.8
* Fix high CPU usage on MAC due to preview column [lp:1812763](https://bugs.launchpad.net/mixxx/+bug/1812763)
* Fix HID controller output on Windows with common-hid-packet-parser.js
* Fix rendering slow down by not using QStylePainter in WSpinny [lp:1530720](https://bugs.launchpad.net/mixxx/+bug/1530720)
* Fix broken Mic mute button [lp:1782568](https://bugs.launchpad.net/mixxx/+bug/1782568)
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
* Fix export of BPM track file metadata. [lp:1816490](https://bugs.launchpad.net/mixxx/+bug/1816490)
* Fix sending of broadcast metadata with TLS enabled libshout 2.4.1. [lp:1817395](https://bugs.launchpad.net/mixxx/+bug/1817395)
* Fix resdicovering purged tracks in all cases. [lp:1821514](https://bugs.launchpad.net/mixxx/+bug/1821514)
* Fix dropping track from OSX Finder. [lp:1822424](https://bugs.launchpad.net/mixxx/+bug/1822424)

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
[Mixxx 2.1.2 running much slower than 2.1.1](https://mixxx.org/forums/viewtopic.php?f=3&t=12082)

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
* Configurable, Resizeable User Interface with 3 Brand New Skins
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
