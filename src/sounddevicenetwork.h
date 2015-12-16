#ifndef SOUNDDEVICENETWORK_H
#define SOUNDDEVICENETWORK_H

#include <QString>
#include <QThread>

#include "sounddevice.h"

#define CPU_USAGE_UPDATE_RATE 30 // in 1/s, fits to display frame rate
#define CPU_OVERLOAD_DURATION 500 // in ms

class SoundManager;
class ControlObjectSlave;
class EngineNetworkStream;
class SoundDeviceNetworkThread;


class SoundDeviceNetwork : public SoundDevice {
  public:
    SoundDeviceNetwork(ConfigObject<ConfigValue> *config,
                       SoundManager *sm,
                       QSharedPointer<EngineNetworkStream> pNetworkStream);
    virtual ~SoundDeviceNetwork();

    virtual Result open(bool isClkRefDevice, int syncBuffers);
    virtual bool isOpen() const;
    virtual Result close();
    virtual void readProcess();
    virtual void writeProcess();
    virtual QString getError() const;

    virtual unsigned int getDefaultSampleRate() const {
        return 44100;
    }

    void callbackProcessClkRef();
  private:
    QSharedPointer<EngineNetworkStream> m_pNetworkStream;
    FIFO<CSAMPLE>* m_outputFifo;
    FIFO<CSAMPLE>* m_inputFifo;
    bool m_outputDrift;
    bool m_inputDrift;

    // A string describing the last error to occur.
    QString m_lastError;
    ControlObjectSlave* m_pMasterAudioLatencyOverloadCount;
    ControlObjectSlave* m_pMasterAudioLatencyUsage;
    ControlObjectSlave* m_pMasterAudioLatencyOverload;
    int m_underflowUpdateCount;
    static volatile int m_underflowHappend;
    qint64 m_nsInAudioCb;
    int m_framesSinceAudioLatencyUsageUpdate;
    SoundDeviceNetworkThread* m_pThread;
    bool m_denormals;
    qint64 m_lastTime;
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
    void run() {
        while(!m_stop) {
            m_pParent->callbackProcessClkRef();
        }
    }
    SoundDeviceNetwork* m_pParent;
    bool m_stop;
};

#endif // SOUNDDEVICENETWORK_H
