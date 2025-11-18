#pragma once

#include <ableton/Link.hpp>
#include <ableton/link/HostTimeFilter.hpp>
#include <ableton/platforms/stl/Clock.hpp>

#include "control/controlpushbutton.h"
#include "engine/channels/enginechannel.h"
#include "engine/enginebuffer.h"
#include "engine/sync/syncable.h"
#include "engine/sync/synccontrol.h"

/// This class manages a link session.
/// Read & update (get & set) this session for Mixxx to be a synced Link
/// participant (bpm & phase)
///
/// Ableton Link Readme (lib/ableton-link/README.md)
/// Documentation in the header (lib/ableton-link/include/ableton/Link.hpp)
/// Ableton provides a command line tool (LinkHut) for debugging Link programs
/// (instructions in the Readme)
///
/// Ableton recommends getting/setting the link session from the audio thread
/// for maximum timing accuracy. Call the appropriate, realtime-safe functions
/// from the audio callback to do this.

// std::chrono::steady_clock
// -> selected by keyword 'stl' in ableton-link
// Note that the resolution of std::chrono::steady_clock is not guaranteed
// to be high resolution, but it is guaranteed to be monotonic.
// However, on all major platforms, it is high resolution enough.
using MixxxClockRef = ableton::platforms::stl::Clock;

class AbletonLink : public QObject, public Syncable {
    Q_OBJECT
  public:
    AbletonLink(const QString& group, EngineSync* pEngineSync);
    ~AbletonLink() override = default;

    const QString& getGroup() const override {
        return m_group;
    }
    EngineChannel* getChannel() const override {
        return nullptr;
    }

    /// Notify a Syncable that their mode has changed. The Syncable must record
    /// this mode and return the latest mode in response to getMode().
    void setSyncMode(SyncMode mode) override;

    /// Notify a Syncable that it is now the only currently-playing syncable.
    void notifyUniquePlaying() override;

    /// Notify a Syncable that they should sync phase.
    void requestSync() override;

    /// Must NEVER return a mode that was not set directly via
    /// notifySyncModeChanged.
    SyncMode getSyncMode() const override;

    /// Only relevant for player Syncables.
    bool isPlaying() const override;
    bool isAudible() const override;
    bool isQuantized() const override;

    /// Gets the current speed of the syncable in bpm (bpm * rate slider), doesn't
    /// include scratch or FF/REW values.
    mixxx::Bpm getBpm() const override;

    /// Gets the beat distance as a fraction from 0 to 1
    double getBeatDistance() const override;

    /// Gets the speed of the syncable if it was playing at 1.0 rate.
    mixxx::Bpm getBaseBpm() const override;

    /// The following functions are used to tell syncables about the state of the
    /// current Sync Master.
    /// Must never result in a call to
    /// SyncableListener::notifyBeatDistanceChanged or signal loops could occur.
    void updateLeaderBeatDistance(double beatDistance) override;

    /// Enforces the immediate change of the beat distance of all Link peers
    void forceUpdateLeaderBeatDistance(double beatDistance);

    /// Must never result in a call to SyncableListener::notifyBpmChanged or
    /// signal loops could occur.
    void updateLeaderBpm(mixxx::Bpm bpm) override;

    void notifyLeaderParamSource() override;

    /// Combines the above three calls into one, since they are often set
    /// simultaneously.  Avoids redundant recalculation that would occur by
    /// using the three calls separately.
    void reinitLeaderParams(double beatDistance, mixxx::Bpm baseBpm, mixxx::Bpm bpm) override;

    /// Must never result in a call to
    /// SyncableListener::notifyInstantaneousBpmChanged or signal loops could
    /// occur.
    void updateInstantaneousBpm(mixxx::Bpm bpm) override;

    void onCallbackStart(int sampleRate,
            size_t bufferSize,
            std::chrono::microseconds absTimeWhenPrevOutputBufferReachesDac);
    void onCallbackEnd(int sampleRate, size_t bufferSize);

  private:
    ableton::link::HostTimeFilter<MixxxClockRef> m_hostTimeFilter;
    QString m_group;
    EngineSync* m_pEngineSync; // unowned, must outlive this.
    SyncMode m_syncMode;

    mixxx::Bpm m_oldTempo;

    std::chrono::microseconds m_audioBufferTimeMicros;
    std::chrono::microseconds m_absTimeWhenPrevOutputBufferReachesDac;
    std::chrono::microseconds m_sampleTimeAtStartCallback;
    std::chrono::microseconds m_timeAtStartCallback;

    std::unique_ptr<ableton::BasicLink<MixxxClockRef>> m_pLink;
    std::unique_ptr<ControlPushButton> m_pLinkButton;
    std::unique_ptr<ControlObject> m_pNumLinkPeers;

    void slotControlSyncEnabled(double value);

    std::chrono::microseconds getHostTime() const;
    std::chrono::microseconds getHostTimeAtSpeaker(std::chrono::microseconds hostTime) const;

    double getQuantum() const {
        // Mixxx doesn't know about bars/time-signatures yet - phase
        // synchronisation can't be implemented therefore yet
        return 1.0;
    }

    // Test/Debug code

    /// Link getters to call from audio thread.
    void audioThreadDebugOutput();
};
