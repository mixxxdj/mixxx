#pragma once
#include "track/cueinfoimporter.h"

namespace mixxx {

class SeratoCueInfoImporter : public CueInfoImporter {
  public:
    using CueInfoImporter::CueInfoImporter;

    double getTimingOffsetMillis(
            const QString& filePath,
            const audio::SignalInfo& signalInfo) const override;
};

} // namespace mixxx
