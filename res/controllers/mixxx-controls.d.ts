// Mixxx control types
// Generated file, don't change by hand, will be overwritten with every controls manual change

declare namespace MixxxControls {
  type GroupName =
    /**
     * The [App] group contains controls that do not belong to a specific channel, the mixer or the effects engine.
     */
    | "[App]"

    /**
     * The [Master] group generally corresponds to controls that affect the mixing engine. This will bear some similarity to what you will find on a DJ mixer (e.g. crossfader controls, headphone cueing controls, etc.).
     */
    | "[Master]"

    /**
     * The [Main] group contains controls for the main mix output meters and clipping indicators.
     */
    | "[Main]"

    /**
     * Each deck in Mixxx corresponds to a [ChannelN] group. Whenever you see [ChannelN], think “Deck N”. N can range from 1 to the number of active decks in Mixxx.
     */
    | `[Channel${number}]`

    /**
     * Preview decks are identical to regular decks, but are used for previewing tracks; their controls mirror [ChannelN].
     */
    | `[PreviewDeck${number}]`

    /**
     * Sample decks are identical to regular decks, but are used for playing samples; their controls mirror [ChannelN].
     */
    | `[Sampler${number}]`

    /**
     * The [Sampler] group contains global controls for managing sampler banks.
     */
    | "[Sampler]"

    /**
     * The [MicrophoneN] group contains controls for microphone input channels, including talkover and monitoring.
     */
    | `[Microphone${number}]`

    /**
     * The [AuxiliaryN] group contains controls for auxiliary input channels.
     */
    | `[Auxiliary${number}]`

    /**
     * The [VinylControl] group can toggle the vinyl control feature.
     */
    | "[VinylControl]"

    /**
     * The controls in the [Recording] group can be used to query and control the recording of your mix.
     */
    | "[Recording]"

    /**
     * The [AutoDJ] controls allow interacting with AutoDJ.
     */
    | "[AutoDJ]"

    /**
     * The controls in the [Library] group can be used to navigate the library. Note that [Library],MoveUp and other Move and Scroll controls emulate keypresses and therefore require the Mixxx window to be focused.
     */
    | "[Library]"

    /**
     * The [Shoutcast] group contains controls for broadcasting to a Shoutcast server.
     */
    | "[Shoutcast]"

    /**
     * [Playlist] controls allow navigating the sidebar and tracks table directly without considering the currently focused widget. This is helpful when another application’s window is focused.
     * This group is going to be deprecated at some point, with its controls added to [Library] above.
     */
    | "[Playlist]"

    /**
     * The [Controls] group contains controls that didn’t fit in any other group.
     */
    | "[Controls]"

    /**
     * The [EffectRack1] group contains global controls for the effects rack.
     */
    | "[EffectRack1]"

    /**
     * The [EffectRack1_EffectUnitN] group contains controls for an individual effects unit.
     */
    | `[EffectRack1_EffectUnit${number}]`

    /**
     * The [EffectRack1_EffectUnitN_EffectM] group contains controls for a single effect slot within an effects unit.
     */
    | `[EffectRack1_EffectUnit${number}_Effect${number}]`

    /**
     * The [QuickEffectRack1] group contains global controls for the quick effects rack.
     */
    | "[QuickEffectRack1]"

    /**
     * The [EqualizerRack1] group contains global controls for the EQ rack.
     */
    | "[EqualizerRack1]"

    /**
     * The [QuickEffectRack1_[ChannelI]] group contains per-deck quick effect controls.
     */
    | `[QuickEffectRack1_[Channel${number}]]`

    /**
     * The [EqualizerRack1_[ChannelI]] group contains per-deck EQ rack controls.
     */
    | `[EqualizerRack1_[Channel${number}]]`

    /**
     * The [QuickEffectRack1_[ChannelI]_Effect1] group contains controls for the single quick effect slot on a deck.
     */
    | `[QuickEffectRack1_[Channel${number}]_Effect1]`

    /**
     * The [EqualizerRack1_[ChannelI]_Effect1] group contains controls for the EQ effect slot on a deck.
     */
    | `[EqualizerRack1_[Channel${number}]_Effect1]`

    /**
     * The [Skin] group contains controls that are used to selective show and hide parts of the graphical user interface of Mixxx to suit your needs.
     */
    | "[Skin]";

  /*
   * Public
   */

  export type Group = Utils.IsStrict extends true ? GroupName : GroupName | (string & {});

  // All controls
  type Ctrl<TGroup> = Utils.MapGroup<TGroup, Controls> | Utils.MapGroup<TGroup, ReadOnly.ReadOnlyControls>;

  // Only read & write controls (subset of Ctrl)
  type CtrlRW<TGroup> = Utils.MapGroup<TGroup, Controls>;

  /*
   * Group <-> control linking
   */

  // Read/Write controls
  type Controls = {
    "[App]": AppControl;
    "[AutoDJ]": AutoDJControl;
    "[Controls]": ControlsControl;
    "[Library]": LibraryControl;
    "[Main]": MainControl;
    "[Master]": MasterControl;
    "[Playlist]": PlaylistControl;
    "[Recording]": RecordingControl;
    "[Sampler]": SamplerControl;
    "[Shoutcast]": ShoutcastControl;
    "[Skin]": SkinControl;
    "[VinylControl]": VinylControlControl;
  } & {
    [key: `[Auxiliary${number}]`]: AuxiliaryNControl;
    [key: `[Channel${number}]`]: ChannelNControl;
    [key: `[EffectRack1_EffectUnit${number}]`]: EffectRack1EffectUnitNControl;
    [key: `[EffectRack1_EffectUnit${number}_Effect${number}]`]: EffectRack1EffectUnitNEffectMControl;
    [key: `[EqualizerRack1_[Channel${number}]]`]: EqualizerRack1ChannelIControl;
    [key: `[EqualizerRack1_[Channel${number}]_Effect1]`]: EqualizerRack1ChannelIEffect1Control;
    [key: `[Microphone${number}]`]: MicrophoneNControl;
    [key: `[PreviewDeck${number}]`]: PreviewDeckNControl;
    [key: `[QuickEffectRack1_[Channel${number}]]`]: QuickEffectRack1ChannelIControl;
    [key: `[QuickEffectRack1_[Channel${number}]_Effect1]`]: QuickEffectRack1ChannelIEffect1Control;
    [key: `[Sampler${number}]`]: SamplerNControl;
  };

  /*
   * Values
   */
  type PotMeterSuffix =
    | ""

    /**
     * Increases the value, e.g. [ChannelN],rate_perm_up sets the speed one step higher (4 % default)
     */
    | "_up"

    /**
     * Decreases the value, sets the speed one step lower (4 % default)
     */
    | "_down"

    /**
     * Increases the value by smaller step, sets the speed one small step higher (1 % default)
     */
    | "_up_small"

    /**
     * Decreases the value by smaller step, sets the speed one small step lower (1 % default)
     */
    | "_down_small"

    /**
     * Sets the value to 1.0, sets the channel volume to full
     */
    | "_set_one"

    /**
     * Sets the value to -1.0, sets the channel volume to zero
     */
    | "_set_minus_one"

    /**
     * Input: sets the control to its default, return to default waveform zoom level
     */
    | "_set_default"

    /**
     * Output: set to 1.0 if the control is at its default, light up the pitch fader center indicator
     */
    | "_set_default"

    /**
     * Sets the value to 0.0, put the crossfader in the middle again
     */
    | "_set_zero"

    /**
     * Sets the value to 0.0 if the value was > 0.0, and to 1.0 if the value was 0.0, will cut off/on a track while you’re playing
     */
    | "_toggle"

    /**
     * Sets the value to -1.0 if the value was > -1.0, and to 1.0 if the value was -1.0, can tilt the crossfader from left to right
     */
    | "_minus_toggle";

  type AppControl =
    /**
     * The number of auxiliary inputs that can be configured.
     *
     * @groups [App]
     * @range integer
     * @feedback None
     * @since New in version 2.4.0: Replaces the deprecated [Master],num_auxiliaries control.
     */
    | "num_auxiliaries"

    /**
     * The number of decks currently enabled.
     *
     * @groups [App]
     * @range integer
     * @feedback None
     * @since New in version 2.4.0: Replaces the deprecated [Master],num_decks control.
     */
    | "num_decks"

    /**
     * The number of microphone inputs that can be configured.
     *
     * @groups [App]
     * @range integer
     * @feedback None
     * @since New in version 2.4.0: Replaces the deprecated [Master],num_microphones control.
     */
    | "num_microphones"

    /**
     * The number of preview decks currently enabled.
     *
     * @groups [App]
     * @range integer
     * @feedback None
     * @since New in version 2.4.0: Replaces the deprecated [Master],num_preview_decks control.
     */
    | "num_preview_decks"

    /**
     * The number of samplers currently enabled.
     *
     * @groups [App]
     * @range integer
     * @feedback None
     * @since New in version 2.4.0: Replaces the deprecated [Master],num_samplers control.
     */
    | "num_samplers"

    /**
     * The current output sample rate (default: 44100 Hz).
     *
     * @groups [App]
     * @range absolute value (in Hz)
     * @feedback None
     * @since New in version 2.4.0: Replaces the deprecated [Master],samplerate control.
     */
    | "samplerate";

  type AutoDJControl =
    /**
     * Adds a random track to the Auto DJ queue.
     *
     * @groups [AutoDJ]
     * @range binary
     * @feedback Track is added to AutoDJ queue.
     * @since New in version 2.4.0.
     */
    | "add_random_track"

    /**
     * Turns Auto DJ on or off.
     *
     * @groups [AutoDJ]
     * @range binary
     * @feedback AutoDJ button
     * @since New in version 1.11.0.
     */
    | "enabled"

    /**
     * Triggers the transition to the next track.
     *
     * @groups [AutoDJ]
     * @range binary
     * @feedback Crossfader slider moves to the other side.
     * @since New in version 1.11.0.
     */
    | "fade_now"

    /**
     * Shuffles the content of the Auto DJ playlist.
     *
     * @groups [AutoDJ]
     * @range binary
     * @feedback Order of tracks in the AutoDJ playlist changes.
     * @since New in version 1.11.0.
     */
    | "shuffle_playlist"

    /**
     * Skips the next track in the Auto DJ playlist.
     *
     * @groups [AutoDJ]
     * @range binary
     * @feedback Skipped track is removed from the AutoDJ playlist.
     * @since New in version 1.11.0.
     */
    | "skip_next";

  type AuxiliaryNControl =
    /**
     * Set channel orientation for the crossfader.
     *
     * @groups [AuxiliaryN]
     * @range
     * |Value|Meaning|
     * |---|---|
     * |0  |Left side of crossfader|
     * |1  |Center (not affected by crossfader)|
     * |2  |Right side of crossfader|
     * @feedback None
     * @since New in version 1.10.0.
     */
    "orientation" | AuxiliaryNChannelNPreviewDeckNSamplerNControl | AuxiliaryNMicrophoneNControl;

  type AuxiliaryNChannelNPreviewDeckNSamplerNControl =
    /**
     * Assign channel to the center of the crossfader.
     *
     * @groups [AuxiliaryN], [ChannelN], [PreviewDeckN], [SamplerN]
     */
    | "orientation_center"

    /**
     * Assign channel to the left side of the crossfader.
     *
     * @groups [AuxiliaryN], [ChannelN], [PreviewDeckN], [SamplerN]
     */
    | "orientation_left"

    /**
     * Assign channel to the right side of the crossfader.
     *
     * @groups [AuxiliaryN], [ChannelN], [PreviewDeckN], [SamplerN]
     */
    | "orientation_right";

  type AuxiliaryNMicrophoneNControl =
    /**
     * Hold value at 1 to mix channel input into the main output.
     * For [MicrophoneN] use [MicrophoneN],talkover instead.
     * Note that [AuxiliaryN] also take [AuxiliaryN],orientation into account.
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range binary
     * @feedback Auxiliary: Play buttonMicrophone: N/A
     */
    | "main_mix"

    /**
     * Mutes the channel
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range binary
     * @feedback Mute button
     * @since New in version 2.0.0.
     */
    | "mute"

    /**
     * Indicates when the signal is clipping (too loud for the hardware and is being distorted)
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range binary
     * @feedback Microphone Clip light
     * @since New in version 2.4.0: Replaces the deprecated [MicrophoneN],PeakIndicator and [AuxiliaryN],PeakIndicator controls.
     */
    | "peak_indicator"

    /**
     * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the left channel
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range binary
     * @feedback Clip light (left)
     * @since New in version 2.4.0: Replaces the deprecated [MicrophoneN],PeakIndicatorL and [AuxiliaryN],PeakIndicatorL controls.
     */
    | "peak_indicator_l"

    /**
     * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the right channel
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range binary
     * @feedback Clip light (right)
     * @since New in version 2.4.0: Replaces the deprecated [MicrophoneN],PeakIndicatorR and [AuxiliaryN],PeakIndicatorR controls.
     */
    | "peak_indicator_r"

    /**
     * Toggles headphone cueing (PFL).
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range binary
     * @feedback Headphone button
     */
    | "pfl"

    /**
     * Adjusts the gain of the input
     * This is a ControlPotMeter control.
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range 0.0..1.0..4.0
     * @feedback Microphone gain knob
     * @kind pot meter control
     */
    | `pregain${PotMeterSuffix}`

    /**
     * Hold value at 1 to mix channel input into the main output.
     * For [AuxiliaryN] use [AuxiliaryN],main_mix instead.
     * Note that [AuxiliaryN] also take [AuxiliaryN],orientation into account.
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range binary
     * @feedback Microphone: Talk buttonAuxiliary: N/A
     * @since New in version 1.10.0.
     */
    | "talkover"

    /**
     * Adjusts the channel volume fader
     * This is a ControlPotMeter control.
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range default
     * @feedback Microphone volume fader changes
     * @since New in version 1.10.0.
     * @kind pot meter control
     */
    | `volume${PotMeterSuffix}`

    /**
     * Outputs the current instantaneous channel volume
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range default
     * @feedback Microphone VU meter changes
     * @since New in version 1.10..
     */
    | "vu_meter"

    /**
     * Outputs the current instantaneous deck volume for the left channel
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range default
     * @feedback Microphone/auxiliary VU meter L
     * @since New in version 2.4.0: Replaces the deprecated [MicrophoneN],VuMeterL and [AuxiliaryN],VuMeterL controls.
     */
    | "vu_meter_l"

    /**
     * Outputs the current instantaneous deck volume for the right channel
     *
     * @groups [AuxiliaryN], [MicrophoneN]
     * @range default
     * @feedback Microphone/auxiliary VU meter R
     * @since New in version 2.4.0: Replaces the deprecated [MicrophoneN],VuMeterR and [AuxiliaryN],VuMeterR controls.
     */
    | "vu_meter_r";

