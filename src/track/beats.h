#pragma once

#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QVector>
#include <memory>

#include "proto/beats.pb.h"
#include "track/bpm.h"
#include "track/frame.h"
#include "util/types.h"

namespace mixxx {

class Beats;
using BeatsPointer = std::shared_ptr<Beats>;
using BeatList = QList<track::io::Beat>;
} // namespace mixxx

#include "track/beatiterator.h"
#include "track/timesignature.h"

class Track;

namespace mixxx {
/// Beats is a class for BPM and beat management classes.
/// It stores beats information including beats position, down beats position,
/// phrase beat position and changes in tempo.
// TODO(JVC) To make it final
// TODO(XXX): Split into 2 classes:
// - Beats: Final, no base class, no mutex, copyable/movable
// - BeatsObject: Derived from QObject, with mutex, not copyable/movable
// TODO(XXX): Remove cyclic dependency on Track object, probably not necessary
class Beats final : public QObject {
    Q_OBJECT
  public:
    /// Construct a BeatMap. iSampleRate may be provided if a more accurate
    /// sample rate is known than the one associated with the Track.
    explicit Beats(const Track* track, SINT iSampleRate = 0);
    /// Construct a BeatMap. iSampleRate may be provided if a more accurate
    /// sample rate is known than the one associated with the Track. If it is
    /// zero then the track's sample rate will be used. The BeatMap will be
    /// deserialized from the byte array.
    Beats(const Track* track, const QByteArray& byteArray, SINT iSampleRate = 0);
    /// Construct a BeatMap. iSampleRate may be provided if a more accurate
    /// sample rate is known than the one associated with the Track. If it is
    /// zero then the track's sample rate will be used. A list of beat locations
    /// in audio frames may be provided.
    Beats(const Track* track, const QVector<Frame>& beats, SINT iSampleRate = 0);
    ~Beats() override = default;

    // TODO(JVC) Is a copy constructor needed? of we can force a move logic??
    Beats(const Beats&);

    // TODO(JVC) Not needed
    enum Capabilities {
        BEATSCAP_NONE = 0x0000,
        BEATSCAP_ADDREMOVE = 0x0001, // Add or remove a single beat
        BEATSCAP_TRANSLATE = 0x0002, // Move all beat markers earlier or later
        BEATSCAP_SCALE = 0x0004,     // Scale beat distance by a fixed ratio
        BEATSCAP_MOVEBEAT = 0x0008,  // Move a single Beat
        BEATSCAP_SETBPM = 0x0010,    // Set new bpm, beat grid only
        BEATSCAP_HASBAR = 0x0020     // Manage Bar beats
    };
    typedef int CapabilitiesFlags; // Allows us to do ORing

    enum BPMScale {
        DOUBLE,
        HALVE,
        TWOTHIRDS,
        THREEFOURTHS,
        FOURTHIRDS,
        THREEHALVES,
    };

    // TODO(hacksdump): These versions are retained for backward compatibility.
    // In future, There will be no versions at all.
    static const QString BEAT_MAP_VERSION;
    static const QString BEAT_GRID_1_VERSION;
    static const QString BEAT_GRID_2_VERSION;

    // TODO(JVC) Not needed
    Beats::CapabilitiesFlags getCapabilities() const {
        return BEATSCAP_TRANSLATE | BEATSCAP_SCALE | BEATSCAP_ADDREMOVE |
                BEATSCAP_MOVEBEAT | BEATSCAP_HASBAR;
    }
    /// Serializes into a protobuf.
    QByteArray toProtobuf() const;
    BeatsPointer clone() const;

    /// Returns a string representing the version of the beat-processing code that
    /// produced this Beats instance. Used by BeatsFactory for associating a
    /// given serialization with the version that produced it.
    QString getVersion() const;
    /// Return a string that represent the preferences used to generate
    /// the beats object.
    QString getSubVersion() const;
    void setSubVersion(QString subVersion);
    bool isValid() const;
    /// Calculates the BPM between two beat positions.
    Bpm calculateBpm(const track::io::Beat& startBeat,
            const track::io::Beat& stopBeat) const;

    /// Initializes the BeatGrid to have a BPM of dBpm and the first beat offset
    /// of firstBeatFrame. Does not generate an updated() signal, since it is
    /// meant for initialization.
    void setGrid(Bpm dBpm, Frame firstBeatFrame = Frame());

    // TODO: We may want to implement these with common code that returns
    //       the triple of closest, next, and prev.

    /// Starting from frame, return the frame number of the next beat
    /// in the track, or -1 if none exists. If frame refers to the location
    /// of a beat, frame is returned.
    Frame findNextBeat(Frame frame) const;

