#include <QVector>
#include <iterator>
#include <vector>

#include "audio/frame.h"
#include "track/beats.h"
#include "track/bpm.h"
#include "util/assert.h"

namespace mixxx {
namespace beats {

class BeatMarker {
  public:
    BeatMarker(mixxx::audio::FramePos position, int beatsTillNextMarker)
            : m_position(position), m_beatsTillNextMarker(beatsTillNextMarker) {
        DEBUG_ASSERT(m_position.isValid());
        DEBUG_ASSERT(!m_position.isFractional());
        DEBUG_ASSERT(m_beatsTillNextMarker > 0);
    }

    mixxx::audio::FramePos position() const {
        return m_position;
    }

    int beatsTillNextMarker() const {
        return m_beatsTillNextMarker;
    }

  private:
    mixxx::audio::FramePos m_position;
    int m_beatsTillNextMarker;
};

inline bool operator==(const BeatMarker& lhs, const BeatMarker& rhs) {
    return (lhs.position() == rhs.position() &&
            lhs.beatsTillNextMarker() == rhs.beatsTillNextMarker());
}

inline bool operator!=(const BeatMarker& lhs, const BeatMarker& rhs) {
    return !(lhs == rhs);
}

class Beats final : public ::mixxx::Beats {
  public:
    class ConstIterator {
      public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = mixxx::audio::FramePos;
        using difference_type = int;
        using pointer = value_type*;
        using reference = value_type&;

        ConstIterator(const Beats* beats,
                std::vector<BeatMarker>::const_iterator it,
                int beatOffset)
                : m_beats(beats), m_it(it), m_beatOffset(beatOffset) {
            updateValue();
        }

        mixxx::audio::FrameDiff_t beatLengthFrames() const;

        // Iterator methods

        const value_type& operator*() const {
            return m_value;
        }

        const value_type* operator->() const {
            return &m_value;
        }

        ConstIterator& operator++() {
            *this += 1;
            return *this;
        }

        ConstIterator& operator--() {
            *this -= 1;
            return *this;
        }

        ConstIterator operator++(difference_type) {
            ConstIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        ConstIterator operator--(difference_type) {
            ConstIterator tmp = *this;
            --(*this);
            return tmp;
        }

        ConstIterator operator+(difference_type n) {
            ConstIterator tmp = *this;
            tmp += n;
            return tmp;
        }

        ConstIterator operator-(difference_type n) {
            ConstIterator tmp = *this;
            tmp -= n;
            return tmp;
        }

        ConstIterator operator+=(difference_type n);
        ConstIterator operator-=(difference_type n);

        difference_type operator-(const ConstIterator& other) const;

        friend bool operator==(const ConstIterator& lhs, const ConstIterator& rhs) {
            return lhs.m_beats == rhs.m_beats && lhs.m_it == rhs.m_it &&
                    lhs.m_beatOffset == rhs.m_beatOffset;
        }

        friend bool operator!=(const ConstIterator& lhs, const ConstIterator& rhs) {
            return !(lhs == rhs);
        }

      private:
        void updateValue();

        mixxx::audio::FramePos m_value;

        const Beats* m_beats;
        std::vector<BeatMarker>::const_iterator m_it;
        int m_beatOffset;
    };

    static mixxx::BeatsPointer fromConstTempo(
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::FramePos endMarkerPosition,
            mixxx::Bpm endMarkerBpm,
            const QString& subVersion = QString());

    static mixxx::BeatsPointer fromBeatPositions(
            mixxx::audio::SampleRate sampleRate,
            const QVector<audio::FramePos>& beatPositions,
            const QString& subVersion = QString());

    Beats(std::vector<BeatMarker> markers,
            mixxx::audio::FramePos endMarkerPosition,
            mixxx::Bpm endMarkerBpm,
            mixxx::audio::SampleRate sampleRate,
            const QString& subVersion)
            : m_markers(std::move(markers)),
              m_endMarkerPosition(endMarkerPosition),
              m_endMarkerBpm(endMarkerBpm),
              m_sampleRate(sampleRate),
              m_subVersion(subVersion) {
        DEBUG_ASSERT(m_endMarkerPosition.isValid());
        DEBUG_ASSERT(!m_endMarkerPosition.isFractional());
        DEBUG_ASSERT(m_endMarkerBpm.isValid());
        DEBUG_ASSERT(m_sampleRate.isValid());
    }

    Beats(mixxx::audio::FramePos endMarkerPosition,
            mixxx::Bpm endMarkerBpm,
            mixxx::audio::SampleRate sampleRate,
            const QString& subVersion)
            : Beats(std::vector<BeatMarker>(),
                      endMarkerPosition,
                      endMarkerBpm,
                      sampleRate,
                      subVersion) {
    }

    ConstIterator cbegin() const {
        return ConstIterator(this, m_markers.cbegin(), 0);
    }

    ConstIterator cend() const {
        return ConstIterator(this, m_markers.cend(), 1);
    }

    ConstIterator cearliest() const {
        return ConstIterator(this, m_markers.cbegin(), std::numeric_limits<int>::lowest());
    }

