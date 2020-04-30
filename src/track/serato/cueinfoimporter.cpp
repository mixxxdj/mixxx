#include "track/serato/cueinfoimporter.h"

#include "track/serato/tags.h"

namespace mixxx {

/// This method simply calls SeratoTags::guessTimingOffsetMillis() and returns
/// its result. We also need the timing offset for exporting our cues to
/// Serato, so the actual cue offset calculation remains a static method of
/// the SeratoTags for the time being.
double SeratoCueInfoImporter::guessTimingOffsetMillis(
        const QString& filePath,
        const audio::SignalInfo& signalInfo) const {
    return SeratoTags::guessTimingOffsetMillis(filePath, signalInfo);
}

} // namespace mixxx
