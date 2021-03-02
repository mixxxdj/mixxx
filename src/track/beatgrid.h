#pragma once

#include <QMutex>

#include "proto/beats.pb.h"
#include "track/beats.h"

#define BEAT_GRID_1_VERSION "BeatGrid-1.0"
#define BEAT_GRID_2_VERSION "BeatGrid-2.0"

class Track;

namespace mixxx {

// BeatGrid is an implementation of the Beats interface that implements an
// infinite grid of beats, aligned to a song simply by a starting offset of the
// first beat and the song's average beats-per-minute.
class BeatGrid final : public Beats {
  public:
    // Construct a BeatGrid. If a more accurate sample rate is known, provide it
    // in the iSampleRate parameter -- otherwise pass 0.
    ~BeatGrid() override = default;

    // Initializes the BeatGrid to have a BPM of dBpm and the first beat offset
    // of dFirstBeatSample. Does not generate an updated() signal, since it is
    // meant for initialization.
    static BeatsPointer makeBeatGrid(const Track& track,
            SINT iSampleRate,
            const QString& subVersion,
            double dBpm,
            double dFirstBeatSample);
    // Construct a BeatGrid. If a more accurate sample rate is known, provide it
    // in the iSampleRate parameter -- otherwise pass 0. The BeatGrid will be
    // deserialized from the byte array.
    static BeatsPointer makeBeatGrid(const Track& track,
            SINT iSampleRate,
            const QString& subVersion,
            const QByteArray& byteArray);

    // The following are all methods from the Beats interface, see method
    // comments in beats.h

    Beats::CapabilitiesFlags getCapabilities() const override {
        return BEATSCAP_TRANSLATE | BEATSCAP_SCALE | BEATSCAP_SETBPM;
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
    BeatGrid(const BeatGrid& other);
    // Constructor to update the beat grid
    BeatGrid(const BeatGrid& other, const mixxx::track::io::BeatGrid& grid, double beatLength);
    BeatGrid(const Track& track,
            SINT iSampleRate,
            const QString& subVersion,
            const mixxx::track::io::BeatGrid& grid,
            double beatLength);
    double firstBeatSample() const;
    double bpm() const;

    // For internal use only.
    bool isValid() const;

    mutable QMutex m_mutex;
    // The sub-version of this beatgrid.
    QString m_subVersion;
    // The number of samples per second
    SINT m_iSampleRate;
    // Data storage for BeatGrid
    mixxx::track::io::BeatGrid m_grid;
    // The length of a beat in samples
    double m_dBeatLength;
};

} // namespace mixxx