  type ChannelNControl =
    /**
     * Toggle the track context menu for the track currently loaded in this deck.
     * The control value is 1 if there is already a menu shown for this deck.
     * The menu can be navigated with the MoveUp/Down controls
     * and selected actions or submenus can be activated with GoToItem.
     *
     * @groups [ChannelN]
     * @range Binary
     * @feedback The deck’s track context menu is shown or hidden.
     * @since New in version 2.5.0.
     */
    "show_track_menu" | AuxiliaryNChannelNPreviewDeckNSamplerNControl | ChannelNPreviewDeckNSamplerNControl;

  type ChannelNPreviewDeckNSamplerNControl =
    /**
     * Clone the given deck number, copying the play state, position, rate, and key. If 0 or a negative number is given, Mixxx will attempt to select the first playing deck as the source for the clone.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range integer between 1 and [Master],num_decks (inclusive)
     * @feedback The channel will start playing at the rate and position of the source deck.
     * @since New in version 2.3.0.
     */
    | "CloneFromDeck"

    /**
     * Clone the given sampler number, copying the play state, position, rate, and key.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range integer between 1 and [App],num_samplers (inclusive)
     * @feedback The channel will start playing at the rate and position of the source deck.
     * @since New in version 2.3.0.
     */
    | "CloneFromSampler"

    /**
     * Loads the currently highlighted track into the deck
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Track name & waveform change
     */
    | "LoadSelectedTrack"

    /**
     * Loads the currently highlighted track into the deck and starts playing.
     * If the player is a preview deck and the selected track is already loaded, toggle play/pause.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Track name & waveform change & Play/pause button
     * @since New in version 1.11.0.
     */
    | "LoadSelectedTrackAndPlay"

    /**
     * Load the track currently loaded to the given deck number.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range integer between 1 and [App],num_decks (inclusive)
     * @feedback Track name & waveform change
     * @since New in version 2.4.0.
     */
    | "LoadTrackFromDeck"

    /**
     * Load the track currently loaded to the given sampler number.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range integer between 1 and [App],num_samplers (inclusive)
     * @feedback Track name & waveform change
     * @since New in version 2.4.0.
     */
    | "LoadTrackFromSampler"

    /**
     * Fast rewind (REW)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback << button
     */
    | "back"

    /**
     * Its value is set to the sample position of the closest beat of the active beat and is used for updating the beat LEDs.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range -1, 0.0, real-valued
     * @feedback None
     */
    | "beat_closest"

    /**
     * Outputs the relative position of the play marker in the section between the the previous and next beat marker.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range 0.0 - 1.0, real-valued
     * @feedback None
     */
    | "beat_distance"

    /**
     * Jump forward (positive) or backward (negative) by N beats. If a loop is active, the loop is moved by X beats.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range any real number within the range, see [ChannelN],beatloop_X_activate
     * @feedback Player jumps forward or backward by X beats.
     * @since New in version 2.0.0.
     */
    | "beatjump"

    /**
     * Jump backward by X beats. A control exists for
     * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512.
     * If a loop is active, the loop is moved backward by X beats.
     * If the loaded track has no beat grid, this value is treated as seconds instead of beats.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player jumps backward by X beats.
     * @since New in version 2.0.0.
     */
    | `beatjump_${number}_backward`

    /**
     * Jump forward by X beats. A control exists for
     * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512.
     * If a loop is active, the loop is moved forward by X beats.
     * If the loaded track has no beat grid, this value is treated as seconds instead of beats.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player jumps forward by X beats.
     * @since New in version 2.0.0.
     */
    | `beatjump_${number}_forward`

    /**
     * Jump backward by beatjump_size.
     * If a loop is active, the loop is moved backward by X beats.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player jumps backward by beatjump_size.
     * @since New in version 2.1.0.
     */
    | "beatjump_backward"

    /**
     * Jump forward by beatjump_size.
     * If a loop is active, the loop is moved forward by X beats.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player jumps forward by beatjump_size.
     * @since New in version 2.1.0.
     */
    | "beatjump_forward"

    /**
     * Set the number of beats to jump with beatjump_forward
     * /beatjump_backward.
     * If the loaded track has no beat grid, this value is treated as seconds instead of beats.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive real number
     * @feedback Beatjump size spinbox
     * @since New in version 2.1.0.
     */
    | "beatjump_size"

    /**
     * Double the value of beatjump_size.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Beatjump size spinbox
     * @since New in version 2.4.0.
     */
    | "beatjump_size_double"

    /**
     * Halve the value of beatjump_size.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Beatjump size spinbox
     * @since New in version 2.4.0.
     */
    | "beatjump_size_halve"

    /**
     * Activates a loop over X beats. A control exists for
     * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * Depending on the state of loop_anchor the loop is created forwards
     * or backwards from the current position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback A loop is shown over X beats.
     * @since New in version 1.10.0.
     */
    | `beatloop_${number}_activate`

    /**
     * 1 if beatloop X is enabled, 0 if not.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Beatloop X button in skin is lit.
     * @since New in version 1.10.0.
     */
    | `beatloop_${number}_enabled`

    /**
     * Toggles a loop over X beats. A control exists for
     * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * Depending on the state of loop_anchor the loop is created forwards
     * or backwards from the current position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback A loop is shown over X beats.
     * @since New in version 1.10.0.
     */
    | `beatloop_${number}_toggle`

    /**
     * Set a loop that is beatloop_size beats long and enables the loop.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * Depending on the state of loop_anchor the loop is created forwards
     * or backwards from the current position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback A loop is shown over beatloop_size beats
     * @since New in version 2.1.0.
     */
    | "beatloop_activate"

    /**
     * Activates a loop over X beats backwards from the current position. A control exists for
     * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512
     * If the loaded track has no beat grid, seconds are used instead of beats.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Beatloop X button in skin is lit. A loop overlay is shown over X beats on waveform.The slip mode toggle is activated.
     * @since New in version 2.5.0.
     */
    | `beatloop_r${number}_activate`

    /**
     * Set the length of the loop in beats that will get set with
     * beatloop_activate and
     * beatlooproll_activate.
     * Changing this will resize an existing loop if the length of the loop matches
     * beatloop_size.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive real number
     * @feedback Beatloop size spinbox and possibly loop section on waveform
     * @since New in version 2.1.0.
     */
    | "beatloop_size"

    /**
     * Activates a rolling loop over X beats. Once disabled, playback will resume where the
     * track would have been if it had not entered the loop. A control exists for
     * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * Depending on the state of loop_anchor, the loop is created forwards
     * or backwards from the current position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Beatloop X button in skin is lit. A loop overlay is shown over X beats on waveform.The slip mode toggle is activated.
     * @since New in version 1.11.0.
     */
    | `beatlooproll_${number}_activate`

    /**
     * Activates a rolling loop over beatloop_size beats.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * Once disabled, playback will resume where the track would have been if it had not entered the loop.
     * Depending on the state of loop_anchor, the loop is created forwards
     * or backwards from the current position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback A loop overlay is shown over beatloop_size beats on waveform.The slip mode toggle is activated.
     * @since New in version 2.1.0.
     */
    | "beatlooproll_activate"

    /**
     * Activates a rolling loop over X beats backwards from the current position.
     * Once disabled, playback will resume where the track would have been if it had
     * not entered the loop. A control exists for
     * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512
     * If the loaded track has no beat grid, seconds are used instead of beats.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Beatloop X button in skin is lit. A loop overlay is shown over X beats on waveform.The slip mode toggle is activated.
     * @since New in version 2.5.0.
     */
    | `beatlooproll_r${number}_activate`

    /**
     * Adjust the average BPM up by +0.01
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback The beatgrid lines move closer to each other.
     * @since New in version 2.0.0.
     */
    | "beats_adjust_faster"

    /**
     * Adjust the average BPM down by -0.01.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback The beatgrid lines move further apart from each other.
     * @since New in version 2.0.0.
     */
    | "beats_adjust_slower"

    /**
     * Adjust beatgrid so closest beat is aligned with the current playposition.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback The beatgrid moves to align with current playposition.
     * @since New in version 1.10.0.
     */
    | "beats_translate_curpos"

    /**
     * Move beatgrid to an earlier position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback The beatgrid moves left by a small amount.
     * @since New in version 2.0.0.
     */
    | "beats_translate_earlier"

    /**
     * Move beatgrid to a later position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback The beatgrid moves right by a small amount.
     * @since New in version 2.0.0.
     */
    | "beats_translate_later"

    /**
     * Adjust beatgrid to match another playing deck.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Instead of syncing the beatgrid to the current playposition, sync the beatgrid so the nearest beat lines up with the other track’s nearest beat.
     * @since New in version 2.0.0.
     */
    | "beats_translate_match_alignment"

    /**
     * Restores the beatgrid state before the last beatgrid adjustment done with the above beats_ controls.
     * The undo stack holds up to ten beatgrid states. For changes done in quick succession
     * (less than 800 milliseconds between actions), e.g. repeated beats_translate_earlier, only the first state is stored.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback The beatgrid is restored.
     * @since New in version 2.5.0.
     */
    | "beats_undo_adjustment"

    /**
     * Syncs the tempo and phase (depending on quantize) to that of the other track (if BPM is detected on both).
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     */
    | "beatsync"

    /**
     * Syncs the phase to that of the other track (if BPM is detected on both).
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback The Sync button flashes and the tempo slider snap to the appropriate value.
     * @since New in version 1.10.0.
     */
    | "beatsync_phase"

    /**
     * Syncs the tempo to that of the other track (if BPM is detected on both).
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback The Sync button flashes and the tempo slider snaps to the appropriate value.
     * @since New in version 1.10.0.
     */
    | "beatsync_tempo"

    /**
     * Reflects the perceived (rate-adjusted) BPM of the loaded file.
     * This is a ControlPotMeter control.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range real-valued
     * @feedback BPM value display
     * @kind pot meter control
     */
    | `bpm${PotMeterSuffix}`

    /**
     * When tapped repeatedly, adjusts the BPM of the track on the deck (not the tempo slider!) to match the taps.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback BPM value display (playback speed doesn’t change)
     * @since New in version 1.9.2.
     */
    | "bpm_tap"

    /**
     * Toggle the beatgrid/BPM lock state.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback The lock icon of the track is activated/deactivated.
     * @since New in version 2.5.0.
     */
    | "bpmlock"

    /**
     * Represents a Cue button that is always in CDJ mode.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback None
     * @since New in version 1.10.0.
     */
    | "cue_cdj"

    /**
     * Deletes the already set cue point and sets [ChannelN],cue_point to -1.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback None
     */
    | "cue_clear"

    /**
     * In CDJ mode, when playing, returns to the cue point and pauses. If stopped, sets a cue point at the current location. If stopped and at a cue point, plays from that point until released (set to 0.)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Cue button
     */
    | "cue_default"

    /**
     * If the cue point is set, recalls the cue point.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position
     */
    | "cue_goto"

    /**
     * If the cue point is set, seeks the player to it and starts playback.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position and start playing.
     * @since New in version 1.11.0.
     */
    | "cue_gotoandplay"

    /**
     * If the cue point is set, seeks the player to it and stops.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position.
     * @since New in version 1.11.0.
     */
    | "cue_gotoandstop"

    /**
     * Indicates the blinking pattern of the CUE button (i.e. 1.0 if the button is illuminated, 0.0 otherwise), depending on the chosen cue mode.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Cue button
     * @since New in version 2.0.0.
     */
    | "cue_indicator"

    /**
     * Represents the currently chosen cue mode.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range
     * |Value|compatible hardware|
     * |---|---|
     * |0.0|Mixxx mode (default)|
     * |1.0|Pioneer mode|
     * |2.0|Denon mode|
     * |3.0|Numark mode|
     * |4.0|Mixxx mode (no blinking)|
     * |5.0|CUP (Cue + Play) mode|
     * @feedback None
     */
    | "cue_mode"

    /**
     * Go to cue point and play after release (CUP button behavior). If stopped, sets a cue point at the current location.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback None
     * @since New in version 2.1.0.
     */
    | "cue_play"

    /**
     * The current position of the cue point in samples.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range absolute value
     * @feedback Cue point marker
     */
    | "cue_point"

    /**
     * Plays from the current cue point.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Cue button lights and waveform moves
     */
    | "cue_preview"

    /**
     * Sets a cue point at the current location.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Cue mark appears on the waveform
     */
    | "cue_set"

    /**
     * If the player is not playing, set the cue point at the current location otherwise seek to the cue point.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Cue button
     */
    | "cue_simple"

    /**
     * Outputs the length of the current song in seconds
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range absolute value
     * @feedback None
     */
    | "duration"

    /**
     * Eject currently loaded track. If no track is loaded the last-ejected track
     * (of any deck) is reloaded.
     * Double-press to reload the last replaced track. If no track is loaded the second-last ejected track is reloaded.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Eject button is lit. Be sure to set back to 0 with scripts so the button does not stay lit.
     * @since New in version 1.9.0.
     */
    | "eject"

    /**
     * Jump to end of track
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Track jumps to end
     */
    | "end"

    /**
     * Fast forward (FF)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback > button
     */
    | "fwd"

    /**
     * If hotcue X is not set, this sets a hotcue at the current play position and saves it as hotcue X of type “Hotcue”.
     * In case a loop is currently enabled (i.e. if [ChannelN],loop_enabled is set to 1), the loop will be saved as hotcue X instead and hotcue_X_type will be set to “Loop”.
     * If hotcue X has been set as a regular cue point, the player seeks to the saved play position.
     * If hotcue_X_type is “Loop”, looping will be enabled and the loop controls (e.g. loop_start_position, loop_end_position and beatloop_size) will be set accordingly.
     * Just like reloop_toggle, the player seeks back to the loop start when the current play position is behind the loop, and enabled without a seek when it is in front of or inside the loop.
     * This allows a loop catching behavior on one hand and a jump back when the loop has been exit by just triggering this control.
     * Setting the control to 1 when the track is currently not playing (i.e. play is set to 0) will start hotcue previewing.
     * After resetting the control to 0, playback will usually be stopped and the player will seek to the hotcue position.
     * If play is set to 1 while previewing is active, the playback will continue and no seek occurs.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position. Hotcue X marker may change on waveform.
     * @since New in version 1.8.0.
     */
    | `hotcue_${number}_activate`

    /**
     * Identical to hotcue_X_activate, but this always sets a regular cue point, regardless of whether a loop is enabled or not.
     * This control can be used for controllers that have dedicated hotcue/saved loop pad modes.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @since New in version 2.4.0.
     */
    | `hotcue_${number}_activatecue`

