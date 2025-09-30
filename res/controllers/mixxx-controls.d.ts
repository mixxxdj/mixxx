declare type MixxxControlName = {
  '[App]': MixxxAppControl;
  '[Master]': MixxxMasterControl;
  '[Microphone]': MixxxMicrophoneControl;
  '[VinylControl]': MixxxVinylControl;
  '[Recording]': MixxxRecordingControl;
  '[AutoDJ]': MixxxAutoDJControl;
  '[Library]': MixxxLibraryControl;
  '[Shoutcast]': MixxxShoutcastControl;
  '[Playlist]': MixxxPlaylistControl;
  '[Controls]': MixxxControlsControl;
  '[EffectRack1]': MixxxEffectEqualizerQuickEffectRack1Control;
  '[EqualizerRack1]': MixxxEffectEqualizerQuickEffectRack1Control;
  '[QuickEffectRack1]': MixxxEffectEqualizerQuickEffectRack1Control;
  '[Skin]': MixxxSkinControl;
} & {
  [key: MixxxChannelGroup]: MixxxChannelControl;
  [key: `[PreviewDeck${number}]`]: MixxxDeckControl;
  [key: `[Sampler${number}]`]: MixxxSamplerControl;
  [key: `[Microphone${number}]`]: MixxxMicrophoneControl;
  [key: `[Auxiliary${number}]`]: MixxxAuxiliaryControl;
  [key: `[EffectRack1_EffectUnit${number}]`]: MixxxEffectRack1UnitControl;
  [
    key: `[EqualizerRack1_[Channel${number}]]`
  ]: EffectEqualizerQuickEffectRack1Control;
  [
    key: `[QuickEffectRack1_[Channel${number}]]`
  ]: EffectEqualizerQuickEffectRack1Control;
  [
    key: `[EffectRack1_EffectUnit${number}_Effect${number}]`
  ]: MixxxEffectEqualizerQuickEffectRack1EffectControl;
  [
    key: `[EqualizerRack1_[Channel${number}]_Effect1]`
  ]: MixxxEffectEqualizerQuickEffectRack1EffectControl;
  [
    key: `[QuickEffectRack1_[Channel${number}]_Effect1]`
  ]: MixxxEffectEqualizerQuickEffectRack1EffectControl;
};

type MixxxChannelGroup = `[Channel${number}]`;

declare type MixxxGroup = keyof MixxxControlName;

/*
 * Values
 */

type MixxxPotMeterControls =
  | '_up'
  | '_down'
  | '_up_small'
  | '_down_small'
  | '_set_one'
  | '_set_minus_one'
  | '_set_default'
  | '_set_zero'
  | '_toggle'
  | '_minus_toggle';

type MixxxAppControl =
  | 'gui_tick_50ms_period_s'
  | 'indicator_250ms'
  | 'indicator_500ms'
  | 'num_decks'
  | 'num_samplers'
  | 'num_preview_decks'
  | 'num_microphones'
  | 'num_auxiliaries'
  | 'samplerate';

type MixxxMasterControl =
  | 'audio_latency_usage'
  | 'audio_latency_overload'
  | 'audio_latency_overload_count'
  | 'balance'
  | 'booth_enabled'
  | 'booth_gain'
  | 'crossfader'
  | 'crossfader_down'
  | 'crossfader_down_small'
  | 'crossfader_up'
  | 'crossfader_up_small'
  | 'duckStrength'
  | 'enabled'
  | 'gain'
  | 'headEnabled'
  | 'headGain'
  | 'headMix'
  | 'headSplit'
  | 'latency'
  | 'num_effectsavailable'
  | 'PeakIndicator'
  | 'PeakIndicatorL'
  | 'PeakIndicatorR'
  | 'talkoverDucking'
  | 'VuMeter'
  | 'VuMeterL'
  | 'VuMeterR';

