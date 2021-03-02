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

    bool hasNext() const override {
        return m_dBeatLength > 0 && m_dCurrentSample <= m_dEndSample;
    }

    double next() override {
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
        SINT iSampleRate,
        const QString& subVersion,
        const mixxx::track::io::BeatGrid& grid,
        double beatLength)
        : m_subVersion(subVersion),
          m_iSampleRate(iSampleRate),
          m_grid(grid),
          m_dBeatLength(beatLength) {
    // BeatGrid should live in the same thread as the track it is associated
    // with.
}

BeatGrid::BeatGrid(const BeatGrid& other, const mixxx::track::io::BeatGrid& grid, double beatLength)
        : m_subVersion(other.m_subVersion),
          m_iSampleRate(other.m_iSampleRate),
          m_grid(grid),
          m_dBeatLength(beatLength) {
}

BeatGrid::BeatGrid(const BeatGrid& other)
        : BeatGrid(other, other.m_grid, other.m_dBeatLength) {
}

// static
BeatsPointer BeatGrid::makeBeatGrid(
        SINT iSampleRate,
        const QString& subVersion,
        double dBpm,
        double dFirstBeatSample) {
    if (dBpm < 0) {
        dBpm = 0.0;
    }

    mixxx::track::io::BeatGrid grid;

    grid.mutable_bpm()->set_bpm(dBpm);
    grid.mutable_first_beat()->set_frame_position(
            static_cast<google::protobuf::int32>(dFirstBeatSample / kFrameSize));
    // Calculate beat length as sample offsets
    double beatLength = (60.0 * iSampleRate / dBpm) * kFrameSize;

    return BeatsPointer(new BeatGrid(iSampleRate, subVersion, grid, beatLength));
}

// static
BeatsPointer BeatGrid::makeBeatGrid(
        SINT sampleRate,
        const QString& subVersion,
        const QByteArray& byteArray) {
    mixxx::track::io::BeatGrid grid;
    if (grid.ParseFromArray(byteArray.constData(), byteArray.length())) {
        double beatLength = (60.0 * sampleRate / grid.bpm().bpm()) * kFrameSize;
        return BeatsPointer(new BeatGrid(sampleRate, subVersion, grid, beatLength));
    }

    // Legacy fallback for BeatGrid-1.0
    if (byteArray.size() != sizeof(BeatGridData)) {
        return BeatsPointer(new BeatGrid(sampleRate, QString(), grid, 0));
    }
    const BeatGridData* blob = reinterpret_cast<const BeatGridData*>(byteArray.constData());

    // We serialize into frame offsets but use sample offsets at runtime
    return makeBeatGrid(sampleRate, subVersion, blob->bpm, blob->firstBeat * kFrameSize);
}

QByteArray BeatGrid::toByteArray() const {
    std::string output;
    m_grid.SerializeToString(&output);
    return QByteArray(output.data(), static_cast<int>(output.length()));
}

BeatsPointer BeatGrid::clone() const {
    BeatsPointer other(new BeatGrid(*this));
    return other;
}

double BeatGrid::firstBeatSample() const {
    return m_grid.first_beat().frame_position() * kFrameSize;
}

double BeatGrid::bpm() const {
    return m_grid.bpm().bpm();
}

QString BeatGrid::getVersion() const {
    return BEAT_GRID_2_VERSION;
}

QString BeatGrid::getSubVersion() const {
    return m_subVersion;
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
    if (!isValid()) {
        *dpPrevBeatSamples = -1.0;
        *dpNextBeatSamples = -1.0;
        return false;
    }
    dFirstBeatSample = firstBeatSample();
    dBeatLength = m_dBeatLength;

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
    if (!isValid()) {
        return 0;
    }
    return bpm();
}

// Note: Also called from the engine thread
double BeatGrid::getBpmAroundPosition(double curSample, int n) const {
    Q_UNUSED(curSample);
    Q_UNUSED(n);

    if (!isValid()) {
        return -1;
    }
    return bpm();
}

BeatsPointer BeatGrid::translate(double dNumSamples) const {
    if (!isValid()) {
        return BeatsPointer(new BeatGrid(*this));
    }
    mixxx::track::io::BeatGrid grid = m_grid;
    double newFirstBeatFrames = (firstBeatSample() + dNumSamples) / kFrameSize;
    grid.mutable_first_beat()->set_frame_position(
            static_cast<google::protobuf::int32>(newFirstBeatFrames));

    return BeatsPointer(new BeatGrid(*this, grid, m_dBeatLength));
}

BeatsPointer BeatGrid::scale(enum BPMScale scale) const {
    mixxx::track::io::BeatGrid grid = m_grid;
    double bpm = grid.bpm().bpm();

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
        return BeatsPointer(new BeatGrid(*this));
    }

    if (bpm > getMaxBpm()) {
        return BeatsPointer(new BeatGrid(*this));
    }
    grid.mutable_bpm()->set_bpm(bpm);

    double beatLength = (60.0 * m_iSampleRate / bpm) * kFrameSize;
    return BeatsPointer(new BeatGrid(*this, grid, beatLength));
}

BeatsPointer BeatGrid::setBpm(double dBpm) {
    if (dBpm > getMaxBpm()) {
        dBpm = getMaxBpm();
    }
    mixxx::track::io::BeatGrid grid = m_grid;
    grid.mutable_bpm()->set_bpm(dBpm);
    double beatLength = (60.0 * m_iSampleRate / dBpm) * kFrameSize;
    return BeatsPointer(new BeatGrid(*this, grid, beatLength));
}

} // namespace mixxx
