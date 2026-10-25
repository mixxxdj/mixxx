#include "analyzer/analyzerenergy.h"

#include <QtDebug>

#include <cmath>

#include "analyzer/analyzertrack.h"
#include "library/autodj/smart/energystore.h"
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
          m_tableReady(EnergyStore::ensureTable(dbConnection)) {
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
    // Already analysed with the current formula? Then skip the audio, but
    // still refresh the Intro End / Outro Start markers from the stored
    // body, so "Analyze" puts back a marker the DJ cleared.
    const auto version = EnergyStore::storedVersion(m_db, m_trackId);
    // The beat grid check belongs to the grid it was made on: if the DJ
    // has changed the grid since (or it was never checked), run again.
    double bpm = 0.0;
    double firstBeatSec = 0.0;
    gridOf(pTrack, &bpm, &firstBeatSec);
    const auto gridCheck = EnergyStore::loadGridCheck(m_db, m_trackId);
    const bool beatMap = beatGrid(pTrack).isMap();
    const bool gridChecked = gridCheck && gridCheck->isFor(bpm, firstBeatSec, beatMap) &&
            gridCheck->isCurrent(beatMap);
    if (version && *version == EnergyCalculator::kVersion && gridChecked) {
        if (const auto body = EnergyStore::loadBody(m_db, m_trackId)) {
            EnergyCalculator::Result stored;
            stored.bodyStartSec = body->startSec;
            stored.bodyEndSec = body->endSec;
            setAutoMarkers(pTrack, stored);
        }
        return false;
    }
    m_pCalculator = std::make_unique<EnergyCalculator>(
            static_cast<double>(sampleRate.value()),
            static_cast<int>(channelCount.value()));
    return true;
}

bool AnalyzerEnergy::processSamples(const CSAMPLE* pIn, SINT count) {
    VERIFY_OR_DEBUG_ASSERT(m_pCalculator) {
        return false;
    }
    m_pCalculator->process(pIn, count);
    return true;
}

void AnalyzerEnergy::storeResults(TrackPointer pTrack) {
    VERIFY_OR_DEBUG_ASSERT(m_pCalculator) {
        return;
    }
    EnergyCalculator::Result result;
    if (!m_pCalculator->finish(&result)) {
        qDebug() << "AnalyzerEnergy: not enough audio for an energy score"
                 << pTrack->getInfo();
        storeGridCheck(pTrack, nullptr); // so it is not tried again and again
        return;
    }
    if (EnergyStore::save(m_db, m_trackId, result, EnergyCalculator::kVersion)) {
        qDebug() << "AnalyzerEnergy:" << pTrack->getInfo()
                 << "energy" << result.energy
                 << "loudness dB" << result.loudnessDb
                 << "bright" << result.brightRatio
                 << "onsets/s" << result.onsetsPerSec;
        setAutoMarkers(pTrack, result);
    }
    storeGridCheck(pTrack, &result);
}

void AnalyzerEnergy::storeGridCheck(
        const TrackPointer& pTrack, const EnergyCalculator::Result* pResult) {
    EnergyStore::GridCheck check;
    const phrasealign::Grid grid = beatGrid(pTrack);
    check.version = grid.isMap() ? EnergyCalculator::kGridCheckMapVersion
                                 : EnergyCalculator::kGridCheckVersion;
    if (gridOf(pTrack, &check.bpm, &check.firstBeatSec) && pResult) {
        check.driftBeats = m_pCalculator->gridDriftBeats(
                grid, pResult->bodyStartSec, pResult->bodyEndSec);
    }
    EnergyStore::saveGridCheck(m_db, m_trackId, check);
    qInfo() << "AnalyzerEnergy: beat grid check" << pTrack->getInfo()
            << "bpm" << check.bpm << (grid.isMap() ? "(beat map)" : "(steady grid)")
            << "drift" << check.driftBeats << "beats"
            << (check.driftBeats > EnergyCalculator::kGridMaxDriftBeats
                               ? "-> grid does NOT stay on the beat, Auto DJ will not beatmatch it"
                               : (check.driftBeats < 0.0 ? "(cannot tell)" : "(OK)"));
}

