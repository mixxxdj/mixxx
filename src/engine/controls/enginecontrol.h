// enginecontrol.h
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef ENGINECONTROL_H
#define ENGINECONTROL_H

#include <QObject>
#include <QList>

#include <gtest/gtest_prod.h>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "control/controlvalue.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/cachingreader/cachingreader.h"

class EngineMaster;
class EngineBuffer;

const int kNoTrigger = -1;

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
 */
class EngineControl : public QObject {
    Q_OBJECT
  public:
    EngineControl(QString group,
                  UserSettingsPointer pConfig);
    ~EngineControl() override;

    // Called by EngineBuffer::process every latency period. See the above
    // comments for information about guarantees that hold during this call. An
    // EngineControl can perform any upkeep operations that are necessary during
    // this call.
    virtual void process(const double dRate,
                         const double dCurrentSample,
                         const int iBufferSize);

    // hintReader allows the EngineControl to provide hints to the reader to
    // indicate that the given portion of a song is a potential imminent seek
    // target.
    virtual void hintReader(HintVector* pHintList);

    virtual void setEngineMaster(EngineMaster* pEngineMaster);
    void setEngineBuffer(EngineBuffer* pEngineBuffer);
    virtual void setCurrentSample(const double dCurrentSample,
            const double dTotalSamples, const double dTrackSampleRate);
    QString getGroup() const;

    // Called to collect player features for effects processing.
    virtual void collectFeatureState(GroupFeatureState* pGroupFeatures) const {
        Q_UNUSED(pGroupFeatures);
    }

    // Called whenever a seek occurs to allow the EngineControl to respond.
    virtual void notifySeek(double dNewPlaypos);
    virtual void trackLoaded(TrackPointer pNewTrack);
    virtual void trackBeatsUpdated(mixxx::BeatsPointer pBeats);

  protected:
    struct SampleOfTrack {
        double current;
        double total;
        double rate;
    };

    SampleOfTrack getSampleOfTrack() const {
        return m_sampleOfTrack.getValue();
    }
    void seek(double fractionalPosition);
    void seekAbs(double sample);
    // Seek to an exact sample and don't allow quantizing adjustment.
    void seekExact(double sample);
    EngineBuffer* pickSyncTarget();

    UserSettingsPointer getConfig();
    EngineMaster* getEngineMaster();
    EngineBuffer* getEngineBuffer();

    QString m_group;
    UserSettingsPointer m_pConfig;

  private:
    ControlValueAtomic<SampleOfTrack> m_sampleOfTrack;
    EngineMaster* m_pEngineMaster;
    EngineBuffer* m_pEngineBuffer;

    friend class CueControlTest;

    FRIEND_TEST(LoopingControlTest, ReloopToggleButton_DoesNotJumpAhead);
    FRIEND_TEST(LoopingControlTest, ReloopAndStopButton);
    FRIEND_TEST(LoopingControlTest, LoopScale_HalvesLoop);
    FRIEND_TEST(LoopingControlTest, LoopMoveTest);
    FRIEND_TEST(LoopingControlTest, LoopResizeSeek);
    FRIEND_TEST(LoopingControlTest, Beatjump_JumpsByBeats);
};

#endif /* ENGINECONTROL_H */
