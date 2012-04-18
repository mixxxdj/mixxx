#ifndef BEATFACTORY_H
#define BEATFACTORY_H

#include <QHash>

#include "track/beats.h"

class BeatFactory {
  public:
    static BeatsPointer loadBeatsFromByteArray(TrackPointer pTrack,
                                               QString beatsVersion,
                                               QString beatsSubVersion,
                                               QByteArray* beatsSerialized);
    static BeatsPointer makeBeatGrid(TrackInfoObject* pTrack,
                                     double dBpm, double dFirstBeatSample);

    static QString getPreferredVersion(
        const bool bEnableFixedTempoCorrection,
        const bool bEnableOffsetCorrection,
        const int iMinBpm, const int iMaxBpm,
        const QHash<QString, QString> extraVersionInfo);

    static QString getPreferredSubVersion(
        const bool bEnableFixedTempoCorrection,
        const bool bEnableOffsetCorrection,
        const int iMinBpm, const int iMaxBpm,
        const QHash<QString, QString> extraVersionInfo);

    static BeatsPointer makePreferredBeats(
        TrackPointer pTrack, QVector<double> beats,
        const QHash<QString, QString> extraVersionInfo,
        const bool bEnableFixedTempoCorrection,
        const bool bEnableOffsetCorrection,
        const int iSampleRate, const int iTotalSamples,
        const int iMinBpm, const int iMaxBpm);

  private:
    static void deleteBeats(Beats* pBeats);
};

#endif /* BEATFACTORY_H */
