#include "track/serato/cueinfoimporter.h"

#include "track/serato/tags.h"

namespace mixxx {

double SeratoCueInfoImporter::getTimingOffsetMillis(
        const QString& filePath,
        const audio::SignalInfo& signalInfo) const {
    return SeratoTags::findTimingOffsetMillis(filePath, signalInfo);
}

} // namespace mixxx
