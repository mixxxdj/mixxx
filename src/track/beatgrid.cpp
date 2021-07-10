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
    BeatGridIterator(audio::FrameDiff_t beatLengthFrames,
            audio::FramePos firstBeatPosition,
            audio::FramePos endPosition)
            : m_beatLengthFrames(beatLengthFrames),
              m_endPosition(endPosition),
              m_currentPosition(firstBeatPosition) {
    }

    bool hasNext() const override {
        return m_beatLengthFrames > 0 && m_currentPosition <= m_endPosition;
    }

    audio::FramePos next() override {
        const audio::FramePos beatPosition = m_currentPosition;
        m_currentPosition += m_beatLengthFrames;
        return beatPosition;
    }

  private:
    const audio::FrameDiff_t m_beatLengthFrames;
    const audio::FramePos m_endPosition;
    audio::FramePos m_currentPosition;
};

BeatGrid::BeatGrid(
        audio::SampleRate sampleRate,
        const QString& subVersion,
        const mixxx::track::io::BeatGrid& grid,
        audio::FrameDiff_t beatLengthFrames)
        : m_subVersion(subVersion),
          m_sampleRate(sampleRate),
          m_grid(grid),
          m_beatLengthFrames(beatLengthFrames) {
    // BeatGrid should live in the same thread as the track it is associated
    // with.
}

BeatGrid::BeatGrid(const BeatGrid& other,
        const mixxx::track::io::BeatGrid& grid,
        audio::FrameDiff_t beatLengthFrames)
        : m_subVersion(other.m_subVersion),
          m_sampleRate(other.m_sampleRate),
          m_grid(grid),
          m_beatLengthFrames(beatLengthFrames) {
}

BeatGrid::BeatGrid(const BeatGrid& other)
        : BeatGrid(other, other.m_grid, other.m_beatLengthFrames) {
}

// static
BeatsPointer BeatGrid::makeBeatGrid(
        audio::SampleRate sampleRate,
        const QString& subVersion,
        mixxx::Bpm bpm,
        mixxx::audio::FramePos firstBeatPosition) {
    // FIXME: Should this be a debug assertion?
    if (!bpm.isValid() || !firstBeatPosition.isValid()) {
        return nullptr;
    }

    mixxx::track::io::BeatGrid grid;

    grid.mutable_bpm()->set_bpm(bpm.value());
    grid.mutable_first_beat()->set_frame_position(
            static_cast<google::protobuf::int32>(firstBeatPosition.value()));
    // Calculate beat length as sample offsets
    const audio::FrameDiff_t beatLengthFrames = 60.0 * sampleRate / bpm.value();

    return BeatsPointer(new BeatGrid(sampleRate, subVersion, grid, beatLengthFrames));
}

