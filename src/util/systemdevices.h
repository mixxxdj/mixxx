#pragma once

#include <QStringList>

namespace mixxx {

/// Names of the devices the operating system currently reports, without
/// touching the audio or MIDI backends that Mixxx uses for playback and
/// controllers. This is cheap enough to poll in order to notice that something
/// was connected or disconnected: a DJ controller always registers a MIDI
/// device on Windows, and its sound card shows up as an ALSA card on Linux.
QStringList systemConnectedDeviceNames();

} // namespace mixxx