    /**
     * Identical to hotcue_X_activate, but this always sets a saved loop, regardless of whether a loop is enabled or not.
     * If no loop is available, this sets and enables a beat loop of of beatloop_size.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * This control can be used for controllers that have dedicated hotcue/saved loop pad modes.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @since New in version 2.4.0.
     */
    | `hotcue_${number}_activateloop`

    /**
     * If hotcue X is set, clears its hotcue status.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Hotcue X marker changes on waveform.
     * @since New in version 1.8.0.
     */
    | `hotcue_${number}_clear`

    /**
     * Color of hotcue X or -1 if the hotcue is not set.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range 3-Byte RGB color code (or -1)
     * @feedback Color of Hotcue X button and waveform marker changes.
     * @since New in version 2.3.0.
     */
    | `hotcue_${number}_color`

    /**
     * Enables or disables a loop from the position of hotcue X.
     * If X is a saved loop, that loop will be used, otherwise it will set a beatloop of beatloop_size from the cue position.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * In case the hotcue is not set, this control will set a regular cue point at the current position and start a beatloop.
     * This control can be used to map the primary action of the “Cue Loop” performance pad mode on Serato-style controllers.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @since New in version 2.4.0.
     */
    | `hotcue_${number}_cueloop`

    /**
     * If hotcue X is set, seeks the player to hotcue X’s position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position.
     * @since New in version 1.8.0.
     */
    | `hotcue_${number}_goto`

    /**
     * If hotcue X is set, seeks the player to hotcue X’s position, starts playback and looping.
     * If the hotcue is a saved loop, the loop is enabled, otherwise a beatloop of beatloop_size is set from the hotcue’s position.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * This control can be used to map the secondary action of the “Cue Loop” performance pad mode on Serato-style controllers.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position and looping is enabled.
     * @since New in version 2.4.0.
     */
    | `hotcue_${number}_gotoandloop`

    /**
     * If hotcue X is set, seeks the player to hotcue X’s position and starts playback.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position.
     * @since New in version 1.11.0.
     */
    | `hotcue_${number}_gotoandplay`

    /**
     * If hotcue X is set, seeks the player to hotcue X’s position and stops.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position.
     * @since New in version 1.8.0.
     */
    | `hotcue_${number}_gotoandstop`

    /**
     * The position of hotcue X in samples, -1 if not set.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive integer
     * @feedback Hotcue X marker changes on waveform.
     * @since New in version 1.8.0.
     */
    | `hotcue_${number}_position`

    /**
     * Set a hotcue at the current play position and saves it as hotcue X of type “Hotcue”.
     * In case a loop is currently enabled (i.e. if [ChannelN],loop_enabled is set to 1), the loop will be saved as hotcue X instead and hotcue_X_type will be set to “Loop”.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Hotcue X marker changes on waveform.
     * @since New in version 1.8.0.
     */
    | `hotcue_${number}_set`

    /**
     * Identical to hotcue_X_set, but this always sets a regular cue point (i.e. hotcue_X_type “Hotcue”), regardless of whether a loop is enabled or not.
     * This control can be used for controllers that have dedicated hotcue/saved loop pad modes.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @since New in version 2.4.0.
     */
    | `hotcue_${number}_setcue`

    /**
     * Identical to hotcue_X_set, but this always saves a loop (i.e. hotcue_X_type “Loop”), regardless of whether a loop is enabled or not.
     * If no loop is available, this sets and enables a beat loop of of beatloop_size.
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * This control can be used for controllers that have dedicated hotcue/saved loop pad modes.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @since New in version 2.4.0.
     */
    | `hotcue_${number}_setloop`

    /**
     * Indicates if hotcue slot X is set, active or empty.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @since New in version 2.4.0: Replaces the deprecated [ChannelN],hotcue_X_enabled.
     */
    | `hotcue_${number}_status`

    /**
     * Indicates the type of the hotcue in hotcue slot X.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @since New in version 2.4.0.
     */
    | `hotcue_${number}_type`

    /**
     * Contains the number of the most recently used hotcue (or -1 if no hotcue was used).
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive integer (or -1)
     * @feedback None
     * @since New in version 2.3.0.
     */
    | "hotcue_focus"

    /**
     * If there is a focused hotcue, sets its color to the next color in the palette.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Color of focused hotcue button and waveform marker changes.
     * @since New in version 2.3.0.
     */
    | "hotcue_focus_color_next"

    /**
     * If there is a focused hotcue, sets its color to the previous color in the palette.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Color of focused hotcue button and waveform marker changes.
     * @since New in version 2.3.0.
     */
    | "hotcue_focus_color_prev"

    /**
     * If the intro end cue is set, seeks the player to the intro end position. If the intro end is not set, sets the intro end to the current play position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position. Intro end marker may change on waveform.
     * @since New in version 2.3.0.
     */
    | "intro_end_activate"

    /**
     * If the intro end cue is set, clears its status.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Intro end marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "intro_end_clear"

    /**
     * The position of the intro end in samples, -1 if not set.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive integer
     * @feedback Intro end marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "intro_end_position"

    /**
     * Set intro end to the current play position. If intro end was previously set, it is moved to the new position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Intro end marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "intro_end_set"

    /**
     * If the intro start cue is set, seeks the player to the intro start position. If the intro start is not set, sets the intro start to the current play position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position. Intro start marker may change on waveform.
     * @since New in version 2.3.0.
     */
    | "intro_start_activate"

    /**
     * If the intro start cue is set, clears its status.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Intro start marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "intro_start_clear"

    /**
     * The position of the intro start in samples, -1 if not set.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive integer
     * @feedback Intro start marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "intro_start_position"

    /**
     * Set intro start to the current play position. If intro start was previously set, it is moved to the new position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Intro start marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "intro_start_set"

    /**
     * Current key of the track
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range
     * |Value|OpenKey|Lancelot|Traditional|
     * |---|---|---|---|
     * |1  |1d |8b |C  |
     * |2  |8d |3b |D♭ |
     * |3  |3d |10b|D  |
     * |4  |10d|5b |E♭ |
     * |5  |5d |12b|E  |
     * |6  |12d|7b |F  |
     * |7  |7d |2b |F♯/G♭|
     * |8  |2d |9b |G  |
     * |9  |9d |4b |A♭ |
     * |10 |4d |11b|A  |
     * |11 |11d|6b |B♭ |
     * |12 |6d |1b |B  |
     * |13 |10m|5a |Cm |
     * |14 |5m |12a|C♯m|
     * |15 |12m|7a |Dm |
     * |16 |7m |2a |D♯m/E♭m|
     * |17 |2m |9a |Em |
     * |18 |9m |4a |Fm |
     * |19 |4m |11a|F♯m|
     * |20 |11m|6a |Gm |
     * |21 |6m |1a |G♯m|
     * |22 |1m |8a |Am |
     * |23 |8m |3a |B♭m|
     * |24 |3m |10a|Bm |
     * @since New in version 2.0.0.
     */
    | "key"

    /**
     * Enable key-lock for the specified deck (rate changes only affect tempo, not key)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback key-lock button activates
     * @since New in version 1.9.0.
     */
    | "keylock"

    /**
     * Reflects the average bpm around the current play position of the loaded file.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive value
     * @feedback None
     */
    | "local_bpm"

    /**
     * Adjusts whether loops created with [ChannelN],beatloop_X_activate,
     * [ChannelN],beatloop_X_toggle or [ChannelN],beatloop_rX_activate
     * span forward or backward from the current play position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range
     * |Value|Direction|
     * |---|---|
     * |0  |forward|
     * |1  |backward|
     * @feedback Loop anchor button
     * @since New in version 2.5.0.
     */
    | "loop_anchor"

    /**
     * Doubles beatloop_size. If beatloop_size equals the size of the loop, the loop is resized.
     * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Beatloop size spinbox changes
     * @since New in version 1.10.0.
     */
    | "loop_double"

    /**
     * Indicates whether or not a loop is enabled.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Loop in waveform is active.
     * @since New in version 1.8.0.
     */
    | "loop_enabled"

    /**
     * The player loop-out position in samples, -1 if not set.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive integer
     * @feedback Loop-out marker shows on waveform.
     * @since New in version 1.8.0.
     */
    | "loop_end_position"

    /**
     * Halves beatloop_size. If beatloop_size equals the size of the loop, the loop is resized.
     * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Beatloop size spinbox changes
     * @since New in version 1.10.0.
     */
    | "loop_halve"

    /**
     * If loop is disabled, sets the player loop in position to the current play position. If loop is enabled, press and hold to move loop in position to the current play position. If quantize is enabled, beatloop_size will be updated to reflect the new loop size.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Loop-in marker changes on waveform.
     * @since New in version 1.8.0.
     */
    | "loop_in"

    /**
     * Seek to the loop in point.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Waveform position jumps
     * @since New in version 2.1.0.
     */
    | "loop_in_goto"

    /**
     * Move loop forward by X beats (positive) or backward by X beats (negative).
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range real number
     * @feedback Loop moves forward or backward by X beats.
     * @since New in version 2.0.0.
     */
    | "loop_move"

    /**
     * Loop moves by X beats. A control exists for
     * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Loop moves backward by X beats.
     * @since New in version 2.0.0.
     */
    | `loop_move_${number}_backward`

    /**
     * Moves the loop in and out points forward by X beats. A control exists for
     * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512
     * If the loaded track has no beat grid, seconds are used instead of beats.
     * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Loop moves forward by X beats.
     * @since New in version 2.0.0.
     */
    | `loop_move_${number}_forward`

    /**
     * If loop is disabled, sets the player loop out position to the current play position. If loop is enabled, press and hold to move loop out position to the current play position. If quantize is enabled, beatloop_size will be updated to reflect the new loop size.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Loop-out marker changes on waveform.
     * @since New in version 1.8.0.
     */
    | "loop_out"

    /**
     * Seek to the loop out point.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Waveform position jumps
     * @since New in version 2.1.0.
     */
    | "loop_out_goto"

    /**
     * Clears the last active loop, i.e. deactivates and removes loop, detaches loop_in,
     * loop_out, reloop_toggle and related
     * controls. It does not affect saved loops.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Last active loop is disabled and removed from waveform and overview.
     * @since New in version 2.4.0.
     */
    | "loop_remove"

    /**
     * Scale the loop length by the value scale is set to by moving the end marker.
     * beatloop_size is not updated to reflect the change.
     * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range 0.0 - infinity
     * @feedback Loop length is scaled by given amount on waveform.
     * @since New in version 1.10.0.
     */
    | "loop_scale"

    /**
     * The player loop-in position in samples, -1 if not set.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive integer
     * @feedback Loop-in marker changes on waveform.
     * @since New in version 1.8.0.
     */
    | "loop_start_position"

    /**
     * Mutes the channel
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Mute button
     * @since New in version 2.0.0.
     */
    | "mute"

    /**
     * Set channel orientation for the crossfader.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range
     * |Value|Meaning|
     * |---|---|
     * |0  |Left side of crossfader|
     * |1  |Center (not affected by crossfader)|
     * |2  |Right side of crossfader|
     * @feedback None
     * @since New in version 1.9.0.
     */
    | "orientation"

    /**
     * If the outro end cue is set, seeks the player to the outro end position. If the outro end is not set, sets the outro end to the current play position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position. Outro end marker may change on waveform.
     * @since New in version 2.3.0.
     */
    | "outro_end_activate"

    /**
     * If the outro end cue is set, clears its status.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Outro end marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "outro_end_clear"

    /**
     * The position of the outro end in samples, -1 if not set.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive integer
     * @feedback Outro end marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "outro_end_position"

    /**
     * Set outro end to the current play position. If outro end was previously set, it is moved to the new position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Outro end marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "outro_end_set"

    /**
     * If the outro start cue is set, seeks the player to the outro start position. If the outro start is not set, sets the outro start to the current play position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Player may change position. Outro start marker may change on waveform.
     * @since New in version 2.3.0.
     */
    | "outro_start_activate"

    /**
     * If the outro start cue is set, clears its status.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Outro start marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "outro_start_clear"

    /**
     * The position of the outro start in samples, -1 if not set.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range positive integer
     * @feedback Outro start marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "outro_start_position"

    /**
     * Set outro start to the current play position. If outro start was previously set, it is moved to the new position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Outro start marker changes on waveform.
     * @since New in version 2.3.0.
     */
    | "outro_start_set"

    /**
     * Connects the vinyl control input for vinyl control on that deck to the channel output. Allows to mix external media into DJ sets.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Passthrough label in the waveform overview and passthrough button
     * @since New in version 2.0.0.
     */
    | "passthrough"

    /**
     * Indicates when the signal is clipping (too loud for the hardware and is being distorted)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Clip light
     * @since New in version 2.4.0: Replaces the deprecated [ChannelN],PeakIndicator, [PreviewDeckN],PeakIndicator and [SamplerN],PeakIndicator controls.
     */
    | "peak_indicator"

    /**
     * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the left channel
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Clip light (left)
     * @since New in version 2.4.0: Replaces the deprecated [ChannelN],PeakIndicatorL, [PreviewDeckN],PeakIndicatorL and [SamplerN],PeakIndicatorL controls.
     */
    | "peak_indicator_l"

    /**
     * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the right channel
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Clip light (right)
     * @since New in version 2.4.0: Replaces the deprecated [ChannelN],PeakIndicatorR, [PreviewDeckN],PeakIndicatorR and [SamplerN],PeakIndicatorR controls.
     */
    | "peak_indicator_r"

    /**
     * Toggles headphone cueing (PFL).
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Headphone button
     */
    | "pfl"

    /**
     * The total adjustment to the track’s pitch, including changes from the rate slider if keylock is off as well as pitch_adjust.
     * This is a ControlPotMeter control.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range -6.0..6.0 semitones
     * @feedback Key display
     * @since New in version 2.0.0.
     * @kind pot meter control
     */
    | `pitch${PotMeterSuffix}`

    /**
     * Adjusts the pitch in addition to the tempo slider pitch and keylock. It is reset after loading a new track.
     * This is a ControlPotMeter control.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range -3.0..3.0 semitones
     * @feedback Key display
     * @since New in version 2.0.0.
     * @kind pot meter control
     */
    | `pitch_adjust${PotMeterSuffix}`

    /**
     * Changes the track pitch down one half step, independent of the tempo.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Key display
     * @since New in version 2.0.0.
     */
    | "pitch_down"

