#ifndef SIDECHAINWORKER_H
#define SIDECHAINWORKER_H

#include "util/types.h"
#include "util/fifo.h"

/*
 * States:
 * Error        Something errornous has happenned and can't go on
 * Unknown      First state before init
 * Init         Initing state don't feed anything in this state
 * Waiting      Waiting something not ready yet
 * Busy         Is busy doing something can't process anything new
 * Ready        Functioning ok
 * Reading      Reading something and can't do anything else
 * Writing      Writing something and can't do anything else
 * Connected    Is connected to storage or server
 * Connecting   Trying to connect storate or server
 * Disconnected Ain't connected to storage or server
 * 
 * First state should be SIDECHAINWORKER_STATE_UNKNOWN and
 * if state handling ain't supported by SideChainWorker-class
 * then SIDECHAINWORKER_STATE_UNKNOWN should be treated as
 * SIDECHAINWORKER_STATE_READY. Newly written SideChainWorker-class
 * should support state handling at leas this SIDECHAINWORKER_STATE_READY state.
 */

enum SidechaingStates {
    SIDECHAINWORKER_STATE_ERROR = -1,
    SIDECHAINWORKER_STATE_UNKNOWN,
    SIDECHAINWORKER_STATE_INIT,
    SIDECHAINWORKER_STATE_WAITING,
    SIDECHAINWORKER_STATE_BUSY,
    SIDECHAINWORKER_STATE_READY,
    SIDECHAINWORKER_STATE_READING,
    SIDECHAINWORKER_STATE_WRITING,
    SICECHAINWORKER_STATE_CONNECTED,
    SICECHAINWORKER_STATE_CONNECTING,
    SIDECHAINWORKER_STATE_DISCONNECTED
};

class SideChainWorker {
  public:
    SideChainWorker() :
    m_iSideChainWorkerState(SIDECHAINWORKER_STATE_UNKNOWN),
    m_sErrorMsg("") { }
    virtual ~SideChainWorker() { }
    virtual void process(const CSAMPLE* pBuffer, const int iBufferSize) = 0;
    virtual void shutdown() = 0;
    virtual void outputAvailabe() {
    };
    virtual void setOutputFifo(FIFO<CSAMPLE>* pOutputFifo) {
        Q_UNUSED(pOutputFifo);
    };
    virtual bool threadWaiting() {
        return false;
    }
    virtual int getState() {
        return m_iSideChainWorkerState;
    }
    virtual QString getErrorMessage() {
        return m_sErrorMsg;
    }
protected:
    virtual void setState(int state) {
        m_iSideChainWorkerState = state;
    }
    virtual void setErrorMessage(QString msg) {
        m_sErrorMsg = msg;
    }
private:
    int m_iSideChainWorkerState;
    QString m_sErrorMsg;
};

#endif /* SIDECHAINWORKER_H */
