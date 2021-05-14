#pragma once

#include "util/fifo.h"
#include "util/sample.h"

class NetworkInputStreamWorker {
  public:
    NetworkInputStreamWorker();
    virtual ~NetworkInputStreamWorker() = default;

    void setSourceFifo(FIFO<CSAMPLE>* pFifo);
};
