// networkinputstreamworker.cpp
// Create on August 11, 2017 by Palakis

#include <engine/sidechain/networkinputstreamworker.h>

NetworkInputStreamWorker::NetworkInputStreamWorker()
    : m_sampleRate(0),
      m_numInputChannels(0) {
}

NetworkInputStreamWorker::~NetworkInputStreamWorker() {
}

void NetworkInputStreamWorker::setSourceFifo(FIFO<CSAMPLE>* pFifo) {
    (void)pFifo;
}
