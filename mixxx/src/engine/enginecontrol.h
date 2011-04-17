// enginecontrol.h
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef ENGINECONTROL_H
#define ENGINECONTROL_H

#include <QObject>
#include <QList>

#include "configobject.h"
#include "trackinfoobject.h"

class EngineBuffer;
struct Hint;

const double kNoTrigger = -1;

/**
 * EngineControl is an abstract base class for objects which implement
 * functionality pertaining to EngineBuffer. An EngineControl is meant to be a
 * succinct implementation of a given EngineBuffer feature. Previously,
 * EngineBuffer was an example of feature creep -- this is the result.
 *
 * When writing an EngineControl class, the following two properties hold:
 *
 * EngineControl::process will only be called during an EngineBuffer::process
 * callback from the sound engine. This implies that any ControlObject accesses
 * made in either of these methods are mutually exclusive, since one is
 * exclusively in the call graph of the other.
 *
 * Furthermore, most slots that are connected to Mixxx ControlObjects are safe
 * to process without locking EngineBuffer/EngineControl owned values, since
 * Mixxx only synchronizes ControlObjects in the SoundManager callback, and thus
 * EngineBuffer::process (and correspondingly, EngineControl::process) are
 * mutually exclusive to any ControlObject related slots that fire on an
 * EngineControl. This is not true in every case, so when in doubt, ask a core
 * developer.
 */
class EngineControl : public QObject {
    Q_OBJECT
  public:
    EngineControl(const char * _group,
                  ConfigObject<ConfigValue> * _config);
    virtual ~EngineControl();

    // Called by EngineBuffer::process every latency period. See the above
    // comments for information about guarantees that hold during this call. An
    // EngineControl can perform any upkeep operations that are necessary during
    // this call. If the EngineControl would like to request the playback
    // position to be altered, it should return the sample to seek to from this
    // method. Otherwise it should return kNoTrigger.
    virtual double process(const double dRate,
                           const double dCurrentSample,
                           const double dTotalSamples,
                           const int iBufferSize);

    virtual double nextTrigger(const double dRate,
                               const double dCurrentSample,
                               const double dTotalSamples,
                               const int iBufferSize);

    virtual double getTrigger(const double dRate,
                              const double dCurrentSample,
                              const double dTotalSamples,
                              const int iBufferSize);

    // hintReader allows the EngineControl to provide hints to the reader to
    // indicate that the given portion of a song is a potential imminent seek
    // target.
    virtual void hintReader(QList<Hint>& hintList);

    void setOtherEngineBuffer(EngineBuffer* pOtherEngineBuffer);
    void setCurrentSample(const double dCurrentSample, const double dTotalSamples);
    double getCurrentSample() const;
    double getTotalSamples() const;

    // Called whenever a seek occurs to allow the EngineControl to respond.
    virtual void notifySeek(double dNewPlaypo);

  public slots:
    virtual void trackLoaded(TrackPointer pTrack);
    virtual void trackUnloaded(TrackPointer pTrack);

  signals:
    void seek(double fractionalPosition);
    void seekAbs(double sample);

  protected:
    const char* getGroup();
    ConfigObject<ConfigValue>* getConfig();
    EngineBuffer* getOtherEngineBuffer();

  private:
    const char* m_pGroup;
    ConfigObject<ConfigValue>* m_pConfig;
    double m_dCurrentSample;
    double m_dTotalSamples;
    EngineBuffer* m_pOtherEngineBuffer;
};

#endif /* ENGINECONTROL_H */
