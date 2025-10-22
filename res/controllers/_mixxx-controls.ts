// Mixxx control types
// Generated file, don't change anything by hand

declare namespace MixxxControls {
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
		 * The number of decks currently enabled.
		 *
		 * @groups [App]
		 * @range integer
		 */
		| "num_decks"

		/**
		 * The number of samplers currently enabled.
		 *
		 * @groups [App]
		 * @range integer
		 */
		| "num_samplers"

		/**
		 * The number of preview decks currently enabled.
		 *
		 * @groups [App]
		 * @range integer
		 */
		| "num_preview_decks"

		/**
		 * The number of microphone inputs that can be configured.
		 *
		 * @groups [App]
		 * @range integer
		 */
		| "num_microphones"

		/**
		 * The number of auxiliary inputs that can be configured.
		 *
		 * @groups [App]
		 * @range integer
		 */
		| "num_auxiliaries"

		/**
		 * The current output sample rate (default: 44100 Hz).
		 *
		 * @groups [App]
		 * @range absolute value (in Hz)
		 */
		| "samplerate";

	type MasterControl =
		/**
		 * Reflects fraction of latency, given by the audio buffer size, spend for audio processing inside Mixxx. At value near 25 % there is a high risk of buffer underflows
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range 0 .. 25 %
		 */
		| `audio_latency_usage${PotMeterSuffix}`

		/**
		 * Indicates a buffer under or over-flow. Resets after 500 ms
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| `audio_latency_overload${PotMeterSuffix}`

		/**
		 * Counts buffer over and under-flows. Max one per 500 ms
		 *
		 * @groups [Master]
		 * @range 0 .. n
		 */
		| "audio_latency_overload_count"

		/**
		 * Adjusts the left/right channel balance on the Main output.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range -1.0..1.0
		 */
		| `balance${PotMeterSuffix}`

		/**
		 * Indicates whether a Booth output is configured in the Sound Hardware Preferences.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| "booth_enabled"

		/**
		 * Adjusts the gain of the Booth output.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range 0.0…1.0…5.0
		 */
		| `booth_gain${PotMeterSuffix}`

		/**
		 * Adjusts the crossfader between players/decks (-1.0 is all the way left).
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range -1.0..1.0
		 */
		| `crossfader${PotMeterSuffix}`

		/**
		 * Moves the crossfader left by 1/10th.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| "crossfader_down"

		/**
		 * Moves the crossfader left by 1/100th.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| "crossfader_down_small"

		/**
		 * Moves the crossfader right by 1/10th.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| "crossfader_up"

		/**
		 * Moves the crossfader right by 1/100th.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| "crossfader_up_small"

		/**
		 * Microphone ducking strength
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range 0.0..1.0
		 */
		| `duckStrength${PotMeterSuffix}`

		/**
		 * Indicator that the main mix is processed.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| "enabled"

		/**
		 * Adjusts the gain for the main output as well as recording and broadcasting signal.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range 0.0..1.0..5.0
		 */
		| `gain${PotMeterSuffix}`

		/**
		 * Indicator that the headphone mix is processed.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| "headEnabled"

		/**
		 * Adjusts the headphone output gain.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range 0.0..1.0..5.0
		 */
		| `headGain${PotMeterSuffix}`

		/**
		 * Adjusts the cue/main mix in the headphone output.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range default
		 */
		| `headMix${PotMeterSuffix}`

		/**
		 * Splits headphone stereo cueing into right (main mono) and left (PFL mono).
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| "headSplit"

		/**
		 * Latency setting (sound buffer size) in milliseconds (default 64).
		 *
		 * @groups [Master]
		 * @range >=0 (absolute value)
		 */
		| "latency"

		/**
		 * Indicates when the signal is clipping (too loud for the hardware and is being distorted) (composite).
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| `PeakIndicator${PotMeterSuffix}`

		/**
		 * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the left channel.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| `PeakIndicator${number}${PotMeterSuffix}`

		/**
		 * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the right channel.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| `PeakIndicator${number}${PotMeterSuffix}`

		/**
		 * Toggle microphone ducking mode (OFF, AUTO, MANUAL)
		 *
		 * @groups [Master]
		 * @range FIXME
		 */
		| "talkoverDucking"

		/**
		 * Outputs the current instantaneous main volume (composite).
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range default
		 */
		| `VuMeter${PotMeterSuffix}`

		/**
		 * Outputs the current instantaneous main volume for the left channel.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range default
		 */
		| `VuMeter${number}${PotMeterSuffix}`

		/**
		 * Outputs the current instantaneous main volume for the right channel.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [Master]
		 * @range default
		 */
		| `VuMeter${number}${PotMeterSuffix}`

		/**
		 * The number of decks currently enabled.
		 *
		 * @groups [Master]
		 * @range integer
		 */
		| "num_decks"

		/**
		 * The number of samplers currently enabled.
		 *
		 * @groups [Master]
		 * @range integer
		 */
		| "num_samplers"

		/**
		 * The number of preview decks currently enabled.
		 *
		 * @groups [Master]
		 * @range integer
		 */
		| "num_preview_decks"

		/**
		 * The number of microphone inputs that can be configured.
		 *
		 * @groups [Master]
		 * @range integer
		 */
		| "num_microphones"

		/**
		 * The number of auxiliary inputs that can be configured.
		 *
		 * @groups [Master]
		 * @range integer
		 */
		| "num_auxiliaries"

		/**
		 * Adjust headphone volume.
		 *
		 * @groups [Master]
		 * @range 0.0..1.0..5.0
		 */
		| "headVolume"

		/**
		 * Adjust main volume.
		 *
		 * @groups [Master]
		 * @range 0.0..1.0..5.0
		 */
		| "volume"

		/**
		 * Toggle maximized view of library.
		 *
		 * @groups [Master]
		 * @range binary
		 */
		| "maximize_library";

	type PreviewDeckNSamplerNChannelNControl =
		/**
		 * Fast rewind (REW)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "back"

		/**
		 * Toggle the beatgrid/BPM lock state.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "bpmlock"

		/**
		 * Its value is set to the sample position of the closest beat of the active beat and is used for updating the beat LEDs.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -1, 0.0, real-valued
		 */
		| "beat_closest"

		/**
		 * Outputs the relative position of the play marker in the section between the the previous and next beat marker.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range 0.0 - 1.0, real-valued
		 */
		| "beat_distance"

		/**
		 * Jump forward (positive) or backward (negative) by N beats. If a loop is active, the loop is moved by X beats.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range any real number within the range, see [ChannelN],beatloop_X_activate
		 */
		| "beatjump"

		/**
		 * Set the number of beats to jump with beatjump_forward
		 * /beatjump_backward.
		 * If the loaded track has no beat grid, this value is treated as seconds instead of beats.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive real number
		 */
		| "beatjump_size"

		/**
		 * Halve the value of beatjump_size.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beatjump_size_halve"

		/**
		 * Double the value of beatjump_size.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beatjump_size_double"

		/**
		 * Jump forward by beatjump_size.
		 * If a loop is active, the loop is moved forward by X beats.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beatjump_forward"

		/**
		 * Jump backward by beatjump_size.
		 * If a loop is active, the loop is moved backward by X beats.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beatjump_backward"

		/**
		 * Jump forward by X beats. A control exists for
		 * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512.
		 * If a loop is active, the loop is moved forward by X beats.
		 * If the loaded track has no beat grid, this value is treated as seconds instead of beats.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `beatjump_${number}_forward`

		/**
		 * Jump backward by X beats. A control exists for
		 * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512.
		 * If a loop is active, the loop is moved backward by X beats.
		 * If the loaded track has no beat grid, this value is treated as seconds instead of beats.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `beatjump_${number}_backward`

		/**
		 * Set a loop that is beatloop_size beats long and enables the loop.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * Depending on the state of loop_anchor the loop is created forwards
		 * or backwards from the current position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beatloop_activate"

		/**
		 * Activates a loop over X beats. A control exists for
		 * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * Depending on the state of loop_anchor the loop is created forwards
		 * or backwards from the current position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `beatloop_${number}_activate`

		/**
		 * Activates a loop over X beats backwards from the current position. A control exists for
		 * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
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
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive real number
		 */
		| "beatloop_size"