type MixxxChannelDeckSamplerControl =
  | 'back'
  | 'bpmlock'
  | 'beat_active'
  | 'beat_closest'
  | 'beat_distance'
  | 'beatjump'
  | 'beatjump_size'
  | 'beatjump_size_halve'
  | 'beatjump_size_double'
  | 'beatjump_forward'
  | 'beatjump_backward'
  | 'beatjump_X_forward'
  | 'beatjump_X_backward'
  | 'beatloop_activate'
  | 'beatloop_X_activate'
  | 'beatloop_rX_activate'
  | 'beatloop_size'
  | 'beatloop_X_toggle'
  | 'beatloop_X_enabled'
  | 'beatlooproll_activate'
  | 'beatlooproll_X_activate'
  | 'beatlooproll_rX_activate'
  | 'beats_adjust_faster'
  | 'beats_adjust_slower'
  | 'beats_translate_curpos'
  | 'beats_translate_match_alignment'
  | 'beats_translate_earlier'
  | 'beats_translate_later'
  | 'shift_cues_earlier'
  | 'shift_cues_later'
  | 'shift_cues_earlier_small'
  | 'shift_cues_later_small'
  | 'beats_undo_adjustment'
  | 'beatsync'
  | 'beatsync_phase'
  | 'beatsync_tempo'
  | 'bpm'
  | 'bpm_tap'
  | 'tempo_tap'
  | 'CloneFromDeck'
  | 'CloneFromSampler'
  | 'LoadTrackFromDeck'
  | 'LoadTrackFromSampler'
  | 'cue_cdj'
  | 'cue_clear'
  | 'cue_goto'
  | 'cue_default'
  | 'cue_gotoandplay'
  | 'cue_gotoandstop'
  | 'cue_indicator'
  | 'cue_mode'
  | 'cue_play'
  | 'cue_point'
  | 'cue_preview'
  | 'cue_set'
  | 'cue_simple'
  | 'duration'
  | 'eject'
  | 'end'
  | 'end_of_track'
  | 'file_bpm'
  | 'file_key'
  | 'fwd'
  | 'hotcue_X_activate'
  | 'hotcue_X_activatecue'
  | 'hotcue_X_activateloop'
  | 'hotcue_X_cueloop'
  | 'hotcue_X_clear'
  | 'hotcue_X_color'
  | 'hotcue_X_status'
  | 'hotcue_X_type'
  | 'hotcue_X_goto'
  | 'hotcue_X_gotoandplay'
  | 'hotcue_X_gotoandloop'
  | 'hotcue_X_gotoandstop'
  | 'hotcue_X_position'
  | 'hotcue_X_set'
  | 'hotcue_X_setcue'
  | 'hotcue_X_setloop'
  | 'hotcue_focus'
  | 'hotcue_focus_color_prev'
  | 'hotcue_focus_color_next'
  | 'intro_end_activate'
  | 'intro_end_clear'
  | 'intro_end_enabled'
  | 'intro_end_position'
  | 'intro_end_set'
  | 'intro_start_activate'
  | 'intro_start_clear'
  | 'intro_start_enabled'
  | 'intro_start_position'
  | 'intro_start_set'
  | 'key'
  | 'keylock'
  | 'LoadSelectedTrack'
  | 'LoadSelectedTrackAndPlay'
  | 'local_bpm'
  | 'loop_anchor'
  | 'loop_double'
  | 'loop_enabled'
  | 'loop_remove'
  | 'loop_end_position'
  | 'loop_halve'
  | 'loop_in'
  | 'loop_in_goto'
  | 'loop_out'
  | 'loop_out_goto'
  | 'loop_move'
  | 'loop_move_X_forward'
  | 'loop_move_X_backward'
  | 'loop_scale'
  | 'loop_start_position'
  | 'orientation'
  | 'orientation_center'
  | 'orientation_left'
  | 'orientation_right'
  | 'outro_end_activate'
  | 'outro_end_clear'
  | 'outro_end_enabled'
  | 'outro_end_position'
  | 'outro_end_set'
  | 'outro_start_activate'
  | 'outro_start_clear'
  | 'outro_start_enabled'
  | 'outro_start_position'
  | 'outro_start_set'
  | 'passthrough'
  | 'PeakIndicator'
  | 'PeakIndicatorL'
  | 'PeakIndicatorR'
  | 'pfl'
  | 'pitch'
  | 'pitch_up'
  | 'pitch_down'
  | 'pitch_adjust'
  | 'play'
  | 'play_indicator'
  | 'play_latched'
  | 'play_stutter'
  | 'playposition'
  | 'pregain'
  | 'quantize'
  | 'rate'
  | 'rate_dir'
  | 'rate_perm_down'
  | 'rate_perm_down_small'
  | 'rate_perm_up'
  | 'rate_perm_up_small'
  | 'rate_temp_down'
  | 'rate_temp_down_small'
  | 'rate_temp_up'
  | 'rate_temp_up_small'
  | 'rateRange'
  | 'rateSearch'
  | 'rateEngine'
  | 'reloop_andstop'
  | 'reloop_toggle'
  | 'repeat'
  | 'reset_key'
  | 'reverse'
  | 'reverseroll'
  | 'scratch2'
  | 'scratch2_enable'
  | 'show_track_menu'
  | 'slip_enabled'
  | 'stars_up'
  | 'stars_down'
  | 'start'
  | 'start_play'
  | 'start_stop'
  | 'stop'
  | 'sync_enabled'
  | 'sync_leader'
  | 'sync_mode'
  | 'sync_key'
  | 'track_color'
  | 'track_loaded'
  | 'track_samplerate'
  | 'track_samples'
  | 'volume'
  | 'mute'
  | 'update_replaygain_from_pregain'
  | 'vinylcontrol_enabled'
  | 'vinylcontrol_cueing'
  | 'vinylcontrol_mode'
  | 'vinylcontrol_status'
  | 'visual_bpm'
  | 'visual_key'
  | 'visual_key_distance'
  | 'VuMeter'
  | 'VuMeterL'
  | 'VuMeterR'
  | 'waveform_zoom'
  | 'waveform_zoom_up'
  | 'waveform_zoom_down'
  | 'waveform_zoom_set_default'
  | 'wheel';

