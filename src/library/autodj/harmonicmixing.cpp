#include "library/autodj/harmonicmixing.h"

#include "track/keyutils.h"
#include "track/keys.h"

namespace mixxx {

namespace {

using mixxx::track::io::key::ChromaticKey;

} // namespace

// static
HarmonicMixing::BpmRange HarmonicMixing::bpmRange(
        double referenceBpm, double tolerancePercent) {
    if (referenceBpm <= 0.0 || tolerancePercent <= 0.0) {
        return BpmRange{};
    }
    const double tolerance = referenceBpm * tolerancePercent / 100.0;
    return BpmRange{referenceBpm - tolerance, referenceBpm + tolerance};
}

// static
QList<HarmonicMixing::BpmRange> HarmonicMixing::bpmFallbackRanges(
        double referenceBpm, double tolerancePercent) {
    const BpmRange configuredRange = bpmRange(referenceBpm, tolerancePercent);
    if (!configuredRange.isValid()) {
        return {};
    }

    // The configured tolerance is the one the user asked for, so it is tried
    // first. Tracklists whose tempos are spread out more than that would
    // otherwise have no harmonically compatible candidate at all, so the
    // tolerance is widened before giving up on the tempo.
    constexpr double kWidenedToleranceFactor = 3.0;
    QList<BpmRange> ranges;
    ranges.append(configuredRange);
    const BpmRange widenedRange = bpmRange(
            referenceBpm, tolerancePercent * kWidenedToleranceFactor);
    if (widenedRange.isValid() && widenedRange.min < configuredRange.min) {
        ranges.append(widenedRange);
    }

    // Last attempt: keep mixing by key and ignore the tempo.
    ranges.append(BpmRange{});
    return ranges;
}

// static
QList<ChromaticKey> HarmonicMixing::compatibleKeys(ChromaticKey referenceKey) {
    if (referenceKey == mixxx::track::io::key::INVALID ||
            !ChromaticKey_IsValid(referenceKey)) {
        return {};
    }
    return KeyUtils::getCompatibleKeys(referenceKey);
}

// static
bool HarmonicMixing::isBpmCompatible(
        double referenceBpm, double candidateBpm, double tolerancePercent) {
    const BpmRange range = bpmRange(referenceBpm, tolerancePercent);
    if (!range.isValid() || candidateBpm <= 0.0) {
        return false;
    }
    return candidateBpm >= range.min && candidateBpm <= range.max;
}

// static
bool HarmonicMixing::isKeyCompatible(
        ChromaticKey referenceKey, ChromaticKey candidateKey) {
    return compatibleKeys(referenceKey).contains(candidateKey);
}

} // namespace mixxx
