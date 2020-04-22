#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "proto/beats.pb.h"
#include "util/logger.h"
#include "util/memory.h"
#include "util/types.h"

namespace mixxx {
class Beats;
using BeatsPointer = std::shared_ptr<Beats>;
using BeatList = QList<track::io::Beat>;
using SamplePos = double;
using FrameNum = double;
} // namespace mixxx

#define BEAT_MAP_VERSION "BeatMap-1.0"
// TODO(JVC) This two are deprecated
#define BEAT_GRID_1_VERSION "BeatGrid-1.0"
#define BEAT_GRID_2_VERSION "BeatGrid-2.0"

#include "track/beatiterator.h"
#include "track/timesignature.h"
#include "track/track.h"

namespace {
}

namespace mixxx {
/// Beats is a class for BPM and beat management classes.
/// It stores beats information including beats position, down beats position,
/// phrase beat position and changes in tempo.
// TODO(JVC) To make it final
class Beats : public QObject {
    Q_OBJECT
  public:
    /// Construct a BeatMap. iSampleRate may be provided if a more accurate
    /// sample rate is known than the one associated with the Track.
    Beats(const Track* track, SINT iSampleRate = 0);
    /// Construct a BeatMap. iSampleRate may be provided if a more accurate
    /// sample rate is known than the one associated with the Track. If it is
    /// zero then the track's sample rate will be used. The BeatMap will be
    /// deserialized from the byte array.
    Beats(const Track* track, const QByteArray& byteArray, SINT iSampleRate = 0);
    /// Construct a BeatMap. iSampleRate may be provided if a more accurate
    /// sample rate is known than the one associated with the Track. If it is
    /// zero then the track's sample rate will be used. A list of beat locations
    /// in audio frames may be provided.
    Beats(const Track* track, const QVector<double>& beats, SINT iSampleRate = 0);

    virtual ~Beats() {
    }

    // TODO(JVC) Is a copy constructor needed? of we can force a move logic??
    Beats(const Beats&);

    /// Populate a Beats with a vector of beat positions.
    void createFromBeatVector(const QVector<double>& beats);
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

    // TODO(JVC) Not needed
    Beats::CapabilitiesFlags getCapabilities() const {
        return BEATSCAP_TRANSLATE | BEATSCAP_SCALE | BEATSCAP_ADDREMOVE |
                BEATSCAP_MOVEBEAT | BEATSCAP_HASBAR;
    }
    /// Serializes into a protobuff.
    virtual QByteArray toByteArray() const;
    virtual BeatsPointer clone() const;

    /// Returns a string representing the version of the beat-processing code that
    /// produced this Beats instance. Used by BeatsFactory for associating a
    /// given serialization with the version that produced it.
    virtual QString getVersion() const;
    /// Return a string that represent the preferences used to generate
    /// the beats object.
    virtual QString getSubVersion() const;
    virtual void setSubVersion(QString subVersion);
    bool isValid() const;
    /// Calculates the BPM between two beat positions.
    double calculateBpm(const track::io::Beat& startBeat,
            const track::io::Beat& stopBeat) const;

    /// Initializes the BeatGrid to have a BPM of dBpm and the first beat offset
    /// of dFirstBeatSample. Does not generate an updated() signal, since it is
    /// meant for initialization.
    void setGrid(double dBpm, double sample = 0) {
        setGridNew(dBpm, sample / 2.0);
    }
    void setGridNew(double dBpm, FrameNum dFirstBeatFrame = 0);

    // TODO: We may want to implement these with common code that returns
    //       the triple of closest, next, and prev.

    /// Starting from frame, return the frame number of the next beat
    /// in the track, or -1 if none exists. If frame refers to the location
    /// of a beat, frame is returned.
    virtual double findNextBeat(double sample) const {
        return findNextBeatNew(sample / 2.0) * 2.0;
    }
    virtual FrameNum findNextBeatNew(FrameNum frame) const;

    /// Starting from frame frame, return the frame number of the previous
    /// beat in the track, or -1 if none exists. If frame refers to the
    /// location of beat, frame is returned.
    virtual double findPrevBeat(FrameNum frame) const {
        return findPrevBeatNew(frame / 2.0) * 2.0;
    }
    virtual FrameNum findPrevBeatNew(FrameNum frame) const;

    /// Starting from frame, fill the frame numbers of the previous beat
    /// and next beat.  Either can be -1 if none exists.  If frame refers
    /// to the location of the beat, the first value is frame, and the second
    /// value is the next beat position.  Non- -1 values are guaranteed to be
    /// even.  Returns false if *at least one* sample is -1.  (Can return false
    /// with one beat successfully filled)
    virtual bool findPrevNextBeats(double sample,
            double* pPrevSample,
            double* pNextSample) const {
        FrameNum prev, next;
        bool result;

        result = findPrevNextBeatsNew(sample / 2.0, &prev, &next);
        *pPrevSample = prev * 2.0;
        *pNextSample = next * 2.0;
        return result;
    }
    virtual bool findPrevNextBeatsNew(FrameNum frame,
            FrameNum* pPrevBeatFrame,
            FrameNum* pNextBeatFrame) const;

    /// Starting from frame, return the frame number of the closest beat
    /// in the track, or -1 if none exists.  Non- -1 values are guaranteed to be
    /// even.
    virtual double findClosestBeat(double sample) const {
        return findClosestBeatNew(sample / 2.0) * 2.0;
    }
    virtual FrameNum findClosestBeatNew(FrameNum frame) const;

    /// Find the Nth beat from frame. Works with both positive and
    /// negative values of n. If frame refers to the location of a beat,
    /// then frame is returned. If no beat can be found, returns -1.
    virtual double findNthBeat(double sample, int offset) const {
        return findNthBeatNew(sample / 2.0, offset) * 2.0;
    }
    virtual FrameNum findNthBeatNew(FrameNum frame, int offset) const;

