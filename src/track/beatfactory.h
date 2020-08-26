#ifndef BEATFACTORY_H
#define BEATFACTORY_H

#include <QHash>

#include "track/beats.h"
#include "track/track.h"

class BeatFactory {
  public:
    
    static void deleteBeats(mixxx::Beats* pBeats);

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
    
};

#endif /* BEATFACTORY_H */