		/**
		 * Toggles a loop over X beats. A control exists for
		 * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * Depending on the state of loop_anchor the loop is created forwards
		 * or backwards from the current position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `beatloop_${number}_toggle`

		/**
		 * 1 if beatloop X is enabled, 0 if not.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `beatloop_${number}_enabled`

		/**
		 * Activates a rolling loop over beatloop_size beats.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * Once disabled, playback will resume where the track would have been if it had not entered the loop.
		 * Depending on the state of loop_anchor, the loop is created forwards
		 * or backwards from the current position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beatlooproll_activate"

		/**
		 * Activates a rolling loop over X beats. Once disabled, playback will resume where the
		 * track would have been if it had not entered the loop. A control exists for
		 * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * Depending on the state of loop_anchor, the loop is created forwards
		 * or backwards from the current position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `beatlooproll_${number}_activate`

		/**
		 * Activates a rolling loop over X beats backwards from the current position.
		 * Once disabled, playback will resume where the track would have been if it had
		 * not entered the loop. A control exists for
		 * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `beatlooproll_r${number}_activate`

		/**
		 * Adjust the average BPM up by +0.01
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beats_adjust_faster"

		/**
		 * Adjust the average BPM down by -0.01.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beats_adjust_slower"

		/**
		 * Adjust beatgrid so closest beat is aligned with the current playposition.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beats_translate_curpos"

		/**
		 * Adjust beatgrid to match another playing deck.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beats_translate_match_alignment"

		/**
		 * Move beatgrid to an earlier position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beats_translate_earlier"

		/**
		 * Move beatgrid to a later position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beats_translate_later"

		/**
		 * (No description)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "shift_cues_earlier"

		/**
		 * (No description)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "shift_cues_later"

		/**
		 * (No description)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "shift_cues_earlier_small"

		/**
		 * (No description)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "shift_cues_later_small"

		/**
		 * Restores the beatgrid state before the last beatgrid adjustment done with the above beats_ controls.
		 * The undo stack holds up to ten beatgrid states. For changes done in quick succession
		 * (less than 800 milliseconds between actions), e.g. repeated beats_translate_earlier, only the first state is stored.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beats_undo_adjustment"

		/**
		 * Syncs the tempo and phase (depending on quantize) to that of the other track (if BPM is detected on both).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| "beatsync"

		/**
		 * Syncs the phase to that of the other track (if BPM is detected on both).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beatsync_phase"

		/**
		 * Syncs the tempo to that of the other track (if BPM is detected on both).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "beatsync_tempo"

		/**
		 * Reflects the perceived (rate-adjusted) BPM of the loaded file.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range real-valued
		 */
		| `bpm${PotMeterSuffix}`

		/**
		 * When tapped repeatedly, adjusts the BPM of the track on the deck (not the tempo slider!) to match the taps.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "bpm_tap"

		/**
		 * When tapped repeatedly, adjusts the rate/tempo of the deck to match the taps.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "tempo_tap"

		/**
		 * Clone the given deck number, copying the play state, position, rate, and key. If 0 or a negative number is given, Mixxx will attempt to select the first playing deck as the source for the clone.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range integer between 1 and [Master],num_decks (inclusive)
		 */
		| "CloneFromDeck"

		/**
		 * Clone the given sampler number, copying the play state, position, rate, and key.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range integer between 1 and [App],num_samplers (inclusive)
		 */
		| "CloneFromSampler"

		/**
		 * Load the track currently loaded to the given deck number.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range integer between 1 and [App],num_decks (inclusive)
		 */
		| "LoadTrackFromDeck"

		/**
		 * Load the track currently loaded to the given sampler number.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range integer between 1 and [App],num_samplers (inclusive)
		 */
		| "LoadTrackFromSampler"

		/**
		 * Represents a Cue button that is always in CDJ mode.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_cdj"

		/**
		 * Deletes the already set cue point and sets [ChannelN],cue_point to -1.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_clear"

		/**
		 * If the cue point is set, recalls the cue point.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_goto"

		/**
		 * In CDJ mode, when playing, returns to the cue point and pauses. If stopped, sets a cue point at the current location. If stopped and at a cue point, plays from that point until released (set to 0.)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_default"

		/**
		 * If the cue point is set, seeks the player to it and starts playback.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_gotoandplay"

		/**
		 * If the cue point is set, seeks the player to it and stops.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_gotoandstop"

		/**
		 * Indicates the blinking pattern of the CUE button (i.e. 1.0 if the button is illuminated, 0.0 otherwise), depending on the chosen cue mode.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_indicator"

		/**
 * Represents the currently chosen cue mode.
 * 
 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
 * @range 



Value

compatible hardware

0.0

Mixxx mode (default)

1.0

Pioneer mode

2.0

Denon mode

3.0

Numark mode

4.0

Mixxx mode (no blinking)

5.0

CUP (Cue + Play) mode
 */
		| "cue_mode"

		/**
		 * Go to cue point and play after release (CUP button behavior). If stopped, sets a cue point at the current location.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_play"

		/**
		 * The current position of the cue point in samples.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range absolute value
		 */
		| "cue_point"

		/**
		 * Plays from the current cue point.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_preview"

		/**
		 * Sets a cue point at the current location.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_set"

		/**
		 * If the player is not playing, set the cue point at the current location otherwise seek to the cue point.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "cue_simple"

		/**
		 * Outputs the length of the current song in seconds
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range absolute value
		 */
		| "duration"

		/**
		 * Eject currently loaded track. If no track is loaded the last-ejected track
		 * (of any deck) is reloaded.
		 * Double-press to reload the last replaced track. If no track is loaded the second-last ejected track is reloaded.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "eject"

		/**
		 * Jump to end of track
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "end"

		/**
		 * Fast forward (FF)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
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
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `hotcue_${number}_activate`

		/**
		 * Identical to hotcue_X_activate, but this always sets a regular cue point, regardless of whether a loop is enabled or not.
		 * This control can be used for controllers that have dedicated hotcue/saved loop pad modes.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| `hotcue_${number}_activatecue`

		/**
		 * Identical to hotcue_X_activate, but this always sets a saved loop, regardless of whether a loop is enabled or not.
		 * If no loop is available, this sets and enables a beat loop of of beatloop_size.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * This control can be used for controllers that have dedicated hotcue/saved loop pad modes.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| `hotcue_${number}_activateloop`

		/**
		 * Enables or disables a loop from the position of hotcue X.
		 * If X is a saved loop, that loop will be used, otherwise it will set a beatloop of beatloop_size from the cue position.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * In case the hotcue is not set, this control will set a regular cue point at the current position and start a beatloop.
		 * This control can be used to map the primary action of the “Cue Loop” performance pad mode on Serato-style controllers.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| `hotcue_${number}_cueloop`

		/**
		 * If hotcue X is set, clears its hotcue status.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `hotcue_${number}_clear`

		/**
		 * Color of hotcue X or -1 if the hotcue is not set.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range 3-Byte RGB color code (or -1)
		 */
		| `hotcue_${number}_color`

		/**
		 * Indicates if hotcue slot X is set, active or empty.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| `hotcue_${number}_status`

		/**
		 * Indicates the type of the hotcue in hotcue slot X.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| `hotcue_${number}_type`

		/**
		 * If hotcue X is set, seeks the player to hotcue X’s position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `hotcue_${number}_goto`

		/**
		 * If hotcue X is set, seeks the player to hotcue X’s position and starts playback.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `hotcue_${number}_gotoandplay`

		/**
		 * If hotcue X is set, seeks the player to hotcue X’s position, starts playback and looping.
		 * If the hotcue is a saved loop, the loop is enabled, otherwise a beatloop of beatloop_size is set from the hotcue’s position.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * This control can be used to map the secondary action of the “Cue Loop” performance pad mode on Serato-style controllers.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `hotcue_${number}_gotoandloop`

		/**
		 * If hotcue X is set, seeks the player to hotcue X’s position and stops.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `hotcue_${number}_gotoandstop`

		/**
		 * The position of hotcue X in samples, -1 if not set.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive integer
		 */
		| `hotcue_${number}_position`

		/**
		 * Set a hotcue at the current play position and saves it as hotcue X of type “Hotcue”.
		 * In case a loop is currently enabled (i.e. if [ChannelN],loop_enabled is set to 1), the loop will be saved as hotcue X instead and hotcue_X_type will be set to “Loop”.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `hotcue_${number}_set`

		/**
		 * Identical to hotcue_X_set, but this always sets a regular cue point (i.e. hotcue_X_type “Hotcue”), regardless of whether a loop is enabled or not.
		 * This control can be used for controllers that have dedicated hotcue/saved loop pad modes.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| `hotcue_${number}_setcue`

		/**
		 * Identical to hotcue_X_set, but this always saves a loop (i.e. hotcue_X_type “Loop”), regardless of whether a loop is enabled or not.
		 * If no loop is available, this sets and enables a beat loop of of beatloop_size.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * This control can be used for controllers that have dedicated hotcue/saved loop pad modes.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| `hotcue_${number}_setloop`

		/**
		 * Contains the number of the most recently used hotcue (or -1 if no hotcue was used).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive integer (or -1)
		 */
		| "hotcue_focus"