    /**
     * Changes the track pitch up one half step, independent of the tempo.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Key display
     * @since New in version 2.0.0.
     */
    | "pitch_up"

    /**
     * Toggles playing or pausing the track.
     * The value is set to 1 when the track is playing or when previewing from cue points and when the play command is adopted and track will be played after loading.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Play/pause button
     */
    | "play"

    /**
     * A play button without pause. Pushing while playing, starts play at cue point again (Stutter).
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Play/Stutter button
     * @since New in version 2.0.0.
     */
    | "play_stutter"

    /**
     * Sets the absolute position in the track.
     * This is a ControlPotMeter control.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range -0.14 to 1.14 (0 = beginning -> Midi 14, 1 = end -> Midi 114)
     * @feedback Waveform
     * @kind pot meter control
     */
    | `playposition${PotMeterSuffix}`

    /**
     * Adjusts the pre-fader gain of the track (to avoid clipping)
     * This is a ControlPotMeter control.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range 0.0..1.0..4.0
     * @feedback GAIN knob
     * @kind pot meter control
     */
    | `pregain${PotMeterSuffix}`

    /**
     * Aligns Hot-cues and Loop In & Out to the next beat from the current position.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Hot-cues or Loop In/Out markers
     * @since New in version 1.10.0.
     */
    | "quantize"

    /**
     * Speed control
     * This is a ControlPotMeter control.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range -1.0..1.0
     * @feedback Speed slider
     * @kind pot meter control
     */
    | `rate${PotMeterSuffix}`

    /**
     * Actual rate (used in visuals, not for control)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     */
    | "rateEngine"

    /**
     * Sets the range of the Speed slider (0.08 = 8%)
     * This is a ControlPotMeter control.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range 0.0..4.0
     * @feedback none, until you move the Speed slider
     * @kind pot meter control
     */
    | `rateRange${PotMeterSuffix}`

    /**
     * Seeks forward (positive values) or backward (negative values) at a speed determined by the value
     * This is a ControlPotMeter control.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range -300..300
     * @feedback Deck seeks
     * @kind pot meter control
     */
    | `rateSearch${PotMeterSuffix}`

    /**
     * Indicates orientation of speed slider.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range -1 or 1
     */
    | "rate_dir"

    /**
     * Sets the speed one step lower (4 % default) lower
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Perm down button & Speed slider
     */
    | "rate_perm_down"

    /**
     * Sets the speed one small step lower (1 % default)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Perm down button & Speed slider
     */
    | "rate_perm_down_small"

    /**
     * Sets the speed one step higher (4 % default)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Perm up button & Speed slider
     */
    | "rate_perm_up"

    /**
     * Sets the speed one small step higher (1 % default)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Perm up button & Speed slider
     */
    | "rate_perm_up_small"

    /**
     * Holds the speed one step lower while active
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Temp down button & Speed slider
     */
    | "rate_temp_down"

    /**
     * Holds the speed one small step lower while active
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Temp down button & Speed slider
     */
    | "rate_temp_down_small"

    /**
     * Holds the speed one step higher while active
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Temp up button & Speed slider
     */
    | "rate_temp_up"

    /**
     * Holds the speed one small step higher while active
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Temp up button & Speed slider
     */
    | "rate_temp_up_small"

    /**
     * Activate current loop, jump to its loop in point, and stop playback.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Loop range in waveform activates or deactivates and play position moves to loop in point.
     * @since New in version 2.1.0.
     */
    | "reloop_andstop"

    /**
     * Toggles the current loop on or off. If the loop is ahead of the current play position, the track will keep playing normally until it reaches the loop.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Loop range in waveform activates or deactivates.
     * @since New in version 2.1.0.
     */
    | "reloop_toggle"

    /**
     * Enable repeat-mode for the specified deck
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback when track finishes, song loops to beginning
     * @since New in version 1.9.0.
     */
    | "repeat"

    /**
     * Resets the key to the original track key.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @since New in version 2.0.0.
     */
    | "reset_key"

    /**
     * Toggles playing the track backwards
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback REV button
     */
    | "reverse"

    /**
     * Enables reverse and slip mode while held (Censor)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback REV button
     * @since New in version 2.0.0.
     */
    | "reverseroll"

    /**
     * Affects absolute play speed & direction whether currently playing or not when [ChannelN],scratch2_enable is active. (multiplicative). Use JavaScript engine.scratch functions to manipulate in controller mappings.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range -3.0..3.0
     * @feedback Waveform
     * @since New in version 1.8.0.
     */
    | "scratch2"

    /**
     * Takes over play speed & direction for [ChannelN],scratch2.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Waveform
     * @since New in version 1.8.0.
     */
    | "scratch2_enable"

    /**
     * (No description)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback All cue markers move left by 10ms.
     * @since New in version 2.3.0.
     */
    | "shift_cues_earlier"

    /**
     * (No description)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback All cue markers move left by 1ms.
     * @since New in version 2.3.0.
     */
    | "shift_cues_earlier_small"

    /**
     * (No description)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback All cue markers move right by 10ms.
     * @since New in version 2.3.0.
     */
    | "shift_cues_later"

    /**
     * (No description)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback All cue markers move right by 1ms.
     * @since New in version 2.3.0.
     */
    | "shift_cues_later_small"

    /**
     * Toggles slip mode. When active, the playback continues muted in the background during a loop, scratch etc. Once disabled, the audible playback will resume where the track would have been.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Slip mode button
     * @since New in version 1.11.0.
     */
    | "slip_enabled"

    /**
     * Decrease the rating of the currently loaded track (if the skin has star widgets in the decks section).
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Star count is decreased in the deck’s star widget and in the library table.
     * @since New in version 2.3.0.
     */
    | "stars_down"

    /**
     * Increase the rating of the currently loaded track (if the skin has star widgets in the decks section).
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Star count is increased in the deck’s star widget and in the library table.
     * @since New in version 2.3.0.
     */
    | "stars_up"

    /**
     * Jump to start of track
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Track jumps to start
     */
    | "start"

    /**
     * Start playback from the beginning of the deck.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Deck plays from beginning
     * @since New in version 1.10.0.
     */
    | "start_play"

    /**
     * Seeks a player to the start and then stops it.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Deck stops at the beginning.
     * @since New in version 1.10.0.
     */
    | "start_stop"

    /**
     * Stops a player.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Pause Button. Deck pauses at the current position.
     * @since New in version 1.10.0.
     */
    | "stop"

    /**
     * Syncs the tempo and phase (depending on quantize) to that of the other track (if BPM is detected on both). Click and hold for at least one second activates sync lock on that deck.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback If enabled, the Sync button stays lit and tempo slider snap to the appropriate value. Slider adjustments are linked on all decks that have sync lock enabled.
     * @since New in version 2.0.0.
     */
    | "sync_enabled"

    /**
     * Match musical key.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @feedback Key value widget
     * @since New in version 2.0.0.
     */
    | "sync_key"

    /**
     * Sets deck as leader clock.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback If enabled, the Sync button stays lit and tempo slider snap to the appropriate value. Slider adjustments are linked on all decks that have sync lock enabled.
     * @since New in version 2.4.0.
     */
    | "sync_leader"

    /**
     * Sets deck as leader clock.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback If enabled, the Sync button stays lit and tempo slider snap to the appropriate value. Slider adjustments are linked on all decks that have sync lock enabled.
     * @since New in version 2.0.0.
     */
    | "sync_master"

    /**
     * (No description)
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range
     * |Value|Meaning|
     * |---|---|
     * |0  |Sync lock disabled for that deck|
     * |1  |Deck is sync follower|
     * |2  |Deck is sync leader|
     * @since New in version 2.0.0.
     */
    | "sync_mode"

    /**
     * When tapped repeatedly, adjusts the rate/tempo of the deck to match the taps.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback Speed slider
     * @since New in version 2.5.0.
     */
    | "tempo_tap"

    /**
     * Color of the currently loaded track or -1 if no track is loaded or the track has no color.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range 3-Byte RGB color code (or -1)
     * @feedback Track color changes in the library view.
     * @since New in version 2.3.0.
     */
    | "track_color"

    /**
     * Applies the deck pregain knob value to the detected ReplayGain value for the
     * current track. This is a way to update the ReplayGain value of a track if it
     * has been detected incorrectly. When this control is triggered, the pregain
     * value for the deck will be centered so that there is no audible difference in
     * track volume, so this operation is safe to use during performance, if the controller mapping uses soft-takeover for the pregain knob.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback ReplayGain value is updated in library, deck pregain is reset to 1.0.
     */
    | "update_replaygain_from_pregain"

    /**
     * Determines how cue points are treated in vinyl control relative mode.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range
     * |Value|Meaning|
     * |---|---|
     * |0  |Cue points ignored|
     * |1  |One Cue - If needle is dropped after the cue point, track will seek to that cue point|
     * |2  |Hot Cue - Track will seek to nearest previous hotcue|
     * @since New in version 1.10.0.
     */
    | "vinylcontrol_cueing"

    /**
     * Toggles whether a deck is being controlled by digital vinyl.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range binary
     * @feedback When enabled, a vinyl indication should appear onscreen indicating green for enabled.
     * @since New in version 1.10.0.
     */
    | "vinylcontrol_enabled"

    /**
     * Determines how vinyl control interprets needle information.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range
     * |Value|Meaning|
     * |---|---|
     * |0  |Absolute Mode (track position equals needle position and speed)|
     * |1  |Relative Mode (track tempo equals needle speed regardless of needle position)|
     * |2  |Constant Mode (track tempo equals last known-steady tempo regardless of needle input|
     * See Control Mode for details.
     * @feedback 3-way button indicates status
     * @since New in version 1.10.0.
     */
    | "vinylcontrol_mode"

    /**
     * BPM to display in the GUI (updated more slowly than the actual BPM).
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range ?
     * @feedback BPM value widget
     * @since New in version 2.0.0.
     */
    | "visual_bpm"

    /**
     * Current musical key after pitch shifting to display in the GUI using the notation selected in the preferences
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range ?
     * @feedback Key value widget
     * @since New in version 2.0.0.
     */
    | "visual_key"

    /**
     * The distance to the nearest key measured in cents
     * This is a ControlPotMeter control.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range -0.5..0.5
     * @feedback Key value widget
     * @since New in version 2.0.0.
     * @kind pot meter control
     */
    | `visual_key_distance${PotMeterSuffix}`

    /**
     * Adjusts the channel volume fader
     * This is a ControlPotMeter control.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range default
     * @feedback Deck volume fader
     * @kind pot meter control
     */
    | `volume${PotMeterSuffix}`

    /**
     * Outputs the current instantaneous deck volume
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range default
     * @feedback Deck VU meter
     * @since New in version 2.4.0: Replaces the deprecated [ChannelN],VuMeter, [PreviewDeckN],VuMeter and [SamplerN],VuMeter controls.
     */
    | "vu_meter"

    /**
     * Outputs the current instantaneous deck volume for the left channel
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range default
     * @feedback Deck VU meter L
     * @since New in version 2.4.0: Replaces the deprecated [ChannelN],VuMeterL, [PreviewDeckN],VuMeterL and [SamplerN],VuMeterL controls.
     */
    | "vu_meter_l"

    /**
     * Outputs the current instantaneous deck volume for the right channel
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range default
     * @feedback Deck VU meter R
     * @since New in version 2.4.0: Replaces the deprecated [ChannelN],VuMeterR, [PreviewDeckN],VuMeterR and [SamplerN],VuMeterR controls.
     */
    | "vu_meter_r"

    /**
     * Zooms the waveform to look ahead or back as needed.
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range 1.0 - 10.0
     * @feedback Waveform zoom buttons
     * @since New in version 1.11.0.
     */
    | "waveform_zoom"

    /**
     * Waveform Zoom In
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range ?
     * @feedback Waveform zoom buttons
     * @since New in version 1.11.0.
     */
    | "waveform_zoom_down"

    /**
     * Return to default waveform zoom level
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range ?
     * @feedback Waveform zoom buttons
     * @since New in version 1.11.0.
     */
    | "waveform_zoom_set_default"

    /**
     * Waveform Zoom Out
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range ?
     * @feedback Waveform zoom buttons
     * @since New in version 1.11.0.
     */
    | "waveform_zoom_up"

    /**
     * Affects relative playback speed and direction persistently (additive offset & must manually be undone).
     *
     * @groups [ChannelN], [PreviewDeckN], [SamplerN]
     * @range -3.0..3.0
     * @feedback Waveform
     */
    | "wheel";

  type ControlsControl =
    /**
     * If enabled, colors will be assigned to newly created hotcues automatically.
     *
     * @groups [Controls]
     * @range binary
     * @feedback None
     * @since New in version 2.3.0.
     */
    | "AutoHotcueColors"

    /**
     * Represents the current state of the remaining time duration display of the loaded track.
     *
     * @groups [Controls]
     * @range
     * |Value|Meaning|
     * |---|---|
     * |0  |currently showing elapsed time, sets to remaining time|
     * |1  |currently showing remaining time , sets to elapsed time|
     * |2  |currently showing both (that means we are showing remaining, set to elapsed|
     * @feedback None
     */
    | "ShowDurationRemaining"

    /**
     * Once enabled, all touch tab events are interpreted as right click. This control has been added to provide touchscreen compatibility in 2.0 and might be replaced by a general modifier solution in the future.
     *
     * @groups [Controls]
     * @range binary
     * @feedback All Widgets
     * @since New in version 2.0.0.
     */
    | "touch_shift";

  type EffectRack1EffectUnitNControl =
    /**
     * Whether or not this EffectChain applies to the Headphone output
     *
     * @groups [EffectRack1_EffectUnitN]
     * @range binary
     */
    | "group_[Headphone]_enable"

    /**
     * Whether or not this EffectChain applies to the Main output
     *
     * @groups [EffectRack1_EffectUnitN]
     * @range binary
     */
    | "group_[Master]_enable"

    /**
     * Whether or not this EffectChain applies to Sampler J
     *
     * @groups [EffectRack1_EffectUnitN]
     * @range binary
     */
    | `group_[Sampler${number}]_enable`
    | EffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl;

  type EffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl =
    /**
     * Select EffectChain preset. > 0 goes one forward; < 0 goes one backward.
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range +1/-1
     */
    | "chain_preset_selector"

    /**
     * Clear the currently loaded EffectChain in this EffectUnit.
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range binary
     */
    | "clear"

    /**
     * If true, the EffectChain in this EffectUnit will be processed. Meant to allow the user a quick toggle for the effect unit.
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range binary, default true
     */
    | "enabled"

    /**
     * 0 indicates no effect is focused; > 0 indicates the index of the focused effect. Focusing an effect only does something if a controller mapping changes how it behaves when an effect is focused.
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range 0..num_effectslots
     */
    | "focused_effect"

