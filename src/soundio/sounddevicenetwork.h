#pragma once

#include <QString>
#include <QSharedPointer>
#include <QThread>

#ifdef __LINUX__
#include <pthread.h>
#endif

#include "util/performancetimer.h"
#include "util/memory.h"
#include "soundio/sounddevice.h"
#include "engine/sidechain/networkoutputstreamworker.h"

#define CPU_USAGE_UPDATE_RATE 30 // in 1/s, fits to display frame rate
#define CPU_OVERLOAD_DURATION 500 // in ms

class SoundManager;
class ControlProxy;
class EngineNetworkStream;
class SoundDeviceNetworkThread;


class SoundDeviceNetwork : public SoundDevice {
  public:
    SoundDeviceNetwork(UserSettingsPointer config,
                       SoundManager* sm,
                       QSharedPointer<EngineNetworkStream> pNetworkStream);
    ~SoundDeviceNetwork() override;

    SoundDeviceError open(bool isClkRefDevice, int syncBuffers) override;
    bool isOpen() const override;
    SoundDeviceError close() override;
    void readProcess() override;
    void writeProcess() override;
    QString getError() const override;

    unsigned int getDefaultSampleRate() const override {
        return 44100;
    }

    void callbackProcessClkRef();

  private:
    void updateCallbackEntryToDacTime();
    void updateAudioLatencyUsage();

    void workerWriteProcess(NetworkOutputStreamWorkerPtr pWorker,
            int outChunkSize, int readAvailable,
            CSAMPLE* dataPtr1, ring_buffer_size_t size1,
            CSAMPLE* dataPtr2, ring_buffer_size_t size2);
    void workerWrite(NetworkOutputStreamWorkerPtr pWorker,
            const CSAMPLE* buffer, int frames);
    void workerWriteSilence(NetworkOutputStreamWorkerPtr pWorker, int frames);

    QSharedPointer<EngineNetworkStream> m_pNetworkStream;
    std::unique_ptr<FIFO<CSAMPLE> > m_outputFifo;
    std::unique_ptr<FIFO<CSAMPLE> > m_inputFifo;
    bool m_inputDrift;

    std::unique_ptr<ControlProxy> m_pMasterAudioLatencyUsage;
    mixxx::Duration m_timeInAudioCallback;
    mixxx::Duration m_audioBufferTime;
    int m_framesSinceAudioLatencyUsageUpdate;
    std::unique_ptr<SoundDeviceNetworkThread> m_pThread;
    bool m_denormals;
    qint64 m_targetTime;
    PerformanceTimer m_clkRefTimer;
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
