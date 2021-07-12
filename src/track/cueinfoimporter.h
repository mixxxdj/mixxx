#pragma once

#include <memory>

#include "track/cueinfo.h"

namespace mixxx {

/// Importer class for CueInfo objects that can correct timing offsets when the
/// signal info (channel number, sample rate, bitrate) is known.
class CueInfoImporter {
  public:
    CueInfoImporter() = default;
    explicit CueInfoImporter(const QList<CueInfo>& cueInfos);
    virtual ~CueInfoImporter() = default;

    /// Returns true if the importer has any cue(s) of the given cueType.
    virtual bool hasCueOfType(CueType cueType) const;

    /// Returns audio signal dependent timing offset correction.
    /// The default implementation just returns 0, but this can be overridden
    /// in subclasses.
    virtual double guessTimingOffsetMillis(
            const QString& filePath,
            const audio::SignalInfo& signalInfo) const;

    void append(const CueInfo& cueInfo);
    void append(const QList<CueInfo>& cueInfos);

    int size() const;
    bool isEmpty() const;

    QList<CueInfo> importCueInfosAndApplyTimingOffset(
            const QString& filePath,
            const audio::SignalInfo& signalInfo);

  private:
    QList<CueInfo> m_cueInfos;
};

typedef std::shared_ptr<CueInfoImporter> CueInfoImporterPointer;

} // namespace mixxx
