#pragma once

#include <gtest/gtest_prod.h>

#include <QList>
#include <QObject>
#include <gsl/pointers>

#include "audio/frame.h"
#include "control/controlvalue.h"
#include "engine/cachingreader/cachingreader.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/sync/syncable.h"
#include "preferences/usersettings.h"
#include "track/beats.h"
#include "track/track_decl.h"

class EngineMixer;
class EngineBuffer;

constexpr int kNoTrigger = -1;
static_assert(
        mixxx::audio::FramePos::kLegacyInvalidEnginePosition == kNoTrigger,
        "Invalid engine position value mismatch");

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
    EngineControl(const QString& group,
            UserSettingsPointer pConfig);
    ~EngineControl() override;

    // Called by EngineBuffer::process every latency period. See the above
    // comments for information about guarantees that hold during this call. An
    // EngineControl can perform any upkeep operations that are necessary during
    // this call.
    virtual void process(const double dRate,
            mixxx::audio::FramePos currentPosition,
            const int iBufferSize);

    // hintReader allows the EngineControl to provide hints to the reader to
    // indicate that the given portion of a song is a potential imminent seek
    // target.
    virtual void hintReader(gsl::not_null<HintVector*> pHintList);

    virtual void setEngineMixer(EngineMixer* pEngineMixer);
    void setEngineBuffer(EngineBuffer* pEngineBuffer);
    virtual void setFrameInfo(mixxx::audio::FramePos currentPosition,
            mixxx::audio::FramePos trackEndPosition,
            mixxx::audio::SampleRate sampleRate);
    QString getGroup() const;

    void setBeatLoop(mixxx::audio::FramePos startPosition, bool enabled);
    void setLoop(mixxx::audio::FramePos startPosition,
            mixxx::audio::FramePos endPosition,
            bool enabled);

    // Called to collect player features for effects processing.
    virtual void collectFeatureState(GroupFeatureState* pGroupFeatures) const {
        Q_UNUSED(pGroupFeatures);
    }

    /// Called whenever a seek occurs to allow the EngineControl to respond.
    virtual void notifySeek(mixxx::audio::FramePos position) {
        Q_UNUSED(position);
    };

    virtual void trackLoaded(TrackPointer pNewTrack);
    virtual void trackBeatsUpdated(mixxx::BeatsPointer pBeats);

  protected:
    struct FrameInfo {
        /// The current playback position. Guaranteed to be valid. If no track
        /// is loaded, this equals `mixxx::audio::kStartFramePos`.
        mixxx::audio::FramePos currentPosition;
        /// The track's end position (a.k.a. the length of the track in frames).
        /// This may be invalid if no track is loaded.
        mixxx::audio::FramePos trackEndPosition;
        /// The track's sample rate.  This may be invalid if no track is loaded.
        mixxx::audio::SampleRate sampleRate;
    };

    FrameInfo frameInfo() const {
        return m_frameInfo.getValue();
    }
    void seek(double fractionalPosition);
    void seekAbs(mixxx::audio::FramePos position);
    // Seek to an exact sample and don't allow quantizing adjustment.
    void seekExact(mixxx::audio::FramePos position);
    // Returns an EngineBuffer to target for syncing. Returns nullptr if none found
    EngineBuffer* pickSyncTarget();

    UserSettingsPointer getConfig();
    EngineMixer* getEngineMixer();
    EngineBuffer* getEngineBuffer();

    const QString m_group;
    UserSettingsPointer m_pConfig;

  private:
    ControlValueAtomic<FrameInfo> m_frameInfo;
    EngineMixer* m_pEngineMixer;
    EngineBuffer* m_pEngineBuffer;

    friend class CueControlTest;

    FRIEND_TEST(LoopingControlTest, ReloopToggleButton_DoesNotJumpAhead);
    FRIEND_TEST(LoopingControlTest, ReloopAndStopButton);
    FRIEND_TEST(LoopingControlTest, LoopScale_HalvesLoop);
    FRIEND_TEST(LoopingControlTest, LoopMoveTest);
    FRIEND_TEST(LoopingControlTest, LoopResizeSeek);
    FRIEND_TEST(LoopingControlTest, Beatjump_JumpsByBeats);
};
