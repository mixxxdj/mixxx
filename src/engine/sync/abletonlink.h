#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#if defined(__UNIX__)
#include <termios.h>
#endif

#include <QObject>
#include <QString>
#include <QTimer>
#include <iostream>

#include "ableton/Link.hpp"
#include "ableton/link/HostTimeFilter.hpp"
#include "engine/channels/enginechannel.h"
#include "engine/sync/syncable.h"

/* This class manages a link session. 
 * Read & update (get & set) this session for Mixxx to be a synced Link participant (bpm & phase)

 * Ableton Link Readme (lib/ableton-link/README.md)
 * Documentation in the header (lib/ableton-link/include/ableton/Link.hpp)
 * Ableton provides a command line tool (LinkHut) for debugging Link programs (instructions in the Readme)
 
 * Ableton recommends getting/setting the link session from the audio thread for maximum timing accuracy. 
 * Call the appropriate, realtime-safe functions from the audio callback to do this.
*/

class AbletonLink : public QObject, public Syncable {
    Q_OBJECT
  public:
    AbletonLink(const QString& group, SyncableListener* pEngineSync);

    const QString& getGroup() const override {
        return m_group;
    }
    EngineChannel* getChannel() const override {
        return nullptr;
    }

    // Notify a Syncable that their mode has changed. The Syncable must record
    // this mode and return the latest mode in response to getMode().
    void setSyncMode(SyncMode mode) override;

    // Notify a Syncable that it is now the only currently-playing syncable.
    void notifyOnlyPlayingSyncable() override;

    // Notify a Syncable that they should sync phase.
    void requestSync() override;

    // Must NEVER return a mode that was not set directly via
    // notifySyncModeChanged.
    SyncMode getSyncMode() const override;

    // Only relevant for player Syncables.
    bool isPlaying() const override;

    // Gets the current speed of the syncable in bpm (bpm * rate slider), doesn't
    // include scratch or FF/REW values.
    double getBpm() const override;

    // Gets the beat distance as a fraction from 0 to 1
    double getBeatDistance() const override;

    // Gets the speed of the syncable if it was playing at 1.0 rate.
    double getBaseBpm() const override;

    // The following functions are used to tell syncables about the state of the
    // current Sync Master.
    // Must never result in a call to
    // SyncableListener::notifyBeatDistanceChanged or signal loops could occur.
    void setMasterBeatDistance(double beatDistance) override;

    // Must never result in a call to SyncableListener::notifyBpmChanged or
    // signal loops could occur.
    void setMasterBpm(double bpm) override;

    // Combines the above three calls into one, since they are often set
    // simultaneously.  Avoids redundant recalculation that would occur by
    // using the three calls separately.
    void setMasterParams(double beatDistance, double baseBpm, double bpm) override;

    // Must never result in a call to
    // SyncableListener::notifyInstantaneousBpmChanged or signal loops could
    // occur.
    void setInstantaneousBpm(double bpm) override;

    void onCallbackStart(int sampleRate, int bufferSize);
    void onCallbackEnd(int sampleRate, int bufferSize);

  private slots:
    void testPrint() {
        log("", "");
        nonAudioPrint();
        audioSafePrint();
    }

  private:
    ableton::Link m_link;
    ableton::link::HostTimeFilter<ableton::link::platform::Clock> m_hostTimeFilter;
    const QString m_group;
    SyncableListener* m_pEngineSync;
    SyncMode m_mode;
    long m_sampleTime;
    std::chrono::microseconds m_currentLatency{0};
    std::chrono::microseconds m_hostTime{0};

    // Uses ableton's HostTimeFilter class to create a smooth linear regression between sample time and system time
    void updateHostTime(size_t sampleTime) {
        m_hostTime = m_hostTimeFilter.sampleTimeToHostTime(static_cast<double>(sampleTime));
    }

    // Approximate the system time when the first sample in the current audio buffer will hit the speakers
    std::chrono::microseconds getCurrentBufferPlayTime() const {
        return m_hostTime + m_currentLatency;
        // return m_link.clock().micros() + m_currentLatency;
    }

    double getQuantum() const {
        return 4.0;
    }

    // -----------     Test/DEBUG stuff below     ----------------

    QTimer* m_pTestTimer;
    const double beat = 0.0;

    template<typename T>
    void log(const std::string& name, T val) {
        std::cout << "link:  " << name << " - " << val << std::endl;
    }

    void logBool(const std::string& name, bool val) {
        log(name, val ? "true" : "false");
    }

    // Link getters to call from audio thread.
    void audioSafePrint() {
        // logBool("isEnabled()", m_link.isEnabled());
        // log("numPeers()", m_link.numPeers());

        ableton::Link::SessionState sessionState = m_link.captureAudioSessionState();

        log("sessionState.tempo()", sessionState.tempo());
        // log("sessionState.beatAtTime()", sessionState.beatAtTime(m_link.clock().micros(), getQuantum()));
        // log("sessionState.phaseAtTime()", sessionState.phaseAtTime(m_link.clock().micros(), getQuantum()));
        // log("sessionState.timeAtBeat(0)", sessionState.timeAtBeat(0.0, getQuantum()).count());
        // logBool("sessionState.isPlaying()", sessionState.isPlaying());
        // log("sessionState.timeForIsPlaying()", sessionState.timeForIsPlaying().count());

        // Est. Delay (micro-seconds) between onCallbackStart() and buffer's first audio sample reaching speakers
        // log("Est. Delay (us)", (getCurrentBufferPlayTime() - m_link.clock().micros()).count());
    }

    // Link getters to call from non-audio thread.
    void nonAudioPrint() {
        logBool("isStartStopSyncEnabled()", m_link.isStartStopSyncEnabled());
    }

    // Link setters to call from audio thread.
    void audioSafeSet() {
        ableton::Link::SessionState sessionState = m_link.captureAudioSessionState();

        sessionState.setTempo(120, m_link.clock().micros());
        sessionState.requestBeatAtTime(beat, m_link.clock().micros(), getQuantum());
        sessionState.setIsPlaying(true, m_link.clock().micros());

        // convenience functions
        sessionState.requestBeatAtStartPlayingTime(beat, getQuantum());
        sessionState.setIsPlayingAndRequestBeatAtTime(true, m_link.clock().micros(), beat, getQuantum());

        m_link.commitAudioSessionState(sessionState);
    }

    // Link setters to call from non-audio thread.
    void nonAudioSet() {
        m_link.enable(true);
        m_link.enableStartStopSync(true);
    }

    void initTestTimer(size_t ms, bool isRepeating) {
        m_pTestTimer = new QTimer(this);
        connect(m_pTestTimer, &QTimer::timeout, this, QOverload<>::of(&AbletonLink::testPrint));
        m_pTestTimer->setSingleShot(!isRepeating);
        m_pTestTimer->start(ms);
    }
};
