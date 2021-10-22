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
            mixxx::Bpm bpm,
            mixxx::audio::FramePos firstBeatPositionOnFrameBoundary,
            const QString& subVersion = QString());

    static BeatsPointer fromByteArray(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const QByteArray& byteArray);

    // The following are all methods from the Beats interface, see method
    // comments in beats.h

    bool hasConstantTempo() const override {
        return true;
    }

    QByteArray toByteArray() const override;
    QString getVersion() const override;
    QString getSubVersion() const override;

    ////////////////////////////////////////////////////////////////////////////
    // Beat calculations
    ////////////////////////////////////////////////////////////////////////////

    bool findPrevNextBeats(audio::FramePos position,
            audio::FramePos* prevBeatPosition,
            audio::FramePos* nextBeatPosition,
            bool snapToNearBeats) const override;
    audio::FramePos findNthBeat(audio::FramePos position, int n) const override;
    std::unique_ptr<BeatIterator> findBeats(audio::FramePos startPosition,
            audio::FramePos endPosition) const override;
    bool hasBeatInRange(audio::FramePos startPosition, audio::FramePos endPosition) const override;
    mixxx::Bpm getBpmInRange(audio::FramePos startPosition,
            audio::FramePos endPosition) const override;
    mixxx::Bpm getBpmAroundPosition(audio::FramePos position, int n) const override;

    audio::SampleRate getSampleRate() const override {
        return m_sampleRate;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Beat mutations
    ////////////////////////////////////////////////////////////////////////////

    std::optional<BeatsPointer> tryTranslate(audio::FrameDiff_t offset) const override;
    std::optional<BeatsPointer> tryScale(BpmScale scale) const override;
    std::optional<BeatsPointer> trySetBpm(mixxx::Bpm bpm) const override;

    ////////////////////////////////////////////////////////////////////////////
    // Hidden constructors
    ////////////////////////////////////////////////////////////////////////////

    BeatGrid(
            MakeSharedTag,
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const mixxx::track::io::BeatGrid& grid,
            double beatLength);
    // Constructor to update the beat grid
    BeatGrid(
            MakeSharedTag,
            const BeatGrid& other,
            const mixxx::track::io::BeatGrid& grid,
            double beatLength);
    BeatGrid(
            MakeSharedTag,
            const BeatGrid& other);

  private:
    audio::FramePos firstBeatPosition() const;
    mixxx::Bpm bpm() const;

    bool isValid() const override;

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
