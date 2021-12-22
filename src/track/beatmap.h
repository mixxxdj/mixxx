/*
 * beatmap.h
 *
 *  Created on: 08/dic/2011
 *      Author: vittorio
 */

#pragma once

#include "proto/beats.pb.h"
#include "track/beats.h"

#define BEAT_MAP_VERSION "BeatMap-1.0"

class Track;

typedef QList<mixxx::track::io::Beat> BeatList;

namespace mixxx {

class BeatMap final : public Beats {
  public:
    ~BeatMap() override = default;

    static BeatsPointer fromByteArray(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const QByteArray& byteArray);

    static BeatsPointer makeBeatMap(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const QVector<mixxx::audio::FramePos>& beats);

    bool hasConstantTempo() const override {
        return false;
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

    BeatMap(
            MakeSharedTag,
            audio::SampleRate sampleRate,
            const QString& subVersion,
            BeatList beats);
    // Constructor to update the beat map
    BeatMap(
            MakeSharedTag,
            const BeatMap& other,
            BeatList beats);
    BeatMap(
            MakeSharedTag,
            const BeatMap& other);

  private:
    bool isValid() const override;

    const QString m_subVersion;
    const audio::SampleRate m_sampleRate;
    const BeatList m_beats;
};

} // namespace mixxx