		/**
		 * If there is a focused hotcue, sets its color to the previous color in the palette.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "hotcue_focus_color_prev"

		/**
		 * If there is a focused hotcue, sets its color to the next color in the palette.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "hotcue_focus_color_next"

		/**
		 * If the intro end cue is set, seeks the player to the intro end position. If the intro end is not set, sets the intro end to the current play position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "intro_end_activate"

		/**
		 * If the intro end cue is set, clears its status.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "intro_end_clear"

		/**
		 * The position of the intro end in samples, -1 if not set.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive integer
		 */
		| "intro_end_position"

		/**
		 * Set intro end to the current play position. If intro end was previously set, it is moved to the new position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "intro_end_set"

		/**
		 * If the intro start cue is set, seeks the player to the intro start position. If the intro start is not set, sets the intro start to the current play position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "intro_start_activate"

		/**
		 * If the intro start cue is set, clears its status.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "intro_start_clear"

		/**
		 * The position of the intro start in samples, -1 if not set.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive integer
		 */
		| "intro_start_position"

		/**
		 * Set intro start to the current play position. If intro start was previously set, it is moved to the new position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "intro_start_set"

		/**
 * Current key of the track
 * 
 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
 * @range 







Value

OpenKey

Lancelot

Traditional

1

1d

8b

C

2

8d

3b

D♭

3

3d

10b

D

4

10d

5b

E♭

5

5d

12b

E

6

12d

7b

F

7

7d

2b

F♯/G♭

8

2d

9b

G

9

9d

4b

A♭

10

4d

11b

A

11

11d

6b

B♭

12

6d

1b

B

13

10m

5a

Cm

14

5m

12a

C♯m

15

12m

7a

Dm

16

7m

2a

D♯m/E♭m

17

2m

9a

Em

18

9m

4a

Fm

19

4m

11a

F♯m

20

11m

6a

Gm

21

6m

1a

G♯m

22

1m

8a

Am

23

8m

3a

B♭m

24

3m

10a

Bm
 */
		| "key"

		/**
		 * Enable key-lock for the specified deck (rate changes only affect tempo, not key)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "keylock"

		/**
		 * Loads the currently highlighted track into the deck
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "LoadSelectedTrack"

		/**
		 * Loads the currently highlighted track into the deck and starts playing.
		 * If the player is a preview deck and the selected track is already loaded, toggle play/pause.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "LoadSelectedTrackAndPlay"

		/**
		 * Reflects the average bpm around the current play position of the loaded file.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive value
		 */
		| "local_bpm"

		/**
 * Adjusts whether loops created with [ChannelN],beatloop_X_activate,
 * [ChannelN],beatloop_X_toggle or [ChannelN],beatloop_rX_activate
 * span forward or backward from the current play position.
 * 
 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
 * @range 



Value

Direction

0

forward

1

backward
 */
		| "loop_anchor"

		/**
		 * Doubles beatloop_size. If beatloop_size equals the size of the loop, the loop is resized.
		 * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "loop_double"

		/**
		 * Indicates whether or not a loop is enabled.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "loop_enabled"

		/**
		 * Clears the last active loop, i.e. deactivates and removes loop, detaches loop_in,
		 * loop_out, reloop_toggle and related
		 * controls. It does not affect saved loops.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "loop_remove"

		/**
		 * The player loop-out position in samples, -1 if not set.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive integer
		 */
		| "loop_end_position"

		/**
		 * Halves beatloop_size. If beatloop_size equals the size of the loop, the loop is resized.
		 * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "loop_halve"

		/**
		 * If loop is disabled, sets the player loop in position to the current play position. If loop is enabled, press and hold to move loop in position to the current play position. If quantize is enabled, beatloop_size will be updated to reflect the new loop size.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "loop_in"

		/**
		 * Seek to the loop in point.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "loop_in_goto"

		/**
		 * If loop is disabled, sets the player loop out position to the current play position. If loop is enabled, press and hold to move loop out position to the current play position. If quantize is enabled, beatloop_size will be updated to reflect the new loop size.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "loop_out"

		/**
		 * Seek to the loop out point.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "loop_out_goto"

		/**
		 * Move loop forward by X beats (positive) or backward by X beats (negative).
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range real number
		 */
		| "loop_move"

		/**
		 * Moves the loop in and out points forward by X beats. A control exists for
		 * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `loop_move_${number}_forward`

		/**
		 * Loop moves by X beats. A control exists for
		 * X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `loop_move_${number}_backward`

		/**
		 * Scale the loop length by the value scale is set to by moving the end marker.
		 * beatloop_size is not updated to reflect the change.
		 * If a saved loop is currently enabled, the modification is saved to the hotcue slot immediately.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range 0.0 - infinity
		 */
		| "loop_scale"

		/**
		 * The player loop-in position in samples, -1 if not set.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive integer
		 */
		| "loop_start_position"

		/**
 * Set channel orientation for the crossfader.
 * 
 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
 * @range 



Value

Meaning

0

Left side of crossfader

1

Center (not affected by crossfader)

2

Right side of crossfader
 */
		| "orientation"

		/**
		 * If the outro end cue is set, seeks the player to the outro end position. If the outro end is not set, sets the outro end to the current play position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "outro_end_activate"

		/**
		 * If the outro end cue is set, clears its status.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "outro_end_clear"

		/**
		 * The position of the outro end in samples, -1 if not set.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive integer
		 */
		| "outro_end_position"

		/**
		 * Set outro end to the current play position. If outro end was previously set, it is moved to the new position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "outro_end_set"

		/**
		 * If the outro start cue is set, seeks the player to the outro start position. If the outro start is not set, sets the outro start to the current play position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "outro_start_activate"

		/**
		 * If the outro start cue is set, clears its status.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "outro_start_clear"

		/**
		 * The position of the outro start in samples, -1 if not set.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive integer
		 */
		| "outro_start_position"

		/**
		 * Set outro start to the current play position. If outro start was previously set, it is moved to the new position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "outro_start_set"

		/**
		 * Connects the vinyl control input for vinyl control on that deck to the channel output. Allows to mix external media into DJ sets.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "passthrough"

		/**
		 * Indicates when the signal is clipping (too loud for the hardware and is being distorted)
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `PeakIndicator${PotMeterSuffix}`

		/**
		 * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the left channel
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `PeakIndicator${number}${PotMeterSuffix}`

		/**
		 * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the right channel
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| `PeakIndicator${number}${PotMeterSuffix}`

		/**
		 * Toggles headphone cueing (PFL).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "pfl"

		/**
		 * The total adjustment to the track’s pitch, including changes from the rate slider if keylock is off as well as pitch_adjust.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -6.0..6.0 semitones
		 */
		| `pitch${PotMeterSuffix}`

		/**
		 * Changes the track pitch up one half step, independent of the tempo.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "pitch_up"

		/**
		 * Changes the track pitch down one half step, independent of the tempo.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "pitch_down"

		/**
		 * Adjusts the pitch in addition to the tempo slider pitch and keylock. It is reset after loading a new track.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -3.0..3.0 semitones
		 */
		| `pitch_adjust${PotMeterSuffix}`

		/**
		 * Toggles playing or pausing the track.
		 * The value is set to 1 when the track is playing or when previewing from cue points and when the play command is adopted and track will be played after loading.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "play"

		/**
		 * A play button without pause. Pushing while playing, starts play at cue point again (Stutter).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "play_stutter"

		/**
		 * Sets the absolute position in the track.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -0.14 to 1.14 (0 = beginning -> Midi 14, 1 = end -> Midi 114)
		 */
		| `playposition${PotMeterSuffix}`

		/**
		 * Adjusts the pre-fader gain of the track (to avoid clipping)
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range 0.0..1.0..4.0
		 */
		| `pregain${PotMeterSuffix}`

		/**
		 * Aligns Hot-cues and Loop In & Out to the next beat from the current position.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "quantize"

		/**
		 * Speed control
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -1.0..1.0
		 */
		| `rate${PotMeterSuffix}`

		/**
		 * Indicates orientation of speed slider.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -1 or 1
		 */
		| "rate_dir"

