#pragma once

#include <QSharedPointer>
#include <QString>
#include <QThread>

#ifdef __LINUX__
#include <pthread.h>
#endif

#include <memory>

#include "control/pollingcontrolproxy.h"
#include "engine/sidechain/networkoutputstreamworker.h"
#include "soundio/sounddevice.h"
#include "util/fifo.h"
#include "util/hosttimefilter.h"
#include "util/performancetimer.h"

#define CPU_USAGE_UPDATE_RATE 30 // in 1/s, fits to display frame rate
#define CPU_OVERLOAD_DURATION 500 // in ms

class SoundManager;
class EngineNetworkStream;
class SoundDeviceNetworkThread;

class SoundDeviceNetwork : public SoundDevice {
  public:
    SoundDeviceNetwork(UserSettingsPointer config,
                       SoundManager* sm,
                       QSharedPointer<EngineNetworkStream> pNetworkStream);
    ~SoundDeviceNetwork() override;

    SoundDeviceStatus open(bool isClkRefDevice, int syncBuffers) override;
    bool isOpen() const override;
    SoundDeviceStatus close() override;
    void readProcess(SINT framesPerBuffer) override;
    void writeProcess(SINT framesPerBuffer) override;
    QString getError() const override;

    mixxx::audio::SampleRate getDefaultSampleRate() const override;

    // NOTE: This does not take a frames per buffer argument because that is
    //       always equal to the configured buffer size for network streams
    void callbackProcessClkRef();

  private:
    void updateCallbackEntryToDacTime(SINT framesPerBuffer);
    void updateAudioLatencyUsage(SINT framesPerBuffer);

    void workerWriteProcess(NetworkOutputStreamWorkerPtr pWorker,
            int outChunkSize, int readAvailable,
            CSAMPLE* dataPtr1, ring_buffer_size_t size1,
            CSAMPLE* dataPtr2, ring_buffer_size_t size2);
    void workerWrite(NetworkOutputStreamWorkerPtr pWorker,
            const CSAMPLE* buffer, int frames);
    void workerWriteSilence(NetworkOutputStreamWorkerPtr pWorker, int frames);

    QSharedPointer<EngineNetworkStream> m_pNetworkStream;
    std::unique_ptr<FIFO<CSAMPLE>> m_outputFifo;
    std::unique_ptr<FIFO<CSAMPLE>> m_inputFifo;
    bool m_inputDrift;

    PollingControlProxy m_audioLatencyUsage;
    mixxx::Duration m_timeInAudioCallback;
    int m_framesSinceAudioLatencyUsageUpdate;
    std::unique_ptr<SoundDeviceNetworkThread> m_pThread;
    bool m_denormals;
    /// The deadline for the next buffer, in microseconds since the Unix epoch.
    qint64 m_targetTime;
    PerformanceTimer m_clkRefTimer;

    HostTimeFilter m_hostTimeFilter;
};

class SoundDeviceNetworkThread : public QThread {
    Q_OBJECT
  public:
    SoundDeviceNetworkThread(SoundDeviceNetwork* pParent)
        : m_pParent(pParent),
          m_stop(false) {
    }

    void stop() {
        m_stop = true;
    }

    void usleep_(unsigned long t) {
        usleep(t);
    }

  private:
    void run() override {
#ifdef __LINUX__
        struct sched_param spm = { 0 };
        spm.sched_priority = 1;
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &spm)) {
            qWarning() << "SoundDeviceNetworkThread: Failed bumping priority";
        }
#endif

        while(!m_stop) {
            m_pParent->callbackProcessClkRef();
        }
    }
    SoundDeviceNetwork* m_pParent;
    bool m_stop;
};
