#ifndef BEATFACTORY_H
#define BEATFACTORY_H

#include <QHash>

#include "track/beats.h"

class Track;

class BeatFactory {
  public:
    static mixxx::BeatsPointer loadBeatsFromByteArray(const Track& track,
            QString beatsVersion,
            QString beatsSubVersion,
            const QByteArray& beatsSerialized);
    static mixxx::BeatsPointer makeBeatGrid(const Track& track,
            double dBpm,
            double dFirstBeatSample);

    static QString getPreferredVersion(const bool bEnableFixedTempoCorrection);

    static QString getPreferredSubVersion(
        const bool bEnableFixedTempoCorrection,
        const bool bEnableOffsetCorrection,
        const int iMinBpm, const int iMaxBpm,
        const QHash<QString, QString> extraVersionInfo);

    static mixxx::BeatsPointer makePreferredBeats(const Track& track,
            QVector<double> beats,
            const QHash<QString, QString> extraVersionInfo,
            const bool bEnableFixedTempoCorrection,
            const bool bEnableOffsetCorrection,
            const int iSampleRate,
            const int iTotalSamples,
            const int iMinBpm,
            const int iMaxBpm);

  private:
    static void deleteBeats(mixxx::Beats* pBeats);
};

#endif /* BEATFACTORY_H */
