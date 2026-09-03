pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick

Item {
    visible: false
    readonly property bool ready: true

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
    Mixxx.SkinControlCreator { defaultValue: 1.0; group: "[Skin]"; key: "latenight_sampler_rows"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 0.0; group: "[Skin]"; key: "latenight_expand_samplers_1_4"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 0.0; group: "[Skin]"; key: "latenight_expand_samplers_1_8"; persist: true }
    Mixxx.SkinControlCreator { defaultValue: 0.0; group: "[Skin]"; key: "latenight_expand_samplers_9_16"; persist: true }
}