    /**
     * Whether or not this EffectChain applies to Deck I
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range binary
     */
    | `group_[Channel${number}]_enable`

    /**
     * 0-based index of the currently loaded EffectChain preset. 0 is the empty/passthrough
     * preset, -1 indicates an unsaved preset (default state of [EffectRack1_EffectUnitN]).
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range integer, -1 .. [num_chain_presets - 1]
     */
    | "loaded_chain_preset"

    /**
     * The dry/wet mixing ratio for this EffectChain with the EngineChannels it is mixed with
     * This is a ControlPotMeter control.
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range 0.0..1.0
     * @kind pot meter control
     */
    | `mix${PotMeterSuffix}`

    /**
     * Cycle to the next EffectChain preset after the currently loaded preset.
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range binary
     */
    | "next_chain_preset"

    /**
     * Cycle to the previous EffectChain preset before the currently loaded preset.
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range binary
     */
    | "prev_chain_preset"

    /**
     * Whether to show focus buttons and draw a border around the focused effect in skins. This should not be manipulated by skins; it should only be changed by controller mappings.
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range binary
     */
    | "show_focus"

    /**
     * Whether to show all the parameters of each effect in skins or only show metaknobs.
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range binary
     */
    | "show_parameters"

    /**
     * The EffectChain superknob. Moves the metaknobs for each effect in the chain.
     * This is a ControlPotMeter control.
     *
     * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
     * @range 0.0..1.0
     * @kind pot meter control
     */
    | `super1${PotMeterSuffix}`;

  type EffectRack1EffectUnitNEffectMEqualizerRack1ChannelIEffect1QuickEffectRack1ChannelIEffect1Control =
    /**
     * The value of the Kth parameter. See the Parameter Values section for more information.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range double
     */
    | `button_parameter${number}`

    /**
     * Clear the currently loaded Effect in this Effect slot from the EffectUnit.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range binary
     */
    | "clear"

    /**
     * Select Effect – >0 goes one forward, <0 goes one backward.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range +1/-1
     */
    | "effect_selector"

    /**
     * If true, the effect in this slot will be processed. Meant to allow the user a quick toggle for this effect.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range binary, default true
     */
    | "enabled"

    /**
     * 0-based index of the currently loaded effect preset, including the
     * empty/passthrough preset “---”.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range integer, 0 .. [num_effectsavailable - 1]
     */
    | "loaded_effect"

    /**
     * Controls the parameters that are linked to the metaknob.
     * This is a ControlPotMeter control.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range 0..1
     * @kind pot meter control
     */
    | `meta${PotMeterSuffix}`

    /**
     * Cycle to the next effect after the currently loaded effect.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range binary
     */
    | "next_effect"

    /**
     * The scaled value of the Kth parameter.
     * See the Parameter Values section for more information.
     * This is a ControlPotMeter control.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range double
     * @kind pot meter control
     */
    | `parameter${number}${PotMeterSuffix}`

    /**
     * The link direction of the Kth parameter to the effect’s metaknob.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range bool
     */
    | `parameter${number}_link_inverse`

    /**
     * The link type of the Kth parameter to the effects’s metaknob.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range enum
     */
    | `parameter${number}_link_type`

    /**
     * Cycle to the previous effect before the currently loaded effect.
     *
     * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
     * @range binary
     */
    | "prev_effect";

  type LibraryControl =
    /**
     * Triggers different actions, depending on which interface element currently has keyboard focus:
     * |Search bar|
     * |---|
     * |text box|moves focus to tracks table|
     * |Clear button|clears search text|
     * |Sidebar|
     * |collapsed node|expands the item (except Tracks and Auto DJ)|
     * |leaf node|moves focus to tracks table|
     * |Tracks table|Performs the action selected in Preferences ‣ Library ‣ Track Double-Click Action (default is “Load selected track”). Also see Preferences ‣ Decks ‣ Playing track protection|
     * |Context menus|presses Enter|
     * |Dialogs / popups|presses Enter. Note: the Move.. controls allow to move button focus.|
     *
     * @groups [Library]
     * @range Binary
     * @feedback Context dependent
     * @since New in version 2.1.0.
     */
    | "GoToItem"

    /**
     * Equivalent to pressing the Down key on the keyboard
     *
     * @groups [Library]
     * @range Binary
     * @feedback Currently selected item changes
     * @since New in version 2.1.0.
     */
    | "MoveDown"

    /**
     * Move focus the specified number of panes forward or backwards. Intended to be mapped to an encoder knob.
     *
     * @groups [Library]
     * @range Relative (positive values move forward, negative values move backward)
     * @feedback Currently focused pane changes
     * @since New in version 2.1.0.
     */
    | "MoveFocus"

    /**
     * Equivalent to pressing the Shift + Tab key on the keyboard
     *
     * @groups [Library]
     * @range Binary
     * @feedback Currently focused pane changes
     * @since New in version 2.1.0.
     */
    | "MoveFocusBackward"

    /**
     * Equivalent to pressing the Tab key on the keyboard
     *
     * @groups [Library]
     * @range Binary
     * @feedback Currently focused pane changes
     * @since New in version 2.1.0.
     */
    | "MoveFocusForward"

    /**
     * Move the specified number of locations left or right. Intended to be mapped to an encoder knob.
     *
     * @groups [Library]
     * @range Relative (positive values move right, negative values move left)
     * @feedback Currently selected item changes
     * @since New in version 2.1.0.
     */
    | "MoveHorizontal"

    /**
     * Equivalent to pressing the Left key on the keyboard
     *
     * @groups [Library]
     * @range Binary
     * @feedback Currently selected item changes
     * @since New in version 2.1.0.
     */
    | "MoveLeft"

    /**
     * Equivalent to pressing the Right key on the keyboard
     *
     * @groups [Library]
     * @range Binary
     * @feedback Currently selected item changes
     * @since New in version 2.1.0.
     */
    | "MoveRight"

    /**
     * Equivalent to pressing the Up key on the keyboard
     *
     * @groups [Library]
     * @range Binary
     * @feedback Currently selected item changes
     * @since New in version 2.1.0.
     */
    | "MoveUp"

    /**
     * Move the specified number of locations up or down. Intended to be mapped to an encoder knob.
     *
     * @groups [Library]
     * @range Relative (positive values move down, negative values move up)
     * @feedback Currently selected item changes
     * @since New in version 2.1.0.
     */
    | "MoveVertical"

    /**
     * Equivalent to pressing the PageDown key on the keyboard
     *
     * @groups [Library]
     * @range Binary
     * @feedback Currently selected item changes
     * @since New in version 2.1.0.
     */
    | "ScrollDown"

    /**
     * Equivalent to pressing the PageUp key on the keyboard
     *
     * @groups [Library]
     * @range Binary
     * @feedback Currently selected item changes
     * @since New in version 2.1.0.
     */
    | "ScrollUp"

    /**
     * Scroll the specified number of pages up or down. Intended to be mapped to an encoder knob.
     *
     * @groups [Library]
     * @range Relative (positive values move down, negative values move up)
     * @feedback Currently selected item changes
     * @since New in version 2.1.0.
     */
    | "ScrollVertical"

    /**
     * Clear the search.
     *
     * @groups [Library]
     * @range Binary
     * @feedback Searchbox query is cleared
     * @since New in version 2.4.0.
     */
    | "clear_search"

    /**
     * Read this control to know which library widget is currently focused, or write in order to focus a specific library widget.
     * This control can be used in controller scripts to trigger context-specific actions. For example, if the
     * tracks table has focus, pressing a button loads the selected track to a specific deck, while the same
     * button would clear the search if the search bar is focused.
     * Note: This control is useful only if a Mixxx window has keyboard focus, otherwise it always returns 0.
     *
     * @groups [Library]
     * @range
     * |Value|writeable|Widget|
     * |---|---|---|
     * |0  |   |none|
     * |1  |X  |Search bar|
     * |2  |X  |Tree view|
     * |3  |X  |Tracks table or root views of library features|
     * |4  |   |Context menu (menus of library widgets or other editable widgets, or main menu bar)|
     * |5  |   |Dialog (any confirmation or error popup, preferences, track properties or cover art window)|
     * |6  |   |Unknown (widgets that don’t fit into any of the above categories)|
     * @feedback Currently focused widget changes
     * @since New in version 2.4.0.
     */
    | "focused_widget"

    /**
     * Decrease the size of the library font
     *
     * @groups [Library]
     * @range Binary
     * @feedback Library view
     * @since New in version 2.0.0.
     */
    | "font_size_decrement"

    /**
     * Increase the size of the library font. If the row height is smaller than the font-size the larger of the two is used.
     *
     * @groups [Library]
     * @range Binary
     * @feedback Library view
     * @since New in version 2.0.0.
     */
    | "font_size_increment"

    /**
     * Increase or decrease the size of the library font
     *
     * @groups [Library]
     * @range Relative
     * @feedback Library view
     * @since New in version 2.0.0.
     */
    | "font_size_knob"

    /**
     * Select the next saved search query. Wraps around at the last item to the empty search.
     *
     * @groups [Library]
     * @range Binary
     * @feedback Searchbox query changes
     * @since New in version 2.4.0.
     */
    | "search_history_next"

    /**
     * Select the previous saved search query. Wraps around at the top to the last item.
     *
     * @groups [Library]
     * @range Binary
     * @feedback Searchbox query changes
     * @since New in version 2.4.0.
     */
    | "search_history_prev"

    /**
     * Select another saved search query. < 0 goes up the list, > 0 goes down. Wraps around at the top and bottom.
     *
     * @groups [Library]
     * @range -N / +N
     * @feedback Searchbox query changes
     * @since New in version 2.4.0.
     */
    | "search_history_selector"

    /**
     * Toggle the track context menu for all tracks selected in the current library view.
     * The control value is 1 if there is already a menu shown for the current view.
     * Note that the control is not aware of other track menus, for example those opened
     * by right-clicking track text labels in decks. Only the most recent menu can be
     * navigated with the MoveUp/Down controls and
     * selected actions or submenus can be activated with GoToItem.
     *
     * @groups [Library]
     * @range Binary
     * @feedback Tracks table context menu is shown or hidden.
     * @since New in version 2.4.0.
     */
    | "show_track_menu"

    /**
     * Indicates the sorting column the track table
     *
     * @groups [Library]
     * @range
     * |Value|Description|Library|Playlist|Crate|Browse|
     * |---|---|---|---|---|---|
     * |1  |Artist|X  |X  |X  |X  |
     * |2  |Title|X  |X  |X  |X  |
     * |3  |Album|X  |X  |X  |X  |
     * |4  |Albumartist|X  |X  |X  |X  |
     * |5  |Year|X  |X  |X  |X  |
     * |6  |Genre|X  |X  |X  |X  |
     * |7  |Composer|X  |X  |X  |X  |
     * |8  |Grouping|X  |X  |X  |X  |
     * |9  |Tracknumber|X  |X  |X  |X  |
     * |10 |Filetype|X  |X  |X  |X  |
     * |11 |Native Location|X  |X  |X  |X  |
     * |12 |Comment|X  |X  |X  |X  |
     * |13 |Duration|X  |X  |X  |X  |
     * |14 |Bitrate|X  |X  |X  |X  |
     * |15 |BPM|X  |X  |X  |X  |
     * |16 |ReplayGain|X  |X  |X  |X  |
     * |17 |Datetime Added|X  |X  |X  |X  |
     * |18 |Times Played|X  |X  |X  |X  |
     * |19 |Rating|X  |X  |X  |X  |
     * |20 |Key|X  |X  |X  |X  |
     * |21 |Preview|X  |X  |X  |X  |
     * |22 |Coverart|X  |X  |X  |   |
     * |23 |Position|   |X  |   |   |
     * |24 |Playlist ID|   |X  |   |   |
     * |25 |Location|   |X  |   |   |
     * |26 |Filename|   |   |   |X  |
     * |27 |File Modified Time|   |   |   |X  |
     * |28 |File Creation Time|   |   |   |X  |
     * |29 |Sample Rate|   |   |   |   |
     * |30 |Track Color|X  |X  |X  |   |
     * |31 |Last Played|X  |X  |X  |   |
     * @feedback Sorting indicator in the column headers of the track table
     * @since New in version 2.3.0.
     */
    | "sort_column"

    /**
     * Equivalent to clicking on column headers. A new value sets [Library],sort_column to that value and [Library],sort_order to 0, setting the same value again will toggle [Library],sort_order.
     *
     * @groups [Library]
     * @range Same as for [Library],sort_column or value 0 for sorting according the current column with the cursor on it
     * @feedback Sorting indicator in the column headers of the track table
     * @since New in version 2.3.0.
     */
    | "sort_column_toggle"

    /**
     * Sort the column of the table cell that is currently focused, which is equivalent to
     * setting [Library],sort_column_toggle to 0. Though unlike that, it can
     * be mapped to pushbuttons directly.
     *
     * @groups [Library]
     * @range Binary
     * @feedback Sorting indicator in the column headers of the track table
     * @since New in version 2.4.0.
     */
    | "sort_focused_column"

    /**
     * Indicate the sort order of the track tables.
     *
     * @groups [Library]
     * @range Binary (0 for ascending, 1 for descending)
     * @feedback Sorting indicator in the column headers of the track table
     * @since New in version 2.3.0.
     */
    | "sort_order"

    /**
     * Set color of selected track to next color in palette.
     *
     * @groups [Library]
     * @range Binary
     * @feedback Track color changes in the library view.
     * @since New in version 2.3.0.
     */
    | "track_color_next"

    /**
     * Set color of selected track to previous color in palette.
     *
     * @groups [Library]
     * @range Binary
     * @feedback Track color changes in the library view.
     * @since New in version 2.3.0.
     */
    | "track_color_prev"
    | LibraryPlaylistControl;

  type LibraryPlaylistControl =
    /**
     * Add selected track(s) to Auto DJ Queue (bottom).
     *
     * @groups [Library], [Playlist]
     * @range Binary
     * @feedback Append track(s) to Auto DJ playlist
     * @since New in version 2.0.0.
     */
    | "AutoDjAddBottom"

    /**
     * Add selected track(s) to Auto DJ Queue (top).
     *
     * @groups [Library], [Playlist]
     * @range Binary
     * @feedback Prepend track(s) to Auto DJ playlist
     * @since New in version 2.0.0.
     */
    | "AutoDjAddTop";

  type MainControl =
    /**
     * Indicates when the signal is clipping (too loud for the hardware and is being distorted) (composite).
     *
     * @groups [Main]
     * @range binary
     * @feedback Clip light (mono)
     * @since New in version 2.4.0: Replaces the deprecated [Master],PeakIndicator control.
     */
    | "peak_indicator"

