/*
 * beatmap.h
 *
 *  Created on: 08/dic/2011
 *      Author: vittorio
 */

#pragma once

#include <QMutex>

#include "proto/beats.pb.h"
#include "track/beats.h"

#define BEAT_MAP_VERSION "BeatMap-1.0"

class Track;

typedef QList<mixxx::track::io::Beat> BeatList;

namespace mixxx {

class BeatMap final : public Beats {
  public:

    ~BeatMap() override = default;

    static BeatsPointer makeBeatMap(
            SINT sampleRate,
            const QString& subVersion,
            const QByteArray& byteArray);

    static BeatsPointer makeBeatMap(
            SINT sampleRate,
            const QString& subVersion,
            const QVector<double>& beats);

    Beats::CapabilitiesFlags getCapabilities() const override {
        return BEATSCAP_TRANSLATE | BEATSCAP_SCALE | BEATSCAP_ADDREMOVE |
                BEATSCAP_MOVEBEAT;
    }

    QByteArray toByteArray() const override;
    QString getVersion() const override;
    QString getSubVersion() const override;

    ////////////////////////////////////////////////////////////////////////////
    // Beat calculations
    ////////////////////////////////////////////////////////////////////////////

    double findNextBeat(double dSamples) const override;
    double findPrevBeat(double dSamples) const override;
    bool findPrevNextBeats(double dSamples,
                           double* dpPrevBeatSamples,
                           double* dpNextBeatSamples) const override;
    double findClosestBeat(double dSamples) const override;
    double findNthBeat(double dSamples, int n) const override;
    std::unique_ptr<BeatIterator> findBeats(double startSample, double stopSample) const override;
    bool hasBeatInRange(double startSample, double stopSample) const override;

    double getBpm() const override;
    double getBpmAroundPosition(double curSample, int n) const override;

    SINT getSampleRate() const override {
        return m_iSampleRate;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Beat mutations
    ////////////////////////////////////////////////////////////////////////////

    BeatsPointer clone() const override;
    BeatsPointer translate(double dNumSamples) const override;
    BeatsPointer scale(enum BPMScale scale) const override;
    BeatsPointer setBpm(double dBpm) override;

  private:
    BeatMap(SINT sampleRate,
            const QString& subVersion,
            BeatList beats,
            double nominalBpm);
    // Constructor to update the beat map
    BeatMap(const BeatMap& other, BeatList beats, double nominalBpm);
    BeatMap(const BeatMap& other);

    // For internal use only.
    bool isValid() const;

    const QString m_subVersion;
    const SINT m_iSampleRate;
    const double m_nominalBpm;
    const BeatList m_beats;
};

} // namespace mixxx
