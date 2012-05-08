#ifndef BEATS_H
#define BEATS_H

#include <QString>
#include <QList>
#include <QByteArray>
#include <QSharedPointer>

class Beats;
typedef QSharedPointer<Beats> BeatsPointer;

class BeatIterator {
  public:
    virtual bool hasNext() const = 0;
    virtual double next() = 0;
};

// Beats is a pure abstract base class for BPM and beat management classes. It
// provides a specification of all methods a beat-manager class must provide, as
// well as a capability model for representing optional features.
class Beats {
  public:
    Beats() { }
    virtual ~Beats() { }

    enum Capabilities {
        BEATSCAP_NONE          = 0x0000,
        BEATSCAP_ADDREMOVE     = 0x0001,
        BEATSCAP_TRANSLATE     = 0x0002,
        BEATSCAP_SCALE         = 0x0004,
        BEATSCAP_MOVEBEAT      = 0x0008,
        BEATSCAP_SET           = 0x0010
    };
    typedef int CapabilitiesFlags; // Allows us to do ORing

    virtual Beats::CapabilitiesFlags getCapabilities() const = 0;

    // Serialization
    virtual QByteArray* toByteArray() const = 0;

    // A string representing the version of the beat-processing code that
    // produced this Beats instance. Used by BeatsFactory for associating a
    // given serialization with the version that produced it.
    virtual QString getVersion() const = 0;
    // A sub-version can be used to represent the preferences used to generate
    // the beats object.
    virtual QString getSubVersion() const = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Beat calculations
    ////////////////////////////////////////////////////////////////////////////

    // Starting from sample dSamples, return the sample of the next beat in the
    // track, or -1 if none exists. If dSamples refers to the location of a
    // beat, dSamples is returned.
    virtual double findNextBeat(double dSamples) const = 0;

    // Starting from sample dSamples, return the sample of the previous beat in
    // the track, or -1 if none exists. If dSamples refers to the location of
    // beat, dSamples is returned.
    virtual double findPrevBeat(double dSamples) const = 0;

    // Starting from sample dSamples, return the sample of the closest beat in
    // the track, or -1 if none exists.
    virtual double findClosestBeat(double dSamples) const = 0;

    // Find the Nth beat from sample dSamples. Works with both positive and
    // negative values of n. Calling findNthBeat with n=0 is invalid. Calling
    // findNthBeat with n=1 or n=-1 is equivalent to calling findNextBeat and
    // findPrevBeat, respectively. If dSamples refers to the location of a beat,
    // then dSamples is returned. If no beat can be found, returns -1.
    virtual double findNthBeat(double dSamples, int n) const = 0;

    // Adds to pBeatsList the position in samples of every beat occuring between
    // startPosition and endPosition. BeatIterator must be iterated while
    // holding a strong references to the Beats object to ensure that the Beats
    // object is not deleted. Caller takes ownership of the returned BeatIterator;
    virtual BeatIterator* findBeats(double startSample, double stopSample) const = 0;

    // Return whether or not a sample lies between startPosition and endPosition
    virtual bool hasBeatInRange(double startSample, double stopSample) const = 0;

    // Return the average BPM over the entire track if the BPM is
    // valid, otherwise returns -1
    virtual double getBpm() const = 0;

    // Return the average BPM over the range from startSample to endSample,
    // specified in samples if the BPM is valid, otherwise returns -1
    virtual double getBpmRange(double startSample, double stopSample) const = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Beat mutations
    ////////////////////////////////////////////////////////////////////////////

    // Add a beat at location dBeatSample. Beats instance must have the
    // capability BEATSCAP_ADDREMOVE.
    virtual void addBeat(double dBeatSample) = 0;

    // Remove a beat at location dBeatSample. Beats instance must have the
    // capability BEATSCAP_ADDREMOVE.
    virtual void removeBeat(double dBeatSample) = 0;

    // Translate all beats in the song by dNumSamples samples. Beats that lie
    // before the start of the track or after the end of the track are not
    // removed. Beats instance must have the capability BEATSCAP_TRANSLATE.
    virtual void translate(double dNumSamples) = 0;

    // Scale the position of every beat in the song by dScalePercentage. Beats
    // class must have the capability BEATSCAP_SCALE.
    virtual void scale(double dScalePercentage) = 0;

    // Adjust the beats so the global average BPM matches dBpm. Beats class must
    // have the capability BEATSCAP_SET.
    virtual void setBpm(double dBpm) = 0;
};

#endif /* BEATS_H */
