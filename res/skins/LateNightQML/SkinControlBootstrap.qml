pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick

Item {
    visible: false
    readonly property bool ready: true

    // All controls used by the layout are declared here before Toolbar and
    // deck children construct their proxies. Existing profile controls are
    // preserved; missing controls receive their QML defaults deterministically.
    // Keep the toolbar bootstrap sentinel in the skin namespace so it can be
    // created by SkinControlCreator before toolbar proxies bind to it.
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "initialized_toolbar_defaults"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_waveforms"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_hotcues"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_8_hotcues"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_intro_outro_cues"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_loop_controls"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_beatjump_controls"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_rate_controls"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_rate_control_buttons"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_key_controls"; persist: true }
    Mixxx.SkinControlCreator { group: "[Skin]"; key: "show_vinylcontrol"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_spinnies"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_coverart"; persist: true }
    Mixxx.SkinControlCreator { group: "[Skin]"; key: "select_big_spinny_or_cover"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_effectrack"; persist: true }
    Mixxx.SkinControlCreator { group: "[Skin]"; key: "show_4effectunits"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 0.0; group: "[Skin]"; key: "show_superknobs"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_eq_knobs"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_eq_kill_buttons"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_xfader"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_main_head_mixer"; persist: true }
    Mixxx.SkinControlCreator { group: "[Skin]"; key: "equal_4deck_waveforms"; persist: true }
    Mixxx.SkinControlCreator { group: "[Skin]"; key: "timing_shift_buttons"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 0.0; group: "[Skin]"; key: "show_sampler_fx"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "sampler_rows"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "show_4samplers"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_8samplers"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "show_16samplers"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "show_32samplers"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "show_48samplers"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "show_64samplers"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "expand_samplers_1-4"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "expand_samplers_1-8"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "expand_samplers_9-16"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "expand_samplers_17-24"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "expand_samplers_25-32"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "expand_samplers_33-40"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "expand_samplers_41-48"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "expand_samplers_49-56"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: -1.0; group: "[Skin]"; key: "expand_samplers_57-64"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 0.0; group: "[Skin]"; key: "show_4decks"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_mixer"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 0.0; group: "[Skin]"; key: "show_samplers"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 0.0; group: "[Skin]"; key: "show_microphones"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 0.0; group: "[Skin]"; key: "show_maximized_library"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 0.0; group: "[Skin]"; key: "show_preview_decks"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_library_coverart"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_rate_controls_compact"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_loop_controls_compact"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_beatjump_controls_compact"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_key_controls_compact"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "show_vumeters_compact"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "latenight_show_sync_button_compact"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "latenight_deck_size_without_mixer"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "latenight_max_lib_show_decks"; persist: true }
}
