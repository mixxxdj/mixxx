#include "analyzer/analyzerenergy.h"

#include <QtDebug>
#include <cmath>

#include "analyzer/analyzertrack.h"
#include "library/autodj/smart/phrasealign.h"
#include "track/beats.h"
#include "track/cue.h"
#include "track/track.h"

namespace {

// Two positions this close are the same marker (rounding in the database).
constexpr double kSameMarkerSec = 0.02;

bool isOurs(double markerSec, double autoSec) {
    return autoSec >= 0.0 && std::fabs(markerSec - autoSec) < kSameMarkerSec;
}

} // namespace

// static
bool AnalyzerEnergy::gridOf(const TrackPointer& pTrack, double* pBpm, double* pFirstBeatSec) {
    *pBpm = 0.0;
    *pFirstBeatSec = 0.0;
    const mixxx::BeatsPointer pBeats = pTrack ? pTrack->getBeats() : nullptr;
    if (!pBeats) {
        return false;
    }
    const double bpm = pTrack->getBpm();
    const double sampleRate = pTrack->getSampleRate().value();
    const mixxx::audio::FramePos firstBeat = pBeats->firstBeat();
    if (!(bpm > 0.0) || !(sampleRate > 0.0) || !firstBeat.isValid()) {
        return false;
    }
    *pBpm = bpm;
    *pFirstBeatSec = firstBeat.value() / sampleRate;
    return true;
}

// static
phrasealign::Grid AnalyzerEnergy::beatGrid(const TrackPointer& pTrack) {
    double bpm = 0.0;
    double firstBeatSec = 0.0;
    if (!gridOf(pTrack, &bpm, &firstBeatSec)) {
        return phrasealign::Grid{};
    }
    const phrasealign::Grid steady{firstBeatSec, 60.0 / bpm};
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (!pBeats || pBeats->hasConstantTempo()) {
        return steady;
    }
    // A beat map: every beat from the first to the end of the track.
    constexpr std::size_t kMaxBeats = 20000; // over an hour at 300 BPM
    const double sampleRate = pTrack->getSampleRate().value();
    const double endSec = pTrack->getDuration();
    std::vector<double> times;
    for (auto it = pBeats->iteratorFrom(pBeats->firstBeat()); times.size() < kMaxBeats; ++it) {
        const double sec = it->value() / sampleRate;
        if ((endSec > 0.0 && sec > endSec + 1.0) || (!times.empty() && sec <= times.back())) {
            break;
        }
        times.push_back(sec);
    }
    const phrasealign::Grid map = phrasealign::Grid::fromBeats(std::move(times));
    return map.isValid() ? map : steady;
}

AnalyzerEnergy::AnalyzerEnergy(const QSqlDatabase& dbConnection)
        : m_db(dbConnection),
          m_tableReady(false) {
    // EnergyStore table creation would be called here when available
}

bool AnalyzerEnergy::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount,
        SINT frameLength) {
    if (!m_tableReady || frameLength <= 0) {
        return false;
    }
    const TrackPointer& pTrack = track.getTrack();
    m_trackId = pTrack->getId();
    if (!m_trackId.isValid()) {
        return false;
    }

    m_pCalculator = std::make_unique<EnergyCalculator>(sampleRate.value(), channelCount);
    return true;
}

bool AnalyzerEnergy::processSamples(const CSAMPLE* pIn, SINT count) {
    if (!m_pCalculator) {
        return false;
    }
    // CSAMPLE is float in Mixxx
    m_pCalculator->process(reinterpret_cast<const float*>(pIn), count);
    return true;
}

void AnalyzerEnergy::storeResults(TrackPointer pTrack) {
    if (!m_pCalculator || !m_trackId.isValid()) {
        return;
    }
    EnergyCalculator::Result result;
    if (!m_pCalculator->finish(&result)) {
        qDebug() << "AnalyzerEnergy: not enough audio for" << pTrack->getInfo();
        return;
    }

    // Beat grid check
    storeGridCheck(pTrack, &result);

    // Set automatic Intro End / Outro Start markers from the body
    setAutoMarkers(pTrack, result);

    qInfo() << "AnalyzerEnergy: analyzed" << pTrack->getInfo()
            << "energy" << result.energy
            << "body" << result.bodyStartSec << "->" << result.bodyEndSec;
}

