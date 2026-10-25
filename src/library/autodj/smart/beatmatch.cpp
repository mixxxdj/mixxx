#include "library/autodj/smart/beatmatch.h"

#include <algorithm>
#include <cmath>

namespace beatmatch {

std::optional<double> matchRatio(double fromBpm, double toTrackBpm, double tolerancePct) {
    if (!(fromBpm > 0.0) || !(toTrackBpm > 0.0)) {
        return std::nullopt;
    }
    // Candidates: same tempo, or the incoming track at half/double time.
    const double candidates[] = {
            fromBpm / toTrackBpm,
            2.0 * fromBpm / toTrackBpm,
            fromBpm / (2.0 * toTrackBpm),
    };
    double best = candidates[0];
    for (double r : candidates) {
        if (std::abs(std::log(r)) < std::abs(std::log(best))) {
            best = r;
        }
    }
    if (std::abs(std::log(best)) * 100.0 > tolerancePct) {
        return std::nullopt;
    }
    return best;
}

double glideRatio(double startRatio, double elapsedSeconds, double glideSeconds) {
    if (glideSeconds <= 0.0 || elapsedSeconds >= glideSeconds) {
        return 1.0;
    }
    const double t = std::clamp(elapsedSeconds / glideSeconds, 0.0, 1.0);
    return startRatio + (1.0 - startRatio) * t;
}

BassState bassSwap(double progress) {
    BassState s;
    if (progress < 0.5) {
        s.toLowKilled = true;
    } else {
        s.fromLowKilled = true;
    }
    return s;
}

EqBlend eqBlend(double progress) {
    const double p = std::isnan(progress) ? 0.0 : std::clamp(progress, 0.0, 1.0);
    EqBlend e;
    if (p < 0.5) {
        e.toMidHigh = kEqBlendFloor + (1.0 - kEqBlendFloor) * (p / 0.5);
    } else {
        e.fromMidHigh = 1.0 - (1.0 - kEqBlendFloor) * ((p - 0.5) / 0.5);
    }
    return e;
}

StemBlend stemBlend(double progress, bool fadeDrums) {
    const double p = std::isnan(progress) ? 0.0 : std::clamp(progress, 0.0, 1.0);
    constexpr double kHalfPi = 1.5707963267948966;
    StemBlend s;
    if (p < 0.5) {
        const double x = p / 0.5; // 0..1 over the first half
        s.fromVocals = std::cos(x * kHalfPi);
        s.toInstrumental = std::sin(x * kHalfPi);
    } else {
        const double x = (p - 0.5) / 0.5; // 0..1 over the second half
        s.fromVocals = 0.0;
        s.toInstrumental = 1.0;
        s.fromInstrumental = std::cos(x * kHalfPi);
        s.toVocals = std::sin(x * kHalfPi);
    }
    // The same moment as the bass swap.
    const BassState bass = bassSwap(p);
    s.fromDrums = s.fromBass = bass.fromLowKilled ? 0.0 : 1.0;
    s.toDrums = s.toBass = bass.toLowKilled ? 0.0 : 1.0;
    if (fadeDrums) {
        if (p < 0.5) {
            s.toDrums = std::sin(p / 0.5 * kHalfPi);
            s.fromDrums = 1.0;
        } else {
            s.toDrums = 1.0;
            s.fromDrums = std::cos((p - 0.5) / 0.5 * kHalfPi);
        }
    }
    return s;
}

StemBlend stemBlend(double progress, const VocalPlan& plan, bool fadeDrums) {
    const double p = std::isnan(progress) ? 0.0 : std::clamp(progress, 0.0, 1.0);
    constexpr double kHalfPi = 1.5707963267948966;
    StemBlend s = stemBlend(p, fadeDrums);
    const double swapAt = std::clamp(std::isnan(plan.swapAt) ? 0.5 : plan.swapAt, 0.15, 0.85);
    if (!plan.fromSings) {
        s.fromVocals = s.fromInstrumental; // nothing sung: leave with the song
    } else if (plan.toSings) {
        // Both sing: the outgoing line ends at swapAt.
        const double w = std::min(0.25, swapAt);
        const double x = std::clamp((p - (swapAt - w)) / w, 0.0, 1.0);
        s.fromVocals = p >= swapAt ? 0.0 : std::cos(x * kHalfPi);
    }
    if (!plan.toSings) {
        s.toVocals = s.toInstrumental; // nothing sung yet: comes in with the song
    } else if (plan.fromSings) {
        const double w = std::min(0.25, 1.0 - swapAt);
        const double x = std::clamp((p - swapAt) / w, 0.0, 1.0);
        s.toVocals = p <= swapAt ? 0.0 : std::sin(x * kHalfPi);
    } else {
        // Only the new song sings: over the old instrumental, no clash.
        s.toVocals = s.toInstrumental;
    }
    return s;
}

double followRatio(double fromBeat,
        double fromBeatRealSec,
        double toBeat,
        double toBeatTrackSec,
        double* pSlipBeats) {
    if (!(fromBeatRealSec > 0.0) || !(toBeatTrackSec > 0.0)) {
        return 1.0;
    }
    // Where in the beat each track is: positive = the incoming beat comes
    // late (it must catch up).
    double slip = fromBeat - toBeat;
    slip -= std::round(slip); // -0.5 .. 0.5
    if (pSlipBeats) {
        *pSlipBeats = slip;
    }
    double nudge = 0.0;
    if (std::fabs(slip) > kLockDeadBeats) {
        nudge = std::clamp(slip / kLockBeats, -kMaxLockNudge, kMaxLockNudge);
    }
    return toBeatTrackSec / fromBeatRealSec * (1.0 + nudge);
}

std::optional<double> kickOffset(const std::vector<float>& level,
        double rate,
        const std::vector<double>& beatTimes) {
    const int n = static_cast<int>(level.size());
    if (n < 100 || !(rate > 0.0) || beatTimes.size() < 17) {
        return std::nullopt;
    }
    // How loud a kick is in this song (the loud 5%).
    std::vector<float> sorted(level);
    const auto k = static_cast<std::size_t>(0.95 * static_cast<double>(sorted.size() - 1));
    std::nth_element(sorted.begin(), sorted.begin() + k, sorted.end());
    const double loud = sorted[k];
    if (loud < 0.02) {
        return std::nullopt; // no drums
    }
    std::vector<double> offsets;
    for (std::size_t b = 0; b + 1 < beatTimes.size(); ++b) {
        const double t = beatTimes[b];
        const double len = beatTimes[b + 1] - t;
        if (!(len > 0.0)) {
            continue;
        }
        // Look a quarter beat either side of the line.
        const int a = static_cast<int>(std::floor((t - 0.25 * len) * rate));
        const int z = static_cast<int>(std::ceil((t + 0.25 * len) * rate));
        if (a < 1 || z >= n) {
            continue;
        }
        int peakAt = a;
        for (int i = a; i <= z; ++i) {
            if (level[i] > level[peakAt]) {
                peakAt = i;
            }
        }
        const double peak = level[peakAt];
        double base = peak;
        for (int i = a; i <= peakAt; ++i) {
            base = std::min(base, static_cast<double>(level[i]));
        }
        if (peak < 0.3 * loud || peak < 2.0 * base + 0.02) {
            continue; // no clear hit on this beat
        }
        // The kick starts where the level first rises half way to its peak.
        const double half = base + 0.5 * (peak - base);
        int i = peakAt;
        while (i > a && level[i - 1] >= half) {
            --i;
        }
        double frac = 0.0;
        if (i > a && level[i] > level[i - 1]) {
            frac = (half - level[i - 1]) / (level[i] - level[i - 1]);
        }
        const double onset = (i - 0.5 + frac) / rate;
        offsets.push_back(onset - t);
    }
    if (offsets.size() < 16) {
        return std::nullopt;
    }
    const std::size_t mid = offsets.size() / 2;
    std::nth_element(offsets.begin(), offsets.begin() + mid, offsets.end());
    return offsets[mid];
}

} // namespace beatmatch