    /**
     * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the left channel.
     *
     * @groups [Main]
     * @range binary
     * @feedback Clip light (left)
     * @since New in version 2.4.0: Replaces the deprecated [Master],PeakIndicatorL control.
     */
    | "peak_indicator_l"

    /**
     * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the right channel.
     *
     * @groups [Main]
     * @range binary
     * @feedback Clip light (right)
     * @since New in version 2.4.0: Replaces the deprecated [Master],PeakIndicatorR control.
     */
    | "peak_indicator_r"

    /**
     * Outputs the current instantaneous main volume (composite).
     *
     * @groups [Main]
     * @range default
     * @feedback Main meter (mono)
     * @since New in version 2.4.0: Replaces the deprecated [Master],VuMeter control.
     */
    | "vu_meter"

    /**
     * Outputs the current instantaneous main volume for the left channel.
     *
     * @groups [Main]
     * @range default
     * @feedback Main meter L
     * @since New in version 2.4.0: Replaces the deprecated [Master],VuMeterL control.
     */
    | "vu_meter_l"

    /**
     * Outputs the current instantaneous main volume for the right channel.
     *
     * @groups [Main]
     * @range default
     * @feedback Main meter R
     * @since New in version 2.4.0: Replaces the deprecated [Master],VuMeterR control.
     */
    | "vu_meter_r";

  type MasterControl =
    /**
     * Indicates a buffer under or over-flow. Resets after 500 ms
     *
     * @groups [Master]
     * @range binary
     * @feedback Overload indicator
     * @since New in version 2.0.0.
     */
    | "audio_latency_overload"

    /**
     * Counts buffer over and under-flows. Max one per 500 ms
     *
     * @groups [Master]
     * @range 0 .. n
     * @feedback Counter in hardware preferences
     * @since New in version 2.0.0.
     */
    | "audio_latency_overload_count"

    /**
     * Reflects fraction of latency, given by the audio buffer size, spend for audio processing inside Mixxx. At value near 25 % there is a high risk of buffer underflows
     *
     * @groups [Master]
     * @range 0 .. 25 %
     * @feedback latency meter
     * @since New in version 2.0.0.
     */
    | "audio_latency_usage"

    /**
     * Adjusts the left/right channel balance on the Main output.
     * This is a ControlPotMeter control.
     *
     * @groups [Master]
     * @range -1.0..1.0
     * @feedback Center Balance knob
     * @kind pot meter control
     */
    | `balance${PotMeterSuffix}`

    /**
     * Indicates whether a Booth output is configured in the Sound Hardware Preferences.
     *
     * @groups [Master]
     * @range binary
     * @feedback Booth gain knob shown or hidden
     * @since New in version 2.1.0.
     */
    | "booth_enabled"

    /**
     * Adjusts the gain of the Booth output.
     * This is a ControlPotMeter control.
     *
     * @groups [Master]
     * @range 0.0…1.0…5.0
     * @feedback Booth gain knob
     * @since New in version 2.1.0.
     * @kind pot meter control
     */
    | `booth_gain${PotMeterSuffix}`

    /**
     * Adjusts the crossfader between players/decks (-1.0 is all the way left).
     * This is a ControlPotMeter control.
     *
     * @groups [Master]
     * @range -1.0..1.0
     * @feedback Crossfader slider
     * @kind pot meter control
     */
    | `crossfader${PotMeterSuffix}`

    /**
     * Moves the crossfader left by 1/10th.
     *
     * @groups [Master]
     * @range binary
     * @feedback Crossfader slider
     */
    | "crossfader_down"

    /**
     * Moves the crossfader left by 1/100th.
     *
     * @groups [Master]
     * @range binary
     * @feedback Crossfader slider
     * @since New in version 1.10.0.
     */
    | "crossfader_down_small"

    /**
     * Moves the crossfader right by 1/10th.
     *
     * @groups [Master]
     * @range binary
     * @feedback Crossfader slider
     */
    | "crossfader_up"

    /**
     * Moves the crossfader right by 1/100th.
     *
     * @groups [Master]
     * @range binary
     * @feedback Crossfader slider
     * @since New in version 1.10.0.
     */
    | "crossfader_up_small"

    /**
     * Microphone ducking strength
     * This is a ControlPotMeter control.
     *
     * @groups [Master]
     * @range 0.0..1.0
     * @feedback Strength knob
     * @since New in version 2.0.0.
     * @kind pot meter control
     */
    | `duckStrength${PotMeterSuffix}`

    /**
     * Indicator that the main mix is processed.
     *
     * @groups [Master]
     * @range binary
     * @feedback None
     * @since New in version 2.0.0.
     */
    | "enabled"

    /**
     * Adjusts the gain for the main output as well as recording and broadcasting signal.
     * This is a ControlPotMeter control.
     *
     * @groups [Master]
     * @range 0.0..1.0..5.0
     * @feedback Main volume knob
     * @since New in version 2.0.0.
     * @kind pot meter control
     */
    | `gain${PotMeterSuffix}`

    /**
     * Indicator that the headphone mix is processed.
     *
     * @groups [Master]
     * @range binary
     * @feedback None
     * @since New in version 2.0.0.
     */
    | "headEnabled"

    /**
     * Adjusts the headphone output gain.
     * This is a ControlPotMeter control.
     *
     * @groups [Master]
     * @range 0.0..1.0..5.0
     * @feedback Headphone volume knob
     * @since New in version 2.0.0.
     * @kind pot meter control
     */
    | `headGain${PotMeterSuffix}`

    /**
     * Adjusts the cue/main mix in the headphone output.
     * This is a ControlPotMeter control.
     *
     * @groups [Master]
     * @range default
     * @feedback Pre/Main knob
     * @kind pot meter control
     */
    | `headMix${PotMeterSuffix}`

    /**
     * Splits headphone stereo cueing into right (main mono) and left (PFL mono).
     *
     * @groups [Master]
     * @range binary
     * @feedback Split Cue button
     * @since New in version 2.0.0.
     */
    | "headSplit"

    /**
     * Latency setting (sound buffer size) in milliseconds (default 64).
     *
     * @groups [Master]
     * @range >=0 (absolute value)
     * @feedback Latency slider in the prefs
     */
    | "latency"

    /**
     * Toggle microphone ducking mode (OFF, AUTO, MANUAL)
     *
     * @groups [Master]
     * @range FIXME
     * @feedback Ducking mode button
     * @since New in version 2.0.0.
     */
    | "talkoverDucking";

  type PlaylistControl =
    /**
     * Scrolls the given number of items (view, playlist, crate, etc.) in the side pane (can be negative for reverse direction).
     *
     * @groups [Playlist]
     * @range relative value
     * @feedback Library sidebar highlight
     */
    | "SelectPlaylist"

    /**
     * Scrolls the given number of tracks in the track table (can be negative for reverse direction).
     *
     * @groups [Playlist]
     * @range relative value
     * @feedback Library track table highlight
     */
    | "SelectTrackKnob"
    | LibraryPlaylistControl;

  type RecordingControl =
    /**
     * Indicates whether Mixxx is currently recording.
     *
     * @groups [Recording]
     * @range
     * |Value|Meaning|
     * |---|---|
     * |0  |Recording Stopped|
     * |1  |Initialize Recording|
     * |2  |Recording Active|
     * @feedback Recording icon
     */
    | "status"

    /**
     * Turns recording on or off.
     *
     * @groups [Recording]
     * @range binary
     * @feedback Recording icon
     */
    | "toggle_recording";

  type SamplerControl =
    /**
     * Load saved sampler configuration file and add tracks to the available samplers.
     *
     * @groups [Sampler]
     * @range binary
     * @feedback Opens file dialog. Select configuration file.
     * @since New in version 2.0.0.
     */
    | "LoadSamplerBank"

    /**
     * Save sampler configuration. Make currently loaded tracks in samplers instantly available at a later point.
     *
     * @groups [Sampler]
     * @range binary
     * @feedback Opens file dialog. Configuration file can be named and saved.
     * @since New in version 2.0.0.
     */
    | "SaveSamplerBank";

  type ShoutcastControl =
    /**
     * Shows if live Internet broadcasting is enabled.
     *
     * @groups [Shoutcast]
     * @range ?
     * @feedback shoutcast only supports mp3 format as field
     */
    | "enabled"

    /**
     * This control displays whether broadcasting connection to Shoutcast server was successfully established.
     *
     * @groups [Shoutcast]
     * @range binary
     * @feedback None
     */
    | "status";

  type SkinControl =
    /**
     * Toggle the display of the effect rack in the user interface.
     *
     * @groups [Skin]
     * @range binary
     * @feedback Effect rack is shown/hidden.
     * @since New in version 2.4.0: Replaces the deprecated [EffectRack1],show control.
     */
    | "show_effectrack"

    /**
     * Toggle the display of cover art in the library section of the user interface.
     *
     * @groups [Skin]
     * @range binary
     * @feedback Cover art in the library is shown/hidden.
     * @since New in version 2.4.0: Replaces the deprecated [Library],show_coverart control.
     */
    | "show_library_coverart"

    /**
     * Toggle maximized view of library section of the user interface.
     *
     * @groups [Skin]
     * @range binary
     * @feedback The library section of the user interface is enlarged/shrunk.
     * @since New in version 2.4.0: Replaces the deprecated [Master],maximize_library control.
     */
    | "show_maximized_library"

    /**
     * Toggle the display of sampler banks in the user interface.
     *
     * @groups [Skin]
     * @range binary
     * @feedback Sampler banks are shown/hidden.
     * @since New in version 2.4.0: Replaces the deprecated [Samplers],show_samplers control.
     */
    | "show_samplers"

    /**
     * Toggle the vinyl control section in the user interface.
     *
     * @groups [Skin]
     * @range binary
     * @feedback Vinyl controls are shown/hidden.
     * @since New in version 2.4.0: Replaces the deprecated [VinylControl],show_vinylcontrol control.
     */
    | "show_vinylcontrol";

  type VinylControlControl =
    /**
     * Moves control by a vinyl control signal from one deck to another if using the single deck vinyl control (VC) feature.
     *
     * @groups [VinylControl]
     * @range binary
     * @feedback If VC isn’t enabled on any decks, enable it on the first one we’re receiving samples for. If VC is enabled on a single (exclusive) deck, and another deck is setup to receive samples, disable it on the former deck and enable it on the next eligible deck (ordered by deck number). If VC is enabled on multiple decks, don’t do anything.
     * @since New in version 1.10.0.
     */
    | "Toggle"

    /**
     * Allows to amplify the “phono” level of attached turntables to “line” level.
     * This is equivalent to setting the turntable boost in Options ‣ Preferences ‣ Vinyl Control
     *
     * @groups [VinylControl]
     * @range binary
     * @feedback position of Boost slider in Options ‣ Preferences ‣ Vinyl Control (is not updated while viewing this Preferences page)
     * @since New in version 1.10.0.
     */
    | "gain";

  type QuickEffectRack1ChannelIControl = EffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl;

  type QuickEffectRack1ChannelIEffect1Control = EffectRack1EffectUnitNEffectMEqualizerRack1ChannelIEffect1QuickEffectRack1ChannelIEffect1Control;

  type EqualizerRack1ChannelIEffect1Control = EffectRack1EffectUnitNEffectMEqualizerRack1ChannelIEffect1QuickEffectRack1ChannelIEffect1Control;

  type EffectRack1EffectUnitNEffectMControl = EffectRack1EffectUnitNEffectMEqualizerRack1ChannelIEffect1QuickEffectRack1ChannelIEffect1Control;

  type SamplerNControl = AuxiliaryNChannelNPreviewDeckNSamplerNControl | ChannelNPreviewDeckNSamplerNControl;

  type PreviewDeckNControl = AuxiliaryNChannelNPreviewDeckNSamplerNControl | ChannelNPreviewDeckNSamplerNControl;

  type MicrophoneNControl = AuxiliaryNMicrophoneNControl;

  type EqualizerRack1ChannelIControl = EffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl;

  namespace ReadOnly {
    // Read-only controls
    type ReadOnlyControls = {
      "[App]": ReadOnly.ReadOnlyAppControl;
      "[EffectRack1]": ReadOnly.ReadOnlyEffectRack1Control;
      "[EqualizerRack1]": ReadOnly.ReadOnlyEqualizerRack1Control;
      "[Master]": ReadOnly.ReadOnlyMasterControl;
      "[QuickEffectRack1]": ReadOnly.ReadOnlyQuickEffectRack1Control;
    } & {
      [key: `[Auxiliary${number}]`]: ReadOnly.ReadOnlyAuxiliaryNControl;
      [key: `[Channel${number}]`]: ReadOnly.ReadOnlyChannelNControl;
      [key: `[EffectRack1_EffectUnit${number}]`]: ReadOnly.ReadOnlyEffectRack1EffectUnitNControl;
      [key: `[EffectRack1_EffectUnit${number}_Effect${number}]`]: ReadOnly.ReadOnlyEffectRack1EffectUnitNEffectMControl;
      [key: `[EqualizerRack1_[Channel${number}]]`]: ReadOnly.ReadOnlyEqualizerRack1ChannelIControl;
      [key: `[EqualizerRack1_[Channel${number}]_Effect1]`]: ReadOnly.ReadOnlyEqualizerRack1ChannelIEffect1Control;
      [key: `[Microphone${number}]`]: ReadOnly.ReadOnlyMicrophoneNControl;
      [key: `[PreviewDeck${number}]`]: ReadOnly.ReadOnlyPreviewDeckNControl;
      [key: `[QuickEffectRack1_[Channel${number}]]`]: ReadOnly.ReadOnlyQuickEffectRack1ChannelIControl;
      [key: `[QuickEffectRack1_[Channel${number}]_Effect1]`]: ReadOnly.ReadOnlyQuickEffectRack1ChannelIEffect1Control;
      [key: `[Sampler${number}]`]: ReadOnly.ReadOnlySamplerNControl;
    };
    type ReadOnlyAppControl =
      /**
       * A throttled timer that provides the time elapsed in seconds since Mixxx was started.
       * This control is updated at a rate of 20 Hz (every 50 milliseconds). It is the preferred timer for scripting animations in controller mappings (like VU meters or spinning animations) as it provides a smooth visual result without the performance overhead of [App],gui_tick_full_period_s.
       * Only available when using the legacy GUI (not the QML interface).
       *
       * @groups [App]
       * @range 0.0 .. n
       * @feedback None
       * @since New in version 2.4.0.
       * @readonly
       */
      | "gui_tick_50ms_period_s"

