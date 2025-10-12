declare namespace MixxxControls {
    /*
     * Public
     */
    export type MixxxGroup = keyof MixxxControlReadWrite | (string & {});

    // All controls
    export type MixxxControl<TGroup> =
        | (string & {})
        | (0 extends 1 & TGroup // is any check
              ? string
              : TGroup extends keyof MixxxControlReadWrite | keyof ReadOnly.MixxxControlReadOnly
              ?
                    | (TGroup extends keyof MixxxControlReadWrite ? MixxxControlReadWrite[TGroup] : never)
                    | (TGroup extends keyof ReadOnly.MixxxControlReadOnly
                          ? ReadOnly.MixxxControlReadOnly[TGroup]
                          : never)
              : string);

    // Controls that are read & write at the same time
    export type MixxxControlReadAndWrite<TGroup> =
        | (string & {})
        | (0 extends 1 & TGroup // is any check
              ? string
              : TGroup extends keyof MixxxControlReadWrite
              ? MixxxControlReadWrite[TGroup]
              : string);

    /*
     * Group <-> control linking
     */
    type MixxxControlReadWrite = {
        "[App]": MixxxAppControl;
        "[Main]": MixxxMainControl;
        "[Master]": MixxxMasterControl;
        "[Microphone]": MixxxMicrophoneControl;
        "[VinylControl]": MixxxVinylControl;
        "[Recording]": MixxxRecordingControl;
        "[AutoDJ]": MixxxAutoDJControl;
        "[Library]": MixxxLibraryControl;
        "[Shoutcast]": MixxxShoutcastControl;
        "[Playlist]": MixxxPlaylistControl;
        "[Controls]": MixxxControlsControl;
        "[Skin]": MixxxSkinControl;
    } & {
        [key: `[Channel${number}]`]: MixxxChannelControl;
        [key: `[PreviewDeck${number}]`]: MixxxDeckControl;
        [key: `[Sampler${number}]`]: MixxxSamplerControl;
        [key: `[Microphone${number}]`]: MixxxMicrophoneControl;
        [key: `[Auxiliary${number}]`]: MixxxAuxiliaryControl;
        [key: `[EffectRack1_EffectUnit${number}]`]: MixxxEffectRack1UnitControl;
        [key: `[EqualizerRack1_[Channel${number}]]`]: MixxxEffectEqualizerQuickEffectRack1Control;
        [key: `[QuickEffectRack1_[Channel${number}]]`]: MixxxEffectEqualizerQuickEffectRack1Control;
        [key: `[EffectRack1_EffectUnit${number}_Effect${number}]`]: MixxxEffectEqualizerQuickEffectRack1EffectControl;
        [key: `[EqualizerRack1_[Channel${number}]_Effect1]`]: MixxxEffectEqualizerQuickEffectRack1EffectControl;
        [key: `[QuickEffectRack1_[Channel${number}]_Effect1]`]: MixxxEffectEqualizerQuickEffectRack1EffectControl;
    };

    /*
     * Values
     */

    // [App] controls
    type MixxxAppControl =
        | "num_decks"
        | "num_samplers"
        | "num_preview_decks"
        | "num_microphones"
        | "num_auxiliaries"
        | "samplerate";

    // [Master] controls
    type MixxxMasterControl =
        | "audio_latency_usage"
        | "audio_latency_overload"
        | "audio_latency_overload_count"
        | "balance"
        | "booth_enabled"
        | "booth_gain"
        | "crossfader"
        | "crossfader_down"
        | "crossfader_down_small"
        | "crossfader_up"
        | "crossfader_up_small"
        | "duckStrength"
        | "enabled"
        | "gain"
        | "headEnabled"
        | "headGain"
        | "headMix"
        | "headSplit"
        | "latency"
        | "talkoverDucking";

    // [Main] controls
    type MixxxMainControl =
        | "peak_indicator"
        | "peak_indicator_l"
        | "peak_indicator_r"
        | "vu_meter"
        | "vu_meter_l"
        | "vu_meter_r";

    // [ChannelN] && [PreviewDeckN] && [SamplerN] controls
    type MixxxChannelPreviewSamplerControl =
        | "back"
        | "bpmlock"
        | "beat_closest"
        | "beat_distance"
        | "beatjump"
        | "beatjump_size"
        | "beatjump_size_halve"
        | "beatjump_size_double"
        | "beatjump_forward"
        | "beatjump_backward"
        | `beatjump_${number}_forward`
        | `beatjump_${number}_backward`
        | "beatloop_activate"
        | `beatloop_${number}_activate`
        | `beatloop_r${number}_activate`
        | "beatloop_size"
        | `beatloop_${number}_toggle`
        | `beatloop_${number}_enabled`
        | "beatlooproll_activate"
        | `beatlooproll_${number}_activate`
        | `beatlooproll_r${number}_activate`
        | "beats_adjust_faster"
        | "beats_adjust_slower"
        | "beats_translate_curpos"
        | "beats_translate_match_alignment"
        | "beats_translate_earlier"
        | "beats_translate_later"
        | "shift_cues_earlier"
        | "shift_cues_later"
        | "shift_cues_earlier_small"
        | "shift_cues_later_small"
        | "beats_undo_adjustment"
        | "beatsync"
        | "beatsync_phase"
        | "beatsync_tempo"
        | "bpm"
        | "bpm_tap"
        | "tempo_tap"
        | "CloneFromDeck"
        | "CloneFromSampler"
        | "LoadTrackFromDeck"
        | "LoadTrackFromSampler"
        | "cue_cdj"
        | "cue_clear"
        | "cue_goto"
        | "cue_default"
        | "cue_gotoandplay"
        | "cue_gotoandstop"
        | "cue_indicator"
        | "cue_mode"
        | "cue_play"
        | "cue_point"
        | "cue_preview"
        | "cue_set"
        | "cue_simple"
        | "duration"
        | "eject"
        | "end"
        | "fwd"
        | `hotcue_${number}_activate`
        | `hotcue_${number}_activatecue`
        | `hotcue_${number}_activateloop`
        | `hotcue_${number}_cueloop`
        | `hotcue_${number}_clear`
        | `hotcue_${number}_color`
        | `hotcue_${number}_goto`
        | `hotcue_${number}_gotoandplay`
        | `hotcue_${number}_gotoandloop`
        | `hotcue_${number}_gotoandstop`
        | `hotcue_${number}_position`
        | `hotcue_${number}_set`
        | `hotcue_${number}_setcue`
        | `hotcue_${number}_setloop`
        | "hotcue_focus"
        | "hotcue_focus_color_prev"
        | "hotcue_focus_color_next"
        | "intro_end_activate"
        | "intro_end_clear"
        | "intro_end_position"
        | "intro_end_set"
        | "intro_start_activate"
        | "intro_start_clear"
        | "intro_start_position"
        | "intro_start_set"
        | "key"
        | "keylock"
        | "LoadSelectedTrack"
        | "LoadSelectedTrackAndPlay"
        | "local_bpm"
        | "loop_anchor"
        | "loop_double"
        | "loop_enabled"
        | "loop_remove"
        | "loop_end_position"
        | "loop_halve"
        | "loop_in"
        | "loop_in_goto"
        | "loop_out"
        | "loop_out_goto"
        | "loop_move"
        | `loop_move_${number}_forward`
        | `loop_move_${number}_backward`
        | "loop_scale"
        | "loop_start_position"
        | "orientation"
        | "orientation_center"
        | "orientation_left"
        | "orientation_right"
        | "outro_end_activate"
        | "outro_end_clear"
        | "outro_end_position"
        | "outro_end_set"
        | "outro_start_activate"
        | "outro_start_clear"
        | "outro_start_position"
        | "outro_start_set"
        | "passthrough"
        | "peak_indicator"
        | "peak_indicator_l"
        | "peak_indicator_r"
        | "pfl"
        | "pitch"
        | "pitch_up"
        | "pitch_down"
        | "pitch_adjust"
        | "play"
        | "play_stutter"
        | "playposition"
        | "pregain"
        | "quantize"
        | "rate"
        | "rate_dir"
        | "rate_perm_down"
        | "rate_perm_down_small"
        | "rate_perm_up"
        | "rate_perm_up_small"
        | "rate_temp_down"
        | "rate_temp_down_small"
        | "rate_temp_up"
        | "rate_temp_up_small"
        | "rateRange"
        | "rateSearch"
        | "rateEngine"
        | "reloop_andstop"
        | "reloop_toggle"
        | "repeat"
        | "reset_key"
        | "reverse"
        | "reverseroll"
        | "scratch2"
        | "scratch2_enable"
        | "show_track_menu"
        | "slip_enabled"
        | "stars_up"
        | "stars_down"
        | "start"
        | "start_play"
        | "start_stop"
        | "stop"
        | "sync_enabled"
        | "sync_leader"
        | "sync_mode"
        | "sync_key"
        | "track_color"
        | "volume"
        | "mute"
        | "update_replaygain_from_pregain"
        | "vinylcontrol_enabled"
        | "vinylcontrol_cueing"
        | "vinylcontrol_mode"
        | "visual_bpm"
        | "visual_key"
        | "visual_key_distance"
        | "vu_meter"
        | "vu_meter_l"
        | "vu_meter_r"
        | "waveform_zoom"
        | "waveform_zoom_up"
        | "waveform_zoom_down"
        | "waveform_zoom_set_default"
        | "wheel";

    // [PreviewDeckN] controls
    type MixxxDeckControl = MixxxChannelPreviewSamplerControl;

    // [SamplerN] controls
    type MixxxSamplerControl = MixxxChannelPreviewSamplerControl | "SaveSamplerBank" | "LoadSamplerBank";

    // [ChannelN] && [Microphone] && [MicrophoneN] && [AuxiliaryN] controls
    type MixxxChannelMicrophoneAuxiliaryControl =
        | "main_mix"
        | "peak_indicator"
        | "peak_indicator_l"
        | "peak_indicator_r"
        | "pfl"
        | "talkover"
        | "volume"
        | "pregain"
        | "mute"
        | "vu_meter"
        | "vu_meter_l"
        | "vu_meter_r";

    // [ChannelN] controls
    type MixxxChannelControl =
        | MixxxChannelPreviewSamplerControl
        | MixxxChannelMicrophoneAuxiliaryControl
        | "input_configured";

    // [Microphone] && [MicrophoneN] controls
    type MixxxMicrophoneControl = MixxxChannelMicrophoneAuxiliaryControl;

    // [AuxiliaryN] controls
    type MixxxAuxiliaryControl = MixxxChannelMicrophoneAuxiliaryControl | "orientation";

    // [VinylControl] controls
    type MixxxVinylControl = "Toggle" | "gain";

    // [Recording] controls
    type MixxxRecordingControl = "toggle_recording" | "status";

    // [AutoDJ] controls
    type MixxxAutoDJControl = "enabled" | "shuffle_playlist" | "skip_next" | "fade_now" | "add_random_track";

    // [Playlist] && [Library] controls
    type MixxxLibraryPlaylistControl = "AutoDjAddBottom" | "AutoDjAddTop";

    // [Library] controls
    type MixxxLibraryControl =
        | MixxxLibraryPlaylistControl
        | "MoveUp"
        | "MoveDown"
        | "MoveVertical"
        | "ScrollUp"
        | "ScrollDown"
        | "ScrollVertical"
        | "MoveLeft"
        | "MoveRight"
        | "MoveHorizontal"
        | "MoveFocusForward"
        | "MoveFocusBackward"
        | "MoveFocus"
        | "focused_widget"
        | "GoToItem"
        | "show_track_menu"
        | "font_size_increment"
        | "font_size_decrement"
        | "font_size_knob"
        | "sort_column"
        | "sort_column_toggle"
        | "sort_order"
        | "sort_focused_column"
        | "track_color_prev"
        | "track_color_next"
        | "search_history_next"
        | "search_history_prev"
        | "search_history_selector"
        | "clear_search";

    // [Shoutcast] controls
    type MixxxShoutcastControl = "enabled" | "status";

    // [Playlist] controls
    type MixxxPlaylistControl = MixxxLibraryPlaylistControl | "SelectPlaylist" | "SelectTrackKnob";

    // [Control] controls
    type MixxxControlsControl = "touch_shift" | "AutoHotcueColors" | "ShowDurationRemaining";

    // [EffectRack1_EffectUnitN] && [EqualizerRack1_[ChannelI]] && [QuickEffectRack1_[ChannelI]] controls
    type MixxxEffectEqualizerQuickEffectRack1Control =
        | "num_effectunits"
        | "chain_preset_selector"
        | "clear"
        | "enabled"
        | "focused_effect"
        | `group_[Channel${number}]_enable`
        | "loaded_chain_preset"
        | "mix"
        | "next_chain_preset"
        | "prev_chain_preset"
        | "show_focus"
        | "show_parameters"
        | "super1";

    // [EffectRack1_EffectUnitN] controls
    type MixxxEffectRack1UnitControl =
        | MixxxEffectEqualizerQuickEffectRack1Control
        | "group_[Headphone]_enable"
        | "group_[Master]_enable"
        | `group_[Sampler${number}]_enable`;

    // [EffectRack1_EffectUnitN_EffectM] && [EqualizerRack1_[ChannelI]_Effect1] && [QuickEffectRack1_[ChannelI]_Effect1] controls
    type MixxxEffectEqualizerQuickEffectRack1EffectControl =
        | "clear"
        | "effect_selector"
        | "enabled"
        | "loaded_effect"
        | "next_effect"
        | "meta"
        | "prev_effect"
        | `parameter${number}`
        | `parameter${number}_link_inverse`
        | `parameter${number}_link_type`
        | `button_parameter${number}`;

    // [Skin] controls
    type MixxxSkinControl =
        | "show_effectrack"
        | "show_library_coverart"
        | "show_maximized_library"
        | "show_samplers"
        | "show_vinylcontrol";

    /*
     *  Read-only controls
     */
    namespace ReadOnly {
        /*
         * group <-> read-only control linking
         */
        export type MixxxControlReadOnly = {
            "[App]": MixxxAppControlReadOnly;
            "[Master]": MixxxMasterControlReadOnly;
            "[EffectRack1]": MixxxRack1ControlReadOnly;
            "[EqualizerRack1]": MixxxRack1ControlReadOnly;
            "[QuickEffectRack1]": MixxxRack1ControlReadOnly;
        } & {
            [key: `[Channel${number}]`]: MixxxChannelControlReadOnly;
            [key: `[PreviewDeck${number}]`]: MixxxChannelPreviewSamplerControlReadOnly;
            [key: `[Sampler${number}]`]: MixxxChannelPreviewSamplerControlReadOnly;
            [key: `[Microphone${number}]`]: MixxxMicrophoneControl;
            [key: `[Auxiliary${number}]`]: MixxxAuxiliaryControl;
        };

        // [App] read-only controls
        type MixxxAppControlReadOnly =
            | "gui_tick_50ms_period_s"
            | "gui_tick_full_period_s"
            | "indicator_250ms"
            | "indicator_500ms";

        // [Master] read-only controls
        type MixxxMasterControlReadOnly = "num_effectsavailable";

        // [ChannelN] && [PreviewDeckN] && [SamplerN] read-only controls
        type MixxxChannelPreviewSamplerControlReadOnly =
            | "beat_active"
            | "end_of_track"
            | "file_bpm"
            | "file_key"
            | `hotcue_${number}_status`
            | `hotcue_${number}_type`
            | "intro_end_enabled"
            | "intro_start_enabled"
            | "outro_end_enabled"
            | "outro_start_enabled"
            | "play_indicator"
            | "play_latched"
            | "track_loaded"
            | "track_samplerate"
            | "track_samples"
            | "vinylcontrol_status";

        // [ChannelN] && [Microphone] && [MicrophoneN] && [AuxiliaryN] read-only controls
        type MixxxChannelMicrophoneAuxiliaryControlReadOnly = "input_configured";

        // [ChannelN] read-only controls
        type MixxxChannelControlReadOnly =
            | MixxxChannelPreviewSamplerControlReadOnly
            | MixxxChannelMicrophoneAuxiliaryControlReadOnly;

        //  [EffectRack1] && [EqualizerRack1] && [QuickEffectRack1] read-only controls
        type MixxxRack1ControlReadOnly = "num_effectunits";

        // [EffectRack1_EffectUnitN] && [EqualizerRack1_[ChannelI]] && [QuickEffectRack1_[ChannelI]] read-only controls
        type MixxxEffectEqualizerQuickEffectRack1ControlReadOnly = "loaded" | "num_chain_presets" | "num_effectslots";

        // [EffectRack1_EffectUnitN_EffectM] && [EqualizerRack1_[ChannelI]_Effect1] && [QuickEffectRack1_[ChannelI]_Effect1] read-only controls
        type MixxxEffectEqualizerQuickEffectRack1EffectControlReadOnly =
            | "loaded"
            | "num_parameters"
            | "num_parameterslots"
            | "num_button_parameters"
            | "num_button_parameterslots"
            | `parameter${number}_loaded`
            | `parameter${number}_type`
            | `button_parameter${number}_loaded`
            | `button_parameter${number}_type`;
    }
}
