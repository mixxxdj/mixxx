#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <QVector>
#include <memory>
#include <optional>

#include "audio/frame.h"
#include "audio/types.h"
#include "track/bpm.h"
#include "util/types.h"

#define BEAT_GRID_1_VERSION "BeatGrid-1.0"
#define BEAT_GRID_2_VERSION "BeatGrid-2.0"
#define BEAT_MAP_VERSION "BeatMap-1.0"

namespace mixxx {

class Beats;
typedef std::shared_ptr<const Beats> BeatsPointer;

/// A beat marker is denotes the border of a tempo section inside a track.
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

/// This class represents the beats of a track.
///
/// Internally, it uses the following data structure:
/// - 0 - N beat markers, followed by
/// - exactly one tempo marker ("last marker").
///
/// If the track has a constant tempo, there are 0 beat markers, and the last
/// marker is positioned at the first downbeat and is set to the tracks BPM.
///
/// All instances of this class are supposed to be managed by std::shared_ptr!
class Beats : private std::enable_shared_from_this<Beats> {
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
                : m_beats(beats),
                  m_it(it),
                  m_beatOffset(beatOffset) {
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
            return lhs.m_beats == rhs.m_beats &&
                    lhs.m_it == rhs.m_it &&
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

    Beats(std::vector<BeatMarker> markers,
            mixxx::audio::FramePos lastMarkerPosition,
            mixxx::Bpm lastMarkerBpm,
            mixxx::audio::SampleRate sampleRate,
            const QString& subVersion)
            : m_markers(std::move(markers)),
              m_lastMarkerPosition(lastMarkerPosition),
              m_lastMarkerBpm(lastMarkerBpm),
              m_sampleRate(sampleRate),
              m_subVersion(subVersion) {
        DEBUG_ASSERT(m_lastMarkerPosition.isValid());
        DEBUG_ASSERT(!m_lastMarkerPosition.isFractional());
        DEBUG_ASSERT(m_lastMarkerBpm.isValid());
        DEBUG_ASSERT(m_sampleRate.isValid());
    }

    Beats(mixxx::audio::FramePos lastMarkerPosition,
            mixxx::Bpm lastMarkerBpm,
            mixxx::audio::SampleRate sampleRate,
            const QString& subVersion)
            : Beats(std::vector<BeatMarker>(),
                      lastMarkerPosition,
                      lastMarkerBpm,
                      sampleRate,
                      subVersion) {
    }

    ~Beats() = default;

    /// Returns an iterator pointing to the position of the first beat marker.
    ConstIterator cfirstmarker() const {
        return ConstIterator(this, m_markers.cbegin(), 0);
    }

    /// Returns an iterator pointing to the position of the first beat after
    /// the end beat marker.
    ConstIterator clastmarker() const {
        return ConstIterator(this, m_markers.cend(), 0);
    }

    /// Returns an iterator pointing to earliest representable beat position
    /// (which is INT_MIN beats before the first beat marker).
    ///
    /// Warning: Decrementing the iterator returned by this function will
    /// result in an integer underflow.
    ConstIterator cbegin() const {
        return ConstIterator(this, m_markers.cbegin(), std::numeric_limits<int>::lowest());
    }

    /// Returns an iterator pointing to latest representable beat position
    /// (which is INT_MAX beats behind the end beat marker).
    ///
    /// Warning: Incrementing the iterator returned by this function will
    /// result in an integer overflow.
    ConstIterator cend() const {
        return ConstIterator(this, m_markers.cend(), std::numeric_limits<int>::max());
    }

    ConstIterator iteratorFrom(audio::FramePos position) const;

    friend bool operator==(const Beats& lhs, const Beats& rhs) {
        return lhs.m_markers == rhs.m_markers &&
                lhs.m_lastMarkerPosition == rhs.m_lastMarkerPosition &&
                lhs.m_lastMarkerBpm == rhs.m_lastMarkerBpm && lhs.m_sampleRate &&
                rhs.m_sampleRate;
    }

    friend bool operator!=(const Beats& lhs, const Beats& rhs) {
        return !(lhs == rhs);
    }

    BeatsPointer clonePointer() const {
        // All instances are immutable and can be shared safely
        return shared_from_this();
    }

    static mixxx::BeatsPointer fromByteArray(
            mixxx::audio::SampleRate sampleRate,
            const QString& beatsVersion,
            const QString& beatsSubVersion,
            const QByteArray& beatsSerialized);

    static BeatsPointer fromBeatGridByteArray(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const QByteArray& byteArray);

    static BeatsPointer fromBeatMapByteArray(
            audio::SampleRate sampleRate,
            const QString& subVersion,
            const QByteArray& byteArray);

    static mixxx::BeatsPointer fromConstTempo(
            audio::SampleRate sampleRate,
            audio::FramePos position,
            Bpm bpm,
            const QString& subVersion = QString());

    static mixxx::BeatsPointer fromBeatPositions(
            audio::SampleRate sampleRate,
            const QVector<audio::FramePos>& beatPositions,
            const QString& subVersion = QString());

    static mixxx::BeatsPointer fromBeatMarkers(
            audio::SampleRate sampleRate,
            const std::vector<BeatMarker>& beatMarker,
            const audio::FramePos lastMarkerPosition,
            const Bpm lastMarkerBpm,
            const QString& subVersion = QString());

    enum class BpmScale {
        Double,
        Halve,
        TwoThirds,
        ThreeFourths,
        FourThirds,
        ThreeHalves,
    };

    /// Returns false if the beats implementation supports non-const beats.
    ///
    /// TODO: This is only needed for the "Asumme Constant Tempo" checkbox in
    /// `DlgTrackInfo`. This should probably be removed or reimplemented to
    /// check if all neighboring beats in this object have the same distance.
    bool hasConstantTempo() const {
        return m_markers.empty();
    }

    /// Serialize beats to QByteArray.
    QByteArray toByteArray() const;

    /// A string representing the version of the beat-processing code that
    /// produced this Beats instance. Used by BeatsFactory for associating a
    /// given serialization with the version that produced it.
    QString getVersion() const;
    /// A sub-version can be used to represent the preferences used to generate
    /// the beats object.
    QString getSubVersion() const {
        return m_subVersion;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Beat calculations
    ////////////////////////////////////////////////////////////////////////////

    // TODO: We may want all of these find functions to return an integer
    //       instead of a double.
    // TODO: We may want to implement these with common code that returns
    //       the triple of closest, next, and prev.

    /// Starting from frame position `position`, return the frame position of
    /// the next beat in the track, or an invalid position if none exists. If
    /// `position` refers to the location of a beat, `position` is returned.
    audio::FramePos findNextBeat(audio::FramePos position) const;

    /// Starting from frame position `position`, return the frame position of
    /// the previous beat in the track, or an invalid position if none exists.
    /// If `position` refers to the location of beat, `position` is returned.
    audio::FramePos findPrevBeat(audio::FramePos position) const;

    /// Starting from frame position `position`, fill the frame position of the
    /// previous beat and next beat. Either can be invalid if none exists. If
    /// `position` refers to the location of the beat, the first value is
    /// `position`, and the second value is the next beat position. Returns
    /// `false` if *at least one* position is invalid.
    bool findPrevNextBeats(audio::FramePos position,
            audio::FramePos* prevBeatPosition,
            audio::FramePos* nextBeatPosition,
            bool snapToNearBeats) const;

    /// Return the frame position of the first beat in the track, or an invalid
    /// position if none exists.
    audio::FramePos firstBeat() const {
        return findNextBeat(mixxx::audio::kStartFramePos);
    }

    /// Starting from frame position `position`, return the frame position of
    /// the closest beat in the track, or an invalid position if none exists.
    audio::FramePos findClosestBeat(audio::FramePos position) const;

    /// Find the Nth beat from frame position `position`. Works with both
    /// positive and negative values of n. Calling findNthBeat with `n=0` is
    /// invalid and always returns an invalid frame position. Calling
    /// findNthBeat with `n=1` or `n=-1` is equivalent to calling
    /// `findNextBeat` and `findPrevBeat`, respectively. If `position` refers
    /// to the location of a beat, then `position` is returned. If no beat can
    /// be found, returns an invalid frame position.
    audio::FramePos findNthBeat(audio::FramePos position, int n) const;

    /// This function snaps the position to a beat if near.
    /// This is used for beat loops, where start and end positions might be slightly off
    /// due to rounding or quantizations by the engine buffer.
    /// It makes use of virtual findPrevNextBeats() as a single instance for snapping.
    audio::FramePos snapPosToNearBeat(audio::FramePos position) const;

    int numBeatsInRange(audio::FramePos startPosition, audio::FramePos endPosition) const;

    /// Find the frame position N beats away from `position`. The number of beats may be
    /// negative and does not need to be an integer. In this case the returned position will
    /// be between two beats as well at the same fraction.
    audio::FramePos findNBeatsFromPosition(
            audio::FramePos position, double beats) const;

    /// Return whether or not a beat exists between `startPosition` and `endPosition`.
    bool hasBeatInRange(audio::FramePos startPosition,
            audio::FramePos endPosition) const;

    /// Return the predominant BPM value between `startPosition` and `endPosition`
    /// if the BPM is valid, otherwise returns an invalid BPM value.
    mixxx::Bpm getBpmInRange(audio::FramePos startPosition,
            audio::FramePos endPosition) const;

    /// Return the arithmetic average BPM over the range of n*2 beats centered around
    /// frame position `position`. For example, n=4 results in an averaging of 8 beats.
    /// The returned Bpm value may be invalid.
    mixxx::Bpm getBpmAroundPosition(audio::FramePos position, int n) const;

    audio::SampleRate getSampleRate() const {
        return m_sampleRate;
    }

    const std::vector<BeatMarker>& getMarkers() const {
        return m_markers;
    }

    mixxx::audio::FramePos getLastMarkerPosition() const {
        return m_lastMarkerPosition;
    }
    mixxx::Bpm getLastMarkerBpm() const {
        return m_lastMarkerBpm;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Beat mutations
    ////////////////////////////////////////////////////////////////////////////

    /// Translate all beats in the song by `offset` frames. Beats that lie
    /// before the start of the track or after the end of the track are *not*
    /// removed.
    //
    /// Returns a pointer to the modified beats object, or `nullopt` on
    /// failure.
    std::optional<BeatsPointer> tryTranslate(audio::FrameDiff_t offset) const;

    /// Translate all beats based on scalar 'xBeats', e.g. half a beat if xBeats
    /// is set to 0.5. Works only for tracks with constant BPM.
    //
    /// Returns a pointer to the modified beats object, or `nullopt` on
    /// failure.
    std::optional<BeatsPointer> tryTranslateBeats(double xBeats) const;

    /// Scale the position of every beat in the song by `scale`.
    //
    /// Returns a pointer to the modified beats object, or `nullopt` on
    /// failure.
    std::optional<BeatsPointer> tryScale(BpmScale scale) const;

    /// Adjust the beats so the global average BPM matches `bpm`.
    //
    /// Returns a pointer to the modified beats object, or `nullopt` on
    /// failure.
    std::optional<BeatsPointer> trySetBpm(mixxx::Bpm bpm) const;

  protected:
    /// Type tag for making public constructors of derived classes inaccessible.
    ///
    /// The constructors must be public for using std::make_shared().
    struct MakeSharedTag {};

    Beats() = default;

    bool isValid() const;

  private:
    Beats(const Beats&) = delete;
    Beats(Beats&&) = delete;

    QByteArray toBeatGridByteArray() const;
    QByteArray toBeatMapByteArray() const;

    mixxx::audio::FrameDiff_t firstBeatLengthFrames() const;
    mixxx::audio::FrameDiff_t lastBeatLengthFrames() const;

    std::vector<BeatMarker> m_markers;
    mixxx::audio::FramePos m_lastMarkerPosition;
    mixxx::Bpm m_lastMarkerBpm;
    mixxx::audio::SampleRate m_sampleRate;

    // The sub-version of this beatgrid.
    const QString m_subVersion;
};

} // namespace mixxx
