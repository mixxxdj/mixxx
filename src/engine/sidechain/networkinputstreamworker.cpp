// networkinputstreamworker.cpp
// Create on August 11, 2017 by Palakis

#include <engine/sidechain/networkinputstreamworker.h>

template<class DataType>
class FIFO;

NetworkInputStreamWorker::NetworkInputStreamWorker() {
}

void NetworkInputStreamWorker::setSourceFifo(FIFO<CSAMPLE>* pFifo) {
    (void)pFifo;
}
