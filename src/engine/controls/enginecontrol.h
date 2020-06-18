#pragma once

#include <gtest/gtest_prod.h>

#include <QList>
#include <QObject>

#include "control/controlvalue.h"
#include "engine/cachingreader/cachingreader.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/sync/syncable.h"
#include "preferences/usersettings.h"
#include "track/beats.h"
#include "track/track_decl.h"

class EngineMaster;
class EngineBuffer;

const int kNoTrigger = -1;

/// EngineControl is an abstract base class for objects which implement
/// functionality pertaining to EngineBuffer. An EngineControl is meant to be a
/// succinct implementation of a given EngineBuffer feature.
class EngineControl : public QObject {
    Q_OBJECT
  public:
    EngineControl(const QString& group, UserSettingsPointer pConfig);
    QString getGroup() const;

    virtual void setEngineMaster(EngineMaster* pEngineMaster);
    void setEngineBuffer(EngineBuffer* pEngineBuffer);

    virtual void setCurrentSample(
            const double dCurrentSample,
            const double dTotalSamples,
            const double dTrackSampleRate);

    void setBeatLoop(double startPosition, bool enabled);
    void setLoop(double startPosition, double endPosition, bool enabled);

    /// Called whenever a seek occurs to allow the EngineControl to respond.
    virtual void notifySeek(double dNewPlaypos);

    /// Called by EngineBuffer::process every latency period. An EngineControl
    /// can perform any necessary upkeep operations during this call.
    ///
    /// EngineControl::process will only be called during an
    /// EngineBuffer::process callback from the sound engine. This implies that
    /// any ControlObject accesses made in either of these methods are mutually
    /// exclusive, since one is exclusively in the call graph of the other.
    virtual void process(
            const double dRate,
            const double dCurrentSample,
            const int iBufferSize) {
        Q_UNUSED(dRate);
        Q_UNUSED(dCurrentSample);
        Q_UNUSED(iBufferSize);
    };

    /// Called to collect player features for effects processing.
    virtual void collectFeatureState(GroupFeatureState* pGroupFeatures) const {
        Q_UNUSED(pGroupFeatures);
    }

    virtual void trackLoaded(TrackPointer pNewTrack) {
        Q_UNUSED(pNewTrack);
    }
    virtual void trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
        Q_UNUSED(pBeats);
    }

    /// hintReader allows the EngineControl to provide hints to the reader to
    /// indicate that the given portion of a song is a potential imminent seek
    /// target.
    virtual void hintReader(HintVector* pHintList) {
        Q_UNUSED(pHintList);
    };

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
    /// Seek to an exact sample and don't allow quantizing adjustment.
    void seekExact(double sample);
    /// Returns an EngineBuffer to target for syncing. Returns nullptr if none found
    EngineBuffer* pickSyncTarget();

    UserSettingsPointer getConfig();
    EngineMaster* getEngineMaster();
    EngineBuffer* getEngineBuffer();

    const QString m_group;
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
