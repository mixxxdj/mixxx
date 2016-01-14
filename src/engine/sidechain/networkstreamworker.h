#ifndef NETWORKSTREAMWORKER_H
#define NETWORKSTREAMWORKER_H

#include "util/types.h"
#include "util/fifo.h"

/*
 * States:
 * Error        Something errornous has happened and can't go on
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

enum NetworkStreamWorkerStates {
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

class NetworkStreamWorker {
  public:
    NetworkStreamWorker()
          : m_networkStreamWorkerState(NETWORKSTREAMWORKER_STATE_NEW) {
    }
    virtual ~NetworkStreamWorker() { }
    virtual void process(const CSAMPLE* pBuffer, const int iBufferSize) = 0;
    virtual void shutdown() = 0;
    virtual void outputAvailable() {
    };
    virtual void setOutputFifo(FIFO<CSAMPLE>* pOutputFifo) {
        Q_UNUSED(pOutputFifo);
    };
    virtual bool threadWaiting() {
        return false;
    }
    virtual int getState() {
        return m_networkStreamWorkerState;
    }
protected:
    virtual void setState(int state) {
        m_networkStreamWorkerState = state;
    }
private:
    int m_networkStreamWorkerState;
};

#endif /* NETWORKSTREAMWORKER_H */
