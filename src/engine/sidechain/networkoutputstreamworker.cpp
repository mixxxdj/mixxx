#include <engine/sidechain/networkoutputstreamworker.h>
#include "engine/sidechain/enginenetworkstream.h"
#include "util/logger.h"
#include "util/sample.h"


namespace {
const mixxx::Logger kLogger("NetworkStreamWorker");
} // namespace

NetworkOutputStreamWorker::NetworkOutputStreamWorker()
    : m_sampleRate(0),
      m_numOutputChannels(0),
      m_workerState(NETWORKSTREAMWORKER_STATE_NEW),
      m_functionCode(0),
      m_runCount(0),
      m_streamStartTimeUs(-1),
      m_streamFramesWritten(0),
      m_writeOverflowCount(0),
      m_outputDrift(false) {
}

void NetworkOutputStreamWorker::outputAvailable() {
}

void NetworkOutputStreamWorker::setOutputFifo(QSharedPointer<FIFO<CSAMPLE>> pOutputFifo) {
    Q_UNUSED(pOutputFifo);
}

QSharedPointer<FIFO<CSAMPLE>> NetworkOutputStreamWorker::getOutputFifo() {
    return QSharedPointer<FIFO<CSAMPLE>>();
}

void NetworkOutputStreamWorker::startStream(double samplerate, int numOutputChannels) {
    m_sampleRate = samplerate;
    m_numOutputChannels = numOutputChannels;

    m_streamStartTimeUs = EngineNetworkStream::getNetworkTimeUs();
    m_streamFramesWritten = 0;
}

void NetworkOutputStreamWorker::stopStream() {
    m_sampleRate = 0;
    m_numOutputChannels = 0;

    m_streamStartTimeUs = -1;
}

bool NetworkOutputStreamWorker::threadWaiting() {
    return false;
}

qint64 NetworkOutputStreamWorker::getStreamTimeFrames() {
    return static_cast<qint64>(static_cast<double>(getStreamTimeUs()) * m_sampleRate / 1000000.0);
}

qint64 NetworkOutputStreamWorker::getStreamTimeUs() {
    return EngineNetworkStream::getNetworkTimeUs() - m_streamStartTimeUs;
}

void NetworkOutputStreamWorker::resetFramesWritten() {
    m_streamFramesWritten = 0;
}

void NetworkOutputStreamWorker::addFramesWritten(qint64 frames) {
    m_streamFramesWritten += frames;
}

qint64 NetworkOutputStreamWorker::framesWritten() {
    return m_streamFramesWritten;
}

void NetworkOutputStreamWorker::resetOverflowCount() {
    m_writeOverflowCount = 0;
}

void NetworkOutputStreamWorker::incOverflowCount() {
    m_writeOverflowCount++;
}

int NetworkOutputStreamWorker::overflowCount() {
    return m_writeOverflowCount;
}

void NetworkOutputStreamWorker::setOutputDrift(bool drift) {
    m_outputDrift = drift;
}

bool NetworkOutputStreamWorker::outputDrift() {
    return m_outputDrift;
}

int NetworkOutputStreamWorker::getState() {
    return m_workerState;
}

int NetworkOutputStreamWorker::getFunctionCode() {
    return m_functionCode;
}

int NetworkOutputStreamWorker::getRunCount() {
    return m_runCount;
}

void NetworkOutputStreamWorker::debugState() {
    kLogger.debug()
            << "NetworkStreamWorker state:"
            << m_workerState
            << m_functionCode
            << m_runCount;
}

void NetworkOutputStreamWorker::setState(int state) {
    m_workerState = state;
}

void NetworkOutputStreamWorker::setFunctionCode(int code) {
    m_functionCode = code;
}

void NetworkOutputStreamWorker::incRunCount() {
    m_runCount++;
}