type MixxxDeckControl = MixxxChannelDeckSamplerControl;

type MixxxSamplerControl =
  | MixxxChannelDeckSamplerControl
  | 'SaveSamplerBank'
  | 'LoadSamplerBank';

type MixxxMicrophoneAuxiliaryControl =
  | 'input_configured'
  | 'main_mix'
  | 'PeakIndicator'
  | 'PeakIndicatorL'
  | 'PeakIndicatorR'
  | 'pfl'
  | 'talkover'
  | 'volume'
  | 'pregain'
  | 'mute'
  | 'VuMeter'
  | 'VuMeterL'
  | 'VuMeterR';

type MixxxChannelControl =
  | MixxxChannelDeckSamplerControl
  | MixxxMicrophoneAuxiliaryControl
  | 'input_configured';

type MixxxMicrophoneControl = MixxxMicrophoneAuxiliaryControl;

type MixxxAuxiliaryControl =
  | MixxxMicrophoneAuxiliaryControl
  | 'orientation';

type MixxxVinylControl = 'Toggle' | 'gain';

type MixxxRecordingControl = 'toggle_recording' | 'status';

type MixxxAutoDJControl =
  | 'enabled'
  | 'shuffle_playlist'
  | 'skip_next'
  | 'fade_now'
  | 'add_random_track';

type MixxxLibraryPlaylistControl = 'AutoDjAddBottom' | 'AutoDjAddTop';
type MixxxLibraryControl =
  | MixxxLibraryPlaylistControl
  | 'MoveUp'
  | 'MoveDown'
  | 'MoveVertical'
  | 'ScrollUp'
  | 'ScrollDown'
  | 'ScrollVertical'
  | 'MoveLeft'
  | 'MoveRight'
  | 'MoveHorizontal'
  | 'MoveFocusForward'
  | 'MoveFocusBackward'
  | 'MoveFocus'
  | 'focused_widget'
  | 'GoToItem'
  | 'show_track_menu'
  | 'font_size_increment'
  | 'font_size_decrement'
  | 'font_size_knob'
  | 'sort_column'
  | 'sort_column_toggle'
  | 'sort_order'
  | 'sort_focused_column'
  | 'track_color_prev'
  | 'track_color_next'
  | 'search_history_next'
  | 'search_history_prev'
  | 'search_history_selector'
  | 'clear_search';

type MixxxShoutcastControl = 'enabled' | 'status';

type MixxxPlaylistControl =
  | MixxxLibraryPlaylistControl
  | 'SelectPlaylist'
  | 'SelectTrackKnob';

type MixxxControlsControl =
  | 'touch_shift'
  | 'AutoHotcueColors'
  | 'ShowDurationRemaining';

type MixxxEffectEqualizerQuickEffectRack1Control = 'num_effectunits';
type EffectEqualizerQuickEffectRack1Control =
  | 'num_effectunits'
  | 'chain_preset_selector'
  | 'clear'
  | 'enabled'
  | 'focused_effect'
  | `group_[Channel${number}]_enable`
  | 'loaded'
  | 'loaded_chain_preset'
  | 'mix'
  | 'next_chain_preset'
  | 'num_chain_presets'
  | 'num_effectslots'
  | 'prev_chain_preset'
  | 'show_focus'
  | 'show_parameters'
  | 'super1';

type MixxxEffectRack1UnitControl =
  | EffectEqualizerQuickEffectRack1Control
  | 'group_[Headphone]_enable'
  | 'group_[Master]_enable'
  | `group_[Sampler${number}]_enable`;

type MixxxEffectEqualizerQuickEffectRack1EffectControl =
  | 'clear'
  | 'effect_selector'
  | 'enabled'
  | 'loaded'
  | 'loaded_effect'
  | 'next_effect'
  | 'num_parameters'
  | 'num_parameterslots'
  | 'num_button_parameters'
  | 'num_button_parameterslots'
  | 'meta'
  | 'prev_effect'
  | `parameter${number}`
  | `parameter${number}_link_inverse`
  | `parameter${number}_link_type`
  | `parameter${number}_loaded`
  | `parameter${number}_type`
  | `button_parameter${number}`
  | `button_parameter${number}_loaded`
  | `button_parameter${number}_type`;

type MixxxSkinControl =
  | 'show_effectrack'
  | 'show_library_coverart'
  | 'show_maximized_library'
  | 'show_samplers'
  | 'show_vinylcontrol';
