#pragma once

// Used for returning errors from sounddevice functions.
enum class SoundDeviceStatus {
    Error = -1,
    Ok = 0,
    ErrorDuplicateOutputChannel,
    ErrorExcessiveOutputChannel,
    ErrorExcessiveInputChannel,
    ErrorDeviceCount
};
