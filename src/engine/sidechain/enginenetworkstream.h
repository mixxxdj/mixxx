#pragma once

#include <engine/sidechain/networkoutputstreamworker.h>
#include <engine/sidechain/networkinputstreamworker.h>
#include <QVector>

#include "util/types.h"
#include "util/fifo.h"

class EngineNetworkStream {
  public:
    EngineNetworkStream(int numOutputChannels,
            int numInputChannels);
    virtual ~EngineNetworkStream();

    void startStream(double sampleRate);
    void stopStream();

    int getReadExpected();
    void read(CSAMPLE* buffer, int frames);

    qint64 getInputStreamTimeUs();
    qint64 getInputStreamTimeFrames();

    int getNumOutputChannels() {
        return m_numOutputChannels;
    }

    int getNumInputChannels() {
        return m_numInputChannels;
    }

    static qint64 getNetworkTimeUs();

    void addOutputWorker(NetworkOutputStreamWorkerPtr pWorker);
    void removeOutputWorker(NetworkOutputStreamWorkerPtr pWorker);
    void setInputWorker(NetworkInputStreamWorker* pInputWorker);

    QVector<NetworkOutputStreamWorkerPtr> outputWorkers() {
        return m_outputWorkers;
    }

  private:
    int nextOutputSlotAvailable();
    void debugOutputSlots();

    FIFO<CSAMPLE>* m_pInputFifo;
    int m_numOutputChannels;
    int m_numInputChannels;
    double m_sampleRate;
    qint64 m_inputStreamStartTimeUs;
    qint64 m_inputStreamFramesWritten;
    qint64 m_inputStreamFramesRead;

    // EngineNetworkStream can't use locking mechanisms to protect its
    // internal worker list against concurrency issues, as it is used by
    // methods called from the audio engine thread.
    // Instead, the internal list has a fixed number of QSharedPointers
    // (which are thread-safe) initialized with null pointers. R/W operations to
    // the workers are then performed on thread-safe QSharedPointers and not
    // onto the thread-unsafe QVector
    QVector<NetworkOutputStreamWorkerPtr> m_outputWorkers;
};
