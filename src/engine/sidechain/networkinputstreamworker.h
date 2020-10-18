// networkinputstreamworker.h
// Create on August 11, 2017 by Palakis

#ifndef ENGINE_SIDECHAIN_NETWORKINPUTSTREAMWORKER_H
#define ENGINE_SIDECHAIN_NETWORKINPUTSTREAMWORKER_H

#include "util/fifo.h"
#include "util/sample.h"

class NetworkInputStreamWorker {
  public:
    NetworkInputStreamWorker();
    virtual ~NetworkInputStreamWorker() = default;

    void setSourceFifo(FIFO<CSAMPLE>* pFifo);
};

#endif // ENGINE_SIDECHAIN_NETWORKINPUTSTREAMWORKER_H
