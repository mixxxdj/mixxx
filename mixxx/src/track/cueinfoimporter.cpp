#include "track/cueinfoimporter.h"

namespace mixxx {

CueInfoImporter::CueInfoImporter(QList<CueInfo> cueInfos)
        : m_cueInfos(std::move(cueInfos)) {
}

bool CueInfoImporter::hasCueOfType(CueType cueType) const {
    for (const CueInfo& cueInfo : std::as_const(m_cueInfos)) {
        if (cueInfo.getType() == cueType) {
            return true;
        }
    }
    return false;
}

double CueInfoImporter::guessTimingOffsetMillis(
        const QString& filePath,
        const QString& fileType,
        const audio::SignalInfo& signalInfo) const {
    Q_UNUSED(filePath);
    Q_UNUSED(fileType);
    Q_UNUSED(signalInfo);
    return 0;
};

int CueInfoImporter::size() const {
    return m_cueInfos.size();
}

bool CueInfoImporter::isEmpty() const {
    return m_cueInfos.isEmpty();
}

QList<CueInfo> CueInfoImporter::importCueInfosAndApplyTimingOffset(
        const QString& filePath,
        const QString& fileType,
        const audio::SignalInfo& signalInfo) {
    // Consume the collected cue points during the import
    QList<CueInfo> cueInfos = m_cueInfos;

    // Do not calculate offset if we don't have any cues to import
    if (cueInfos.isEmpty()) {
        return cueInfos;
    }

    double timingOffsetMillis = guessTimingOffsetMillis(filePath, fileType, signalInfo);

    // If we don't have any offset, we can just return the CueInfo objects
    // unchanged.
    if (timingOffsetMillis == 0) {
        return cueInfos;
    }

    // Create list of CueInfo object with correct positions
    for (CueInfo& cueInfo : cueInfos) {
        if (cueInfo.getStartPositionMillis()) {
            cueInfo.setStartPositionMillis(
                    *cueInfo.getStartPositionMillis() + timingOffsetMillis);
        }
        if (cueInfo.getEndPositionMillis()) {
            cueInfo.setEndPositionMillis(
                    *cueInfo.getEndPositionMillis() + timingOffsetMillis);
        }
    }

    return cueInfos;
}

} // namespace mixxx
