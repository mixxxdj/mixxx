#include "track/cueinfoimporter.h"

namespace mixxx {

CueInfoImporter::CueInfoImporter(const QList<CueInfo>& cueInfos)
        : m_cueInfos(cueInfos) {
}

double CueInfoImporter::guessTimingOffsetMillis(
        const QString& filePath,
        const audio::SignalInfo& signalInfo) const {
    Q_UNUSED(filePath);
    Q_UNUSED(signalInfo);
    return 0;
};

void CueInfoImporter::append(const CueInfo& cueInfo) {
    m_cueInfos.append(cueInfo);
}

void CueInfoImporter::append(const QList<CueInfo>& cueInfos) {
    m_cueInfos.append(cueInfos);
}

int CueInfoImporter::size() const {
    return m_cueInfos.size();
}

bool CueInfoImporter::isEmpty() const {
    return m_cueInfos.isEmpty();
}

QList<CueInfo> CueInfoImporter::getCueInfosWithCorrectTiming(
        const QString& filePath,
        const audio::SignalInfo& signalInfo) {
    QList<CueInfo> cueInfos;

    // Do not calculate offset if we don't have any cues to import
    if (m_cueInfos.isEmpty()) {
        return {};
    }

    double timingOffsetMillis = guessTimingOffsetMillis(filePath, signalInfo);

    // If we don't have any offset, we can just return the CueInfo objects
    // unchanged.
    if (timingOffsetMillis == 0) {
        cueInfos.append(m_cueInfos);
        m_cueInfos.clear();
        return cueInfos;
    }

    // Create list of CueInfo object with correct positions
    cueInfos.reserve(m_cueInfos.size());
    for (const CueInfo& cueInfo : qAsConst(m_cueInfos)) {
        CueInfo newCueInfo(cueInfo);
        if (cueInfo.getStartPositionMillis()) {
            newCueInfo.setStartPositionMillis(
                    *cueInfo.getStartPositionMillis() + timingOffsetMillis);
        }
        if (cueInfo.getEndPositionMillis()) {
            newCueInfo.setEndPositionMillis(
                    *cueInfo.getEndPositionMillis() + timingOffsetMillis);
        }
        cueInfos.append(newCueInfo);
    }

    // Clear pending cueinfos
    m_cueInfos.clear();

    return cueInfos;
}

} // namespace mixxx
