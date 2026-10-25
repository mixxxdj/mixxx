#include "library/autodj/smart/mixscorer.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr double kUnknownKeyCost = 2.5;
constexpr double kUnknownBpmCost = 2.0;
constexpr double kUnknownEnergyCost = 0.5;
} // namespace

MixScorer::MixScorer(MixScoreWeights weights)
        : m_weights(weights) {
}

// static
int MixScorer::camelotDistance(int from, int to) {
    const int d = std::abs(from - to) % 12;
    return std::min(d, 12 - d);
}

// static
int MixScorer::camelotStep(int from, int to) {
    const int s = ((to - from) % 12 + 12) % 12; // 0..11
    return s > 6 ? s - 12 : s;
}

// static
double MixScorer::camelotCost(const TrackFeatures& from,
        const TrackFeatures& to,
        double energyDelta,
        bool boostExcusesKeyJump) {
    if (!from.hasKey() || !to.hasKey()) {
        return kUnknownKeyCost;
    }
    const int d = camelotDistance(from.camelotNumber, to.camelotNumber);
    const bool sameLetter = from.camelotMinor == to.camelotMinor;
    if (d == 0) {
        return sameLetter ? 0.0 : 0.5; // 8A->8A, 8A->8B
    }
    if (d == 1) {
        return sameLetter ? 1.0 : 2.0; // 8A->9A, 8A->9B
    }
    // DJ "energy boost" moves: +2 steps (+2 semitones) or +7 steps
    // (+1 semitone), same letter. Only excused when energy really rises.
    const int step = camelotStep(from.camelotNumber, to.camelotNumber);
    const bool boostMove = sameLetter && (step == 2 || step == -5);
    if (boostExcusesKeyJump && boostMove && energyDelta >= 1.0) {
        return 2.0;
    }
    return 4.0 + 2.0 * (d - 2); // 4, 6, 8, 10, 12
}

// static
TrackFeatures MixScorer::shiftKey(const TrackFeatures& track, int semitones) {
    TrackFeatures shifted = track;
    if (track.hasKey()) {
        const int steps = ((7 * semitones) % 12 + 12) % 12;
        shifted.camelotNumber = (track.camelotNumber - 1 + steps) % 12 + 1;
    }
    return shifted;
}

// static
int MixScorer::keyMorphSemitones(
        const TrackFeatures& from, const TrackFeatures& to, int maxShift) {
    if (!from.hasKey() || !to.hasKey()) {
        return 0;
    }
    const auto cost = [&from](const TrackFeatures& t) {
        return camelotCost(from, t, 0.0, false);
    };
    if (cost(to) < kClashKeyCost) {
        return 0; // already fits
    }
    for (int size = 1; size <= maxShift; ++size) {
        const double down = cost(shiftKey(to, -size));
        const double up = cost(shiftKey(to, size));
        if (std::min(down, up) < kClashKeyCost) {
            return down <= up ? -size : size;
        }
    }
    return 0;
}

// static
double MixScorer::tempoCost(double bpmFrom,
        double bpmTo,
        double tolerancePct,
        bool allowHalfDoubleTime) {
    if (bpmFrom <= 0.0 || bpmTo <= 0.0) {
        return kUnknownBpmCost;
    }
    if (tolerancePct <= 0.0) {
        tolerancePct = 0.1; // avoid division by zero
    }
    // Log ratio, so speeding up and slowing down by the same amount cost the
    // same (118 -> 128 and 128 -> 118 are equal). A plain percentage made
    // going up dearer than going down, which pushed every sort to run from
    // fast to slow.
    double diff = std::abs(std::log(bpmTo / bpmFrom));
    if (allowHalfDoubleTime) {
        diff = std::min({diff,
                std::abs(std::log(2.0 * bpmTo / bpmFrom)),
                std::abs(std::log(bpmTo / (2.0 * bpmFrom)))});
    }
    const double pct = diff * 100.0;
    if (pct <= tolerancePct) {
        return pct / tolerancePct; // 0..1: mixable
    }
    // Past the tolerance the crowd can hear the tempo change (DJ's own
    // judgement: 5%), so it is a clash straight away and grows from there.
    return kClashTempoCost + 1.5 * (pct - tolerancePct);
}

