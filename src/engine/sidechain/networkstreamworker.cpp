#include "networkstreamworker.h"

int NetworkStreamWorker::s_networkStreamWorkerState = NETWORKSTREAMWORKER_STATE_NEW;
int NetworkStreamWorker::s_functionCode = 0;
int NetworkStreamWorker::s_runCount = 0;

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
