#pragma once

#include <QByteArray>
#include <QList>
#include <QSharedPointer>
#include <QString>

#include "audio/frame.h"
#include "audio/types.h"
#include "track/bpm.h"
#include "util/memory.h"
#include "util/types.h"

namespace mixxx {

class Beats;
typedef QSharedPointer<Beats> BeatsPointer;

class BeatIterator {
  public:
    virtual ~BeatIterator() = default;
    virtual bool hasNext() const = 0;
    virtual audio::FramePos next() = 0;
};

/// Beats is the base class for BPM and beat management classes. It provides a
/// specification of all methods a beat-manager class must provide, as well as
/// a capability model for representing optional features.
class Beats {
  public:
    virtual ~Beats() = default;

    enum Capabilities {
        BEATSCAP_NONE = 0x0000,
        /// Add or remove a single beat
        BEATSCAP_ADDREMOVE = 0x0001,
        /// Move all beat markers earlier or later
        BEATSCAP_TRANSLATE = 0x0002,
        /// Scale beat distance by a fixed ratio
        BEATSCAP_SCALE = 0x0004,
        /// Move a single Beat
        BEATSCAP_MOVEBEAT = 0x0008,
        /// Set new bpm, beat grid only
        BEATSCAP_SETBPM = 0x0010
    };
    /// Allows us to do ORing
    typedef int CapabilitiesFlags;

    enum class BpmScale {
        Double,
        Halve,
        TwoThirds,
        ThreeFourths,
        FourThirds,
        ThreeHalves,
    };

    /// Retrieve the capabilities supported by the beats implementation.
    virtual Beats::CapabilitiesFlags getCapabilities() const = 0;

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
    virtual audio::FramePos findNextBeat(audio::FramePos position) const = 0;

    /// Starting from frame position `position`, return the frame position of
    /// the previous beat in the track, or an invalid position if none exists.
    /// If `position` refers to the location of beat, `position` is returned.
    virtual audio::FramePos findPrevBeat(audio::FramePos position) const = 0;

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
    /// the closest beat in the track, or an invalid positon if none exists.
    virtual audio::FramePos findClosestBeat(audio::FramePos position) const = 0;

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

    /// Return the average BPM over the entire track if the BPM is valid,
    /// otherwise returns -1
    virtual mixxx::Bpm getBpm() const = 0;

    /// Return the average BPM over the range of n*2 beats centered around
    /// frame position `position`. For example, n=4 results in an averaging of 8 beats.
    /// The returned Bpm value may be invalid.
    virtual mixxx::Bpm getBpmAroundPosition(audio::FramePos position, int n) const = 0;

    virtual audio::SampleRate getSampleRate() const = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Beat mutations
    ////////////////////////////////////////////////////////////////////////////

    /// Translate all beats in the song by `offset` frames. Beats that lie
    /// before the start of the track or after the end of the track are *not*
    /// removed. The `Beats` instance must have the capability
    /// `BEATSCAP_TRANSLATE`.
    virtual BeatsPointer translate(audio::FrameDiff_t offset) const = 0;

    /// Scale the position of every beat in the song by `scale`. The `Beats`
    /// class must have the capability `BEATSCAP_SCALE`.
    virtual BeatsPointer scale(BpmScale scale) const = 0;

    /// Adjust the beats so the global average BPM matches `bpm`. The `Beats`
    /// class must have the capability `BEATSCAP_SET`.
    virtual BeatsPointer setBpm(mixxx::Bpm bpm) = 0;
};

} // namespace mixxx
