/*
 * beatmap.h
 *
 *  Created on: 08/dic/2011
 *      Author: vittorio
 */

#ifndef BEATMAP_H_
#define BEATMAP_H_

#include <QMutex>

#include "track/track.h"
#include "track/beats.h"
#include "proto/beats.pb.h"

#define BEAT_MAP_VERSION "BeatMap-1.0"

typedef QList<mixxx::track::io::Beat> BeatList;

class BeatMap final : public Beats {
  public:
    // Construct a BeatMap. iSampleRate may be provided if a more accurate
    // sample rate is known than the one associated with the Track.
    BeatMap(const Track& track, SINT iSampleRate);
    // Construct a BeatMap. iSampleRate may be provided if a more accurate
    // sample rate is known than the one associated with the Track. If it is
    // zero then the track's sample rate will be used. The BeatMap will be
    // deserialized from the byte array.
    BeatMap(const Track& track, SINT iSampleRate,
            const QByteArray& byteArray);
    // Construct a BeatMap. iSampleRate may be provided if a more accurate
    // sample rate is known than the one associated with the Track. If it is
    // zero then the track's sample rate will be used. A list of beat locations
    // in audio frames may be provided.
    BeatMap(const Track& track, SINT iSampleRate,
            const QVector<double>& beats);

    ~BeatMap() override = default;

    // See method comments in beats.h

    virtual Beats::CapabilitiesFlags getCapabilities() const {
        return BEATSCAP_TRANSLATE | BEATSCAP_SCALE | BEATSCAP_ADDREMOVE |
                BEATSCAP_MOVEBEAT;
    }

    virtual QByteArray toByteArray() const;
    BeatsPointer clone() const;
    virtual QString getVersion() const;
    virtual QString getSubVersion() const;
    virtual void setSubVersion(QString subVersion);

    ////////////////////////////////////////////////////////////////////////////
    // Beat calculations
    ////////////////////////////////////////////////////////////////////////////

    virtual double findNextBeat(double dSamples) const;
    virtual double findPrevBeat(double dSamples) const;
    virtual bool findPrevNextBeats(double dSamples,
                                   double* dpPrevBeatSamples,
                                   double* dpNextBeatSamples) const;
    virtual double findClosestBeat(double dSamples) const;
    virtual double findNthBeat(double dSamples, int n) const;
    virtual std::unique_ptr<BeatIterator> findBeats(double startSample, double stopSample) const;
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
    virtual void scale(enum BPMScale scale);
    virtual void setBpm(double dBpm);

  private:
    BeatMap(const BeatMap& other);
    bool readByteArray(const QByteArray& byteArray);
    void createFromBeatVector(const QVector<double>& beats);
    void onBeatlistChanged();

    double calculateBpm(const mixxx::track::io::Beat& startBeat,
                        const mixxx::track::io::Beat& stopBeat) const;
    // For internal use only.
    bool isValid() const;

    void scaleDouble();
    void scaleTriple();
    void scaleQuadruple();
    void scaleHalve();
    void scaleThird();
    void scaleFourth();

    mutable QMutex m_mutex;
    QString m_subVersion;
    SINT m_iSampleRate;
    double m_dCachedBpm;
    double m_dLastFrame;
    BeatList m_beats;
};

#endif /* BEATMAP_H_ */
