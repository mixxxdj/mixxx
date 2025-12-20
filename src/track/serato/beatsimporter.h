#pragma once

#ifdef BUILD_TESTING
#include <gtest/gtest_prod.h>
#endif

#include <QList>

#include "track/beats.h"
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
    BeatsPointer importBeatsAndApplyTimingOffset(
            const QString& filePath,
            const QString& fileType,
            const audio::StreamInfo& streamInfo) override;

  private:
    FRIEND_TEST(SeratoBeatGridTest, SerializeBeatMap);

    BeatsPointer importBeatsAndApplyTimingOffset(
            double timingOffsetMillis, const audio::SignalInfo& signalInfo);

    QList<SeratoBeatGridNonTerminalMarkerPointer> m_nonTerminalMarkers;
    SeratoBeatGridTerminalMarkerPointer m_pTerminalMarker;
};

} // namespace mixxx
