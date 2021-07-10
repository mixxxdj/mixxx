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

    static BeatsPointer makeBeatMap(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const QByteArray& byteArray);

    static BeatsPointer makeBeatMap(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const QVector<mixxx::audio::FramePos>& beats);

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
    BeatMap(audio::SampleRate sampleRate,
            const QString& subVersion,
            BeatList beats,
            mixxx::Bpm nominalBpm);
    // Constructor to update the beat map
    BeatMap(const BeatMap& other, BeatList beats, mixxx::Bpm nominalBpm);
    BeatMap(const BeatMap& other);

    // For internal use only.
    bool isValid() const;

    const QString m_subVersion;
    const audio::SampleRate m_sampleRate;
    const mixxx::Bpm m_nominalBpm;
    const BeatList m_beats;
};

} // namespace mixxx
