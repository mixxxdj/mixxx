#pragma once

#include "util/types.h"

template<class DataType>
class FIFO;

class NetworkInputStreamWorker {
  public:
    NetworkInputStreamWorker();
    virtual ~NetworkInputStreamWorker() = default;

    void setSourceFifo(FIFO<CSAMPLE>* pFifo);
};