		/**
		 * Sets the speed one step lower (4 % default) lower
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "rate_perm_down"

		/**
		 * Sets the speed one small step lower (1 % default)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "rate_perm_down_small"

		/**
		 * Sets the speed one step higher (4 % default)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "rate_perm_up"

		/**
		 * Sets the speed one small step higher (1 % default)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "rate_perm_up_small"

		/**
		 * Holds the speed one step lower while active
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "rate_temp_down"

		/**
		 * Holds the speed one small step lower while active
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "rate_temp_down_small"

		/**
		 * Holds the speed one step higher while active
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "rate_temp_up"

		/**
		 * Holds the speed one small step higher while active
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "rate_temp_up_small"

		/**
		 * Sets the range of the Speed slider (0.08 = 8%)
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range 0.0..4.0
		 */
		| `rateRange${PotMeterSuffix}`

		/**
		 * Seeks forward (positive values) or backward (negative values) at a speed determined by the value
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -300..300
		 */
		| `rateSearch${PotMeterSuffix}`

		/**
		 * Actual rate (used in visuals, not for control)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| "rateEngine"

		/**
		 * Activate current loop, jump to its loop in point, and stop playback.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "reloop_andstop"

		/**
		 * Toggles the current loop on or off. If the loop is ahead of the current play position, the track will keep playing normally until it reaches the loop.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "reloop_toggle"

		/**
		 * Enable repeat-mode for the specified deck
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "repeat"

		/**
		 * Resets the key to the original track key.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "reset_key"

		/**
		 * Toggles playing the track backwards
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "reverse"

		/**
		 * Enables reverse and slip mode while held (Censor)
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "reverseroll"

		/**
		 * Affects absolute play speed & direction whether currently playing or not when [ChannelN],scratch2_enable is active. (multiplicative). Use JavaScript engine.scratch functions to manipulate in controller mappings.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -3.0..3.0
		 */
		| "scratch2"

		/**
		 * Takes over play speed & direction for [ChannelN],scratch2.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "scratch2_enable"

		/**
		 * Toggles slip mode. When active, the playback continues muted in the background during a loop, scratch etc. Once disabled, the audible playback will resume where the track would have been.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "slip_enabled"

		/**
		 * Increase the rating of the currently loaded track (if the skin has star widgets in the decks section).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "stars_up"

		/**
		 * Decrease the rating of the currently loaded track (if the skin has star widgets in the decks section).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "stars_down"

		/**
		 * Jump to start of track
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "start"

		/**
		 * Start playback from the beginning of the deck.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "start_play"

		/**
		 * Seeks a player to the start and then stops it.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "start_stop"

		/**
		 * Stops a player.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "stop"

		/**
		 * Syncs the tempo and phase (depending on quantize) to that of the other track (if BPM is detected on both). Click and hold for at least one second activates sync lock on that deck.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "sync_enabled"

		/**
		 * Sets deck as leader clock.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "sync_leader"

		/**
 * (No description)
 * 
 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
 * @range 



Value

Meaning

0

Sync lock disabled for that deck

1

Deck is sync follower

2

Deck is sync leader
 */
		| "sync_mode"

		/**
		 * Match musical key.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| "sync_key"

		/**
		 * Color of the currently loaded track or -1 if no track is loaded or the track has no color.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range 3-Byte RGB color code (or -1)
		 */
		| "track_color"

		/**
		 * Adjusts the channel volume fader
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range default
		 */
		| `volume${PotMeterSuffix}`

		/**
		 * Mutes the channel
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "mute"

		/**
		 * Applies the deck pregain knob value to the detected ReplayGain value for the
		 * current track. This is a way to update the ReplayGain value of a track if it
		 * has been detected incorrectly. When this control is triggered, the pregain
		 * value for the deck will be centered so that there is no audible difference in
		 * track volume, so this operation is safe to use during performance, if the controller mapping uses soft-takeover for the pregain knob.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "update_replaygain_from_pregain"

		/**
		 * Toggles whether a deck is being controlled by digital vinyl.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "vinylcontrol_enabled"

		/**
 * Determines how cue points are treated in vinyl control relative mode.
 * 
 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
 * @range 



Value

Meaning

0

Cue points ignored

1

One Cue - If needle is dropped after the cue point, track will seek to that cue point

2

Hot Cue - Track will seek to nearest previous hotcue
 */
		| "vinylcontrol_cueing"

		/**
 * Determines how vinyl control interprets needle information.
 * 
 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
 * @range 



Value

Meaning

0

Absolute Mode (track position equals needle position and speed)

1

Relative Mode (track tempo equals needle speed regardless of needle position)

2

Constant Mode (track tempo equals last known-steady tempo regardless of needle input

See Control Mode for details.
 */
		| "vinylcontrol_mode"

		/**
		 * BPM to display in the GUI (updated more slowly than the actual BPM).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range ?
		 */
		| "visual_bpm"

		/**
		 * Current musical key after pitch shifting to display in the GUI using the notation selected in the preferences
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range ?
		 */
		| "visual_key"

		/**
		 * The distance to the nearest key measured in cents
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -0.5..0.5
		 */
		| `visual_key_distance${PotMeterSuffix}`

		/**
		 * Outputs the current instantaneous deck volume
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range default
		 */
		| `VuMeter${PotMeterSuffix}`

		/**
		 * Outputs the current instantaneous deck volume for the left channel
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range default
		 */
		| `VuMeter${number}${PotMeterSuffix}`

		/**
		 * Outputs the current instantaneous deck volume for the right channel
		 * This is a ControlPotMeter control.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range default
		 */
		| `VuMeter${number}${PotMeterSuffix}`

		/**
		 * Zooms the waveform to look ahead or back as needed.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range 1.0 - 10.0
		 */
		| "waveform_zoom"

		/**
		 * Waveform Zoom Out
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range ?
		 */
		| "waveform_zoom_up"

		/**
		 * Waveform Zoom In
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range ?
		 */
		| "waveform_zoom_down"

		/**
		 * Return to default waveform zoom level
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range ?
		 */
		| "waveform_zoom_set_default"

		/**
		 * Affects relative playback speed and direction persistently (additive offset & must manually be undone).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -3.0..3.0
		 */
		| "wheel"

		/**
		 * Indicates if hotcue slot X is set, active or empty.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 */
		| `hotcue_${number}_enabled`

		/**
		 * Sets deck as leader clock.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "sync_master"

		/**
		 * Setup a loop over the set number of beats.
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range positive real number
		 */
		| "beatloop"

		/**
		 * Toggles the current loop on or off. If the loop is ahead of the current play position, the track will keep playing normally until it reaches the loop.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "reloop_exit"

		/**
		 * Affects relative playback speed and direction for short instances (additive & is automatically reset to 0).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -3.0..3.0
		 */
		| "jog"

		/**
		 * Affects playback speed and direction (differently whether currently playing or not) (multiplicative).
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range -3.0..3.0
		 */
		| "scratch"

		/**
		 * Toggles the filter effect.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "filter"

		/**
		 * Adjusts the intensity of the filter effect.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range default
		 */
		| "filterDepth"

		/**
		 * Adjusts the gain of the low EQ filter.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range 0.0..1.0..4.0
		 */
		| "filterLow"

		/**
		 * Holds the gain of the low EQ to -inf while active
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "filterLowKill"

		/**
		 * Adjusts the gain of the mid EQ filter..
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range 0.0..1.0..4.0
		 */
		| "filterMid"

		/**
		 * Holds the gain of the mid EQ to -inf while active.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "filterMidKill"

		/**
		 * Adjusts the gain of the high EQ filter.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range 0.0..1.0..4.0
		 */
		| "filterHigh"

		/**
		 * Holds the gain of the high EQ to -inf while active.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range binary
		 */
		| "filterHighKill"

		/**
		 * Setup a loop over X beats. A control exists for X = 0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64
		 * If the loaded track has no beat grid, seconds are used instead of beats.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
		 * @range toggle
		 */
		| `beatloop_${number}`;

	type PreviewDeckNSamplerNChannelNAuxiliaryNControl =
		/**
		 * Assign channel to the center of the crossfader.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN], [AuxiliaryN]
		 */
		| "orientation_center"

		/**
		 * Assign channel to the left side of the crossfader.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN], [AuxiliaryN]
		 */
		| "orientation_left"

		/**
		 * Assign channel to the right side of the crossfader.
		 *
		 * @groups [PreviewDeckN], [SamplerN], [ChannelN], [AuxiliaryN]
		 */
		| "orientation_right";

