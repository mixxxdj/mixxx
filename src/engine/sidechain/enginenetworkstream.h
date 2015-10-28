#ifndef ENGINENETWORKSTREAM_H_
#define ENGINENETWORKSTREAM_H_

#include "util/types.h"
#include "util/fifo.h"
#include "engine/sidechain/networkstreamworker.h"

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

    qint64 getStreamTimeUs();
    qint64 getStreamTimeFrames();

    int getNumOutputChannels() {
        return m_numOutputChannels;
    }

    int getNumInputChannels() {
        return m_numInputChannels;
    }

    static qint64 getNetworkTimeUs();

    void addWorker(QSharedPointer<NetworkStreamWorker> pWorker);

  private:
    void scheduleWorker();

    FIFO<CSAMPLE>* m_pOutputFifo;
    FIFO<CSAMPLE>* m_pInputFifo;
    int m_numOutputChannels;
    int m_numInputChannels;
    double m_sampleRate;
    qint64 m_streamStartTimeUs;
    qint64 m_streamFramesWritten;
    qint64 m_streamFramesRead;
    QSharedPointer<NetworkStreamWorker> m_pWorker;
    int m_writeOverflowCount;
};

#endif /* ENGINENETWORKSTREAM_H_ */