void AnalyzerEnergy::cleanup() {
    m_pCalculator.reset();
}

void AnalyzerEnergy::setAutoMarkers(
        const TrackPointer& pTrack, const EnergyCalculator::Result& result) {
    const double sampleRate = pTrack->getSampleRate().value();
    const phrasealign::Grid grid = beatGrid(pTrack);
    if (!grid.isValid() || !(sampleRate > 0.0)) {
        return; // no grid to snap to
    }
    const CuePointer pIntro = pTrack->findCueByType(mixxx::CueType::Intro);
    const CuePointer pOutro = pTrack->findCueByType(mixxx::CueType::Outro);
    if (!pIntro || !pOutro) {
        return; // made by AnalyzerSilence, which runs before us
    }
    const auto toSec = [sampleRate](mixxx::audio::FramePos pos) {
        return pos.isValid() ? pos.value() / sampleRate : -1.0;
    };
    const auto toFrame = [sampleRate](double sec) {
        return mixxx::audio::FramePos(static_cast<mixxx::audio::framepos_t>(sec * sampleRate));
    };

    // Intro End: only if the beat comes in at least one bar after the start.
    const auto introStartEnd = pIntro->getStartAndEndPosition();
    const double introStartSec = toSec(introStartEnd.startPosition);
    const double introEndNowSec = toSec(introStartEnd.endPosition);
    // body start is pinpointed to the first kick, so it is only snapped to
    // the nearest beat (like a marker set by ear).
    const double entry = phrasealign::entryBeat(grid, result.bodyStartSec, true);
    const double introEndSec = grid.beatTime(entry);
    const bool introFree = introEndNowSec < 0.0 || isOurs(introEndNowSec, introEndSec);
    if (introFree) {
        if (entry >= phrasealign::kBeatsPerBar && introEndSec > introStartSec) {
            pIntro->setEndPosition(toFrame(introEndSec));
        } else if (introEndNowSec >= 0.0) {
            // Ours, but the beat now starts right away: remove it again.
            pIntro->setEndPosition(mixxx::audio::kInvalidFramePos);
        }
    }

    // Outro Start: at least one bar before the outro end (last sound).
    const auto outroStartEnd = pOutro->getStartAndEndPosition();
    const double outroStartNowSec = toSec(outroStartEnd.startPosition);
    const double outroEndSec = toSec(outroStartEnd.endPosition);
    const double outroStartSec = phrasealign::bodyEndBarSec(grid, result.bodyEndSec);
    const bool outroFree = outroStartNowSec < 0.0 || isOurs(outroStartNowSec, outroStartSec);
    if (outroFree && outroEndSec > 0.0) {
        if (outroStartSec > std::max(0.0, introEndSec) &&
                outroStartSec + grid.beatSecAt(outroStartSec) * phrasealign::kBeatsPerBar <=
                        outroEndSec) {
            pOutro->setStartPosition(toFrame(outroStartSec));
        } else if (outroStartNowSec >= 0.0) {
            pOutro->setStartPosition(mixxx::audio::kInvalidFramePos);
        }
    }

    qDebug() << "AnalyzerEnergy: markers" << pTrack->getInfo() << "intro end"
             << (introFree ? QString::number(introEndSec)
                           : QStringLiteral("kept (set by DJ)"))
             << "outro start"
             << (outroFree ? QString::number(outroStartSec)
                           : QStringLiteral("kept (set by DJ)"));
}

void AnalyzerEnergy::storeGridCheck(const TrackPointer& pTrack, const EnergyCalculator::Result* pResult) {
    const phrasealign::Grid grid = beatGrid(pTrack);
    if (!grid.isValid()) {
        return; // no grid, nothing to check
    }
    double drift = -1.0;
    if (pResult) {
        drift = pResult->gridDriftBeats(grid, pResult->bodyStartSec, pResult->bodyEndSec);
    }
    qInfo() << "AnalyzerEnergy: beat grid check" << pTrack->getInfo()
            << "bpm" << pTrack->getBpm() << (grid.isMap() ? "(beat map)" : "(steady grid)")
            << "drift" << drift << "beats"
            << (drift > EnergyCalculator::kGridMaxDriftBeats
                               ? "-> grid does NOT stay on the beat, Auto DJ will not beatmatch it"
                               : (drift < 0.0 ? "(cannot tell)" : "(OK)"));
}