	type ChannelNControl =
		/**
		 * Toggle the track context menu for the track currently loaded in this deck.
		 * The control value is 1 if there is already a menu shown for this deck.
		 * The menu can be navigated with the MoveUp/Down controls
		 * and selected actions or submenus can be activated with GoToItem.
		 *
		 * @groups [ChannelN]
		 * @range Binary
		 */
		| "show_track_menu"

		/**
		 * Toggles the flanger effect.
		 *
		 * @groups [ChannelN]
		 */
		| "flanger"

		/**
		 * (No description)
		 *
		 * @groups [ChannelN]
		 */
		| "Hercules1"

		/**
		 * (No description)
		 *
		 * @groups [ChannelN]
		 */
		| "NextTask"

		/**
		 * (No description)
		 *
		 * @groups [ChannelN]
		 */
		| "NextTrack"

		/**
		 * (No description)
		 *
		 * @groups [ChannelN]
		 */
		| "PrevTask"

		/**
		 * (No description)
		 *
		 * @groups [ChannelN]
		 */
		| "PrevTrack"

		/**
		 * (No description)
		 *
		 * @groups [ChannelN]
		 */
		| "transform"
		| PreviewDeckNSamplerNChannelNAuxiliaryNControl
		| PreviewDeckNSamplerNChannelNControl;

	type SamplerControl =
		/**
		 * Save sampler configuration. Make currently loaded tracks in samplers instantly available at a later point.
		 *
		 * @groups [Sampler]
		 * @range binary
		 */
		| "SaveSamplerBank"

		/**
		 * Load saved sampler configuration file and add tracks to the available samplers.
		 *
		 * @groups [Sampler]
		 * @range binary
		 */
		| "LoadSamplerBank";

	type MicrophoneNAuxiliaryNControl =
		/**
		 * Hold value at 1 to mix channel input into the main output.
		 * For [MicrophoneN] use [MicrophoneN],talkover instead.
		 * Note that [AuxiliaryN] also take [AuxiliaryN],orientation into account.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range binary
		 */
		| "main_mix"

		/**
		 * Indicates when the signal is clipping (too loud for the hardware and is being distorted)
		 * This is a ControlPotMeter control.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range binary
		 */
		| `PeakIndicator${PotMeterSuffix}`

		/**
		 * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the left channel
		 * This is a ControlPotMeter control.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range binary
		 */
		| `PeakIndicator${number}${PotMeterSuffix}`

		/**
		 * Indicates when the signal is clipping (too loud for the hardware and is being distorted) for the right channel
		 * This is a ControlPotMeter control.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range binary
		 */
		| `PeakIndicator${number}${PotMeterSuffix}`

		/**
		 * Toggles headphone cueing (PFL).
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range binary
		 */
		| "pfl"

		/**
		 * Hold value at 1 to mix channel input into the main output.
		 * For [AuxiliaryN] use [AuxiliaryN],main_mix instead.
		 * Note that [AuxiliaryN] also take [AuxiliaryN],orientation into account.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range binary
		 */
		| "talkover"

		/**
		 * Adjusts the channel volume fader
		 * This is a ControlPotMeter control.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range default
		 */
		| `volume${PotMeterSuffix}`

		/**
		 * Adjusts the gain of the input
		 * This is a ControlPotMeter control.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range 0.0..1.0..4.0
		 */
		| `pregain${PotMeterSuffix}`

		/**
		 * Mutes the channel
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range binary
		 */
		| "mute"

		/**
		 * Outputs the current instantaneous channel volume
		 * This is a ControlPotMeter control.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range default
		 */
		| `VuMeter${PotMeterSuffix}`

		/**
		 * Outputs the current instantaneous deck volume for the left channel
		 * This is a ControlPotMeter control.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range default
		 */
		| `VuMeter${number}${PotMeterSuffix}`

		/**
		 * Outputs the current instantaneous deck volume for the right channel
		 * This is a ControlPotMeter control.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range default
		 */
		| `VuMeter${number}${PotMeterSuffix}`

		/**
		 * 1 if a channel input is enabled, 0 if not.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range binary
		 */
		| "enabled"

		/**
		 * Hold value at 1 to mix channel input into the main output.
		 * For [MicrophoneN] use [MicrophoneN],talkover instead.
		 * Note that [AuxiliaryN] also take [AuxiliaryN],orientation into account.
		 *
		 * @groups [MicrophoneN], [AuxiliaryN]
		 * @range binary
		 */
		| "master";

	type AuxiliaryNControl =
		/**
 * Set channel orientation for the crossfader.
 * 
 * @groups [AuxiliaryN]
 * @range 



Value

Meaning

0

Left side of crossfader

1

Center (not affected by crossfader)

2

Right side of crossfader
 */
		"orientation" | PreviewDeckNSamplerNChannelNAuxiliaryNControl | MicrophoneNAuxiliaryNControl;

	type VinylControlControl =
		/**
		 * Moves control by a vinyl control signal from one deck to another if using the single deck vinyl control (VC) feature.
		 *
		 * @groups [VinylControl]
		 * @range binary
		 */
		| "Toggle"

		/**
		 * Allows to amplify the “phono” level of attached turntables to “line” level.
		 * This is equivalent to setting the turntable boost in Options ‣ Preferences ‣ Vinyl Control
		 *
		 * @groups [VinylControl]
		 * @range binary
		 */
		| "gain"

		/**
		 * Toggle the vinyl control section in skins.
		 *
		 * @groups [VinylControl]
		 * @range binary
		 */
		| "show_vinylcontrol";

	type RecordingControl =
		/**
		 * Turns recording on or off.
		 *
		 * @groups [Recording]
		 * @range binary
		 */
		| "toggle_recording"

		/**
 * Indicates whether Mixxx is currently recording.
 * 
 * @groups [Recording]
 * @range 



Value

Meaning

0

Recording Stopped

1

Initialize Recording

2

Recording Active
 */
		| "status";

	type AutoDJControl =
		/**
		 * Turns Auto DJ on or off.
		 *
		 * @groups [AutoDJ]
		 * @range binary
		 */
		| "enabled"

		/**
		 * Shuffles the content of the Auto DJ playlist.
		 *
		 * @groups [AutoDJ]
		 * @range binary
		 */
		| "shuffle_playlist"

		/**
		 * Skips the next track in the Auto DJ playlist.
		 *
		 * @groups [AutoDJ]
		 * @range binary
		 */
		| "skip_next"

		/**
		 * Triggers the transition to the next track.
		 *
		 * @groups [AutoDJ]
		 * @range binary
		 */
		| "fade_now"

		/**
		 * Adds a random track to the Auto DJ queue.
		 *
		 * @groups [AutoDJ]
		 * @range binary
		 */
		| "add_random_track";

	type LibraryControl =
		/**
		 * Equivalent to pressing the Up key on the keyboard
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "MoveUp"

		/**
		 * Equivalent to pressing the Down key on the keyboard
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "MoveDown"

		/**
		 * Move the specified number of locations up or down. Intended to be mapped to an encoder knob.
		 *
		 * @groups [Library]
		 * @range Relative (positive values move down, negative values move up)
		 */
		| "MoveVertical"

		/**
		 * Equivalent to pressing the PageUp key on the keyboard
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "ScrollUp"

		/**
		 * Equivalent to pressing the PageDown key on the keyboard
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "ScrollDown"

		/**
		 * Scroll the specified number of pages up or down. Intended to be mapped to an encoder knob.
		 *
		 * @groups [Library]
		 * @range Relative (positive values move down, negative values move up)
		 */
		| "ScrollVertical"

		/**
		 * Equivalent to pressing the Left key on the keyboard
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "MoveLeft"

		/**
		 * Equivalent to pressing the Right key on the keyboard
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "MoveRight"

		/**
		 * Move the specified number of locations left or right. Intended to be mapped to an encoder knob.
		 *
		 * @groups [Library]
		 * @range Relative (positive values move right, negative values move left)
		 */
		| "MoveHorizontal"

		/**
		 * Equivalent to pressing the Tab key on the keyboard
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "MoveFocusForward"

		/**
		 * Equivalent to pressing the Shift + Tab key on the keyboard
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "MoveFocusBackward"

		/**
		 * Move focus the specified number of panes forward or backwards. Intended to be mapped to an encoder knob.
		 *
		 * @groups [Library]
		 * @range Relative (positive values move forward, negative values move backward)
		 */
		| "MoveFocus"

