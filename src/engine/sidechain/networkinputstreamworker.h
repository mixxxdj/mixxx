// networkinputstreamworker.h
// Create on August 11, 2017 by Palakis

#ifndef ENGINE_SIDECHAIN_NETWORKINPUTSTREAMWORKER_H
#define ENGINE_SIDECHAIN_NETWORKINPUTSTREAMWORKER_H

#include "util/fifo.h"
#include "util/sample.h"

class NetworkInputStreamWorker {
  public:
    NetworkInputStreamWorker();
    virtual ~NetworkInputStreamWorker();

    void setSourceFifo(FIFO<CSAMPLE>* pFifo);

  private:
    double m_sampleRate;
    int m_numInputChannels;
};

#endif // ENGINE_SIDECHAIN_NETWORKINPUTSTREAMWORKER_H
