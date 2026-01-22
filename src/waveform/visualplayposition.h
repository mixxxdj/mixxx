#pragma once

#include <QAtomicPointer>
#include <QMap>
#include <QTime>
#include <atomic>

#include "control/controlvalue.h"
#include "engine/slipmodestate.h"
#include "util/performancetimer.h"

class ControlProxy;
class VSyncThread;

// This class is for synchronizing the sound device DAC time with the waveforms, displayed on the
// graphic device, using the CPU time
//
// DAC: ------|--------------|-------|-------------------|-----------------------|-----
//            ^Audio Callback Entry  |                   |                       ^Last Sample to DAC
//            |              ^Buffer prepared            ^Waveform sample X
//            |                      ^First sample transferred to DAC
// CPU: ------|-------------------------------------------------------------------------
//            ^Start m_timeInfoTime                      |
//                                                       |
// GPU: ---------|----------------------------------- |--|-------------------------------
//               ^Render Waveform sample X            |  ^VSync (New waveform is displayed
//                by use usFromTimerToNextSync        ^swap Buffer

class VisualPlayPositionData {
  public:
    PerformanceTimer m_referenceTime;
    int m_callbackEntrytoDac; // Time from Audio Callback Entry to first sample of Buffer is transferred to DAC
    double m_playPos;         // Play position of first Sample in Buffer
    double m_playRate;
    double m_positionStep;
    double m_slipPos;
    double m_slipRate;
    SlipModeState m_slipModeState;
    bool m_loopEnabled;
    bool m_loopInAdjustActive;
    bool m_loopOutAdjustActive;
    double m_loopStartPos;
    double m_loopEndPos;
    double m_tempoTrackSeconds; // total track time, taking the current tempo into account
    double m_audioBufferMicroS;
};


class VisualPlayPosition : public QObject {
    Q_OBJECT
  public:
    VisualPlayPosition(const QString& m_key);
    virtual ~VisualPlayPosition();

    // WARNING: Not thread safe. This function must be called only from the
    // engine thread.
    void set(double playPos,
            double playRate,
            double positionStep,
            double slipPos,
            double slipRate,
            SlipModeState slipModeState,
            bool loopEnabled,
            bool loopInAdjustActive,
            bool loopOutAdjustActive,
            double loopStartPos,
            double loopEndPos,
            double tempoTrackSeconds,
            double audioBufferMicroS);

    void getPlaySlipAtNextVSync(VSyncThread* pVSyncThread,
            double* playPosition,
            double* slipPosition);
    double determinePlayPosInLoopBoundries(
            const VisualPlayPositionData& data, const double& offset);
    double getEnginePlayPos();
    void getTrackTime(double* pPlayPosition, double* pTempoTrackSeconds);

    // WARNING: Not thread safe. This function must only be called from the main
    // thread.
    static QSharedPointer<VisualPlayPosition> getVisualPlayPosition(const QString& group);

    // This is called by SoundDevicePortAudio just after the callback starts.
    static void setCallbackEntryToDacSecs(double secs, const PerformanceTimer& time);

    void setInvalid() {
        m_valid.store(false);
    };
    bool isValid() const {
        return m_valid.load();
    }

  private:
    class DelayRing {
      public:
        void push(const VisualPlayPositionData& data) {
            size_t write = writeIndex.load(std::memory_order_relaxed);
            ring[write % kRingSize] = data;
            writeIndex.store(write + 1, std::memory_order_release);
            return;
        }

        /// returns a delayed value
        /// getAt(0) returns the most recent value
        /// getAt(1) returns the value that has been pushed before
        /// keep 'at' small compared to kRingSize to make a ring lap during the
        /// call of getAt() unlikely
        VisualPlayPositionData getAt(std::size_t at) {
            size_t writeBefore;
            size_t writeAfter;
            VisualPlayPositionData data;
            do {
                writeBefore = writeIndex.load(std::memory_order_acquire);
                size_t read = (writeBefore - 1 - at);
                data = ring[read % kRingSize];
                writeAfter = writeIndex.load(std::memory_order_acquire);
                // try again in case of a ring lap
            } while (writeAfter - writeBefore >= kRingSize - at);
            return data;
        }

      private:
        static constexpr size_t kRingSize = 16;
        std::array<VisualPlayPositionData, kRingSize> ring;
        std::atomic<std::size_t> writeIndex = 0;
    };

    double calcOffsetAtNextVSync(VSyncThread* pVSyncThread, const VisualPlayPositionData& data);
    DelayRing m_data;
    std::atomic<bool> m_valid;
    QString m_key;
    bool m_noTransport;

    static QMap<QString, QWeakPointer<VisualPlayPosition>> m_listVisualPlayPosition;
    // Time info from the Sound device, updated just after audio callback is called
    static double m_dCallbackEntryToDacSecs;
    // Time stamp for m_timeInfo in main CPU time
    static PerformanceTimer m_timeInfoTime;
};
