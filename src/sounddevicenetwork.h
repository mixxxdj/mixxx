#ifndef SOUNDDEVICENETWORK_H
#define SOUNDDEVICENETWORK_H

#include <QString>

#include "sounddevice.h"

#define CPU_USAGE_UPDATE_RATE 30 // in 1/s, fits to display frame rate
#define CPU_OVERLOAD_DURATION 500 // in ms

class SoundManager;
class ControlObjectSlave;
class EngineNetworkStream;

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

  private:
    QSharedPointer<EngineNetworkStream> m_pNetworkStream;
    FIFO<CSAMPLE>* m_outputFifo;
    FIFO<CSAMPLE>* m_inputFifo;
    bool m_outputDrift;
    bool m_inputDrift;

    // A string describing the last PortAudio error to occur.
    QString m_lastError;
    // Whether we have set the thread priority to realtime or not.
    bool m_bSetThreadPriority;
    ControlObjectSlave* m_pMasterAudioLatencyOverloadCount;
    ControlObjectSlave* m_pMasterAudioLatencyUsage;
    ControlObjectSlave* m_pMasterAudioLatencyOverload;
    int m_underflowUpdateCount;
    static volatile int m_underflowHappend;
    qint64 m_nsInAudioCb;
    int m_framesSinceAudioLatencyUsageUpdate;
};

#endif // SOUNDDEVICENETWORK_H
