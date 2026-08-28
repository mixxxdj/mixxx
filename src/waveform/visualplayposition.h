#pragma once

#include <QAtomicPointer>
#include <QMap>
#include <QTime>
#include <array>
#include <atomic>
#include <cstring>

#include "control/controlvalue.h"
#include "engine/slipmodestate.h"
#include "util/performancetimer.h"

class ControlProxy;
class VSyncTimeProvider;

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

    void getPlaySlipAtNextVSync(VSyncTimeProvider* pSyncTimeProvider,
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
        m_data.reset();
    };

  private:
    /// This is a single producer single consumer ring buffer that allows
    /// to access historic (delayed) values
    class DelayRing {
      public:
        /// Push new data to the ring
        /// Must be called from a single thread only
        void push(const VisualPlayPositionData& data) {
            size_t write = m_writeIndex.load(std::memory_order_relaxed);
            std::memcpy(&m_ring[write % kRingSize], &data, sizeof(VisualPlayPositionData));
            m_writeIndex.store(write + 1, std::memory_order_release);
            if (m_level.load(std::memory_order_relaxed) < kRingSize) {
                m_level.fetch_add(1, std::memory_order_release);
            }
            return;
        }

        /// returns true on success and a delayed value via pData
        /// getAt(0) returns the most recent value
        /// getAt(1) returns the value that has been pushed before
        /// keep 'at' small compared to kRingSize to make a ring lap during the
        /// call of getAt() unlikely
        /// Must be called from a single thread only
        bool getAt(std::size_t at, VisualPlayPositionData* pData) {
            // not enough data available
            if (m_level.load(std::memory_order_acquire) <= at) {
                return false;
            }

            size_t writeBefore;
            size_t writeAfter;
            do {
                // snapshot the published write count (acquire)
                writeBefore = m_writeIndex.load(std::memory_order_acquire);

                // compute read index (unsigned arithmetic handles wrap)
                const size_t read = writeBefore - 1 - at;

                // copy the payload (non-atomic)
                std::memcpy(pData, &m_ring[read % kRingSize], sizeof(VisualPlayPositionData));

                // re-check the published write count (acquire)
                writeAfter = m_writeIndex.load(std::memory_order_acquire);

                // loop condition below will retry only if producer could have overwritten the slot
            } while (writeAfter - writeBefore >= kRingSize - at);

            // At this point the producer did not advance enough to have
            // overwritten the slot we copied. Extra guard: ensure level didn't
            // drop below 'at' while we retried.
            if (m_level.load(std::memory_order_relaxed) <= at) {
                return false;
            }
            return true;
        }

        /// Clears the ring, can be called from any thread
        void reset() {
            m_level.store(0, std::memory_order_release);
        }

      private:
        static constexpr size_t kRingSize = 16;
        std::array<VisualPlayPositionData, kRingSize> m_ring;
        std::atomic<std::size_t> m_writeIndex{0};
        std::atomic<std::size_t> m_level{0};
    };

    double calcOffsetAtNextVSync(VSyncTimeProvider* pSyncTimeProvider,
            const VisualPlayPositionData& data);
    DelayRing m_data;
    QString m_key;
    bool m_noTransport;

    static QMap<QString, QWeakPointer<VisualPlayPosition>> m_listVisualPlayPosition;
    // Time info from the Sound device, updated just after audio callback is called
    static double m_dCallbackEntryToDacSecs;
    // Time stamp for m_timeInfo in main CPU time
    static PerformanceTimer m_timeInfoTime;
};
