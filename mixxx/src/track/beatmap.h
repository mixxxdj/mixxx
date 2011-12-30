/*
 * beatmap.h
 *
 *  Created on: 08/dic/2011
 *      Author: vittorio
 */

#ifndef BEATMAP_H_
#define BEATMAP_H_

#include <QObject>
#include <QMutex>

#include "trackinfoobject.h"
#include "track/beats.h"



struct SignedBeat{

    double position;
    bool isOn;
    operator double() const
      {
        return position;
      }
};

typedef QList<SignedBeat> SignedBeatList;

class BeatMap : public QObject, public Beats {
    Q_OBJECT
  public:
    BeatMap(TrackPointer pTrack, const QByteArray* pByteArray=NULL);
    virtual ~BeatMap();

    // See method comments in beats.h

    virtual Beats::CapabilitiesFlags getCapabilities() const {
        return BEATSCAP_TRANSLATE | BEATSCAP_SCALE | BEATSCAP_ADDREMOVE | BEATSCAP_MOVEBEAT;
    }

    virtual QByteArray* toByteArray() const;
    virtual QString getVersion() const;
    virtual void createFromVector(QVector <double> beats);

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
    void slotTrackBpmPluginKey(QString ver);

  private:
    void readByteArray(const QByteArray* pByteArray);
    double calculateBpm(SignedBeat startBeat, SignedBeat stopBeat) const;
    // For internal use only.
    bool isValid() const;


    mutable QMutex m_mutex;
    int m_iSampleRate;
    double m_dLastFrame;
    QString m_ssubVer;
    SignedBeatList m_signedBeatList;
};

#endif /* BEATMAP_H_ */
