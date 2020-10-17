#pragma once

// It would be preferable to use QFlags here since it will enable checks
// like beatGridMode.testFlag(), but in order to match with QComboBox indices,
// we will hardcode these indices which correspond to the order of choices in
// beatGridModeComboBox in DlgPrefWaveform.
enum class BeatGridMode : int {
    Beats = 0,
    BeatsAndDownbeats = 1
};

constexpr BeatGridMode kDefaultBeatGridMode = BeatGridMode::BeatsAndDownbeats;