void AnalyzerEnergy::setAutoMarkers(
        const TrackPointer& pTrack, const EnergyCalculator::Result& result) {
    // Intro End = where the main beat kicks in, Outro Start = where the track
    // starts to lose its energy. Both snapped to the beat grid with the same
    // rules Auto DJ uses, so they are exactly what Auto DJ would do anyway;
    // the point is that the DJ can SEE them on the waveform and move the
    // wrong ones. A marker the DJ set or moved is never touched: we only
    // fill an empty marker, or update one that is still where we put it.
    const double sampleRate = pTrack->getSampleRate().value();
    // Steady grid or a beat map that bends with the music.
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
        return mixxx::audio::FramePos(sec * sampleRate);
    };

    const EnergyStore::AutoMarkers before = EnergyStore::loadAutoMarkers(m_db, m_trackId);
    EnergyStore::AutoMarkers after = before;

    // Intro End: only if the beat comes in at least one bar after the start.
    const auto introStartEnd = pIntro->getStartAndEndPosition();
    const double introStartSec = toSec(introStartEnd.startPosition);
    const double introEndNowSec = toSec(introStartEnd.endPosition);
    // Since analysis v5 the body start is pinpointed to the first kick, so
    // it is only snapped to the nearest beat (like a marker set by ear).
    const double entry = phrasealign::entryBeat(grid, result.bodyStartSec, true);
    const double introEndSec = grid.beatTime(entry);
    const bool introFree = introEndNowSec < 0.0 || isOurs(introEndNowSec, before.introEndSec);
    if (introFree) {
        if (entry >= phrasealign::kBeatsPerBar && introEndSec > introStartSec) {
            pIntro->setEndPosition(toFrame(introEndSec));
            after.introEndSec = introEndSec;
        } else if (introEndNowSec >= 0.0) {
            // Ours, but the beat now starts right away: remove it again.
            pIntro->setEndPosition(mixxx::audio::kInvalidFramePos);
            after.introEndSec = -1.0;
        }
    }

    // Outro Start: at least one bar before the outro end (last sound).
    const auto outroStartEnd = pOutro->getStartAndEndPosition();
    const double outroStartNowSec = toSec(outroStartEnd.startPosition);
    const double outroEndSec = toSec(outroStartEnd.endPosition);
    const double outroStartSec = phrasealign::bodyEndBarSec(grid, result.bodyEndSec);
    const bool outroFree =
            outroStartNowSec < 0.0 || isOurs(outroStartNowSec, before.outroStartSec);
    if (outroFree && outroEndSec > 0.0) {
        if (outroStartSec > std::max(0.0, introEndSec) &&
                outroStartSec + grid.beatSecAt(outroStartSec) * phrasealign::kBeatsPerBar <=
                        outroEndSec) {
            pOutro->setStartPosition(toFrame(outroStartSec));
            after.outroStartSec = outroStartSec;
        } else if (outroStartNowSec >= 0.0) {
            pOutro->setStartPosition(mixxx::audio::kInvalidFramePos);
            after.outroStartSec = -1.0;
        }
    }

    if (after.introEndSec != before.introEndSec ||
            after.outroStartSec != before.outroStartSec) {
        EnergyStore::saveAutoMarkers(m_db, m_trackId, after);
    }
    qDebug() << "AnalyzerEnergy: markers" << pTrack->getInfo()
             << "intro end" << (introFree ? QString::number(after.introEndSec)
                                          : QStringLiteral("kept (set by DJ)"))
             << "outro start" << (outroFree ? QString::number(after.outroStartSec)
                                            : QStringLiteral("kept (set by DJ)"));
}

void AnalyzerEnergy::cleanup() {
    m_pCalculator.reset();
}