      /**
       * A high-resolution timer that provides the elapsed time in seconds since Mixxx was started.
       * This control is updated on every GUI tick, which corresponds to the waveform rendering frame rate. It is suitable for very smooth, high-framerate animations in scripts. However, for most use cases like VU meters, consider using [App],gui_tick_50ms_period_s to improve performance by reducing the script execution rate.
       * Only available when using the legacy GUI (not the QML interface).
       *
       * @groups [App]
       * @range 0.0 .. n
       * @feedback None
       * @since New in version 2.4.0.
       * @readonly
       */
      | "gui_tick_full_period_s"

      /**
       * Alternates between 0.0 and 1.0 every 250 milliseconds.
       * This control may be used to implement a blinking LED in JavaScript and is
       * guaranteed to light up at the same time as
       * [ChannelN],cue_indicator and
       * [ChannelN],play_indicator when these are blinking (depending
       * on the currently chosen cue mode).
       *
       * @groups [App]
       * @range binary
       * @feedback None
       * @since New in version 2.4.0.
       * @readonly
       */
      | "indicator_250ms"

      /**
       * Alternates between 0.0 and 1.0 every 500 milliseconds.
       * This control may be used to implement a blinking LED in JavaScript and is
       * guaranteed to light up at the same time as
       * [ChannelN],cue_indicator and
       * [ChannelN],play_indicator when these are blinking (depending
       * on the currently chosen cue mode).
       *
       * @groups [App]
       * @range binary
       * @feedback None
       * @since New in version 2.4.0.
       * @readonly
       */
      | "indicator_500ms";

    type ReadOnlyAuxiliaryNChannelNMicrophoneNControl =
      /**
       * 1 if there is input is configured for this channel, 0 if not.
       * In the case of [ChannelN] it corresponds to
       * Vinyl Control. A configured input is required to enable [ChannelN],passthrough
       *
       * @groups [AuxiliaryN], [ChannelN], [MicrophoneN]
       * @range binary
       * @feedback Configured channel in the sound preferences.
       * @readonly
       */
      "input_configured";

    type ReadOnlyChannelNPreviewDeckNSamplerNControl =
      /**
       * Indicates, depending on the play direction, how the player is currently positioned to the closest beat.
       * An LED controlled by beat_active can be used for beat matching or for finding a beat using jog or control vinyl.
       * |Value|Play direction|Position|
       * |---|---|---|
       * |0  |   |Set when play direction changes or +-20% of the distance to the previous/next beat is reached|
       * |1  |Forward|Set at a beat|
       * |2  |Reverse|Set at a beat|
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range real number
       * @feedback None
       * @since New in version 1.10.0: (Reverse indication added in 2.4.0)
       * @readonly
       */
      | "beat_active"

      /**
       * Switches to 1 if the play position is within the end range defined in Preferences ‣ Waveforms ‣ End of track warning.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Waveform and Overview widgets show a flashing border
       * @readonly
       */
      | "end_of_track"

      /**
       * The detected BPM of the loaded track.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range positive value
       * @feedback None
       * @readonly
       */
      | "file_bpm"

      /**
       * The detected key of the loaded track.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range ?
       * @feedback None
       * @since New in version 2.0.0.
       * @readonly
       */
      | "file_key"

      /**
       * 1 if intro end cue is set, (position is not -1), 0 otherwise.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Intro end button lights up.
       * @since New in version 2.3.0.
       * @readonly
       */
      | "intro_end_enabled"

      /**
       * 1 if intro start cue is set, (position is not -1), 0 otherwise.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Intro start button lights up.
       * @since New in version 2.3.0.
       * @readonly
       */
      | "intro_start_enabled"

      /**
       * 1 if outro end cue is set, (position is not -1), 0 otherwise.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Outro end button lights up.
       * @since New in version 2.3.0.
       * @readonly
       */
      | "outro_end_enabled"

      /**
       * 1 if outro start cue is set, (position is not -1), 0 otherwise.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Outro start button lights up.
       * @since New in version 2.3.0.
       * @readonly
       */
      | "outro_start_enabled"

      /**
       * Provides information to be bound with the a Play/Pause button e.g blinking when play is possible
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Play/pause button
       * @since New in version 2.0.0.
       * @readonly
       */
      | "play_indicator"

      /**
       * This is set to 1 when the track is playing, but not when previewing (see play).
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Play/pause button
       * @since New in version 2.3.0.
       * @readonly
       */
      | "play_latched"

      /**
       * Whether a track is loaded in the specified deck
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Waveform and track metadata shown in deck
       * @since New in version 2.1.0.
       * @readonly
       */
      | "track_loaded"

      /**
       * Sample rate of the track loaded on the specified deck
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range absolute value
       * @feedback None
       * @since New in version 1.9.0.
       * @readonly
       */
      | "track_samplerate"

      /**
       * Number of sound samples in the track loaded on the specified deck
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range absolute value
       * @feedback None
       * @readonly
       */
      | "track_samples"

      /**
       * Provides visual feedback with regards to vinyl control status.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range 0.0-3.0
       * @feedback Off for control disabled, green for control enabled, blinking yellow for when the needle reaches the end of the record, and red for needle skip detected
       * @since New in version 1.10.0.
       * @readonly
       */
      | "vinylcontrol_status";

    type ReadOnlyEffectRack1EqualizerRack1QuickEffectRack1Control =
      /**
       * The number of EffectUnits in this rack
       *
       * @groups [EffectRack1], [EqualizerRack1], [QuickEffectRack1]
       * @range integer
       * @readonly
       */
      "num_effectunits";

    type ReadOnlyEffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl =
      /**
       * Whether an EffectChain is loaded into the EffectUnit
       *
       * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
       * @range binary
       * @readonly
       */
      | "loaded"

      /**
       * The number of effect chain presets available in this EffectUnit, including the
       * empty/passthrough preset “---”.
       *
       * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
       * @range integer, >=1
       * @readonly
       */
      | "num_chain_presets"

      /**
       * The number of effect slots available in this EffectUnit.
       *
       * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
       * @range integer
       * @readonly
       */
      | "num_effectslots";

    type ReadOnlyEffectRack1EffectUnitNEffectMEqualizerRack1ChannelIEffect1QuickEffectRack1ChannelIEffect1Control =
      /**
       * Whether or not the Kth parameter slot has an effect parameter loaded into it.
       *
       * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
       * @range binary
       * @readonly
       */
      | `button_parameter${number}_loaded`

      /**
       * The type of the Kth parameter value. See the Parameter Value Types table.
       *
       * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
       * @range integer
       * @readonly
       */
      | `button_parameter${number}_type`

      /**
       * Whether an Effect is loaded into this EffectSlot
       *
       * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
       * @range binary
       * @readonly
       */
      | "loaded"

      /**
       * The number of button parameters the currently loaded effect has.
       *
       * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
       * @range integer, 0 if no effect is loaded
       * @readonly
       */
      | "num_button_parameters"

      /**
       * The number of button parameter slots available.
       *
       * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
       * @range integer
       * @readonly
       */
      | "num_button_parameterslots"

      /**
       * The number of parameters the currently loaded effect has.
       *
       * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
       * @range integer,  0 if no effect is loaded
       * @readonly
       */
      | "num_parameters"

      /**
       * The number of parameter slots available.
       *
       * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
       * @range integer
       * @readonly
       */
      | "num_parameterslots"

      /**
       * Whether or not the Kth parameter slot has an effect parameter loaded into it.
       *
       * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
       * @range binary
       * @readonly
       */
      | `parameter${number}_loaded`

      /**
       * The type of the Kth parameter value. See the Parameter Value Types table.
       *
       * @groups [EffectRack1_EffectUnitN_EffectM], [EqualizerRack1_[ChannelI]_Effect1], [QuickEffectRack1_[ChannelI]_Effect1]
       * @range integer
       * @readonly
       */
      | `parameter${number}_type`;

    type ReadOnlyMasterControl =
      /**
       * The number of available effects that can be selected in an effect slot.
       *
       * @groups [Master]
       * @range integer
       * @feedback None
       * @since New in version 2.1.0.
       * @readonly
       */
      "num_effectsavailable";

    type ReadOnlyQuickEffectRack1ChannelIControl = ReadOnlyEffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl;

    type ReadOnlyEffectRack1EffectUnitNControl = ReadOnlyEffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl;

    type ReadOnlyEffectRack1Control = ReadOnlyEffectRack1EqualizerRack1QuickEffectRack1Control;

    type ReadOnlyEffectRack1EffectUnitNEffectMControl =
      ReadOnlyEffectRack1EffectUnitNEffectMEqualizerRack1ChannelIEffect1QuickEffectRack1ChannelIEffect1Control;

    type ReadOnlyAuxiliaryNControl = ReadOnlyAuxiliaryNChannelNMicrophoneNControl;

    type ReadOnlyEqualizerRack1Control = ReadOnlyEffectRack1EqualizerRack1QuickEffectRack1Control;

    type ReadOnlySamplerNControl = ReadOnlyChannelNPreviewDeckNSamplerNControl;

    type ReadOnlyPreviewDeckNControl = ReadOnlyChannelNPreviewDeckNSamplerNControl;

    type ReadOnlyEqualizerRack1ChannelIEffect1Control =
      ReadOnlyEffectRack1EffectUnitNEffectMEqualizerRack1ChannelIEffect1QuickEffectRack1ChannelIEffect1Control;

    type ReadOnlyMicrophoneNControl = ReadOnlyAuxiliaryNChannelNMicrophoneNControl;

    type ReadOnlyQuickEffectRack1Control = ReadOnlyEffectRack1EqualizerRack1QuickEffectRack1Control;

    type ReadOnlyQuickEffectRack1ChannelIEffect1Control =
      ReadOnlyEffectRack1EffectUnitNEffectMEqualizerRack1ChannelIEffect1QuickEffectRack1ChannelIEffect1Control;

    type ReadOnlyEqualizerRack1ChannelIControl = ReadOnlyEffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl;

    type ReadOnlyChannelNControl = ReadOnlyAuxiliaryNChannelNMicrophoneNControl | ReadOnlyChannelNPreviewDeckNSamplerNControl;
  }

  namespace Deprecated {
    type DeprecatedAuxiliaryNMicrophoneNControl =
      /**
       * Indicates when the signal is clipping (too loud for the hardware and is being distorted)
       *
       * @groups [AuxiliaryN], [MicrophoneN]
       * @range binary
       * @feedback Microphone Clip light
       * @since New in version ?.?.?.
       * @deprecated since  version 2.4.0: Use [MicrophoneN],peak_indicator and [AuxiliaryN],peak_indicator instead.
       */
      | "PeakIndicator"

      /**
       * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the left channel
       *
       * @groups [AuxiliaryN], [MicrophoneN]
       * @range binary
       * @feedback Clip light (left)
       * @since New in version 2.0.0.
       * @deprecated since  version 2.4.0: Use [MicrophoneN],peak_indicator_l and [AuxiliaryN],peak_indicator_l instead.
       */
      | `PeakIndicator${number}`

      /**
       * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the right channel
       *
       * @groups [AuxiliaryN], [MicrophoneN]
       * @range binary
       * @feedback Clip light (right)
       * @since New in version 2.0.0.
       * @deprecated since  version 2.4.0: Use [MicrophoneN],peak_indicator_r and [AuxiliaryN],peak_indicator_r instead.
       */
      | `PeakIndicator${number}`

      /**
       * Outputs the current instantaneous channel volume
       *
       * @groups [AuxiliaryN], [MicrophoneN]
       * @range default
       * @feedback Microphone/auxiliary VU meter changes
       * @since New in version 1.10..
       * @deprecated since  version 2.4.0: Use [MicrophoneN],vu_meter and [AuxiliaryN],vu_meter instead.
       */
      | "VuMeter"

      /**
       * Outputs the current instantaneous channel volume
       *
       * @groups [AuxiliaryN], [MicrophoneN]
       * @range default
       * @feedback Microphone/auxiliary VU meter changes
       * @since New in version 2.0.0.
       * @deprecated since  version 2.4.0: Use [MicrophoneN],vu_meter_l and [AuxiliaryN],vu_meter_l instead.
       */
      | `VuMeter${number}`

      /**
       * Outputs the current instantaneous channel volume
       *
       * @groups [AuxiliaryN], [MicrophoneN]
       * @range default
       * @feedback Microphone/auxiliary VU meter changes
       * @since New in version 2.0.0.
       * @deprecated since  version 2.4.0: Use [MicrophoneN],vu_meter_r and [AuxiliaryN],vu_meter_r instead.
       */
      | `VuMeter${number}`

      /**
       * 1 if a channel input is enabled, 0 if not.
       *
       * @groups [AuxiliaryN], [MicrophoneN]
       * @range binary
       * @feedback Microphone is enabled.
       * @since New in version 1.10.0.
       * @deprecated since  version 2.0.0: Use [MicrophoneN],input_configured instead.
       */
      | "enabled"

      /**
       * Hold value at 1 to mix channel input into the main output.
       * For [MicrophoneN] use [MicrophoneN],talkover instead.
       * Note that [AuxiliaryN] also take [AuxiliaryN],orientation into account.
       *
       * @groups [AuxiliaryN], [MicrophoneN]
       * @range binary
       * @feedback Auxiliary: Play buttonMicrophone: N/A
       * @deprecated since  version 2.4.0: Use [MicrophoneN],talkover and [AuxiliaryN],main_mix instead.
       */
      | "master";

    type DeprecatedChannelNPreviewDeckNSamplerNControl =
      /**
       * Indicates when the signal is clipping (too loud for the hardware and is being distorted)
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Clip light
       * @deprecated since  version 2.4.0: Use [ChannelN],peak_indicator, [PreviewDeckN],peak_indicator and [SamplerN],peak_indicator instead.
       */
      | "PeakIndicator"

      /**
       * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the left channel
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Clip light (left)
       * @since New in version 2.0.0.
       * @deprecated since  version 2.4.0: Use [ChannelN],peak_indicator_l, [PreviewDeckN],peak_indicator_l and [SamplerN],peak_indicator_l instead.
       */
      | `PeakIndicator${number}`

      /**
       * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the right channel
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Clip light (right)
       * @since New in version 2.0.0.
       * @deprecated since  version 2.4.0: Use [ChannelN],peak_indicator_r, [PreviewDeckN],peak_indicator_r and [SamplerN],peak_indicator_r instead.
       */
      | `PeakIndicator${number}`

      /**
       * Outputs the current instantaneous deck volume
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range default
       * @feedback Deck VU meter
       * @since New in version ?.?.?.
       * @deprecated since  version 2.4.0: Use [ChannelN],vu_meter, Use [PreviewDeckN],vu_meter and [SamplerN],vu_meter instead.
       */
      | "VuMeter"

