#include <QMutexLocker>
#include <QDebug>

#include "track/beatgrid.h"
#include "mathstuff.h"

static const int kFrameSize = 2;

struct BeatGridData {
	double bpm;
	double firstBeat;
};

class BeatGridIterator : public BeatIterator {
  public:
    BeatGridIterator(double dBeatLength, double dFirstBeat, double dEndSample)
            : m_dBeatLength(dBeatLength),
              m_dCurrentSample(dFirstBeat),
              m_dEndSample(dEndSample) {
    }

    virtual bool hasNext() const {
        return m_dBeatLength > 0 && m_dCurrentSample <= m_dEndSample;
    }

    virtual double next() {
        double beat = m_dCurrentSample;
        m_dCurrentSample += m_dBeatLength;
        return beat;
    }

  private:
    double m_dBeatLength;
    double m_dCurrentSample;
    double m_dEndSample;
};

BeatGrid::BeatGrid(TrackInfoObject* pTrack, const QByteArray* pByteArray)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_iSampleRate(pTrack->getSampleRate()),
          m_dBeatLength(0.0f) {
    qDebug() << "New BeatGrid";
    if (pByteArray != NULL) {
        readByteArray(pByteArray);
    }
}

BeatGrid::~BeatGrid() {
}

void BeatGrid::setGrid(double dBpm, double dFirstBeatSample) {
    if (dBpm < 0) {
        dBpm = 0.0;
    }

    QMutexLocker lock(&m_mutex);
    m_grid.mutable_bpm()->set_bpm(dBpm);
    m_grid.mutable_first_beat()->set_frame_position(dFirstBeatSample / kFrameSize);
    // Calculate beat length as sample offsets
    m_dBeatLength = (60.0 * m_iSampleRate / dBpm) * kFrameSize;
}

QByteArray* BeatGrid::toByteArray() const {
    QMutexLocker locker(&m_mutex);
    std::string output;
    m_grid.SerializeToString(&output);
    QByteArray* pByteArray = new QByteArray(output.data(), output.length());
    // Caller is responsible for delete
    return pByteArray;
}

void BeatGrid::readByteArray(const QByteArray* pByteArray) {
    mixxx::track::io::BeatGrid grid;
    if (grid.ParseFromArray(pByteArray->constData(), pByteArray->length())) {
        m_grid = grid;
        m_dBeatLength = (60.0 * m_iSampleRate / bpm()) * kFrameSize;
        return;
    }

    // Legacy fallback for BeatGrid-1.0
    if (pByteArray->size() != sizeof(BeatGridData))
        return;
    const BeatGridData* blob = (const BeatGridData*)pByteArray->constData();

    // We serialize into frame offsets but use sample offsets at runtime
    setGrid(blob->bpm, blob->firstBeat * kFrameSize);
}

double BeatGrid::firstBeatSample() const {
    return m_grid.first_beat().frame_position() * kFrameSize;
}

double BeatGrid::bpm() const {
    return m_grid.bpm().bpm();
}

QString BeatGrid::getVersion() const {
    QMutexLocker locker(&m_mutex);
    return BEAT_GRID_2_VERSION;
}

QString BeatGrid::getSubVersion() const {
    QMutexLocker locker(&m_mutex);
    return m_subVersion;
}

void BeatGrid::setSubVersion(QString subVersion) {
    m_subVersion = subVersion;
}

// internal use only
bool BeatGrid::isValid() const {
    return m_iSampleRate > 0 && bpm() > 0;
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

    double beatFraction = (dSamples - firstBeatSample()) / m_dBeatLength;
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
        dClosestBeat = ceilf(beatFraction) * m_dBeatLength + firstBeatSample();
        n = n - 1;
    } else {
        // We're going backward, so use floorf to round down to the next multiple
        // of m_dBeatLength
        dClosestBeat = floorf(beatFraction) * m_dBeatLength + firstBeatSample();
        n = n + 1;
    }

    double dResult = dClosestBeat + n * m_dBeatLength;
    if (!even(dResult)) {
        dResult--;
    }
    return dResult;
}

BeatIterator* BeatGrid::findBeats(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return NULL;
    }
    // qDebug() << "BeatGrid::findBeats startSample" << startSample << "stopSample"
    //          << stopSample << "beatlength" << m_dBeatLength << "BPM" << bpm();
    double curBeat = findNextBeat(startSample);
    if (curBeat == -1.0) {
        return NULL;
    }
    return new BeatGridIterator(m_dBeatLength, curBeat, stopSample);
}

bool BeatGrid::hasBeatInRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return false;
    }
    double curBeat = findNextBeat(startSample);
    if (curBeat != -1.0 && curBeat <= stopSample) {
        return true;
    }
    return false;
}

double BeatGrid::getBpm() const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return 0;
    }
    return bpm();
}

double BeatGrid::getBpmRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return -1;
    }
    return bpm();
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
    double newFirstBeatFrames = (firstBeatSample() + dNumSamples) / kFrameSize;
    m_grid.mutable_first_beat()->set_frame_position(newFirstBeatFrames);
    locker.unlock();
    emit(updated());
}

void BeatGrid::scale(double dScalePercentage) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }
    double newBpm = bpm() * dScalePercentage;
    m_grid.mutable_bpm()->set_bpm(newBpm);
    m_dBeatLength = (60.0 * m_iSampleRate / newBpm) * kFrameSize;
    locker.unlock();
    emit(updated());
}

void BeatGrid::setBpm(double dBpm) {
    QMutexLocker locker(&m_mutex);
    m_grid.mutable_bpm()->set_bpm(dBpm);
    m_dBeatLength = (60.0 * m_iSampleRate / dBpm) * kFrameSize;
    locker.unlock();
    emit(updated());
}
