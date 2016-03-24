#include "networkstreamworker.h"

int NetworkStreamWorker::s_networkStreamWorkerState;
int NetworkStreamWorker::s_functionCode;
int NetworkStreamWorker::s_runCount;

NetworkStreamWorker::NetworkStreamWorker() {
}

NetworkStreamWorker::~NetworkStreamWorker() {
}

void NetworkStreamWorker::outputAvailable() {
}

void NetworkStreamWorker::setOutputFifo(FIFO<CSAMPLE>* pOutputFifo) {
    Q_UNUSED(pOutputFifo);
}

bool NetworkStreamWorker::threadWaiting() {
    return false;
}

int NetworkStreamWorker::getState() {
    return s_networkStreamWorkerState;
}

int NetworkStreamWorker::getFunctionCode() {
    return s_functionCode;
}

int NetworkStreamWorker::getRunCount() {
    return s_runCount;
}

void NetworkStreamWorker::debugState() {
    qDebug() << "NetworkStreamWorker state:"
             << s_networkStreamWorkerState
             << s_functionCode
             << s_runCount;
}

void NetworkStreamWorker::setState(int state) {
    s_networkStreamWorkerState = state;
}

void NetworkStreamWorker::setFunctionCode(int code) {
    s_functionCode = code;
}

void NetworkStreamWorker::incRunCount() {
    s_runCount++;
}