		/**
 * Read this control to know which library widget is currently focused, or write in order to focus a specific library widget.
 * This control can be used in controller scripts to trigger context-specific actions. For example, if the
 * tracks table has focus, pressing a button loads the selected track to a specific deck, while the same
 * button would clear the search if the search bar is focused.
 * Note: This control is useful only if a Mixxx window has keyboard focus, otherwise it always returns 0.
 * 
 * @groups [Library]
 * @range 





Value

writeable

Widget

0



none

1

X

Search bar

2

X

Tree view

3

X

Tracks table or root views of library features

4



Context menu (menus of library widgets or other editable widgets, or main menu bar)

5



Dialog (any confirmation or error popup, preferences, track properties or cover art window)

6



Unknown (widgets that don’t fit into any of the above categories)
 */
		| "focused_widget"

		/**
		 * Triggers different actions, depending on which interface element currently has keyboard focus:
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "GoToItem"

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
		 */
		| "show_track_menu"

		/**
		 * Increase the size of the library font. If the row height is smaller than the font-size the larger of the two is used.
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "font_size_increment"

		/**
		 * Decrease the size of the library font
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "font_size_decrement"

		/**
		 * Increase or decrease the size of the library font
		 *
		 * @groups [Library]
		 * @range Relative
		 */
		| "font_size_knob"

		/**
 * Indicates the sorting column the track table
 * 
 * @groups [Library]
 * @range 











Value

Description

Library

Playlist

Crate

Browse

1

Artist

X

X

X

X

2

Title

X

X

X

X

3

Album

X

X

X

X

4

Albumartist

X

X

X

X

5

Year

X

X

X

X

6

Genre

X

X

X

X

7

Composer

X

X

X

X

8

Grouping

X

X

X

X

9

Tracknumber

X

X

X

X

10

Filetype

X

X

X

X

11

Native Location

X

X

X

X

12

Comment

X

X

X

X

13

Duration

X

X

X

X

14

Bitrate

X

X

X

X

15

BPM

X

X

X

X

16

ReplayGain

X

X

X

X

17

Datetime Added

X

X

X

X

18

Times Played

X

X

X

X

19

Rating

X

X

X

X

20

Key

X

X

X

X

21

Preview

X

X

X

X

22

Coverart

X

X

X



23

Position



X





24

Playlist ID



X





25

Location



X





26

Filename







X

27

File Modified Time







X

28

File Creation Time







X

29

Sample Rate









30

Track Color

X

X

X



31

Last Played

X

X

X


 */
		| "sort_column"

		/**
		 * Equivalent to clicking on column headers. A new value sets [Library],sort_column to that value and [Library],sort_order to 0, setting the same value again will toggle [Library],sort_order.
		 *
		 * @groups [Library]
		 * @range Same as for [Library],sort_column or value 0 for sorting according the current column with the cursor on it
		 */
		| "sort_column_toggle"

		/**
		 * Indicate the sort order of the track tables.
		 *
		 * @groups [Library]
		 * @range Binary (0 for ascending, 1 for descending)
		 */
		| "sort_order"

		/**
		 * Sort the column of the table cell that is currently focused, which is equivalent to
		 * setting [Library],sort_column_toggle to 0. Though unlike that, it can
		 * be mapped to pushbuttons directly.
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "sort_focused_column"

		/**
		 * Set color of selected track to previous color in palette.
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "track_color_prev"

		/**
		 * Set color of selected track to next color in palette.
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "track_color_next"

		/**
		 * Select the next saved search query. Wraps around at the last item to the empty search.
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "search_history_next"

		/**
		 * Select the previous saved search query. Wraps around at the top to the last item.
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "search_history_prev"

		/**
		 * Select another saved search query. < 0 goes up the list, > 0 goes down. Wraps around at the top and bottom.
		 *
		 * @groups [Library]
		 * @range -N / +N
		 */
		| "search_history_selector"

		/**
		 * Clear the search.
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "clear_search"

		/**
		 * Toggle the Cover Art in Library
		 *
		 * @groups [Library]
		 * @range Binary
		 */
		| "show_coverart"
		| PlaylistLibraryControl;

	type PlaylistLibraryControl =
		/**
		 * Add selected track(s) to Auto DJ Queue (bottom).
		 *
		 * @groups [Playlist], [Library]
		 * @range Binary
		 */
		| "AutoDjAddBottom"

		/**
		 * Add selected track(s) to Auto DJ Queue (top).
		 *
		 * @groups [Playlist], [Library]
		 * @range Binary
		 */
		| "AutoDjAddTop";

	type ShoutcastControl =
		/**
		 * Shows if live Internet broadcasting is enabled.
		 *
		 * @groups [Shoutcast]
		 * @range ?
		 */
		| "enabled"

		/**
		 * This control displays whether broadcasting connection to Shoutcast server was successfully established.
		 *
		 * @groups [Shoutcast]
		 * @range binary
		 */
		| "status";

	type PlaylistControl =
		/**
		 * Scrolls the given number of items (view, playlist, crate, etc.) in the side pane (can be negative for reverse direction).
		 *
		 * @groups [Playlist]
		 * @range relative value
		 */
		| "SelectPlaylist"

		/**
		 * Scrolls the given number of tracks in the track table (can be negative for reverse direction).
		 *
		 * @groups [Playlist]
		 * @range relative value
		 */
		| "SelectTrackKnob"

		/**
		 * Performs the same action action like [Library],GoToItem does when the tracks table has focus,
		 * just regardless of the focus.
		 *
		 * @groups [Playlist]
		 */
		| "LoadSelectedIntoFirstStopped"

		/**
		 * Switches to the next view (Library, Queue, etc.)
		 *
		 * @groups [Playlist]
		 */
		| "SelectNextPlaylist"

		/**
		 * Switches to the previous view (Library, Queue, etc.)
		 *
		 * @groups [Playlist]
		 */
		| "SelectPrevPlaylist"

		/**
		 * Toggles (expands/collapses) the currently selected sidebar item.
		 *
		 * @groups [Playlist]
		 */
		| "ToggleSelectedSidebarItem"

		/**
		 * Scrolls to the next track in the track table.
		 *
		 * @groups [Playlist]
		 */
		| "SelectNextTrack"

		/**
		 * Scrolls to the previous track in the track table.
		 *
		 * @groups [Playlist]
		 */
		| "SelectPrevTrack"
		| PlaylistLibraryControl;

	type ControlsControl =
		/**
		 * Once enabled, all touch tab events are interpreted as right click. This control has been added to provide touchscreen compatibility in 2.0 and might be replaced by a general modifier solution in the future.
		 *
		 * @groups [Controls]
		 * @range binary
		 */
		| "touch_shift"

		/**
		 * If enabled, colors will be assigned to newly created hotcues automatically.
		 *
		 * @groups [Controls]
		 * @range binary
		 */
		| "AutoHotcueColors"

		/**
 * Represents the current state of the remaining time duration display of the loaded track.
 * 
 * @groups [Controls]
 * @range 



Value

Meaning

0

currently showing elapsed time, sets to remaining time

1

currently showing remaining time , sets to elapsed time

2

currently showing both (that means we are showing remaining, set to elapsed
 */
		| "ShowDurationRemaining";

	type EffectRack1QuickEffectRack1EqualizerRack1Control =
		/**
		 * Clear the Effect Rack
		 *
		 * @groups [EffectRack1], [QuickEffectRack1], [EqualizerRack1]
		 */
		"clear";

	type QuickEffectRack1ChannelIEffectRack1EffectUnitNEqualizerRack1ChannelIControl =
		/**
		 * Select EffectChain preset. > 0 goes one forward; < 0 goes one backward.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range +1/-1
		 */
		| "chain_preset_selector"

		/**
		 * Clear the currently loaded EffectChain in this EffectUnit.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range binary
		 */
		| "clear"

		/**
		 * If true, the EffectChain in this EffectUnit will be processed. Meant to allow the user a quick toggle for the effect unit.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range binary, default true
		 */
		| "enabled"

		/**
		 * 0 indicates no effect is focused; > 0 indicates the index of the focused effect. Focusing an effect only does something if a controller mapping changes how it behaves when an effect is focused.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range 0..num_effectslots
		 */
		| "focused_effect"

		/**
		 * Whether or not this EffectChain applies to Deck I
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range binary
		 */
		| `group_[Channel${number}]_enable`

		/**
		 * 0-based index of the currently loaded EffectChain preset. 0 is the empty/passthrough
		 * preset, -1 indicates an unsaved preset (default state of [EffectRack1_EffectUnitN]).
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range integer, -1 .. [num_chain_presets - 1]
		 */
		| "loaded_chain_preset"

		/**
		 * The dry/wet mixing ratio for this EffectChain with the EngineChannels it is mixed with
		 * This is a ControlPotMeter control.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range 0.0..1.0
		 */
		| `mix${PotMeterSuffix}`

