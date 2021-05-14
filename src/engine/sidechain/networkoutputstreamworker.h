#pragma once

#include <QSharedPointer>

#include "util/types.h"
#include "util/fifo.h"

/*
 * States:
 * Error        Something erroneous has happened and can't go on
 * New          First state before init
 * Init         Initing state don't feed anything in this state
 * Waiting      Waiting something not ready yet
 * Busy         Is busy doing something can't process anything new
 * Ready        Functioning ok
 * Reading      Reading something and can't do anything else
 * Writing      Writing something and can't do anything else
 * Connected    Is connected to storage or server
 * Connecting   Trying to connect storage or server
 * Disconnected Ain't connected to storage or server
 *
 * First state should be NETWORKSTREAMWORKER_STATE_UNKNOWN and
 * if state handling ain't supported by NetworkStreamWorker-class
 * then 'NETWORKSTREAMWORKER_STATE_NEW' should be treated as
 * NETWORKSTREAMWORKER_STATE_READY. Newly written SideChainWorker-class
 * should support state handling at least this NETWORKSTREAMWORKER_STATE_READY state.
 */

enum NetworkOutputStreamWorkerStates {
    NETWORKSTREAMWORKER_STATE_ERROR = -1,
    NETWORKSTREAMWORKER_STATE_NEW,
    NETWORKSTREAMWORKER_STATE_INIT,
    NETWORKSTREAMWORKER_STATE_WAITING,
    NETWORKSTREAMWORKER_STATE_BUSY,
    NETWORKSTREAMWORKER_STATE_READY,
    NETWORKSTREAMWORKER_STATE_READING,
    NETWORKSTREAMWORKER_STATE_WRITING,
    NETWORKSTREAMWORKER_STATE_CONNECTED,
    NETWORKSTREAMWORKER_STATE_CONNECTING,
    NETWORKSTREAMWORKER_STATE_DISCONNECTED
};

class NetworkOutputStreamWorker {
  public:
    NetworkOutputStreamWorker();
    virtual ~NetworkOutputStreamWorker() = default;

    virtual void process(const CSAMPLE* pBuffer, const int iBufferSize) = 0;
    virtual void shutdown() = 0;

    virtual void outputAvailable();
    virtual void setOutputFifo(QSharedPointer<FIFO<CSAMPLE>> pOutputFifo);
    virtual QSharedPointer<FIFO<CSAMPLE>> getOutputFifo();

    void startStream(double samplerate, int numOutputChannels);
    void stopStream();

    virtual bool threadWaiting();

    qint64 getStreamTimeUs();
    qint64 getStreamTimeFrames();

    void resetFramesWritten();
    void addFramesWritten(qint64 frames);
    qint64 framesWritten();

    void resetOverflowCount();
    void incOverflowCount();
    int overflowCount();

    void setOutputDrift(bool drift);
    bool outputDrift();

    int getState();
    int getFunctionCode();
    int getRunCount();

    void debugState();

protected:
    void setState(int state);
    void setFunctionCode(int code);
    void incRunCount();

private:
    double m_sampleRate;
    int m_numOutputChannels;

    int m_workerState;
    int m_functionCode;
    int m_runCount;

    qint64 m_streamStartTimeUs;
    qint64 m_streamFramesWritten;
    int m_writeOverflowCount;
    bool m_outputDrift;
};

typedef QSharedPointer<NetworkOutputStreamWorker> NetworkOutputStreamWorkerPtr;
