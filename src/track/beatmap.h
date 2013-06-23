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
#include "proto/beats.pb.h"

#define BEAT_MAP_VERSION "BeatMap-1.0"

typedef QList<mixxx::track::io::Beat> BeatList;

class BeatMap : public QObject, public Beats {
    Q_OBJECT
  public:
    BeatMap(TrackPointer pTrack, const QByteArray* pByteArray=NULL);
    // Construct a BeatMap, optionally providing a list of beat locations in
    // audio frames.
    BeatMap(TrackPointer pTrack, const QVector<double> beats = QVector<double>());
    virtual ~BeatMap();

    // See method comments in beats.h

    virtual Beats::CapabilitiesFlags getCapabilities() const {
        return BEATSCAP_TRANSLATE | BEATSCAP_SCALE | BEATSCAP_ADDREMOVE |
                BEATSCAP_MOVEBEAT | BEATSCAP_SET;
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
    void initialize(TrackPointer pTrack);
    void readByteArray(const QByteArray* pByteArray);
    void createFromBeatVector(QVector<double> beats);
    void onBeatlistChanged();

    double calculateBpm(const mixxx::track::io::Beat& startBeat,
                        const mixxx::track::io::Beat& stopBeat) const;
    // For internal use only.
    bool isValid() const;

    mutable QMutex m_mutex;
    QString m_subVersion;
    double m_dCachedBpm;
    int m_iSampleRate;
    double m_dLastFrame;
    BeatList m_beats;
};

#endif /* BEATMAP_H_ */