    /// Starting from frame frame, return the frame number of the previous
    /// beat in the track, or -1 if none exists. If frame refers to the
    /// location of beat, frame is returned.
    Frame findPrevBeat(Frame frame) const;

    /// Starting from frame, fill the frame numbers of the previous beat
    /// and next beat.  Either can be -1 if none exists.  If frame refers
    /// to the location of the beat, the first value is frame, and the second
    /// value is the next beat position.  Non- -1 values are guaranteed to be
    /// even.  Returns false if *at least one* frame is -1.  (Can return false
    /// with one beat successfully filled)
    bool findPrevNextBeats(Frame frame,
            Frame* pPrevBeatFrame,
            Frame* pNextBeatFrame) const;

    /// Starting from frame, return the frame number of the closest beat
    /// in the track, or -1 if none exists.  Non- -1 values are guaranteed to be
    /// even.
    Frame findClosestBeat(Frame frame) const;

    /// Find the Nth beat from frame. Works with both positive and
    /// negative values of n. If frame refers to the location of a beat,
    /// then frame is returned. If no beat can be found, returns -1.
    Frame findNthBeat(Frame frame, int offset) const;

    int numBeatsInRange(Frame startFrame, Frame endFrame);

    /// Find the frame N beats away from frame. The number of beats may be
    /// negative and does not need to be an integer.
    Frame findNBeatsFromFrame(Frame fromFrame, double beats) const;

    /// Return an iterator to a container of Beats containing the Beats
    /// between startFrameNum and endFrameNum. THe BeatIterator must be iterated
    /// while a strong reference to the Beats object to ensure that the Beats
    /// object is not deleted. Caller takes ownership of the returned BeatsIterator
    std::unique_ptr<BeatIterator> findBeats(Frame startFrame,
            Frame stopFrame) const;

    /// Return whether or not a Beat lies between startFrameNum and endFrameNum
    bool hasBeatInRange(Frame startFrame,
            Frame stopFrame) const;

    /// Return the average BPM over the entire track if the BPM is
    /// valid, otherwise returns -1
    Bpm getBpm() const;

    /// Return the average BPM over the range from startFrameNum to endFrameNum,
    /// specified in frames if the BPM is valid, otherwise returns -1
    double getBpmRange(Frame startFrame,
            Frame stopFrame) const;

    /// Return the average BPM over the range of n*2 beats centered around
    /// curFrameNum.  (An n of 4 results in an averaging of 8 beats).  Invalid
    /// BPM returns -1.
    Bpm getBpmAroundPosition(Frame curFrame, int n) const;

    /// Sets the track signature at the nearest frame
    void setSignature(TimeSignature signature, Frame frame = Frame());

    /// Return the track signature at the given frame position
    TimeSignature getSignature(Frame frame = Frame()) const;

    /// Sets the nearest beat as a bar beat
    void setDownBeat(Frame frame = Frame());

    /// Add a beat at location frame. Beats instance must have the
    /// capability BEATSCAP_ADDREMOVE.
    void addBeat(Frame frame);

    /// Remove a beat at location frame. Beats instance must have the
    /// capability BEATSCAP_ADDREMOVE.
    void removeBeat(Frame frame);

    /// Translate all beats in the song by numFrames. Beats that lie
    /// before the start of the track or after the end of the track are not
    /// removed. Beats instance must have the capability BEATSCAP_TRANSLATE.
    void translate(Frame numFrames);

    /// Scale the position of every beat in the song by dScalePercentage. Beats
    /// class must have the capability BEATSCAP_SCALE.
    void scale(enum BPMScale scale);

    /// Adjust the beats so the global average BPM matches dBpm. Beats class must
    /// have the capability BEATSCAP_SET.
    void setBpm(Bpm dBpm);

    /// Returns the number of beats
    inline int size() {
        return m_beats.size();
    }

    /// Prints debuging information in stderr
    void printDebugInfo() const;
    /// Returns the frame number for the first beat, -1 is no beats
    Frame getFirstBeatPosition() const;
    /// Returns the frame number for the last beat, -1 if no beats
    Frame getLastBeatPosition() const;
    /// Return the sample rate
    SINT getSampleRate() const {
        return m_iSampleRate;
    }

    /// Prints debuging information in stderr
    friend QDebug operator<<(QDebug dbg, const BeatsPointer& arg);

  private:
    void onBeatlistChanged();
    void scaleDouble();
    void scaleTriple();
    void scaleQuadruple();
    void scaleHalve();
    void scaleThird();
    void scaleFourth();

    mutable QMutex m_mutex;
    const Track* m_track;
    QString m_subVersion;
    SINT m_iSampleRate;
    Bpm m_dCachedBpm;
    BeatList m_beats;

  signals:
    void updated();
};

} // namespace mixxx
