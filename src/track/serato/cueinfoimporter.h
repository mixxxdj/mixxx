#pragma once

#include "track/cueinfoimporter.h"

namespace mixxx {

class SeratoCueInfoImporter : public CueInfoImporter {
  public:
    using CueInfoImporter::CueInfoImporter;

    ~SeratoCueInfoImporter() override = default;

    bool canImportCueType(mixxx::CueType) const override;

    double guessTimingOffsetMillis(
            const QString& filePath,
            const audio::SignalInfo& signalInfo) const override;
};

} // namespace mixxx
