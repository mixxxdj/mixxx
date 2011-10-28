#ifndef BEATMATRIX_H
#define BEATMATRIX_H

#include <QObject>
#include <QMutex>

#include "trackinfoobject.h"
#include "track/beats.h"

// BeatMatrix is an implementation of the Beats interface that implements a list
// of finite beats that have been extracted or otherwise annotated for a track.
class BeatMatrix : public QObject, public Beats {
    Q_OBJECT
  public:
    BeatMatrix(TrackPointer pTrack, const QByteArray* pByteArray=NULL);
    virtual ~BeatMatrix();

    // See method comments in beats.h

    virtual Beats::CapabilitiesFlags getCapabilities() const {
        return BEATSCAP_TRANSLATE | BEATSCAP_SCALE | BEATSCAP_ADDREMOVE | BEATSCAP_MOVEBEAT;
    }

    virtual QByteArray* toByteArray() const;
    virtual QString getVersion() const;

    ////////////////////////////////////////////////////////////////////////////
    // Beat calculations
    ////////////////////////////////////////////////////////////////////////////

    virtual double findNextBeat(double dSamples) const;
    virtual double findPrevBeat(double dSamples) const;
    virtual double findClosestBeat(double dSamples) const;
    virtual double findNthBeat(double dSamples, int n) const;
    virtual void findBeats(double startSample, double stopSample, QList<double>* pBeatsList) const;
    virtual bool hasBeatInRange(double startSample, double stopSample) const;
    virtual double getBpm() const;
    virtual double getBpmRange(double startSample, double stopSample) const;

    ////////////////////////////////////////////////////////////////////////////
    // Beat mutations
    ////////////////////////////////////////////////////////////////////////////

    virtual void addBeat(double dBeatSample);
    virtual void removeBeat(double dBeatSample);
    virtual void moveBeat(double dBeatSample, double dNewBeatSample);
    virtual void translate(double dNumSamples);
    virtual void scale(double dScalePercentage);

  signals:
    void updated();

  private slots:
    void slotTrackBpmUpdated(double bpm);

  private:
    void readByteArray(const QByteArray* pByteArray);
    // For internal use only.
    bool isValid() const;
    unsigned int numBeats() const;

    mutable QMutex m_mutex;
    int m_iSampleRate;
    BeatList m_beatList;
};

#endif /* BEATMATRIX_H */