// static
BeatsPointer BeatGrid::makeBeatGrid(
        audio::SampleRate sampleRate,
        const QString& subVersion,
        const QByteArray& byteArray) {
    mixxx::track::io::BeatGrid grid;
    if (grid.ParseFromArray(byteArray.constData(), byteArray.length())) {
        const audio::FrameDiff_t beatLengthFrames = (60.0 * sampleRate / grid.bpm().bpm());
        return BeatsPointer(new BeatGrid(sampleRate, subVersion, grid, beatLengthFrames));
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

audio::FramePos BeatGrid::firstBeatPosition() const {
    return audio::FramePos(m_grid.first_beat().frame_position());
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
    return m_sampleRate.isValid() && bpm().isValid() && firstBeatPosition().isValid();
}

// This could be implemented in the Beats Class itself.
// If necessary, the child class can redefine it.
audio::FramePos BeatGrid::findNextBeat(audio::FramePos position) const {
    return findNthBeat(position, 1);
}

// This could be implemented in the Beats Class itself.
// If necessary, the child class can redefine it.
audio::FramePos BeatGrid::findPrevBeat(audio::FramePos position) const {
    return findNthBeat(position, -1);
}

// This is an internal call. This could be implemented in the Beats Class itself.
audio::FramePos BeatGrid::findClosestBeat(audio::FramePos position) const {
    if (!isValid()) {
        return audio::kInvalidFramePos;
    }
    audio::FramePos prevBeatPosition;
    audio::FramePos nextBeatPosition;
    findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, true);
    if (!prevBeatPosition.isValid()) {
        // If both positions are invalid, we correctly return an invalid position.
        return nextBeatPosition;
    }

    if (!nextBeatPosition.isValid()) {
        return prevBeatPosition;
    }

    // Both position are valid, return the closest position.
    return (nextBeatPosition - position > position - prevBeatPosition)
            ? prevBeatPosition
            : nextBeatPosition;
}

audio::FramePos BeatGrid::findNthBeat(audio::FramePos position, int n) const {
    if (!isValid() || n == 0) {
        return audio::kInvalidFramePos;
    }

    double beatFraction = (position - firstBeatPosition()) / m_beatLengthFrames;
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

    audio::FramePos closestBeatPosition;
    if (n > 0) {
        // We're going forward, so use ceil to round up to the next multiple of
        // m_dBeatLength
        closestBeatPosition = firstBeatPosition() + nextBeat * m_beatLengthFrames;
        n = n - 1;
    } else {
        // We're going backward, so use floor to round down to the next multiple
        // of m_dBeatLength
        closestBeatPosition = firstBeatPosition() + prevBeat * m_beatLengthFrames;
        n = n + 1;
    }

    const audio::FramePos result = closestBeatPosition + n * m_beatLengthFrames;
    return result;
}

bool BeatGrid::findPrevNextBeats(audio::FramePos position,
        audio::FramePos* prevBeatPosition,
        audio::FramePos* nextBeatPosition,
        bool snapToNearBeats) const {
    if (!isValid()) {
        *prevBeatPosition = audio::kInvalidFramePos;
        *nextBeatPosition = audio::kInvalidFramePos;
        return false;
    }

    double beatFraction = (position - firstBeatPosition()) / m_beatLengthFrames;
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
    *prevBeatPosition = firstBeatPosition() + prevBeat * m_beatLengthFrames;
    *nextBeatPosition = firstBeatPosition() + nextBeat * m_beatLengthFrames;
    return true;
}

std::unique_ptr<BeatIterator> BeatGrid::findBeats(
        audio::FramePos startPosition, audio::FramePos endPosition) const {
    // FIXME: Should this be a VERIFY_OR_DEBUG_ASSERT?
    if (!isValid() || !startPosition.isValid() || !endPosition.isValid() ||
            startPosition > endPosition) {
        return std::unique_ptr<BeatIterator>();
    }
    const audio::FramePos startBeatPosition = findNextBeat(startPosition);
    if (!startBeatPosition.isValid()) {
        return std::unique_ptr<BeatIterator>();
    }
    return std::make_unique<BeatGridIterator>(m_beatLengthFrames, startBeatPosition, endPosition);
}

bool BeatGrid::hasBeatInRange(audio::FramePos startPosition, audio::FramePos endPosition) const {
    // FIXME: Should this be a VERIFY_OR_DEBUG_ASSERT?
    if (!isValid() || !startPosition.isValid() || !endPosition.isValid() ||
            startPosition > endPosition) {
        return false;
    }
    const audio::FramePos currentPosition = findNextBeat(startPosition);
    if (currentPosition.isValid() && currentPosition <= endPosition) {
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
mixxx::Bpm BeatGrid::getBpmAroundPosition(audio::FramePos position, int n) const {
    Q_UNUSED(position);
    Q_UNUSED(n);

    if (!isValid()) {
        return {};
    }
    return bpm();
}

BeatsPointer BeatGrid::translate(audio::FrameDiff_t offset) const {
    if (!isValid()) {
        return BeatsPointer(new BeatGrid(*this));
    }
    mixxx::track::io::BeatGrid grid = m_grid;
    const audio::FramePos newFirstBeatPosition = firstBeatPosition() + offset;
    grid.mutable_first_beat()->set_frame_position(
            static_cast<google::protobuf::int32>(
                    newFirstBeatPosition.toLowerFrameBoundary().value()));

    return BeatsPointer(new BeatGrid(*this, grid, m_beatLengthFrames));
}

BeatsPointer BeatGrid::scale(BpmScale scale) const {
    mixxx::track::io::BeatGrid grid = m_grid;
    auto bpm = mixxx::Bpm(grid.bpm().bpm());

    switch (scale) {
    case BpmScale::Double:
        bpm *= 2;
        break;
    case BpmScale::Halve:
        bpm *= 1.0 / 2;
        break;
    case BpmScale::TwoThirds:
        bpm *= 2.0 / 3;
        break;
    case BpmScale::ThreeFourths:
        bpm *= 3.0 / 4;
        break;
    case BpmScale::FourThirds:
        bpm *= 4.0 / 3;
        break;
    case BpmScale::ThreeHalves:
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