    int numBeatsInRange(double startSampleNum, double endSampleNum) {
        return numBeatsInRangeNew(startSampleNum / 2.0, endSampleNum / 2.0);
    }
    int numBeatsInRangeNew(FrameNum startFrameNum, FrameNum endFrameNum);

    /// Find the frame N beats away from frame. The number of beats may be
    /// negative and does not need to be an integer.
    double findNBeatsFromSample(double sample, double beats) const {
        return findNBeatsFromSampleNew(sample / 2.0, beats) * 2.0;
    }
    FrameNum findNBeatsFromSampleNew(FrameNum frame, double beats) const;

    /// Return an iterator to a container of Beats containing the Beats
    /// between startFrameNum and endFrameNum. THe BeatIterator must be iterated
    /// while a strong reference to the Beats object to ensure that the Beats
    /// object is not deleted. Caller takes ownership of the returned BeatsIterator
    virtual std::unique_ptr<BeatIterator> findBeats(double startSampleNum,
            double stopSampleNum) const {
        return findBeatsNew(startSampleNum / 2.0, stopSampleNum / 2.0);
    }
    virtual std::unique_ptr<BeatIterator> findBeatsNew(FrameNum startFrameNum,
            FrameNum stopFrameNum) const;

    /// Return whether or not a Beat lies between startFrameNum and endFrameNum
    virtual bool hasBeatInRange(double startSampleNum,
            double stopSampleNum) const {
        return hasBeatInRangeNew(startSampleNum / 2.0, stopSampleNum / 2.0);
    }
    virtual bool hasBeatInRangeNew(FrameNum startFrameNum,
            FrameNum stopFrameNum) const;

    /// Return the average BPM over the entire track if the BPM is
    /// valid, otherwise returns -1
    virtual double getBpm() const {
        return getBpmNew();
    }
    virtual double getBpmNew() const;

    /// Return the average BPM over the range from startFrameNum to endFrameNum,
    /// specified in frames if the BPM is valid, otherwise returns -1
    virtual double getBpmRange(double startSampleNum,
            FrameNum stopSampleNum) const {
        return getBpmRangeNew(startSampleNum / 2.0, stopSampleNum / 2.0);
    }
    virtual double getBpmRangeNew(FrameNum startFrameNum,
            FrameNum stopFrameNum) const;

    /// Return the average BPM over the range of n*2 beats centered around
    /// curFrameNum.  (An n of 4 results in an averaging of 8 beats).  Invalid
    /// BPM returns -1.
    virtual double getBpmAroundPosition(double curSampleNum, int n) const {
        return getBpmAroundPositionNew(curSampleNum / 2.0, n);
    }
    virtual double getBpmAroundPositionNew(FrameNum curFrameNum, int n) const;

    virtual double getMaxBpm() const {
        // TODO(JVC) Why is returning a constant? MaxBpm must be taken from somewhere
        constexpr double kMaxBpm = 500;
        return kMaxBpm;
    }

    /// Sets the track signature at the nearest frame
    virtual void setSignature(TimeSignature signature, double sample = 0) {
        setSignatureNew(signature, sample / 2.0);
    }
    virtual void setSignatureNew(TimeSignature signature, FrameNum frame = 0);

    /// Return the track signature at the given frame position
    virtual TimeSignature getSignature(double sample = 0) const {
        return getSignatureNew(sample / 2.0);
    }
    virtual TimeSignature getSignatureNew(FrameNum frame = 0) const;

    /// Sets the nearest beat as a bar beat
    virtual void setDownBeat(double sample = 0) {
        setDownBeatNew(sample / 2.0);
    }
    virtual void setDownBeatNew(FrameNum frame = 0);

    /// Add a beat at location frame. Beats instance must have the
    /// capability BEATSCAP_ADDREMOVE.
    virtual void addBeat(double sample) {
        addBeatNew(sample / 2.0);
    }
    virtual void addBeatNew(FrameNum frame);

    /// Remove a beat at location frame. Beats instance must have the
    /// capability BEATSCAP_ADDREMOVE.
    virtual void removeBeat(double sample) {
        removeBeatNew(sample / 2.0);
    }
    virtual void removeBeatNew(FrameNum frame);

    // TODO(JVC) Do we want to move the beats a number of frames??
    /// Translate all beats in the song by dNumSamples samples. Beats that lie
    /// before the start of the track or after the end of the track are not
    /// removed. Beats instance must have the capability BEATSCAP_TRANSLATE.
    virtual void translate(double dNumSamples);

    /// Scale the position of every beat in the song by dScalePercentage. Beats
    /// class must have the capability BEATSCAP_SCALE.
    virtual void scale(enum BPMScale scale);

    /// Adjust the beats so the global average BPM matches dBpm. Beats class must
    /// have the capability BEATSCAP_SET.
    virtual void setBpm(double dBpm);

    /// Returns the number of beats
    inline int size() {
        return m_beats.size();
    }

    /// Prints debuging information in stderr
    void printDebugInfo() const;
    /// Returns the frame number for the first beat, -1 is no beats
    FrameNum getFirstBeatPosition() const;
    /// Returns the frame number for the last beat, -1 if no beats
    FrameNum getLastBeatPosition() const;

    /// Prints debuging information in stderr
    friend QDebug operator<<(QDebug dbg, const BeatsPointer& arg);

  private:
    void onBeatlistChanged();
    bool readByteArray(const QByteArray& byteArray);
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
    double m_dCachedBpm;
    FrameNum m_dLastFrame;
    BeatList m_beats;
    Logger m_logger;

    virtual SINT getSampleRate() const = 0;

  signals:
    void updated();
};

} // namespace mixxx
