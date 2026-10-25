#include "library/autodj/smart/phrasealign.h"

#include <algorithm>
#include <cmath>

namespace phrasealign {

namespace {
// Allow a few milliseconds of rounding when comparing times.
constexpr double kEpsSec = 0.005;
} // namespace

double Grid::beatTime(double n) const {
    if (!isMap()) {
        return firstBeatSec + n * beatSec;
    }
    const int last = static_cast<int>(beats.size()) - 1;
    if (n <= 0.0) {
        return beats[0] + n * (beats[1] - beats[0]);
    }
    if (n >= last) {
        return beats[last] + (n - last) * (beats[last] - beats[last - 1]);
    }
    const int i = static_cast<int>(std::floor(n));
    return beats[i] + (n - i) * (beats[i + 1] - beats[i]);
}

double Grid::beatAt(double sec) const {
    if (!isMap()) {
        return (sec - firstBeatSec) / beatSec;
    }
    const int last = static_cast<int>(beats.size()) - 1;
    if (sec <= beats[0]) {
        return (sec - beats[0]) / (beats[1] - beats[0]);
    }
    if (sec >= beats[last]) {
        return last + (sec - beats[last]) / (beats[last] - beats[last - 1]);
    }
    const int i = static_cast<int>(
                          std::upper_bound(beats.begin(), beats.end(), sec) - beats.begin()) -
            1;
    return i + (sec - beats[i]) / (beats[i + 1] - beats[i]);
}

double Grid::beatSecAt(double sec, int span) const {
    if (!isMap()) {
        return beatSec;
    }
    const int last = static_cast<int>(beats.size()) - 1;
    span = std::max(1, span);
    const int centre = static_cast<int>(std::lround(std::clamp(beatAt(sec), 0.0, 1.0 * last)));
    const int to = std::min(last, std::max(centre + span, 2 * span));
    const int from = std::max(0, to - 2 * span);
    // The middle beat length, not the average: one false or missed beat
    // (a pickup note before the first real beat) must not change the tempo.
    std::vector<double> lengths;
    lengths.reserve(to - from);
    for (int i = from; i < to; ++i) {
        lengths.push_back(beats[i + 1] - beats[i]);
    }
    const std::size_t mid = lengths.size() / 2;
    std::nth_element(lengths.begin(), lengths.begin() + mid, lengths.end());
    if (lengths.size() % 2 == 1) {
        return lengths[mid];
    }
    const double upper = lengths[mid];
    const double lower = *std::max_element(lengths.begin(), lengths.begin() + mid);
    return 0.5 * (lower + upper);
}

double Grid::steadyBeatSecAt(double sec) const {
    if (!isMap()) {
        return beatSec;
    }
    const int last = static_cast<int>(beats.size()) - 1;
    if (last < 17) {
        return beatSecAt(sec);
    }
    // The middle beat length from beat `a` to beat `b`.
    auto middle = [this](int a, int b) {
        std::vector<double> lengths;
        for (int i = a; i < b; ++i) {
            lengths.push_back(beats[i + 1] - beats[i]);
        }
        const std::size_t mid = lengths.size() / 2;
        std::nth_element(lengths.begin(), lengths.begin() + mid, lengths.end());
        return lengths[mid];
    };
    const double main = middle(0, last);
    const int centre = static_cast<int>(std::lround(std::clamp(beatAt(sec), 0.0, 1.0 * last)));
    for (int back = 0; back <= 128; back += 4) {
        const int i = centre - back;
        if (i - 8 < 0) {
            break;
        }
        const double local = middle(i - 8, std::min(last, i + 8));
        const bool nearMain = std::fabs(local / main - 1.0) <= 0.1;
        const bool held = i - 64 >= 0 && std::fabs(middle(i - 64, i - 8) / local - 1.0) <= 0.03;
        if (nearMain || held) {
            return local;
        }
    }
    return main;
}

Grid Grid::atSpeed(double rateRatio) const {
    Grid g = *this;
    if (!(rateRatio > 0.0)) {
        return g;
    }
    g.firstBeatSec /= rateRatio;
    g.beatSec /= rateRatio;
    for (double& t : g.beats) {
        t /= rateRatio;
    }
    return g;
}

Grid Grid::fromBeats(std::vector<double> beatTimes) {
    std::sort(beatTimes.begin(), beatTimes.end());
    // Two beats at the same moment would divide by zero.
    beatTimes.erase(std::unique(beatTimes.begin(),
                            beatTimes.end(),
                            [](double a, double b) { return b - a < 1e-4; }),
            beatTimes.end());
    Grid g;
    if (beatTimes.size() < 2) {
        return g; // not valid
    }
    g.firstBeatSec = beatTimes.front();
    g.beatSec = (beatTimes.back() - beatTimes.front()) / (beatTimes.size() - 1);
    g.beats = std::move(beatTimes);
    return g;
}

int barsForSeconds(double wantedSec, double beatSec) {
    if (!(beatSec > 0.0) || !(wantedSec > 0.0)) {
        return kBarsPerPhrase;
    }
    const double wantedBars = wantedSec / (beatSec * kBeatsPerBar);
    const int phrases = std::max(1, static_cast<int>(std::lround(wantedBars / kBarsPerPhrase)));
    return phrases * kBarsPerPhrase;
}

double entryBeat(const Grid& grid, double bodyStartSec, bool marked) {
    const double bodyBeat = grid.beatAt(bodyStartSec);
    if (marked) {
        // The DJ marked it by ear: trust it, just land on a beat.
        return std::max(0.0, std::round(bodyBeat));
    }
    // Body start is measured in 1 s steps (about 1 s early to 2 s late), so
    // a phrase start that close is taken as the real entry; otherwise the
    // nearest bar line.
    constexpr double kEarlySec = 1.0;
    constexpr double kLateSec = 2.0;
    const double nearestPhrase =
            std::max(0.0, std::round(bodyBeat / kBeatsPerPhrase) * kBeatsPerPhrase);
    const double phraseTime = grid.beatTime(nearestPhrase);
    if (phraseTime >= bodyStartSec - kLateSec && phraseTime <= bodyStartSec + kEarlySec) {
        return nearestPhrase;
    }
    return std::max(0.0, std::round(bodyBeat / kBeatsPerBar) * kBeatsPerBar);
}

double bodyEndBarSec(const Grid& grid, double bodyEndSec) {
    const double beat = grid.beatAt(bodyEndSec);
    const double bar = std::floor((beat + kEpsSec / grid.beatSecAt(bodyEndSec)) / kBeatsPerBar) *
            kBeatsPerBar;
    return grid.beatTime(std::max(0.0, bar));
}

std::optional<Plan> plan(const Grid& from,
        const Grid& to,
        double fromNowSec,
        double fromLimitSec,
        double toEarliestSec,
        int bars,
        double toBodyStartSec,
        bool toBodyMarked,
        double entryAt) {
    if (!from.isValid() || !to.isValid() || bars <= 0) {
        return std::nullopt;
    }
    const double fadeBeats = static_cast<double>(bars) * kBeatsPerBar;

    // Outgoing track: the LAST phrase start from which a whole fade still
    // ends before the limit, so as much of the track as possible plays.
    const double lastStartBeat = from.beatAt(fromLimitSec) - fadeBeats;
    if (lastStartBeat < 0.0) {
        return std::nullopt;
    }
    const double phraseStartBeat =
            std::floor((lastStartBeat + kEpsSec / from.beatSecAt(fromLimitSec)) /
                               kBeatsPerPhrase) *
            kBeatsPerPhrase;
    const double fadeBegin = from.beatTime(phraseStartBeat);
    // Auto DJ re-plans on every tempo step (e.g. while a track glides back
    // to its own tempo), and a re-plan can land just after the phrase start
    // has passed. Up to one bar late, keep the plan: the fade starts now and
    // the incoming track starts further in by the same time, so beats and
    // phrases still line up. More than that (the DJ jumped ahead): give up.
    const double lateSec = std::max(0.0, fromNowSec - fadeBegin);
    if (lateSec > kLateStartBeats * from.beatSecAt(fadeBegin) + kEpsSec) {
        return std::nullopt; // that phrase has already passed
    }

    // Incoming track: the first phrase start at or after its earliest start.
    // Up to one beat early is fine: the first sound is often a few
    // milliseconds after the grid's first beat.
    double toPhraseBeat = std::ceil(
            (to.beatAt(toEarliestSec) - 1.0) / kBeatsPerPhrase);
    toPhraseBeat = std::max(0.0, toPhraseBeat) * kBeatsPerPhrase;

    if (toBodyStartSec >= 0.0) {
        // Where the beat kicks in, snapped to the grid.
        const double entry = entryBeat(to, toBodyStartSec, toBodyMarked);
        // Start that part of the fade before it (half: the beat kicks in at
        // the bass swap; none: the song starts on it). Whole bars only.
        // Never earlier than the intro start found above.
        const double leadBeats =
                std::round(std::clamp(entryAt, 0.0, 1.0) * fadeBeats / kBeatsPerBar) *
                kBeatsPerBar;
        toPhraseBeat = std::max(toPhraseBeat, entry - leadBeats);
    }

    Plan p;
    p.fromFadeBeginSec = fadeBegin;
    p.fromFadeEndSec = from.beatTime(phraseStartBeat + fadeBeats);
    p.toStartSec = to.beatTime(toPhraseBeat) + lateSec;
    p.bars = bars;
    return p;
}

double fadeNowLimitSec(const Grid& from, double nowSec, int bars, double minLeadSec) {
    if (!from.isValid()) {
        return -1.0;
    }
    const double beat = from.beatAt(nowSec + minLeadSec);
    const double phrase = std::max(0.0, std::ceil(beat / kBeatsPerPhrase)) * kBeatsPerPhrase;
    return from.beatTime(phrase + static_cast<double>(bars) * kBeatsPerBar);
}

std::optional<Plan> planUnmatched(const Grid& from,
        const Grid& to,
        double fromNowSec,
        double fromLimitSec,
        double toEarliestSec,
        int bars,
        double toBodyStartSec,
        bool toBodyMarked) {
    // The outgoing side is the same as a beatmatched mix: a whole number of
    // phrases, over before its energy drops. Only the incoming side differs.
    const auto outgoing = plan(from, from, fromNowSec, fromLimitSec, 0.0, bars);
    if (!outgoing) {
        return std::nullopt;
    }
    Plan p = *outgoing;
    double toStart = toEarliestSec;
    if (toBodyStartSec >= 0.0) {
        const double entrySec = to.isValid()
                ? to.beatTime(entryBeat(to, toBodyStartSec, toBodyMarked))
                : toBodyStartSec;
        // The tempos differ, so the two beats must not play together: the
        // new beat kicks in as the fade ENDS, and during the fade only the
        // incoming intro plays under the outgoing beat. If the intro is
        // shorter than the fade (or there is none), the fade is shortened
        // to fit it - down to a quick switch of kQuickSwitchBars - and still
        // ends on the outgoing phrase ending.
        // Counted in the outgoing track's own bars (they may bend).
        const double introSec = entrySec - toEarliestSec;
        const double fadeSec = p.fromFadeEndSec - p.fromFadeBeginSec;
        if (introSec < fadeSec) {
            const double endBeat = std::round(from.beatAt(p.fromFadeEndSec));
            int fitBars = 0;
            while (fitBars < p.bars &&
                    p.fromFadeEndSec - from.beatTime(endBeat - (fitBars + 1) * kBeatsPerBar) <=
                            introSec + 1e-6) {
                ++fitBars;
            }
            const int bars = std::clamp(fitBars, kQuickSwitchBars, p.bars);
            p.fromFadeBeginSec = from.beatTime(endBeat - bars * kBeatsPerBar);
            p.bars = bars;
        }
        toStart = std::max(toEarliestSec, entrySec - (p.fromFadeEndSec - p.fromFadeBeginSec));
    }
    // A fade that should have started a moment ago: the incoming track
    // starts that much further in, so its beat still arrives on time.
    const double lateSec = std::max(0.0, fromNowSec - p.fromFadeBeginSec);
    p.toStartSec = toStart + lateSec;
    return p;
}

} // namespace phrasealign
