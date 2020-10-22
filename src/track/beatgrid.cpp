#include "track/beatgrid.h"

#include <QMutexLocker>
#include <QtDebug>

#include "track/track.h"
#include "util/math.h"

static const int kFrameSize = 2;

struct BeatGridData {
    double bpm;
    double firstBeat;
};

namespace mixxx {

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

BeatGrid::BeatGrid(
        const Track& track,
        SINT iSampleRate)
        : m_mutex(QMutex::Recursive),
          m_iSampleRate(iSampleRate > 0 ? iSampleRate : track.getSampleRate()),
          m_dBeatLength(0.0) {
    // BeatGrid should live in the same thread as the track it is associated
    // with.
    moveToThread(track.thread());
}

BeatGrid::BeatGrid(
        const Track& track,
        SINT iSampleRate,
        const QByteArray& byteArray)
        : BeatGrid(track, iSampleRate) {
    readByteArray(byteArray);
}

BeatGrid::BeatGrid(const BeatGrid& other)
        : m_mutex(QMutex::Recursive),
          m_subVersion(other.m_subVersion),
          m_iSampleRate(other.m_iSampleRate),
          m_grid(other.m_grid),
          m_dBeatLength(other.m_dBeatLength) {
    moveToThread(other.thread());
}

void BeatGrid::setGrid(double dBpm, double dFirstBeatSample) {
    if (dBpm < 0) {
        dBpm = 0.0;
    }

    QMutexLocker lock(&m_mutex);
    m_grid.mutable_bpm()->set_bpm(dBpm);
    m_grid.mutable_first_beat()->set_frame_position(
            static_cast<google::protobuf::int32>(dFirstBeatSample / kFrameSize));
    // Calculate beat length as sample offsets
    m_dBeatLength = (60.0 * m_iSampleRate / dBpm) * kFrameSize;
}

QByteArray BeatGrid::toByteArray() const {
    QMutexLocker locker(&m_mutex);
    std::string output;
    m_grid.SerializeToString(&output);
    return QByteArray(output.data(), output.length());
}

BeatsPointer BeatGrid::clone() const {
    QMutexLocker locker(&m_mutex);
    BeatsPointer other(new BeatGrid(*this));
    return other;
}

void BeatGrid::readByteArray(const QByteArray& byteArray) {
    mixxx::track::io::BeatGrid grid;
    if (grid.ParseFromArray(byteArray.constData(), byteArray.length())) {
        m_grid = grid;
        m_dBeatLength = (60.0 * m_iSampleRate / bpm()) * kFrameSize;
        return;
    }

    // Legacy fallback for BeatGrid-1.0
    if (byteArray.size() != sizeof(BeatGridData)) {
        return;
    }
    const BeatGridData* blob = reinterpret_cast<const BeatGridData*>(byteArray.constData());

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
    double prevBeat;
    double nextBeat;
    findPrevNextBeats(dSamples, &prevBeat, &nextBeat);
    if (prevBeat == -1) {
        // If both values are -1, we correctly return -1.
        return nextBeat;
    } else if (nextBeat == -1) {
        return prevBeat;
    }
    return (nextBeat - dSamples > dSamples - prevBeat) ? prevBeat : nextBeat;
}

double BeatGrid::findNthBeat(double dSamples, int n) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || n == 0) {
        return -1;
    }

    double beatFraction = (dSamples - firstBeatSample()) / m_dBeatLength;
    double prevBeat = floor(beatFraction);
    double nextBeat = ceil(beatFraction);

    // If the position is within 1/100th of the next or previous beat, treat it
    // as if it is that beat.
    const double kEpsilon = .01;

    if (fabs(nextBeat - beatFraction) < kEpsilon) {
        // If we are going to pretend we were actually on nextBeat then prevBeat
        // needs to be re-calculated. Since it is floor(beatFraction), that's
        // the same as nextBeat.  We only use prevBeat so no need to increment
        // nextBeat.
        prevBeat = nextBeat;
    } else if (fabs(prevBeat - beatFraction) < kEpsilon) {
        // If we are going to pretend we were actually on prevBeat then nextBeat
        // needs to be re-calculated. Since it is ceil(beatFraction), that's
        // the same as prevBeat.  We will only use nextBeat so no need to
        // decrement prevBeat.
        nextBeat = prevBeat;
    }

    double dClosestBeat;
    if (n > 0) {
        // We're going forward, so use ceil to round up to the next multiple of
        // m_dBeatLength
        dClosestBeat = nextBeat * m_dBeatLength + firstBeatSample();
        n = n - 1;
    } else {
        // We're going backward, so use floor to round down to the next multiple
        // of m_dBeatLength
        dClosestBeat = prevBeat * m_dBeatLength + firstBeatSample();
        n = n + 1;
    }

    double dResult = dClosestBeat + n * m_dBeatLength;
    return dResult;
}