    ConstIterator clatest() const {
        return ConstIterator(this, m_markers.cend(), std::numeric_limits<int>::max());
    }

    ConstIterator iteratorFrom(audio::FramePos position) const;

    friend bool operator==(const Beats& lhs, const Beats& rhs) {
        return lhs.m_markers == rhs.m_markers &&
                lhs.m_endMarkerPosition == rhs.m_endMarkerPosition &&
                lhs.m_endMarkerBpm == rhs.m_endMarkerBpm && lhs.m_sampleRate &&
                rhs.m_sampleRate;
    }

    friend bool operator!=(const Beats& lhs, const Beats& rhs) {
        return !(lhs == rhs);
    }

    static BeatsPointer fromByteArray(
            audio::SampleRate sampleRate,
            const QString& beatsVersion,
            const QString& beatsSubVersion,
            const QByteArray& byteArray);
    static BeatsPointer fromBeatGridByteArray(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const QByteArray& byteArray);
    static BeatsPointer fromBeatMapByteArray(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const QByteArray& byteArray);

    /// Serialize beats to QByteArray.
    QByteArray toByteArray() const override;

    /// A string representing the version of the beat-processing code that
    /// produced this Beats instance. Used by BeatsFactory for associating a
    /// given serialization with the version that produced it.
    QString getVersion() const override;

    /// A sub-version can be used to represent the preferences used to generate
    /// the beats object.
    QString getSubVersion() const override {
        return m_subVersion;
    }

    /// Returns false if the beats implementation supports non-const beats.
    bool hasConstantTempo() const override {
        return m_markers.empty();
    }

    /// Starting from frame position `position`, fill the frame position of the
    /// previous beat and next beat. Either can be invalid if none exists. If
    /// `position` refers to the location of the beat, the first value is
    /// `position`, and the second value is the next beat position. Returns
    /// `false` if *at least one* position is invalid.
    bool findPrevNextBeats(audio::FramePos position,
            audio::FramePos* prevBeatPosition,
            audio::FramePos* nextBeatPosition,
            bool snapToNearBeats) const override;

    /// Find the Nth beat from frame position `position`. Works with both
    /// positive and negative values of n. Calling findNthBeat with `n=0` is
    /// invalid and always returns an invalid frame position. Calling
    /// findNthBeat with `n=1` or `n=-1` is equivalent to calling
    /// `findNextBeat` and `findPrevBeat`, respectively. If `position` refers
    /// to the location of a beat, then `position` is returned. If no beat can
    /// be found, returns an invalid frame position.
    audio::FramePos findNthBeat(audio::FramePos position, int n) const override;

    /// Returns an iterator that yields frame position of every beat occurring
    /// between `startPosition` and `endPosition`. `BeatIterator` must be iterated
    /// while holding a strong references to the `Beats` object to ensure that
    /// the `Beats` object is not deleted. Caller takes ownership of the returned
    /// `BeatIterator`.
    std::unique_ptr<BeatIterator> findBeats(
            audio::FramePos startPosition,
            audio::FramePos endPosition) const override;

    /// Return whether or not a beat exists between `startPosition` and `endPosition`.
    bool hasBeatInRange(audio::FramePos startPosition, audio::FramePos endPosition) const override;

    /// Return the average BPM over the entire track if the BPM is valid,
    /// otherwise returns an invalid bpm value.
    mixxx::Bpm getBpmInRange(audio::FramePos startPosition,
            audio::FramePos endPosition) const override;

    /// Return the average BPM over the range of n*2 beats centered around
    /// frame position `position`. For example, n=4 results in an averaging of 8 beats.
    /// The returned Bpm value may be invalid.
    mixxx::Bpm getBpmAroundPosition(audio::FramePos position, int n) const override;
    audio::SampleRate getSampleRate() const override {
        return m_sampleRate;
    }

    /// Translate all beats in the song by `offset` frames. Beats that lie
    /// before the start of the track or after the end of the track are *not*
    /// removed.
    std::optional<BeatsPointer> tryTranslate(audio::FrameDiff_t offset) const override;

    /// Scale the position of every beat in the song by `scale`.
    std::optional<BeatsPointer> tryScale(BpmScale scale) const override;

    /// Adjust the beats so the global average BPM matches `bpm`.
    std::optional<BeatsPointer> trySetBpm(mixxx::Bpm bpm) const override;

  private:
    bool isValid() const override;

    QByteArray toBeatGridByteArray() const;
    QByteArray toBeatMapByteArray() const;

    mixxx::audio::FrameDiff_t firstBeatLengthFrames() const;
    mixxx::audio::FrameDiff_t endBeatLengthFrames() const;

    std::vector<BeatMarker> m_markers;
    mixxx::audio::FramePos m_endMarkerPosition;
    mixxx::Bpm m_endMarkerBpm;
    mixxx::audio::SampleRate m_sampleRate;

    // The sub-version of this beatgrid.
    const QString m_subVersion;
};

} // namespace beats
} // namespace mixxx
