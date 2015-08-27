#ifndef ENGINENETWORKSTREAM_H_
#define ENGINENETWORKSTREAM_H_

#include "util/types.h"
#include "util/fifo.h"

class EngineNetworkStream {
  public:
    EngineNetworkStream(double sampleRate,
            int numOutputChannels,
            int numInputChannels);
    virtual ~EngineNetworkStream();

    void startStream();
    void stopStream();

    int getWriteExpected();
    int getReadExpected();

    void write(const CSAMPLE* buffer, int frames);
    void read(CSAMPLE* buffer, int frames);
    void writeSilence(int frames);

    qint64 getStreamTimeMs();
    qint64 getStreamTimeFrames();

    static qint64 getNetworkTimeMs();

  private:
    FIFO<CSAMPLE>* m_pOutputFifo;
    FIFO<CSAMPLE>* m_pInputFifo;
    int m_numOutputChannels;
    int m_numInputChannels;
    double m_sampleRate;
    qint64 m_streamStartTimeMs;
    qint64 m_streamFramesWritten;
    qint64 m_streamFramesRead;
};

#endif /* ENGINENETWORKSTREAM_H_ */
