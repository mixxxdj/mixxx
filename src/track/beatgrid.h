#pragma once

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
    ~BeatGrid() override = default;

    static BeatsPointer makeBeatGrid(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            mixxx::Bpm bpm,
            mixxx::audio::FramePos firstBeatPos);

    static BeatsPointer makeBeatGrid(
            audio::SampleRate sampleRate,
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

    audio::FramePos findNextBeat(audio::FramePos position) const override;
    audio::FramePos findPrevBeat(audio::FramePos position) const override;
    bool findPrevNextBeats(audio::FramePos position,
            audio::FramePos* prevBeatPosition,
            audio::FramePos* nextBeatPosition,
            bool snapToNearBeats) const override;
    audio::FramePos findClosestBeat(audio::FramePos position) const override;
    audio::FramePos findNthBeat(audio::FramePos position, int n) const override;
    std::unique_ptr<BeatIterator> findBeats(audio::FramePos startPosition,
            audio::FramePos endPosition) const override;
    bool hasBeatInRange(audio::FramePos startPosition, audio::FramePos endPosition) const override;
    mixxx::Bpm getBpm() const override;
    mixxx::Bpm getBpmAroundPosition(audio::FramePos position, int n) const override;

    audio::SampleRate getSampleRate() const override {
        return m_sampleRate;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Beat mutations
    ////////////////////////////////////////////////////////////////////////////

    BeatsPointer translate(audio::FrameDiff_t offset) const override;
    BeatsPointer scale(BpmScale scale) const override;
    BeatsPointer setBpm(mixxx::Bpm bpm) override;

  private:
    BeatGrid(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const mixxx::track::io::BeatGrid& grid,
            double beatLength);
    // Constructor to update the beat grid
    BeatGrid(const BeatGrid& other, const mixxx::track::io::BeatGrid& grid, double beatLength);
    BeatGrid(const BeatGrid& other);

    audio::FramePos firstBeatPosition() const;
    mixxx::Bpm bpm() const;

    // For internal use only.
    bool isValid() const;

    // The sub-version of this beatgrid.
    const QString m_subVersion;
    // The number of samples per second
    const audio::SampleRate m_sampleRate;
    // Data storage for BeatGrid
    const mixxx::track::io::BeatGrid m_grid;
    // The length of a beat in samples
    const audio::FrameDiff_t m_beatLengthFrames;
};

} // namespace mixxx