      /**
       * Outputs the current instantaneous deck volume for the left channel
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range default
       * @feedback Deck VU meter L
       * @since New in version ?.?.?.
       * @deprecated since  version 2.4.0: Use [ChannelN],vu_meter_l, Use [PreviewDeckN],vu_meter_l and [SamplerN],vu_meter_l instead.
       */
      | `VuMeter${number}`

      /**
       * Outputs the current instantaneous deck volume for the right channel
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range default
       * @feedback Deck VU meter R
       * @since New in version ?.?.?.
       * @deprecated since  version 2.4.0: Use [ChannelN],vu_meter_r, Use [PreviewDeckN],vu_meter_r and [SamplerN],vu_meter_r instead.
       */
      | `VuMeter${number}`

      /**
       * Setup a loop over the set number of beats.
       * If the loaded track has no beat grid, seconds are used instead of beats.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range positive real number
       * @feedback A loop is shown over the set number of beats.
       * @deprecated since  version 2.1.0: Use [ChannelN],beatloop_size and [ChannelN],beatloop_activate instead.
       */
      | "beatloop"

      /**
       * Setup a loop over X beats. A control exists for X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64
       * If the loaded track has no beat grid, seconds are used instead of beats.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range toggle
       * @feedback A loop is shown over X beats.
       * @since New in version 1.10.0.
       * @deprecated since  version 2.0.0: Use [ChannelN],beatloop_X_activate instead.
       */
      | `beatloop_${number}`

      /**
       * Toggles the filter effect.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Filter button
       * @since New in version 2.0.0.
       * @deprecated since  version 2.0.0: Use [QuickEffectRack1_[ChannelN]_Effect1],enabled instead.
       */
      | "filter"

      /**
       * Adjusts the intensity of the filter effect.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range default
       * @feedback Filter depth knob
       * @since New in version 2.0.0.
       * @deprecated since  version 2.0.0: Use [QuickEffectRack1_[ChannelN]],super1 instead.
       */
      | "filterDepth"

      /**
       * Adjusts the gain of the high EQ filter.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range 0.0..1.0..4.0
       * @feedback High EQ knob
       * @deprecated since  version 2.0.0: Use [EqualizerRack1_[ChannelI]_Effect1],parameter3 instead.
       */
      | "filterHigh"

      /**
       * Holds the gain of the high EQ to -inf while active.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback High EQ kill switch
       * @deprecated since  version 2.0.0: Use [EqualizerRack1_[ChannelI]_Effect1],button_parameter3 instead.
       */
      | "filterHighKill"

      /**
       * Adjusts the gain of the low EQ filter.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range 0.0..1.0..4.0
       * @feedback Low EQ knob
       * @deprecated since  version 2.0.0: Use [EqualizerRack1_[ChannelN]_Effect1],parameter1 instead.
       */
      | "filterLow"

      /**
       * Holds the gain of the low EQ to -inf while active
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Low EQ kill switch
       * @deprecated since  version 2.0.0: Use [EqualizerRack1_[ChannelI]_Effect1],button_parameter1 instead.
       */
      | "filterLowKill"

      /**
       * Adjusts the gain of the mid EQ filter..
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range 0.0..1.0..4.0
       * @feedback Mid EQ knob
       * @deprecated since  version 2.0.0: Use [EqualizerRack1_[ChannelI]_Effect1],parameter2 instead.
       */
      | "filterMid"

      /**
       * Holds the gain of the mid EQ to -inf while active.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Mid EQ kill switch
       * @deprecated since  version 2.0.0: Use [EqualizerRack1_[ChannelI]_Effect1],button_parameter2 instead.
       */
      | "filterMidKill"

      /**
       * Indicates if hotcue slot X is set, active or empty.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @since New in version 1.8.0.
       * @deprecated since  version 2.4.0: Use [ChannelN],hotcue_X_status instead.
       */
      | `hotcue_${number}_enabled`

      /**
       * Affects relative playback speed and direction for short instances (additive & is automatically reset to 0).
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range -3.0..3.0
       * @feedback waveform
       * @deprecated since  version ??: Use the JavaScript engine.scratch functions instead.
       */
      | "jog"

      /**
       * Toggles the current loop on or off. If the loop is ahead of the current play position, the track will keep playing normally until it reaches the loop.
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range binary
       * @feedback Loop range in waveform activates or deactivates.
       * @deprecated since  version 2.1.0: Use [ChannelN],reloop_toggle instead.
       */
      | "reloop_exit"

      /**
       * Affects playback speed and direction (differently whether currently playing or not) (multiplicative).
       *
       * @groups [ChannelN], [PreviewDeckN], [SamplerN]
       * @range -3.0..3.0
       * @feedback Waveform
       * @deprecated since  version ??: Use the JavaScript engine.scratch functions instead.
       */
      | "scratch";

    type DeprecatedEffectRack1Control =
      /**
       * Show the Effect Rack
       *
       * @groups [EffectRack1]
       * @range binary
       * @deprecated since  version 2.4.0: Use [Skin],show_effectrack instead.
       */
      "show";

    type DeprecatedEffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl =
      /**
       * Select EffectChain preset. > 0 goes one forward; < 0 goes one backward.
       *
       * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
       * @deprecated since  version 2.4.0: Use [EffectRack1_EffectUnitN],chain_preset_selector instead.
       */
      | "chain_selector"

      /**
       * Cycle to the next EffectChain preset after the currently loaded preset.
       *
       * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
       * @deprecated since  version 2.4.0: Use [EffectRack1_EffectUnitN],next_chain_preset instead.
       */
      | "next_chain"

      /**
       * Cycle to the next EffectChain preset after the currently loaded preset.
       *
       * @groups [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]], [QuickEffectRack1_[ChannelI]]
       * @deprecated since  version 2.4.0: Use [EffectRack1_EffectUnitN],prev_chain_preset instead.
       */
      | "prev_chain";

    type DeprecatedLibraryControl =
      /**
       * Toggle the Cover Art in Library
       *
       * @groups [Library]
       * @range Binary
       * @deprecated since  version 2.4.0: Use [Skin],show_library_coverart instead.
       */
      "show_coverart";

    type DeprecatedMasterControl =
      /**
       * Indicates when the signal is clipping (too loud for the hardware and is being distorted) (composite).
       *
       * @groups [Master]
       * @range binary
       * @feedback Clip light (mono)
       * @deprecated since  version 2.4.0: Use [Main],peak_indicator instead.
       */
      | "PeakIndicator"

      /**
       * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the left channel.
       *
       * @groups [Master]
       * @range binary
       * @feedback Clip light (left)
       * @deprecated since  version 2.4.0: Use [Main],peak_indicator_l instead.
       */
      | `PeakIndicator${number}`

      /**
       * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the right channel.
       *
       * @groups [Master]
       * @range binary
       * @feedback Clip light (right)
       * @deprecated since  version 2.4.0: Use [Main],peak_indicator_r instead.
       */
      | `PeakIndicator${number}`

      /**
       * Outputs the current instantaneous main volume (composite).
       *
       * @groups [Master]
       * @range default
       * @feedback Main meter (mono)
       * @deprecated since  version 2.4.0: Use [Main],vu_meter instead.
       */
      | "VuMeter"

      /**
       * Outputs the current instantaneous main volume for the left channel.
       *
       * @groups [Master]
       * @range default
       * @feedback Main meter L
       * @deprecated since  version 2.4.0: Use [Main],vu_meter_l instead.
       */
      | `VuMeter${number}`

      /**
       * Outputs the current instantaneous main volume for the right channel.
       *
       * @groups [Master]
       * @range default
       * @feedback Main meter R
       * @deprecated since  version 2.4.0: Use [Main],vu_meter_r instead.
       */
      | `VuMeter${number}`

      /**
       * A throttled timer that provides the time elapsed in seconds since Mixxx was started.
       *
       * @groups [Master]
       * @range 0.0 .. n
       * @feedback None
       * @since New in version 2.4.0.
       * @readonly
       * @deprecated since  version 2.5.0: Use [App],gui_tick_50ms_period_s instead.
       */
      | "guiTick50ms"

      /**
       * A high-resolution timer that provides the elapsed time in seconds since Mixxx was started.
       *
       * @groups [Master]
       * @range 0.0 .. n
       * @feedback None
       * @since New in version 2.4.0.
       * @readonly
       * @deprecated since  version 2.5.0: Use [App],gui_tick_full_period_s instead.
       */
      | "guiTickTime"

      /**
       * Adjust headphone volume.
       *
       * @groups [Master]
       * @range 0.0..1.0..5.0
       * @feedback Headphone Gain knob
       * @deprecated since  version 2.0.0: Use [Master],headGain instead.
       */
      | "headVolume"

      /**
       * Toggle maximized view of library.
       *
       * @groups [Master]
       * @range binary
       * @feedback Toggle maximized view of library
       * @since New in version 2.0.0.
       * @deprecated since  version 2.4.0: Use [Skin],show_maximized_library instead.
       */
      | "maximize_library"

      /**
       * The number of auxiliary inputs that can be configured.
       *
       * @groups [Master]
       * @range integer
       * @feedback None
       * @since New in version 2.2.4.
       * @deprecated since  version 2.4.0: Use [App],num_auxiliaries instead.
       */
      | "num_auxiliaries"

      /**
       * The number of decks currently enabled.
       *
       * @groups [Master]
       * @range integer
       * @feedback None
       * @since New in version 1.9.0.
       * @deprecated since  version 2.4.0: Use [App],num_decks instead.
       */
      | "num_decks"

      /**
       * The number of microphone inputs that can be configured.
       *
       * @groups [Master]
       * @range integer
       * @feedback None
       * @since New in version 2.2.4.
       * @deprecated since  version 2.4.0: Use [App],num_microphones instead.
       */
      | "num_microphones"

      /**
       * The number of preview decks currently enabled.
       *
       * @groups [Master]
       * @range integer
       * @feedback None
       * @since New in version 1.9.0.
       * @deprecated since  version 2.4.0: Use [App],num_preview_decks instead.
       */
      | "num_preview_decks"

      /**
       * The number of samplers currently enabled.
       *
       * @groups [Master]
       * @range integer
       * @feedback None
       * @since New in version 1.9.0.
       * @deprecated since  version 2.4.0: Use [App],num_samplers instead.
       */
      | "num_samplers"

      /**
       * The current output sample rate (default: 44100 Hz).
       *
       * @groups [Master]
       * @range absolute value (in Hz)
       * @feedback None
       * @deprecated since  version 2.4.0: Use [App],samplerate instead.
       */
      | "samplerate"

      /**
       * Adjust main volume.
       *
       * @groups [Master]
       * @range 0.0..1.0..5.0
       * @feedback Main Gain knob
       * @deprecated since  version 2.0.0: Use [Master],gain instead.
       */
      | "volume";

    type DeprecatedMicrophoneNControl =
      /**
       * (No description)
       *
       * @groups [MicrophoneN]
       * @since New in version 1.10.0.
       * @deprecated since  version 1.10.0: The control is not processed in the Mixer, which is also why there are no orientation controls for Microphones in the GUI.
       */
      "orientation" | DeprecatedAuxiliaryNMicrophoneNControl;

    type DeprecatedPlaylistControl =
      /**
       * Performs the same action action like [Library],GoToItem does when the tracks table has focus,
       * just regardless of the focus.
       *
       * @groups [Playlist]
       * @deprecated since  version 2.1.0: Use [Library],GoToItem instead.
       */
      | "LoadSelectedIntoFirstStopped"

      /**
       * Switches to the next view (Library, Queue, etc.)
       *
       * @groups [Playlist]
       * @deprecated since  version 2.1.0: Use [Library],MoveDown instead.
       */
      | "SelectNextPlaylist"

      /**
       * Scrolls to the next track in the track table.
       *
       * @groups [Playlist]
       * @deprecated since  version 2.1.0: Use [Library],MoveDown instead.
       */
      | "SelectNextTrack"

      /**
       * Switches to the previous view (Library, Queue, etc.)
       *
       * @groups [Playlist]
       * @deprecated since  version 2.1.0: Use [Library],MoveUp instead.
       */
      | "SelectPrevPlaylist"

      /**
       * Scrolls to the previous track in the track table.
       *
       * @groups [Playlist]
       * @deprecated since  version 2.1.0: Use [Library],MoveUp instead.
       */
      | "SelectPrevTrack"

      /**
       * Toggles (expands/collapses) the currently selected sidebar item.
       *
       * @groups [Playlist]
       * @since New in version 1.11.0.
       * @deprecated since  version 2.1.0: Use [Library],GoToItem instead.
       */
      | "ToggleSelectedSidebarItem";

    type DeprecatedSamplersControl =
      /**
       * (No description)
       *
       * @groups [Samplers]
       * @range binary
       * @feedback Shows Sampler bank(s)
       * @deprecated since  version 2.4.0: Use [Skin],show_samplers instead.
       */
      "show_samplers";

    type DeprecatedVinylControlControl =
      /**
       * Toggle the vinyl control section in skins.
       *
       * @groups [VinylControl]
       * @range binary
       * @feedback Vinyl controls are shown
       * @since New in version 1.10.0.
       * @deprecated since  version 2.4.0: Use [Skin],show_vinylcontrol instead.
       */
      "show_vinylcontrol";

    type DeprecatedQuickEffectRack1ChannelIControl = DeprecatedEffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl;

    type DeprecatedEffectRack1EffectUnitNControl = DeprecatedEffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl;

    type DeprecatedAuxiliaryNControl = DeprecatedAuxiliaryNMicrophoneNControl;

    type DeprecatedSamplerNControl = DeprecatedChannelNPreviewDeckNSamplerNControl;

    type DeprecatedPreviewDeckNControl = DeprecatedChannelNPreviewDeckNSamplerNControl;

    type DeprecatedEqualizerRack1ChannelIControl = DeprecatedEffectRack1EffectUnitNEqualizerRack1ChannelIQuickEffectRack1ChannelIControl;

    type DeprecatedChannelNControl = DeprecatedChannelNPreviewDeckNSamplerNControl;
  }

  export interface Config {
    strict?: false; // default = loose
  }

  // Internal helper types
  namespace Utils {
    type IsStrict = Config["strict"] extends true ? true : false;
    type Default = IsStrict extends true ? never : string & {};
    type MapGroup<TGroup, TMap> = 0 extends 1 & TGroup
      ? string // any check
      : TGroup extends keyof TMap
        ? TMap[TGroup] // found in map
        : Default; // fallback
  }
}
