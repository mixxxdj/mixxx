#include <QtDebug>
#include <QMutexLocker>

#include "beatmatrix.h"

BeatMatrix::BeatMatrix(TrackPointer pTrack, QByteArray* pByteArray)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_iSampleRate(pTrack->getSampleRate()) {
    if (pByteArray != NULL) {
        readByteArray(pByteArray);
    }
}

BeatMatrix::~BeatMatrix() {

}

unsigned int BeatMatrix::numBeats() const {
    return m_beatList.size();
}

QByteArray* BeatMatrix::toByteArray() const {
    QMutexLocker locker(&m_mutex);
    // No guarantees BeatLists are made of a data type which located adjacent
    // items in adjacent memory locations.
    double* pBuffer = new double[m_beatList.size()];
    for (int i = 0; i < m_beatList.size(); ++i) {
        pBuffer[i] = m_beatList[i];
    }
    QByteArray* pByteArray = new QByteArray((char*)pBuffer, sizeof(pBuffer[0]) * m_beatList.size());
    delete [] pBuffer;
    return pByteArray;
}

void BeatMatrix::readByteArray(QByteArray* pByteArray) {
    if (pByteArray->size() % sizeof(double) != 0) {
        qDebug() << "ERROR: Could not parse BeatMatrix from QByteArray of size" << pByteArray->size();
        return;
    }

    int numBeats = pByteArray->size() / sizeof(double);
    double* pBuffer = (double*)pByteArray->data();
    for (int i = 0; i < numBeats; ++i) {
        m_beatList.append(pBuffer[i]);
    }
}


QString BeatMatrix::getVersion() const {
    QMutexLocker locker(&m_mutex);
    return "BeatMatrix-1.0";
}

// internal use only
bool BeatMatrix::isValid() const {
    return m_iSampleRate > 0 && m_beatList.size() > 0;
}

double BeatMatrix::findNextBeat(double dSamples) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    BeatList::const_iterator it = qLowerBound(m_beatList.begin(),
                                              m_beatList.end(), dSamples);

    if (it != m_beatList.end()) {
        return *it;
    }

    return -1;
}

double BeatMatrix::findPrevBeat(double dSamples) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }

    BeatList::const_iterator it = qLowerBound(m_beatList.begin(),
                                              m_beatList.end(), dSamples);

    if (it != m_beatList.begin()) {
        it--;
        return *it;
    }
    return -1;
}

double BeatMatrix::findClosestBeat(double dSamples) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    double nextBeat = findNextBeat(dSamples);
    double prevBeat = findPrevBeat(dSamples);
    return (nextBeat - dSamples > dSamples - prevBeat) ? prevBeat : nextBeat;
}

void BeatMatrix::findBeats(double startSample, double stopSample, QList<double>* pBeatsList) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return;
    }
    BeatList::const_iterator curBeat = qLowerBound(m_beatList.begin(),
                                                   m_beatList.end(),
                                                   startSample);
    BeatList::const_iterator stopBeat = qUpperBound(m_beatList.begin(),
                                                    m_beatList.end(),
                                                    stopSample);

    for (; curBeat != stopBeat; curBeat++) {
        pBeatsList->append(*curBeat);
    }
}

bool BeatMatrix::hasBeatInRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return false;
    }
    BeatList::const_iterator startBeat = qLowerBound(m_beatList.begin(),
                                                     m_beatList.end(),
                                                     startSample);
    BeatList::const_iterator stopBeat = qUpperBound(m_beatList.begin(),
                                                    m_beatList.end(),
                                                    stopSample);

    if (startBeat != stopBeat)
        return true;
    return false;
}

double BeatMatrix::getBpm() const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }

    // TODO(XXX) not actually correct. We need the true song length.
    double startSample = *m_beatList.begin();
    double stopSample = *(m_beatList.end()-1);
    double songDurationMinutes =
            (stopSample - startSample) / (60.0f * m_iSampleRate);
    return m_beatList.size() / songDurationMinutes;
}

double BeatMatrix::getBpmRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return -1;
    }

    BeatList::const_iterator startBeat = qLowerBound(m_beatList.begin(),
                                               m_beatList.end(),
                                               startSample);
    BeatList::const_iterator stopBeat = qUpperBound(m_beatList.begin(),
                                                    m_beatList.end(),
                                                    stopSample);
    double rangeDurationMinutes =
            (stopSample - startSample) / (60.0f * m_iSampleRate);
    // Subtracting returns the number of beats between the samples referred to
    // by the start and end.
    double beatsInRange = stopBeat - startBeat;

    return beatsInRange / rangeDurationMinutes;
}

void BeatMatrix::addBeat(double dBeatSample) {
    QMutexLocker locker(&m_mutex);

    BeatList::iterator it = qLowerBound(m_beatList.begin(),
                                        m_beatList.end(),
                                        dBeatSample);
    // Don't insert a duplicate beat. TODO(XXX) determine what epsilon to
    // consider a beat identical to another.
    if (*it == dBeatSample)
        return;

    m_beatList.insert(it, dBeatSample);
    locker.unlock();
    emit(updated());
}

void BeatMatrix::removeBeat(double dBeatSample) {
    QMutexLocker locker(&m_mutex);
    BeatList::iterator it = qLowerBound(m_beatList.begin(),
                                        m_beatList.end(), dBeatSample);

    // In case there are duplicates, remove every instance of dBeatSample
    // TODO(XXX) add invariant checks against this
    // TODO(XXX) determine what epsilon to consider a beat identical to another
    while (*it == dBeatSample) {
        it = m_beatList.erase(it);
    }
    locker.unlock();
    emit(updated());
}

void BeatMatrix::moveBeat(double dBeatSample, double dNewBeatSample) {
    QMutexLocker locker(&m_mutex);

    BeatList::iterator it = qLowerBound(m_beatList.begin(),
                                        m_beatList.end(), dBeatSample);

    // Remove all beats from dBeatSample
    while (*it == dBeatSample) {
        it = m_beatList.erase(it);
    }

    // Now add a beat to dNewBeatSample
    it = qLowerBound(m_beatList.begin(),
                     m_beatList.end(), dNewBeatSample);

    // TODO(XXX) beat epsilon
    if (*it != dNewBeatSample) {
        m_beatList.insert(it, dNewBeatSample);
    }
    locker.unlock();
    emit(updated());
}

void BeatMatrix::translate(double dNumSamples) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }

    for (BeatList::iterator it = m_beatList.begin();
         it != m_beatList.end(); ++it) {
        *it += dNumSamples;
    }
    locker.unlock();
    emit(updated());
}

void BeatMatrix::scale(double dScalePercentage) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }
    for (BeatList::iterator it = m_beatList.begin();
         it != m_beatList.end(); ++it) {
        *it *= dScalePercentage;
    }
    locker.unlock();
    emit(updated());
}

void BeatMatrix::slotTrackBpmUpdated(double dBpm) {
    //QMutexLocker locker(&m_mutex);
    // TODO(XXX) How do we handle this?
}