double MixScorer::energyCost(double energyFrom, double energyTo) const {
    if (energyFrom <= 0.0 || energyTo <= 0.0) {
        return kUnknownEnergyCost;
    }
    const double d = energyTo - energyFrom;
    switch (m_weights.direction) {
    case MixScoreWeights::EnergyDirection::Build:
        if (d < 0.0) {
            return -d * 1.5;
        }
        return d > 2.0 ? d - 2.0 : 0.0;
    case MixScoreWeights::EnergyDirection::Hold:
        return std::abs(d) * 0.75;
    case MixScoreWeights::EnergyDirection::Wave:
        return std::max(0.0, std::abs(d) - 1.5);
    }
    return 0.0;
}

MixScore MixScorer::score(const TrackFeatures& from, const TrackFeatures& to) const {
    MixScore s;
    const double energyDelta = (from.hasEnergy() && to.hasEnergy())
            ? to.energy - from.energy
            : 0.0;
    // Only hand-rated energy is trusted enough to excuse a key jump.
    const bool bothManual = from.energyIsManual && to.energyIsManual;
    s.keyCost = camelotCost(from,
            to,
            energyDelta,
            m_weights.energyBoostExcusesKeyJump && bothManual);
    s.tempoCost = tempoCost(from.bpm,
            to.bpm,
            m_weights.bpmTolerancePct,
            m_weights.allowHalfDoubleTime);
    s.energyCost = energyCost(from.energy, to.energy);
    if (!bothManual) {
        s.energyCost *= m_weights.measuredEnergyTrust;
    }
    s.total = m_weights.key * s.keyCost +
            m_weights.tempo * s.tempoCost +
            m_weights.energy * s.energyCost;
    // Unknown values show as "?" so they are not mistaken for measurements.
    const auto bpmText = [](const TrackFeatures& t) {
        return t.hasBpm() ? QString::number(t.bpm, 'f', 1) : QStringLiteral("?");
    };
    QString energyText = QStringLiteral("?");
    if (from.hasEnergy() && to.hasEnergy()) {
        energyText = (energyDelta >= 0.0 ? QStringLiteral("+") : QString()) +
                QString::number(energyDelta, 'f', 1) +
                (bothManual ? QStringLiteral(" (rated)") : QStringLiteral(" (measured)"));
    }
    s.reason = QStringLiteral("%1 -> %2 | %3 -> %4 BPM | energy %5")
                       .arg(from.camelotText(),
                               to.camelotText(),
                               bpmText(from),
                               bpmText(to),
                               energyText);
    return s;
}

double MixScorer::startCost(const TrackFeatures& track) const {
    if (m_weights.direction != MixScoreWeights::EnergyDirection::Build || !track.hasEnergy()) {
        return 0.0;
    }
    const double trust = track.energyIsManual ? 1.0 : m_weights.measuredEnergyTrust;
    return m_weights.energy * m_weights.setShape * trust * std::max(0.0, track.energy - 1.0);
}

double MixScorer::endCost(const TrackFeatures& track) const {
    if (m_weights.direction != MixScoreWeights::EnergyDirection::Build || !track.hasEnergy()) {
        return 0.0;
    }
    const double trust = track.energyIsManual ? 1.0 : m_weights.measuredEnergyTrust;
    return m_weights.energy * m_weights.setShape * trust * std::max(0.0, 10.0 - track.energy);
}

// static
QString MixScorer::trackLine(int position, const TrackFeatures& track) {
    return QStringLiteral("%1. %2").arg(position, 2).arg(trackText(track));
}

// static
QString MixScorer::trackText(const TrackFeatures& track) {
    QString energy = QStringLiteral("energy ?");
    if (track.hasEnergy()) {
        energy = track.energyIsManual
                ? QStringLiteral("energy %1 (rated)").arg(track.energy, 0, 'f', 0)
                : QStringLiteral("energy %1 (measured)").arg(track.energy, 0, 'f', 1);
    }
    const QString bpm = track.hasBpm()
            ? QStringLiteral("%1 BPM").arg(track.bpm, 5, 'f', 1)
            : QStringLiteral("  ? BPM");
    return QStringLiteral("%1  %2  %3  %4")
            .arg(track.camelotText(), 3)
            .arg(bpm, energy, track.displayName);
}

// static
QString MixScorer::clashLabel(const MixScore& score) {
    const bool keyClash = score.keyCost >= kClashKeyCost;
    const bool tempoClash = score.tempoCost >= kClashTempoCost;
    if (keyClash && tempoClash) {
        return QStringLiteral("key + tempo clash");
    }
    if (keyClash) {
        return QStringLiteral("key clash");
    }
    if (tempoClash) {
        return QStringLiteral("tempo clash");
    }
    return QString();
}
