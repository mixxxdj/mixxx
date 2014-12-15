#ifndef BEATGRID_H
#define BEATGRID_H

#include <QMutex>
#include <QObject>

#include "trackinfoobject.h"
#include "track/beats.h"
#include "proto/beats.pb.h"

#define BEAT_GRID_1_VERSION "BeatGrid-1.0"
#define BEAT_GRID_2_VERSION "BeatGrid-2.0"

// BeatGrid is an implementation of the Beats interface that implements an
// infinite grid of beats, aligned to a song simply by a starting offset of the
// first beat and the song's average beats-per-minute.
class BeatGrid : public QObject, public virtual Beats {
    Q_OBJECT
  public:
    // Construct a BeatGrid. If a more accurate sample rate is known, provide it
    // in the iSampleRate parameter -- otherwise pass 0. If pByteArray is
    // non-NULL, the BeatGrid will be deserialized from the byte array.
    BeatGrid(TrackInfoObject* pTrack, int iSampleRate,
             const QByteArray* pByteArray=NULL);
    virtual ~BeatGrid();

    // Initializes the BeatGrid to have a BPM of dBpm and the first beat offset
    // of dFirstBeatSample. Does not generate an updated() signal, since it is
    // meant for initialization.
    void setGrid(double dBpm, double dFirstBeatSample);

    // The following are all methods from the Beats interface, see method
    // comments in beats.h

    virtual Beats::CapabilitiesFlags getCapabilities() const {
        return BEATSCAP_TRANSLATE | BEATSCAP_SCALE | BEATSCAP_SET;
    }

    virtual QByteArray* toByteArray() const;
    virtual QString getVersion() const;
    virtual QString getSubVersion() const;
    virtual void setSubVersion(QString subVersion);

    ////////////////////////////////////////////////////////////////////////////
    // Beat calculations
    ////////////////////////////////////////////////////////////////////////////

    virtual double findNextBeat(double dSamples) const;
    virtual double findPrevBeat(double dSamples) const;
    virtual double findClosestBeat(double dSamples) const;
    virtual double findNthBeat(double dSamples, int n) const;
    virtual BeatIterator* findBeats(double startSample, double stopSample) const;
    virtual bool hasBeatInRange(double startSample, double stopSample) const;
    virtual double getBpm() const;
    virtual double getBpmRange(double startSample, double stopSample) const;
    virtual double getBpmAroundPosition(double curSample, int n) const;

    ////////////////////////////////////////////////////////////////////////////
    // Beat mutations
    ////////////////////////////////////////////////////////////////////////////

    virtual void addBeat(double dBeatSample);
    virtual void removeBeat(double dBeatSample);
    virtual void moveBeat(double dBeatSample, double dNewBeatSample);
    virtual void translate(double dNumSamples);
    virtual void scale(double dScalePercentage);
    virtual void setBpm(double dBpm);

  signals:
    void updated();

  private:
    double firstBeatSample() const;
    double bpm() const;

    void readByteArray(const QByteArray* pByteArray);
    // For internal use only.
    bool isValid() const;

    mutable QMutex m_mutex;
    // The sub-version of this beatgrid.
    QString m_subVersion;
    // The number of samples per second
    int m_iSampleRate;
    // Data storage for BeatGrid
    mixxx::track::io::BeatGrid m_grid;
    // The length of a beat in samples
    double m_dBeatLength;
};


#endif /* BEATGRID_H */