		/**
		 * Cycle to the next EffectChain preset after the currently loaded preset.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range binary
		 */
		| "next_chain_preset"

		/**
		 * Cycle to the previous EffectChain preset before the currently loaded preset.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range binary
		 */
		| "prev_chain_preset"

		/**
		 * Whether to show focus buttons and draw a border around the focused effect in skins. This should not be manipulated by skins; it should only be changed by controller mappings.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range binary
		 */
		| "show_focus"

		/**
		 * Whether to show all the parameters of each effect in skins or only show metaknobs.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range binary
		 */
		| "show_parameters"

		/**
		 * The EffectChain superknob. Moves the metaknobs for each effect in the chain.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 * @range 0.0..1.0
		 */
		| `super1${PotMeterSuffix}`

		/**
		 * Cycle to the next EffectChain preset after the currently loaded preset.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 */
		| "next_chain"

		/**
		 * Cycle to the next EffectChain preset after the currently loaded preset.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 */
		| "prev_chain"

		/**
		 * Select EffectChain preset. > 0 goes one forward; < 0 goes one backward.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
		 */
		| "chain_selector";

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
		| QuickEffectRack1ChannelIEffectRack1EffectUnitNEqualizerRack1ChannelIControl;

	type QuickEffectRack1ChannelIEffect1EqualizerRack1ChannelIEffect1EffectRack1EffectUnitNEffectMControl =
		/**
		 * Clear the currently loaded Effect in this Effect slot from the EffectUnit.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range binary
		 */
		| "clear"

		/**
		 * Select Effect – >0 goes one forward, <0 goes one backward.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range +1/-1
		 */
		| "effect_selector"

		/**
		 * If true, the effect in this slot will be processed. Meant to allow the user a quick toggle for this effect.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range binary, default true
		 */
		| "enabled"

		/**
		 * 0-based index of the currently loaded effect preset, including the
		 * empty/passthrough preset “---”.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range integer, 0 .. [num_effectsavailable - 1]
		 */
		| "loaded_effect"

		/**
		 * Cycle to the next effect after the currently loaded effect.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range binary
		 */
		| "next_effect"

		/**
		 * Controls the parameters that are linked to the metaknob.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range 0..1
		 */
		| `meta${PotMeterSuffix}`

		/**
		 * Cycle to the previous effect before the currently loaded effect.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range binary
		 */
		| "prev_effect"

		/**
		 * The scaled value of the Kth parameter.
		 * See the Parameter Values section for more information.
		 * This is a ControlPotMeter control.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range double
		 */
		| `parameter${number}${PotMeterSuffix}`

		/**
		 * The link direction of the Kth parameter to the effect’s metaknob.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range bool
		 */
		| `parameter${number}_link_inverse`

		/**
		 * The link type of the Kth parameter to the effects’s metaknob.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range enum
		 */
		| `parameter${number}_link_type`

		/**
		 * The value of the Kth parameter. See the Parameter Values section for more information.
		 *
		 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
		 * @range double
		 */
		| `button_parameter${number}`;

	type SkinControl =
		/**
		 * Toggle the display of the effect rack in the user interface.
		 *
		 * @groups [Skin]
		 * @range binary
		 */
		| "show_effectrack"

		/**
		 * Toggle the display of cover art in the library section of the user interface.
		 *
		 * @groups [Skin]
		 * @range binary
		 */
		| "show_library_coverart"

		/**
		 * Toggle maximized view of library section of the user interface.
		 *
		 * @groups [Skin]
		 * @range binary
		 */
		| "show_maximized_library"

		/**
		 * Toggle the display of sampler banks in the user interface.
		 *
		 * @groups [Skin]
		 * @range binary
		 */
		| "show_samplers"

		/**
		 * Toggle the vinyl control section in the user interface.
		 *
		 * @groups [Skin]
		 * @range binary
		 */
		| "show_vinylcontrol";

	type SamplersControl =
		/**
		 * (No description)
		 *
		 * @groups [Samplers]
		 * @range binary
		 */
		"show_samplers";

	type EffectRack1Control =
		/**
		 * Show the Effect Rack
		 *
		 * @groups [EffectRack1]
		 * @range binary
		 */
		"show" | EffectRack1QuickEffectRack1EqualizerRack1Control;

	type MicrophoneNControl =
		/**
		 * (No description)
		 *
		 * @groups [MicrophoneN]
		 */
		"orientation" | MicrophoneNAuxiliaryNControl;

	type FlangerControl =
		/**
		 * Adjusts the intensity of the flange effect
		 *
		 * @groups [Flanger]
		 */
		| "lfoDepth"

		/**
		 * Adjusts the phase delay of the flange effect in microseconds
		 *
		 * @groups [Flanger]
		 */
		| "lfoDelay"

		/**
		 * Adjusts the wavelength of the flange effect in microseconds
		 *
		 * @groups [Flanger]
		 */
		| "lfoPeriod";

	type PreviewDeckNControl = PreviewDeckNSamplerNChannelNAuxiliaryNControl | PreviewDeckNSamplerNChannelNControl;

	type EqualizerRack1ChannelIEffect1Control =
		QuickEffectRack1ChannelIEffect1EqualizerRack1ChannelIEffect1EffectRack1EffectUnitNEffectMControl;

	type EqualizerRack1Control = EffectRack1QuickEffectRack1EqualizerRack1Control;

	type QuickEffectRack1ChannelIEffect1Control =
		QuickEffectRack1ChannelIEffect1EqualizerRack1ChannelIEffect1EffectRack1EffectUnitNEffectMControl;

	type SamplerNControl = PreviewDeckNSamplerNChannelNAuxiliaryNControl | PreviewDeckNSamplerNChannelNControl;

	type QuickEffectRack1Control = EffectRack1QuickEffectRack1EqualizerRack1Control;

	type QuickEffectRack1ChannelIControl = QuickEffectRack1ChannelIEffectRack1EffectUnitNEqualizerRack1ChannelIControl;

	type EffectRack1EffectUnitNEffectMControl =
		QuickEffectRack1ChannelIEffect1EqualizerRack1ChannelIEffect1EffectRack1EffectUnitNEffectMControl;

	type EqualizerRack1ChannelIControl = QuickEffectRack1ChannelIEffectRack1EffectUnitNEqualizerRack1ChannelIControl;

	namespace ReadOnly {
		type ReadOnlyAppControl =
			/**
			 * A throttled timer that provides the time elapsed in seconds since Mixxx was started.
			 * This control is updated at a rate of 20 Hz (every 50 milliseconds). It is the preferred timer for scripting animations in controller mappings (like VU meters or spinning animations) as it provides a smooth visual result without the performance overhead of [App],gui_tick_full_period_s.
			 * Only available when using the legacy GUI (not the QML interface).
			 *
			 * @groups [App]
			 * @range 0.0 .. n, read-only
			 * @readonly
			 */
			| "gui_tick_50ms_period_s"

			/**
			 * A high-resolution timer that provides the elapsed time in seconds since Mixxx was started.
			 * This control is updated on every GUI tick, which corresponds to the waveform rendering frame rate. It is suitable for very smooth, high-framerate animations in scripts. However, for most use cases like VU meters, consider using [App],gui_tick_50ms_period_s to improve performance by reducing the script execution rate.
			 * Only available when using the legacy GUI (not the QML interface).
			 *
			 * @groups [App]
			 * @range 0.0 .. n, read-only
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
			 * @range binary, read-only
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
			 * @range binary, read-only
			 * @readonly
			 */
			| "indicator_500ms";

		type ReadOnlyMasterControl =
			/**
			 * The number of available effects that can be selected in an effect slot.
			 *
			 * @groups [Master]
			 * @range integer, read-only
			 * @readonly
			 */
			| "num_effectsavailable"

			/**
			 * A throttled timer that provides the time elapsed in seconds since Mixxx was started.
			 *
			 * @groups [Master]
			 * @range 0.0 .. n, read-only
			 * @readonly
			 */
			| "guiTick50ms"

			/**
			 * A high-resolution timer that provides the elapsed time in seconds since Mixxx was started.
			 *
			 * @groups [Master]
			 * @range 0.0 .. n, read-only
			 * @readonly
			 */
			| "guiTickTime";

		type ReadOnlyPreviewDeckNSamplerNChannelNControl =
			/**
			 * Indicates, depending on the play direction, how the player is currently positioned to the closest beat.
			 * An LED controlled by beat_active can be used for beat matching or for finding a beat using jog or control vinyl.
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range real number, read-only
			 * @readonly
			 */
			| "beat_active"

