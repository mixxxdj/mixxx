#ifndef SIDECHAINWORKER_H
#define SIDECHAINWORKER_H

#include "defs.h"

class SideChainWorker {
  public:
    SideChainWorker() { }
    virtual ~SideChainWorker() { }
    virtual void process(const CSAMPLE* pBuffer, const int iBufferSize) = 0;
    virtual void shutdown() = 0;
};

#endif /* SIDECHAINWORKER_H */
