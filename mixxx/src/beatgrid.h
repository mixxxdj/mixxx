#ifndef BEATGRID_H
#define BEATGRID_H

#include <QByteArray>
#include <QString>
#include <QMutex>
#include <QObject>

#include "trackinfoobject.h"
#include "beats.h"

class BeatGrid : public QObject, public virtual Beats {
    Q_OBJECT
  public:
    BeatGrid(QObject* pParent, TrackPointer pTrack, QByteArray* pByteArray=NULL);
    virtual ~BeatGrid();

    // See method comments in beast.h

    virtual Beats::CapabilitiesFlags getCapabilities() const {
        return BEATSCAP_TRANSLATE | BEATSCAP_SCALE;
    }

    virtual QByteArray* toByteArray() const;
    virtual QString getVersion() const;

    ////////////////////////////////////////////////////////////////////////////
    // Beat calculations
    ////////////////////////////////////////////////////////////////////////////

    double findNextBeat(double dSamples) const;
    double findPrevBeat(double dSamples) const;
    double findClosestBeat(double dSamples) const;
    void findBeats(double startSample, double stopSample, QList<double>* pBeatsList) const;
    bool hasBeatInRange(double startSample, double stopSample) const;
    double getBpm() const;
    double getBpmRange(double startSample, double stopSample) const;

    ////////////////////////////////////////////////////////////////////////////
    // Beat mutations
    ////////////////////////////////////////////////////////////////////////////

    virtual void addBeat(double dBeatSample);
    virtual void removeBeat(double dBeatSample);
    virtual void moveBeat(double dBeatSample, double dNewBeatSample);
    virtual void translate(double dNumSamples);
    virtual void scale(double dScalePercentage);

  signals:
    void bpmUpdated(double);

  private slots:
    void slotTrackBpmUpdated(double bpm);

  private:
    void readByteArray(QByteArray* pByteArray);
    bool isValid() const;

    mutable QMutex m_mutex;
    int m_iSampleRate;
    double m_dBpm, m_dFirstBeat;
    double m_dBeatLength;
};


#endif /* BEATGRID_H */
