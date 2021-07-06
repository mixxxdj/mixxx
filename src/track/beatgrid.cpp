#include "track/beatgrid.h"

#include <QMutexLocker>
#include <QtDebug>

#include "track/beatutils.h"
#include "track/track.h"
#include "util/math.h"

namespace {

struct BeatGridData {
    double bpm;
    double firstBeat;
};

constexpr int kFrameSize = 2;
constexpr double kBpmScaleRounding = 0.001;

} // namespace

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
        audio::SampleRate sampleRate,
        const QString& subVersion,
        const mixxx::track::io::BeatGrid& grid,
        double beatLength)
        : m_subVersion(subVersion),
          m_sampleRate(sampleRate),
          m_grid(grid),
          m_dBeatLength(beatLength) {
    // BeatGrid should live in the same thread as the track it is associated
    // with.
}

BeatGrid::BeatGrid(const BeatGrid& other, const mixxx::track::io::BeatGrid& grid, double beatLength)
        : m_subVersion(other.m_subVersion),
          m_sampleRate(other.m_sampleRate),
          m_grid(grid),
          m_dBeatLength(beatLength) {
}

BeatGrid::BeatGrid(const BeatGrid& other)
        : BeatGrid(other, other.m_grid, other.m_dBeatLength) {
}

// static
BeatsPointer BeatGrid::makeBeatGrid(
        audio::SampleRate sampleRate,
        const QString& subVersion,
        mixxx::Bpm bpm,
        mixxx::audio::FramePos firstBeatPos) {
    // FIXME: Should this be a debug assertion?
    if (!bpm.isValid()) {
        return nullptr;
    }

    mixxx::track::io::BeatGrid grid;

    grid.mutable_bpm()->set_bpm(bpm.value());
    grid.mutable_first_beat()->set_frame_position(
            static_cast<google::protobuf::int32>(firstBeatPos.value()));
    // Calculate beat length as sample offsets
    double beatLength = (60.0 * sampleRate / bpm.value()) * kFrameSize;

    return BeatsPointer(new BeatGrid(sampleRate, subVersion, grid, beatLength));
}

// static
BeatsPointer BeatGrid::makeBeatGrid(
        audio::SampleRate sampleRate,
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
    const auto firstBeat = mixxx::audio::FramePos(blob->firstBeat);
    const auto bpm = mixxx::Bpm(blob->bpm);

    return makeBeatGrid(sampleRate, subVersion, bpm, firstBeat);
}

QByteArray BeatGrid::toByteArray() const {
    std::string output;
    m_grid.SerializeToString(&output);
    return QByteArray(output.data(), static_cast<int>(output.length()));
}

double BeatGrid::firstBeatSample() const {
    return m_grid.first_beat().frame_position() * kFrameSize;
}

mixxx::Bpm BeatGrid::bpm() const {
    return mixxx::Bpm(m_grid.bpm().bpm());
}

QString BeatGrid::getVersion() const {
    return BEAT_GRID_2_VERSION;
}

QString BeatGrid::getSubVersion() const {
    return m_subVersion;
}

// internal use only
bool BeatGrid::isValid() const {
    return m_sampleRate.isValid() && bpm().isValid();
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
    findPrevNextBeats(dSamples, &prevBeat, &nextBeat, true);
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
        double* dpNextBeatSamples,
        bool snapToNearBeats) const {
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

    const double kEpsilon = .01;

    if ((!snapToNearBeats && ((nextBeat - beatFraction) == 0.0)) ||
            (snapToNearBeats && (fabs(nextBeat - beatFraction) < kEpsilon))) {
        // In snapToNearBeats mode: If the position is within 1/100th of the next or previous beat,
        // treat it as if it is that beat.

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

mixxx::Bpm BeatGrid::getBpm() const {
    if (!isValid()) {
        return {};
    }
    return bpm();
}

// Note: Also called from the engine thread
mixxx::Bpm BeatGrid::getBpmAroundPosition(double curSample, int n) const {
    Q_UNUSED(curSample);
    Q_UNUSED(n);

    if (!isValid()) {
        return {};
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
    auto bpm = mixxx::Bpm(grid.bpm().bpm());

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

    if (!bpm.isValid()) {
        return BeatsPointer(new BeatGrid(*this));
    }

    bpm = BeatUtils::roundBpmWithinRange(bpm - kBpmScaleRounding, bpm, bpm + kBpmScaleRounding);
    grid.mutable_bpm()->set_bpm(bpm.value());
    double beatLength = (60.0 * m_sampleRate / bpm.value()) * kFrameSize;
    return BeatsPointer(new BeatGrid(*this, grid, beatLength));
}

BeatsPointer BeatGrid::setBpm(mixxx::Bpm bpm) {
    VERIFY_OR_DEBUG_ASSERT(bpm.isValid()) {
        return nullptr;
    }
    mixxx::track::io::BeatGrid grid = m_grid;
    grid.mutable_bpm()->set_bpm(bpm.value());
    double beatLength = (60.0 * m_sampleRate / bpm.value()) * kFrameSize;
    return BeatsPointer(new BeatGrid(*this, grid, beatLength));
}

} // namespace mixxx
