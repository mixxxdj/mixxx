#pragma once

#include <QString>

#include "track/cueinfoimporter.h"

namespace mixxx {
namespace audio {
class SignalInfo;
} // namespace audio

class SeratoCueInfoImporter : public CueInfoImporter {
  public:
    using CueInfoImporter::CueInfoImporter;

    ~SeratoCueInfoImporter() override = default;

    double guessTimingOffsetMillis(
            const QString& filePath,
            const audio::SignalInfo& signalInfo) const override;
};

} // namespace mixxx
