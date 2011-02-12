#include <QMutexLocker>

#include "beatgrid.h"

BeatGrid::BeatGrid(QObject* pParent, TrackPointer pTrack, QByteArray* pByteArray)
        : QObject(pParent),
          m_mutex(QMutex::Recursive),
          m_iSampleRate(pTrack->getSampleRate()),
          m_dBpm(0.0f),
          m_dFirstBeat(0.0f),
          m_dBeatLength(0.0f) {
    connect(pTrack.data(), SIGNAL(bpmUpdated(double)),
            this, SLOT(slotTrackBpmUpdated(double)));

    // TODO(XXX) setBpm isn't a slot
    // connect(this, SIGNAL(bpmUpdated(double)),
    //         pTrack.data(), SLOT(setBpm(double)));

    if (pByteArray != NULL) {
        readByteArray(pByteArray);
    }

}

BeatGrid::~BeatGrid() {

}

QByteArray* BeatGrid::toByteArray() const {
    QMutexLocker locker(&m_mutex);
    double* pBuffer = new double[2];
    pBuffer[0] = m_dBpm;
    pBuffer[1] = m_dFirstBeat;
    QByteArray* pByteArray = new QByteArray((char*)pBuffer, sizeof(double) * 2);
    delete [] pBuffer;
    return pByteArray;
}

void BeatGrid::readByteArray(QByteArray* pByteArray) {
    if (pByteArray->size() != sizeof(double) * 2) {
        return;
    }
    double* pBuffer = (double*)pByteArray->data();
    m_dBpm = pBuffer[0];
    m_dFirstBeat = pBuffer[1];
    m_dBeatLength = 60.0 * m_iSampleRate / m_dBpm;
}

QString BeatGrid::getVersion() const {
    QMutexLocker locker(&m_mutex);
    return "BeatGrid-1.0";
}

// internal use only
bool BeatGrid::isValid() const {
    return m_iSampleRate > 0 && m_dBpm > 0;
}

double BeatGrid::findNextBeat(double dSamples) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    return ceilf(dSamples/m_dBeatLength)*m_dBeatLength;
}

double BeatGrid::findPrevBeat(double dSamples) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    return floorf(dSamples/m_dBeatLength)*m_dBeatLength;
}

double BeatGrid::findClosestBeat(double dSamples) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    double nextBeat = ceilf(dSamples/m_dBeatLength)*m_dBeatLength;
    double prevBeat = floorf(dSamples/m_dBeatLength)*m_dBeatLength;
    return (nextBeat - dSamples > dSamples - prevBeat) ? prevBeat : nextBeat;
}

void BeatGrid::findBeats(double startSample, double stopSample, QList<double>* pBeatsList) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }
    double curBeat = ceilf(startSample/m_dBeatLength)*m_dBeatLength;
    while (curBeat <= stopSample) {
        pBeatsList->append(curBeat);
        curBeat += m_dBeatLength;
    }
}

bool BeatGrid::hasBeatInRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return false;
    }
    double curBeat = ceilf(startSample/m_dBeatLength)*m_dBeatLength;
    if (curBeat <= stopSample) {
        return true;
    }
    return false;
}

double BeatGrid::getBpm() const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    return m_dBpm;
}

double BeatGrid::getBpmRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    return m_dBpm;
}

void BeatGrid::addBeat(double dBeatSample) {
    //QMutexLocker locker(&m_mutex);
    return;
}

void BeatGrid::removeBeat(double dBeatSample) {
    //QMutexLocker locker(&m_mutex);
    return;
}

void BeatGrid::moveBeat(double dBeatSample, double dNewBeatSample) {
    //QMutexLocker locker(&m_mutex);
    return;
}

void BeatGrid::translate(double dNumSamples) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }
    m_dFirstBeat += dNumSamples;
    locker.unlock();
    emit(updated());
}

void BeatGrid::scale(double dScalePercentage) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }
    m_dBpm *= dScalePercentage;
    locker.unlock();
    emit(updated());
}

void BeatGrid::slotTrackBpmUpdated(double dBpm) {
    QMutexLocker locker(&m_mutex);
    m_dBpm = dBpm;
}
