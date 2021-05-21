#pragma once

#include <gtest/gtest_prod.h>

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
    FRIEND_TEST(SeratoBeatGridTest, SerializeBeatMap);

    QVector<double> importBeatsAndApplyTimingOffset(
            double timingOffsetMillis, const audio::SignalInfo& signalInfo);

    QList<SeratoBeatGridNonTerminalMarkerPointer> m_nonTerminalMarkers;
    SeratoBeatGridTerminalMarkerPointer m_pTerminalMarker;
};

} // namespace mixxx