bool BeatGrid::findPrevNextBeats(double dSamples,
                                 double* dpPrevBeatSamples,
                                 double* dpNextBeatSamples) const {
    double dFirstBeatSample;
    double dBeatLength;
    {
        QMutexLocker locker(&m_mutex);
        if (!isValid()) {
            *dpPrevBeatSamples = -1.0;
            *dpNextBeatSamples = -1.0;
            return false;
        }
        dFirstBeatSample = firstBeatSample();
        dBeatLength = m_dBeatLength;
    }

    double beatFraction = (dSamples - dFirstBeatSample) / dBeatLength;
    double prevBeat = floor(beatFraction);
    double nextBeat = ceil(beatFraction);

    // If the position is within 1/100th of the next or previous beat, treat it
    // as if it is that beat.
    const double kEpsilon = .01;

    if (fabs(nextBeat - beatFraction) < kEpsilon) {
        beatFraction = nextBeat;
        // If we are going to pretend we were actually on nextBeat then prevBeatFraction
        // needs to be re-calculated. Since it is floor(beatFraction), that's
        // the same as nextBeat.
        prevBeat = nextBeat;
        // And nextBeat needs to be incremented.
        ++nextBeat;
    }
    *dpPrevBeatSamples = prevBeat * dBeatLength + dFirstBeatSample;
    *dpNextBeatSamples = nextBeat * dBeatLength + dFirstBeatSample;
    return true;
}


std::unique_ptr<BeatIterator> BeatGrid::findBeats(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return std::unique_ptr<BeatIterator>();
    }
    //qDebug() << "BeatGrid::findBeats startSample" << startSample << "stopSample"
    //         << stopSample << "beatlength" << m_dBeatLength << "BPM" << bpm();
    double curBeat = findNextBeat(startSample);
    if (curBeat == -1.0) {
        return std::unique_ptr<BeatIterator>();
    }
    return std::make_unique<BeatGridIterator>(m_dBeatLength, curBeat, stopSample);
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

double BeatGrid::getBpmAroundPosition(double curSample, int n) const {
    Q_UNUSED(curSample);
    Q_UNUSED(n);

    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    return bpm();
}

void BeatGrid::addBeat(double dBeatSample) {
    Q_UNUSED(dBeatSample);
    //QMutexLocker locker(&m_mutex);
    return;
}

void BeatGrid::removeBeat(double dBeatSample) {
    Q_UNUSED(dBeatSample);
    //QMutexLocker locker(&m_mutex);
    return;
}

void BeatGrid::translate(double dNumSamples) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }
    double newFirstBeatFrames = (firstBeatSample() + dNumSamples) / kFrameSize;
    m_grid.mutable_first_beat()->set_frame_position(
            static_cast<google::protobuf::int32>(newFirstBeatFrames));
    locker.unlock();
    emit updated();
}

void BeatGrid::scale(enum BPMScale scale) {
    double bpm = getBpm();

    switch (scale) {
    case DOUBLE:
        bpm *= 2;
        break;
    case HALVE:
        bpm *= 1.0 / 2;
        break;
    case TWOTHIRDS:
        bpm *= 2.0 / 3;
        break;
    case THREEFOURTHS:
        bpm *= 3.0 / 4;
        break;
    case FOURTHIRDS:
        bpm *= 4.0 / 3;
        break;
    case THREEHALVES:
        bpm *= 3.0 / 2;
        break;
    default:
        DEBUG_ASSERT(!"scale value invalid");
        return;
    }
    setBpm(bpm);
}

void BeatGrid::setBpm(double dBpm) {
    QMutexLocker locker(&m_mutex);
    if (dBpm > getMaxBpm()) {
        dBpm = getMaxBpm();
    }
    m_grid.mutable_bpm()->set_bpm(dBpm);
    m_dBeatLength = (60.0 * m_iSampleRate / dBpm) * kFrameSize;
    locker.unlock();
    emit updated();
}

} // namespace mixxx
