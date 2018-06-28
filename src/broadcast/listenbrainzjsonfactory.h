#pragma once

#include <QByteArray>

#include "track/track.h"

namespace ListenBrainzJSONFactory {
enum JsonType { NowListening,
    Single };
QByteArray getJSONFromTrack(TrackPointer pTrack, JsonType type);
}; // namespace ListenBrainzJSONFactory