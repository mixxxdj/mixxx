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
#include "util/memory.h"
#include "util/types.h"

namespace mixxx {

class Beats;
typedef std::shared_ptr<const Beats> BeatsPointer;

class BeatIterator {
  public:
    virtual ~BeatIterator() = default;
    virtual bool hasNext() const = 0;
    virtual audio::FramePos next() = 0;
};

/// Beats is the base class for BPM and beat management classes. It provides a
/// specification of all methods a beat-manager class must provide, as well as
/// a capability model for representing optional features.
///
/// All instances of this class are supposed to be managed by std::shared_ptr!
class Beats : private std::enable_shared_from_this<Beats> {
  public:
    virtual ~Beats() = default;

    BeatsPointer clonePointer() const {
        // All instances are immutable and can be shared safely
        return shared_from_this();
    }

    static mixxx::BeatsPointer fromByteArray(
            mixxx::audio::SampleRate sampleRate,
            const QString& beatsVersion,
            const QString& beatsSubVersion,
            const QByteArray& beatsSerialized);

    static mixxx::BeatsPointer fromConstTempo(
            audio::SampleRate sampleRate,
            audio::FramePos position,
            Bpm bpm,
            const QString& subVersion = QString());

    static mixxx::BeatsPointer fromBeatPositions(
            audio::SampleRate sampleRate,
            const QVector<audio::FramePos>& beatPositions,
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
    virtual bool hasConstantTempo() const = 0;

    /// Serialize beats to QByteArray.
    virtual QByteArray toByteArray() const = 0;

    /// A string representing the version of the beat-processing code that
    /// produced this Beats instance. Used by BeatsFactory for associating a
    /// given serialization with the version that produced it.
    virtual QString getVersion() const = 0;
    /// A sub-version can be used to represent the preferences used to generate
    /// the beats object.
    virtual QString getSubVersion() const = 0;

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
    virtual bool findPrevNextBeats(audio::FramePos position,
            audio::FramePos* prevBeatPosition,
            audio::FramePos* nextBeatPosition,
            bool snapToNearBeats) const = 0;

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
    virtual audio::FramePos findNthBeat(audio::FramePos position, int n) const = 0;

    int numBeatsInRange(audio::FramePos startPosition, audio::FramePos endPosition) const;

    /// Find the frame position N beats away from `position`. The number of beats may be
    /// negative and does not need to be an integer.
    audio::FramePos findNBeatsFromPosition(audio::FramePos position, double beats) const;

    /// Reutns an iterator that yields frame position of every beat occurring
    /// between `startPosition` and `endPosition`. `BeatIterator` must be iterated
    /// while holding a strong references to the `Beats` object to ensure that
    /// the `Beats` object is not deleted. Caller takes ownership of the returned
    /// `BeatIterator`.
    virtual std::unique_ptr<BeatIterator> findBeats(
            audio::FramePos startPosition,
            audio::FramePos endPosition) const = 0;

    /// Return whether or not a beat exists between `startPosition` and `endPosition`.
    virtual bool hasBeatInRange(audio::FramePos startPosition,
            audio::FramePos endPosition) const = 0;

    /// Return the predominant BPM value between `startPosition` and `endPosition`
    /// if the BPM is valid, otherwise returns an invalid BPM value.
    virtual mixxx::Bpm getBpmInRange(audio::FramePos startPosition,
            audio::FramePos endPosition) const = 0;

    /// Return the arithmetic average BPM over the range of n*2 beats centered around
    /// frame position `position`. For example, n=4 results in an averaging of 8 beats.
    /// The returned Bpm value may be invalid.
    virtual mixxx::Bpm getBpmAroundPosition(audio::FramePos position, int n) const = 0;

    virtual audio::SampleRate getSampleRate() const = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Beat mutations
    ////////////////////////////////////////////////////////////////////////////

    /// Translate all beats in the song by `offset` frames. Beats that lie
    /// before the start of the track or after the end of the track are *not*
    /// removed.
    //
    /// Returns a pointer to the modified beats object, or `nullopt` on
    /// failure.
    virtual std::optional<BeatsPointer> tryTranslate(audio::FrameDiff_t offset) const = 0;

    /// Scale the position of every beat in the song by `scale`.
    //
    /// Returns a pointer to the modified beats object, or `nullopt` on
    /// failure.
    virtual std::optional<BeatsPointer> tryScale(BpmScale scale) const = 0;

    /// Adjust the beats so the global average BPM matches `bpm`.
    //
    /// Returns a pointer to the modified beats object, or `nullopt` on
    /// failure.
    virtual std::optional<BeatsPointer> trySetBpm(mixxx::Bpm bpm) const = 0;

  protected:
    /// Type tag for making public constructors of derived classes inaccessible.
    ///
    /// The constructors must be public for using std::make_shared().
    struct MakeSharedTag {};

    Beats() = default;

    virtual bool isValid() const = 0;

  private:
    Beats(const Beats&) = delete;
    Beats(Beats&&) = delete;
};

} // namespace mixxx
