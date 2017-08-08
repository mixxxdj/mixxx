#include "engine/sidechain/enginenetworkstream.h"
#include "util/sample.h"

#include "networkstreamworker.h"

NetworkStreamWorker::NetworkStreamWorker()
    : m_networkStreamWorkerState(NETWORKSTREAMWORKER_STATE_NEW),
      m_functionCode(0),
      m_runCount(0),
      m_streamStartTimeUs(-1),
      m_streamFramesWritten(0),
      m_writeOverflowCount(0),
      m_sampleRate(0),
      m_numOutputChannels(0),
      m_outputDrift(false) {
}

NetworkStreamWorker::~NetworkStreamWorker() {
}

void NetworkStreamWorker::outputAvailable() {
}

void NetworkStreamWorker::setOutputFifo(QSharedPointer<FIFO<CSAMPLE>> pOutputFifo) {
    Q_UNUSED(pOutputFifo);
}

QSharedPointer<FIFO<CSAMPLE>> NetworkStreamWorker::getOutputFifo() {
    return QSharedPointer<FIFO<CSAMPLE>>();
}

void NetworkStreamWorker::startStream(double samplerate, int numOutputChannels) {
    m_sampleRate = samplerate;
    m_numOutputChannels = numOutputChannels;

    m_streamStartTimeUs = EngineNetworkStream::getNetworkTimeUs();
    m_streamFramesWritten = 0;
}

void NetworkStreamWorker::stopStream() {
    m_sampleRate = 0;
    m_numOutputChannels = 0;

    m_streamStartTimeUs = -1;
}

bool NetworkStreamWorker::threadWaiting() {
    return false;
}

qint64 NetworkStreamWorker::getStreamTimeFrames() {
    return static_cast<double>(getStreamTimeUs()) * m_sampleRate / 1000000.0;
}

qint64 NetworkStreamWorker::getStreamTimeUs() {
    return EngineNetworkStream::getNetworkTimeUs() - m_streamStartTimeUs;
}

void NetworkStreamWorker::resetFramesWritten() {
    m_streamFramesWritten = 0;
}

void NetworkStreamWorker::addFramesWritten(qint64 frames) {
    m_streamFramesWritten += frames;
}

qint64 NetworkStreamWorker::framesWritten() {
    return m_streamFramesWritten;
}

void NetworkStreamWorker::resetOverflowCount() {
    m_writeOverflowCount = 0;
}

void NetworkStreamWorker::incOverflowCount() {
    m_writeOverflowCount++;
}

int NetworkStreamWorker::overflowCount() {
    return m_writeOverflowCount;
}

void NetworkStreamWorker::setOutputDrift(bool drift) {
    m_outputDrift = drift;
}

bool NetworkStreamWorker::outputDrift() {
    return m_outputDrift;
}

int NetworkStreamWorker::getState() {
    return m_networkStreamWorkerState;
}

int NetworkStreamWorker::getFunctionCode() {
    return m_functionCode;
}

int NetworkStreamWorker::getRunCount() {
    return m_runCount;
}

void NetworkStreamWorker::debugState() {
    qDebug() << "NetworkStreamWorker state:"
             << m_networkStreamWorkerState
             << m_functionCode
             << m_runCount;
}

void NetworkStreamWorker::setState(int state) {
    m_networkStreamWorkerState = state;
}

void NetworkStreamWorker::setFunctionCode(int code) {
    m_functionCode = code;
}

void NetworkStreamWorker::incRunCount() {
    m_runCount++;
}
