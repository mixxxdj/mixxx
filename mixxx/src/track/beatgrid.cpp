#include <QMutexLocker>
#include <QDebug>

#include "track/beatgrid.h"
#include "mathstuff.h"

static int kFrameSize = 2;

struct BeatGridData {
	double bpm;
	double firstBeat;
};

BeatGrid::BeatGrid(TrackPointer pTrack, const QByteArray* pByteArray)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_iSampleRate(pTrack->getSampleRate()),
          m_dBpm(0.0),
          m_dFirstBeat(0.0f),
          m_dBeatLength(0.0f) {
    connect(pTrack.data(), SIGNAL(bpmUpdated(double)),
            this, SLOT(slotTrackBpmUpdated(double)),
            Qt::DirectConnection);
    slotTrackBpmUpdated(pTrack->getBpm());

    qDebug() << "New BeatGrid";
    if (pByteArray != NULL) {
        readByteArray(pByteArray);
    }
}

BeatGrid::~BeatGrid() {

}

void BeatGrid::setGrid(double dBpm, double dFirstBeatSample) {
    QMutexLocker lock(&m_mutex);
    m_dBpm = dBpm;
    m_dFirstBeat = dFirstBeatSample;
    // Calculate beat length as sample offsets
    m_dBeatLength = (60.0 * m_iSampleRate / m_dBpm) * kFrameSize;
}

QByteArray* BeatGrid::toByteArray() const {
    QMutexLocker locker(&m_mutex);
    BeatGridData blob = { m_dBpm, (m_dFirstBeat / kFrameSize) };
    QByteArray* pByteArray = new QByteArray((char *)&blob, sizeof(blob));
    // Caller is responsible for delete
    return pByteArray;
}

void BeatGrid::readByteArray(const QByteArray* pByteArray) {
    if ( pByteArray->size() != sizeof(BeatGridData))
        return;
    BeatGridData *blob = (BeatGridData *)pByteArray->data();
    // We serialize into frame offsets but use sample offsets at runtime
    setGrid(blob->bpm, blob->firstBeat * kFrameSize);
}

QString BeatGrid::getVersion() const {
    QMutexLocker locker(&m_mutex);
    return "BeatGrid-1.0";
}

// internal use only
bool BeatGrid::isValid() const {
    return m_iSampleRate > 0 && m_dBpm > 0;
}

// This could be implemented in the Beats Class itself.
// If necessary, the child class can redefine it.
double BeatGrid::findNextBeat(double dSamples) const {
    return findNthBeat(dSamples, +1);
}

// This could be implemented in the Beats Class itself.
// If necessary, the child class can redefine it.
double BeatGrid::findPrevBeat(double dSamples) const {
    return findNthBeat(dSamples, -1);
}

// This is an internal call. This could be implemented in the Beats Class itself.
double BeatGrid::findClosestBeat(double dSamples) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    double nextBeat = findNextBeat(dSamples);
    double prevBeat = findPrevBeat(dSamples);
    return (nextBeat - dSamples > dSamples - prevBeat) ? prevBeat : nextBeat;
}

double BeatGrid::findNthBeat(double dSamples, int n) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || n == 0) {
        return -1;
    }

    double beatFraction = (dSamples - m_dFirstBeat) / m_dBeatLength;
    double prevBeat = floorf(beatFraction);
    double nextBeat = ceilf(beatFraction);

    // If the position is within 1/100th of the next or previous beat, treat it
    // as if it is that beat.
    const double kEpsilon = .01;

    if (fabs(nextBeat - beatFraction) < kEpsilon) {
        beatFraction = nextBeat;
    } else if (fabs(prevBeat - beatFraction) < kEpsilon) {
        beatFraction = prevBeat;
    }

    double dClosestBeat;
    if (n > 0) {
        // We're going forward, so use ceilf to round up to the next multiple of
        // m_dBeatLength
        dClosestBeat = ceilf(beatFraction) * m_dBeatLength + m_dFirstBeat;
        n = n - 1;
    } else {
        // We're going backward, so use floorf to round down to the next multiple
        // of m_dBeatLength
        dClosestBeat = floorf(beatFraction) * m_dBeatLength + m_dFirstBeat;
        n = n + 1;
    }

    double dResult = dClosestBeat + n * m_dBeatLength;
    if (!even(dResult)) {
        dResult--;
    }
    return dResult;
}

void BeatGrid::findBeats(double startSample, double stopSample, QList<double>* pBeatsList) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return;
    }
    double curBeat = findNextBeat(startSample);
    while (curBeat <= stopSample) {
        pBeatsList->append(curBeat);
        curBeat += m_dBeatLength;
    }
}

bool BeatGrid::hasBeatInRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return false;
    }
    double curBeat = findNextBeat(startSample);
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
    if (!isValid() || startSample > stopSample) {
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
    m_dBeatLength = (60.0 * m_iSampleRate / m_dBpm) * kFrameSize;
}
