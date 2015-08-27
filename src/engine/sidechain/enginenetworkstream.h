#ifndef ENGINENETWORKSTREAM_H_
#define ENGINENETWORKSTREAM_H_

#include "util/types.h"
#include "util/fifo.h"

class SideChainWorker;

class EngineNetworkStream {
  public:
    EngineNetworkStream(int numOutputChannels,
            int numInputChannels);
    virtual ~EngineNetworkStream();

    void startStream(double sampleRate);
    void stopStream();

    int getWriteExpected();
    int getReadExpected();

    void write(const CSAMPLE* buffer, int frames);
    void read(CSAMPLE* buffer, int frames);
    void writeSilence(int frames);

    qint64 getStreamTimeMs();
    qint64 getStreamTimeFrames();

    int getNumOutputChannels() {
        return m_numOutputChannels;
    }

    int getNumInputChannels() {
        return m_numInputChannels;
    }

    static qint64 getNetworkTimeMs();

    void addWorker(QSharedPointer<SideChainWorker> pWorker) {
        m_pWorker = pWorker;
    }

  private:
    FIFO<CSAMPLE>* m_pOutputFifo;
    FIFO<CSAMPLE>* m_pInputFifo;
    int m_numOutputChannels;
    int m_numInputChannels;
    double m_sampleRate;
    qint64 m_streamStartTimeMs;
    qint64 m_streamFramesWritten;
    qint64 m_streamFramesRead;
    QSharedPointer<SideChainWorker> m_pWorker;
};

#endif /* ENGINENETWORKSTREAM_H_ */