			/**
			 * Switches to 1 if the play position is within the end range defined in Preferences ‣ Waveforms ‣ End of track warning.
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range binary, read-only
			 * @readonly
			 */
			| "end_of_track"

			/**
			 * The detected BPM of the loaded track.
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range positive value, read-only
			 * @readonly
			 */
			| "file_bpm"

			/**
			 * The detected key of the loaded track.
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range ?, read-only
			 * @readonly
			 */
			| "file_key"

			/**
			 * 1 if intro end cue is set, (position is not -1), 0 otherwise.
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range binary, read-only
			 * @readonly
			 */
			| "intro_end_enabled"

			/**
			 * 1 if intro start cue is set, (position is not -1), 0 otherwise.
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range binary, read-only
			 * @readonly
			 */
			| "intro_start_enabled"

			/**
			 * 1 if outro end cue is set, (position is not -1), 0 otherwise.
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range binary, read-only
			 * @readonly
			 */
			| "outro_end_enabled"

			/**
			 * 1 if outro start cue is set, (position is not -1), 0 otherwise.
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range binary, read-only
			 * @readonly
			 */
			| "outro_start_enabled"

			/**
			 * Provides information to be bound with the a Play/Pause button e.g blinking when play is possible
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range binary, read-only
			 * @readonly
			 */
			| "play_indicator"

			/**
			 * This is set to 1 when the track is playing, but not when previewing (see play).
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range binary, read-only
			 * @readonly
			 */
			| "play_latched"

			/**
			 * Whether a track is loaded in the specified deck
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range binary, read-only
			 * @readonly
			 */
			| "track_loaded"

			/**
			 * Sample rate of the track loaded on the specified deck
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range absolute value, read-only
			 * @readonly
			 */
			| "track_samplerate"

			/**
			 * Number of sound samples in the track loaded on the specified deck
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range absolute value, read-only
			 * @readonly
			 */
			| "track_samples"

			/**
			 * Provides visual feedback with regards to vinyl control status.
			 *
			 * @groups [PreviewDeckN], [SamplerN], [ChannelN]
			 * @range 0.0-3.0, read-only
			 * @readonly
			 */
			| "vinylcontrol_status";

		type ReadOnlyMicrophoneNChannelNAuxiliaryNControl =
			/**
			 * 1 if there is input is configured for this channel, 0 if not.
			 * In the case of [ChannelN] it corresponds to
			 * Vinyl Control. A configured input is required to enable [ChannelN],passthrough
			 *
			 * @groups [MicrophoneN], [ChannelN], [AuxiliaryN]
			 * @range binary, read-only
			 * @readonly
			 */
			"input_configured";

		type ReadOnlyEffectRack1QuickEffectRack1EqualizerRack1Control =
			/**
			 * The number of EffectUnits in this rack
			 *
			 * @groups [EffectRack1], [QuickEffectRack1], [EqualizerRack1]
			 * @range integer, read-only
			 * @readonly
			 */
			"num_effectunits";

		type ReadOnlyQuickEffectRack1ChannelIEffectRack1EffectUnitNEqualizerRack1ChannelIControl =
			/**
			 * Whether an EffectChain is loaded into the EffectUnit
			 *
			 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
			 * @range binary, read-only
			 * @readonly
			 */
			| "loaded"

			/**
			 * The number of effect chain presets available in this EffectUnit, including the
			 * empty/passthrough preset “---”.
			 *
			 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
			 * @range integer, read-only, >=1
			 * @readonly
			 */
			| "num_chain_presets"

			/**
			 * The number of effect slots available in this EffectUnit.
			 *
			 * @groups [QuickEffectRack1_[ChannelI]], [EffectRack1_EffectUnitN], [EqualizerRack1_[ChannelI]]
			 * @range integer, read-only
			 * @readonly
			 */
			| "num_effectslots";

		type ReadOnlyQuickEffectRack1ChannelIEffect1EqualizerRack1ChannelIEffect1EffectRack1EffectUnitNEffectMControl =
			/**
			 * Whether an Effect is loaded into this EffectSlot
			 *
			 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
			 * @range binary, read-only
			 * @readonly
			 */
			| "loaded"

			/**
			 * The number of parameters the currently loaded effect has.
			 *
			 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
			 * @range integer, read-only,  0 if no effect is loaded
			 * @readonly
			 */
			| "num_parameters"

			/**
			 * The number of parameter slots available.
			 *
			 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
			 * @range integer, read-only
			 * @readonly
			 */
			| "num_parameterslots"

			/**
			 * The number of button parameters the currently loaded effect has.
			 *
			 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
			 * @range integer, read-only, 0 if no effect is loaded
			 * @readonly
			 */
			| "num_button_parameters"

			/**
			 * The number of button parameter slots available.
			 *
			 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
			 * @range integer, read-only
			 * @readonly
			 */
			| "num_button_parameterslots"

			/**
			 * Whether or not the Kth parameter slot has an effect parameter loaded into it.
			 *
			 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
			 * @range binary, read-only
			 * @readonly
			 */
			| `parameter${number}_loaded`

			/**
			 * The type of the Kth parameter value. See the Parameter Value Types table.
			 *
			 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
			 * @range integer, read-only
			 * @readonly
			 */
			| `parameter${number}_type`

			/**
			 * Whether or not the Kth parameter slot has an effect parameter loaded into it.
			 *
			 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
			 * @range binary, read-only
			 * @readonly
			 */
			| `button_parameter${number}_loaded`

			/**
			 * The type of the Kth parameter value. See the Parameter Value Types table.
			 *
			 * @groups [QuickEffectRack1_[ChannelI]_Effect1], [EqualizerRack1_[ChannelI]_Effect1], [EffectRack1_EffectUnitN_EffectM]
			 * @range integer, read-only
			 * @readonly
			 */
			| `button_parameter${number}_type`;

		type ReadOnlyPreviewDeckNControl = ReadOnlyPreviewDeckNSamplerNChannelNControl;

		type ReadOnlyEqualizerRack1ChannelIControl =
			ReadOnlyQuickEffectRack1ChannelIEffectRack1EffectUnitNEqualizerRack1ChannelIControl;

		type ReadOnlyQuickEffectRack1ChannelIEffect1Control =
			ReadOnlyQuickEffectRack1ChannelIEffect1EqualizerRack1ChannelIEffect1EffectRack1EffectUnitNEffectMControl;

		type ReadOnlyEqualizerRack1ChannelIEffect1Control =
			ReadOnlyQuickEffectRack1ChannelIEffect1EqualizerRack1ChannelIEffect1EffectRack1EffectUnitNEffectMControl;

		type ReadOnlySamplerNControl = ReadOnlyPreviewDeckNSamplerNChannelNControl;

		type ReadOnlyChannelNControl =
			| ReadOnlyMicrophoneNChannelNAuxiliaryNControl
			| ReadOnlyPreviewDeckNSamplerNChannelNControl;

		type ReadOnlyAuxiliaryNControl = ReadOnlyMicrophoneNChannelNAuxiliaryNControl;

		type ReadOnlyEqualizerRack1Control = ReadOnlyEffectRack1QuickEffectRack1EqualizerRack1Control;

		type ReadOnlyEffectRack1Control = ReadOnlyEffectRack1QuickEffectRack1EqualizerRack1Control;

		type ReadOnlyQuickEffectRack1ChannelIControl =
			ReadOnlyQuickEffectRack1ChannelIEffectRack1EffectUnitNEqualizerRack1ChannelIControl;

		type ReadOnlyMicrophoneNControl = ReadOnlyMicrophoneNChannelNAuxiliaryNControl;

		type ReadOnlyQuickEffectRack1Control = ReadOnlyEffectRack1QuickEffectRack1EqualizerRack1Control;

		type ReadOnlyEffectRack1EffectUnitNControl =
			ReadOnlyQuickEffectRack1ChannelIEffectRack1EffectUnitNEqualizerRack1ChannelIControl;

		type ReadOnlyEffectRack1EffectUnitNEffectMControl =
			ReadOnlyQuickEffectRack1ChannelIEffect1EqualizerRack1ChannelIEffect1EffectRack1EffectUnitNEffectMControl;
	}

	namespace Deprecated {
		type DeprecatedMasterControl =
			/**
			 * The current output sample rate (default: 44100 Hz).
			 *
			 * @groups [Master]
			 * @range absolute value (in Hz)
			 * @deprecated since  version 2.4.0: Use [App],samplerate instead.
			 */
			"samplerate";
	}
}
