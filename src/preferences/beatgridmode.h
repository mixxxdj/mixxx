#pragma once

// It would be preferable to use QFlags here since it will enable checks
// like beatGridMode.testFlag(), but in order to match with QComboBox indices,
// we will hardcode these indices which correspond to the order of choices in
// beatGridModeComboBox in DlgPrefWaveform.
enum BeatGridMode {
    BEATS = 0,
    BEATS_DOWNBEATS = 1
};
