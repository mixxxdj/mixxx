#ifndef SIDECHAINWORKER_H
#define SIDECHAINWORKER_H

#include "util/types.h"
#include "util/fifo.h"

class SideChainWorker {
  public:
    SideChainWorker() { }
    virtual ~SideChainWorker() { }
    virtual void process(const CSAMPLE* pBuffer, const int iBufferSize) = 0;
    virtual void shutdown() = 0;
    virtual void outputAvailabe(FIFO<CSAMPLE>* pOutputFifo) {
        Q_UNUSED(pOutputFifo);
    };
    virtual bool threadWaiting() {
        return false;
    }
};

#endif /* SIDECHAINWORKER_H */
