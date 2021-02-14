#pragma once

#include <QList>

#include "track/beatsimporter.h"
#include "track/serato/beatgrid.h"

namespace mixxx {

class SeratoBeatsImporter : public BeatsImporter {
  public:
    SeratoBeatsImporter();
    SeratoBeatsImporter(
            const QList<SeratoBeatGridNonTerminalMarkerPointer>& nonTerminalMarkers,
            SeratoBeatGridTerminalMarkerPointer pTerminalMarker);
    ~SeratoBeatsImporter() override = default;

    bool isEmpty() const override;
    QVector<double> importBeatsAndApplyTimingOffset(
            const QString& filePath,
            const audio::StreamInfo& streamInfo) override;

  private:
    QList<SeratoBeatGridNonTerminalMarkerPointer> m_nonTerminalMarkers;
    SeratoBeatGridTerminalMarkerPointer m_pTerminalMarker;
};

} // namespace mixxx
