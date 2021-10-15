#include "track/beatgrid.h"

#include <QtDebug>

#include "track/beatutils.h"
#include "track/track.h"
#include "util/math.h"

namespace {

struct BeatGridData {
    double bpm;
    double firstBeat;
};

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
        MakeSharedTag,
        audio::SampleRate sampleRate,
        const QString& subVersion,
        const mixxx::track::io::BeatGrid& grid,
        audio::FrameDiff_t beatLengthFrames)
        : m_subVersion(subVersion),
          m_sampleRate(sampleRate),
          m_grid(grid),
          m_beatLengthFrames(beatLengthFrames) {
}

BeatGrid::BeatGrid(
        MakeSharedTag,
        const BeatGrid& other,
        const mixxx::track::io::BeatGrid& grid,
        audio::FrameDiff_t beatLengthFrames)
        : BeatGrid(
                  MakeSharedTag{},
                  other.m_sampleRate,
                  other.m_subVersion,
                  grid,
                  beatLengthFrames) {
}

BeatGrid::BeatGrid(
        MakeSharedTag,
        const BeatGrid& other)
        : BeatGrid(MakeSharedTag{}, other, other.m_grid, other.m_beatLengthFrames) {
}

// static
BeatsPointer BeatGrid::makeBeatGrid(
        audio::SampleRate sampleRate,
        mixxx::Bpm bpm,
        mixxx::audio::FramePos firstBeatPositionOnFrameBoundary,
        const QString& subVersion) {
    VERIFY_OR_DEBUG_ASSERT(bpm.isValid() && firstBeatPositionOnFrameBoundary.isValid()) {
        return nullptr;
    }
    VERIFY_OR_DEBUG_ASSERT(!firstBeatPositionOnFrameBoundary.isFractional()) {
        // The beat grid only stores integer frame positions. The caller
        // is responsible to ensure that the position of the first beat
        // is on a frame boundary. Implicitly rounding the given position
        // to the nearest frame boundary might not be appropriate for all
        // use cases.
        firstBeatPositionOnFrameBoundary =
                firstBeatPositionOnFrameBoundary.toNearestFrameBoundary();
    }

    mixxx::track::io::BeatGrid grid;

    grid.mutable_bpm()->set_bpm(bpm.value());
    grid.mutable_first_beat()->set_frame_position(
            static_cast<google::protobuf::int32>(firstBeatPositionOnFrameBoundary.value()));
    // Calculate beat length as sample offsets
    const audio::FrameDiff_t beatLengthFrames = 60.0 * sampleRate / bpm.value();

    return std::make_shared<BeatGrid>(
            MakeSharedTag{}, sampleRate, subVersion, grid, beatLengthFrames);
}

// static
BeatsPointer BeatGrid::fromByteArray(
        audio::SampleRate sampleRate,
        const QString& subVersion,
        const QByteArray& byteArray) {
    mixxx::track::io::BeatGrid grid;
    if (grid.ParseFromArray(byteArray.constData(), byteArray.length())) {
        const audio::FrameDiff_t beatLengthFrames = (60.0 * sampleRate / grid.bpm().bpm());
        return std::make_shared<BeatGrid>(MakeSharedTag{},
                sampleRate,
                subVersion,
                grid,
                beatLengthFrames);
    }

    // Legacy fallback for BeatGrid-1.0
    if (byteArray.size() != sizeof(BeatGridData)) {
        return std::make_shared<BeatGrid>(MakeSharedTag{}, sampleRate, QString(), grid, 0);
    }
    const BeatGridData* blob = reinterpret_cast<const BeatGridData*>(byteArray.constData());
    const auto firstBeat = mixxx::audio::FramePos(blob->firstBeat);
    const auto bpm = mixxx::Bpm(blob->bpm);

    return makeBeatGrid(sampleRate, bpm, firstBeat, subVersion);
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

audio::FramePos BeatGrid::findNthBeat(audio::FramePos position, int n) const {
    if (!isValid() || n == 0) {
        return audio::kInvalidFramePos;
    }

    const double beatFraction = (position - firstBeatPosition()) / m_beatLengthFrames;
    const double prevBeat = floor(beatFraction);
    const double nextBeat = ceil(beatFraction);

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

mixxx::Bpm BeatGrid::getBpmInRange(
        audio::FramePos startPosition, audio::FramePos endPosition) const {
    Q_UNUSED(startPosition);
    Q_UNUSED(endPosition);

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

std::optional<BeatsPointer> BeatGrid::tryTranslate(audio::FrameDiff_t offset) const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return std::nullopt;
    }

    mixxx::track::io::BeatGrid grid = m_grid;
    const audio::FramePos newFirstBeatPosition = firstBeatPosition() + offset;
    grid.mutable_first_beat()->set_frame_position(
            static_cast<google::protobuf::int32>(
                    newFirstBeatPosition.toLowerFrameBoundary().value()));

    return std::make_shared<BeatGrid>(MakeSharedTag{}, *this, grid, m_beatLengthFrames);
}

std::optional<BeatsPointer> BeatGrid::tryScale(BpmScale scale) const {
    VERIFY_OR_DEBUG_ASSERT(isValid()) {
        return std::nullopt;
    }

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
        return std::nullopt;
    }

    if (!bpm.isValid()) {
        qWarning() << "BeatGrid: Scaling would result in invalid BPM!";
        return std::nullopt;
    }

    bpm = BeatUtils::roundBpmWithinRange(bpm - kBpmScaleRounding, bpm, bpm + kBpmScaleRounding);
    grid.mutable_bpm()->set_bpm(bpm.value());
    const mixxx::audio::FrameDiff_t beatLengthFrames = (60.0 * m_sampleRate / bpm.value());
    return std::make_shared<BeatGrid>(MakeSharedTag{}, *this, grid, beatLengthFrames);
}

std::optional<BeatsPointer> BeatGrid::trySetBpm(mixxx::Bpm bpm) const {
    VERIFY_OR_DEBUG_ASSERT(bpm.isValid()) {
        return std::nullopt;
    }

    mixxx::track::io::BeatGrid grid = m_grid;
    grid.mutable_bpm()->set_bpm(bpm.value());
    const mixxx::audio::FrameDiff_t beatLengthFrames = (60.0 * m_sampleRate / bpm.value());
    return std::make_shared<BeatGrid>(MakeSharedTag{}, *this, grid, beatLengthFrames);
}

} // namespace mixxx
