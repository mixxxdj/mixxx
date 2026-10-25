#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "library/autodj/smart/phrasealign.h"

using phrasealign::Grid;

namespace {
// 120 BPM: one beat = 0.5 s, one bar = 2 s, one 8-bar phrase = 16 s.
const Grid k120{0.0, 0.5};
} // namespace

TEST(PhraseAlignTest, BarsAreWholePhrases) {
    EXPECT_EQ(8, phrasealign::barsForSeconds(16.0, 0.5));  // 8 bars = 16 s
    EXPECT_EQ(8, phrasealign::barsForSeconds(5.0, 0.5));   // at least 1 phrase
    EXPECT_EQ(16, phrasealign::barsForSeconds(30.0, 0.5)); // ~15 bars -> 16
    EXPECT_EQ(8, phrasealign::barsForSeconds(16.0, 0.0));  // no grid: default
}

TEST(PhraseAlignTest, FadeStartsOnLastPhraseThatFits) {
    // Track ends (last sound) at 200 s. An 8-bar fade (16 s) must start on a
    // phrase start (multiple of 16 s) and end by 200 s: 176 -> 192.
    const auto p = phrasealign::plan(k120, k120, 0.0, 200.0, 0.0, 8);
    ASSERT_TRUE(p.has_value());
    EXPECT_DOUBLE_EQ(176.0, p->fromFadeBeginSec);
    EXPECT_DOUBLE_EQ(192.0, p->fromFadeEndSec);
    EXPECT_DOUBLE_EQ(0.0, p->toStartSec);
    EXPECT_EQ(8, p->bars);
}

TEST(PhraseAlignTest, FollowsTheGridOffset) {
    // Grid starts 0.3 s in; phrases are 0.3 + 16 k.
    const Grid from{0.3, 0.5};
    const auto p = phrasealign::plan(from, k120, 0.0, 200.0, 0.0, 8);
    ASSERT_TRUE(p.has_value());
    EXPECT_NEAR(176.3, p->fromFadeBeginSec, 1e-9);
    EXPECT_NEAR(192.3, p->fromFadeEndSec, 1e-9);
}

TEST(PhraseAlignTest, IncomingStartsOnAPhraseStart) {
    // Incoming grid starts at 0.5 s; first sound at 0.52 s (just after the
    // first beat) -> start at the first beat, not a whole phrase later.
    const Grid to{0.5, 0.5};
    const auto p = phrasealign::plan(k120, to, 0.0, 200.0, 0.52, 8);
    ASSERT_TRUE(p.has_value());
    EXPECT_NEAR(0.5, p->toStartSec, 1e-9);
    // Intro marked at 20 s: phrases start at 0.5, 16.5, 32.5 s -> 32.5.
    const auto p2 = phrasealign::plan(k120, to, 0.0, 200.0, 20.0, 8);
    ASSERT_TRUE(p2.has_value());
    EXPECT_NEAR(32.5, p2->toStartSec, 1e-9);
}

TEST(PhraseAlignTest, GivesUpWhenNothingFits) {
    // Already past the last phrase that fits.
    EXPECT_FALSE(phrasealign::plan(k120, k120, 180.0, 200.0, 0.0, 8).has_value());
    // Track shorter than the fade.
    EXPECT_FALSE(phrasealign::plan(k120, k120, 0.0, 10.0, 0.0, 8).has_value());
    // No grid.
    EXPECT_FALSE(phrasealign::plan(Grid{}, k120, 0.0, 200.0, 0.0, 8).has_value());
}

TEST(PhraseAlignTest, BassSwapLandsOnABarLine) {
    // The fade is a whole number of bars, so its middle is a bar line.
    const auto p = phrasealign::plan(k120, k120, 0.0, 200.0, 0.0, 8);
    ASSERT_TRUE(p.has_value());
    const double middle = (p->fromFadeBeginSec + p->fromFadeEndSec) / 2.0;
    const double bars = middle / (4 * 0.5);
    EXPECT_DOUBLE_EQ(bars, std::round(bars));
}

TEST(PhraseAlignTest, IncomingBeatKicksInAtTheBassSwap) {
    // Incoming beat kicks in at 64 s (4 phrases in); fade is 8 bars = 16 s.
    // Start at 56 s so the beat arrives at the middle of the fade (8 s in),
    // exactly when the bass swaps. The long intro before 56 s is skipped.
    const auto p = phrasealign::plan(k120, k120, 0.0, 200.0, 0.0, 8, 64.0);
    ASSERT_TRUE(p.has_value());
    EXPECT_DOUBLE_EQ(56.0, p->toStartSec);
    const double half = (p->fromFadeEndSec - p->fromFadeBeginSec) / 2.0;
    EXPECT_DOUBLE_EQ(64.0, p->toStartSec + half);
    // Body measured a little late (65.2 s): still the 64 s phrase.
    const auto p2 = phrasealign::plan(k120, k120, 0.0, 200.0, 0.0, 8, 65.2);
    ASSERT_TRUE(p2.has_value());
    EXPECT_DOUBLE_EQ(56.0, p2->toStartSec);
}

TEST(PhraseAlignTest, IncomingBeatCanComeInAtTheStart) {
    // The DJ's choice: the new song starts on its first downbeat (64 s), so
    // its full sound comes in over the whole fade; the intro is skipped.
    const auto p = phrasealign::plan(k120, k120, 0.0, 200.0, 0.0, 8, 64.0, false, 0.0);
    ASSERT_TRUE(p.has_value());
    EXPECT_DOUBLE_EQ(64.0, p->toStartSec);
    // The outgoing side does not change.
    const auto middle = phrasealign::plan(k120, k120, 0.0, 200.0, 0.0, 8, 64.0, false, 0.5);
    ASSERT_TRUE(middle.has_value());
    EXPECT_DOUBLE_EQ(middle->fromFadeBeginSec, p->fromFadeBeginSec);
    EXPECT_DOUBLE_EQ(56.0, middle->toStartSec);
}

TEST(PhraseAlignTest, ShortIntroStartsAtTheBeginning) {
    // Beat kicks in at 6 s, before the swap point: start at the first beat.
    const auto p = phrasealign::plan(k120, k120, 0.0, 200.0, 0.0, 8, 6.0);
    ASSERT_TRUE(p.has_value());
    EXPECT_DOUBLE_EQ(0.0, p->toStartSec);
}

TEST(PhraseAlignTest, BeatOnABarThatIsNotAPhraseStart) {
    // Beat kicks in at 26 s = bar 14 (not a phrase start): the entry snaps
    // to that bar, and the track starts 4 bars (8 s) earlier, at 18 s.
    const auto p = phrasealign::plan(k120, k120, 0.0, 200.0, 0.0, 8, 26.4);
    ASSERT_TRUE(p.has_value());
    EXPECT_DOUBLE_EQ(18.0, p->toStartSec);
}

TEST(PhraseAlignTest, FadeEndsBeforeTheOutgoingFadeOut) {
    // The caller passes min(outro end, body end) as the limit: a song whose
    // own fade-out starts at 180 s must be mixed out before then.
    const auto p = phrasealign::plan(k120, k120, 0.0, 180.0, 0.0, 8);
    ASSERT_TRUE(p.has_value());
    EXPECT_LE(p->fromFadeEndSec, 180.0);
    EXPECT_DOUBLE_EQ(160.0, p->fromFadeBeginSec);
}

TEST(PhraseAlignTest, NothingStaysBeatLandsInTheMiddle) {
    // Real track: 111.68 BPM, first beat 0.272 s, beat kicks in at 34.66 s
    // (measured body start 34 s). With the intro start (0.28 s) as the
    // earliest start, the track starts 4 bars (16 beats) before the beat,
    // so the beat lands at the middle of an 8-bar fade.
    phrasealign::Grid ns;
    ns.firstBeatSec = 0.272;
    ns.beatSec = 60.0 / 111.68;
    const auto p = phrasealign::plan(k120, ns, 0.0, 250.0, 0.28, 8, 34.0);
    ASSERT_TRUE(p.has_value());
    const double beatAt = ns.beatTime(64);
    EXPECT_NEAR(34.66, beatAt, 0.01);
    EXPECT_NEAR(beatAt - 16 * ns.beatSec, p->toStartSec, 1e-9);
    // Planning again from the same intro start gives the same answer.
    const auto again = phrasealign::plan(k120, ns, 0.0, 250.0, 0.28, 8, 34.0);
    ASSERT_TRUE(again.has_value());
    EXPECT_DOUBLE_EQ(p->toStartSec, again->toStartSec);
}

TEST(PhraseAlignTest, MarkedBeatIsTrustedToTheBeat) {
    // Lack of Sense: 120.39 BPM, first beat 0.394 s. Measured body start
    // 15.0 s would snap to the bar at beat 28 (14.35 s). The DJ marked the
    // beat at 15.71 s: that is beat 30.7, so the nearest beat, 31 (15.84 s),
    // lands at the middle of the fade and the start is 16 beats earlier.
    phrasealign::Grid ls;
    ls.firstBeatSec = 0.394;
    ls.beatSec = 60.0 / 120.387;
    const auto measured = phrasealign::plan(k120, ls, 0.0, 250.0, 0.39, 8, 15.0);
    ASSERT_TRUE(measured.has_value());
    EXPECT_NEAR(ls.beatTime(28 - 16), measured->toStartSec, 1e-9);
    const auto marked = phrasealign::plan(k120, ls, 0.0, 250.0, 0.39, 8, 15.71, true);
    ASSERT_TRUE(marked.has_value());
    EXPECT_NEAR(ls.beatTime(31 - 16), marked->toStartSec, 1e-9);
}

TEST(PhraseAlignTest, EntryBeatForTheAutomaticIntroEndMarker) {
    // Nothing Stays: measured 34.0 s -> the phrase at beat 64 (34.66 s).
    phrasealign::Grid ns;
    ns.firstBeatSec = 0.272;
    ns.beatSec = 60.0 / 111.68;
    EXPECT_DOUBLE_EQ(64.0, phrasealign::entryBeat(ns, 34.0, false));
    // A marked time is only snapped to the nearest beat...
    EXPECT_DOUBLE_EQ(63.0, phrasealign::entryBeat(ns, 34.0, true));
    // ...and a value that is already on the grid stays where it is, so a
    // marker set by the analyzer gives Auto DJ the same answer again.
    const double marker = ns.beatTime(phrasealign::entryBeat(ns, 34.0, false));
    EXPECT_DOUBLE_EQ(64.0, phrasealign::entryBeat(ns, marker, true));
}

TEST(PhraseAlignTest, BodyEndSnapsBackToABar) {
    // 120 BPM from 0 s: bars every 2 s. 181.3 s -> 180 s; 182 s stays.
    EXPECT_DOUBLE_EQ(180.0, phrasealign::bodyEndBarSec(k120, 181.3));
    EXPECT_DOUBLE_EQ(182.0, phrasealign::bodyEndBarSec(k120, 182.0));
}

TEST(PhraseAlignTest, AReplanJustAfterThePhraseStartStillMixesOnTheBeat) {
    // Limit 200 s at 120 BPM: the fade is 176 -> 192 s. A re-plan at
    // 176.4 s (0.4 s late, e.g. a tempo step while gliding) keeps it: the
    // fade starts now and the incoming track starts 0.4 s further in.
    const auto p = phrasealign::plan(k120, k120, 176.4, 200.0, 0.0, 8);
    ASSERT_TRUE(p.has_value());
    EXPECT_DOUBLE_EQ(176.0, p->fromFadeBeginSec);
    EXPECT_NEAR(0.4, p->toStartSec, 1e-9);
    // More than one bar (2 s) late, e.g. the DJ jumped ahead: give up.
    EXPECT_FALSE(phrasealign::plan(k120, k120, 178.5, 200.0, 0.0, 8).has_value());
}

TEST(PhraseAlignTest, UnmatchedMixBringsTheBeatInAsTheFadeEnds) {
    // Outgoing 120 BPM, limit 200 s: fade 176 -> 192 s (16 s). Incoming at
    // 111.68 BPM whose beat kicks in at 34.66 s: it starts 16 s earlier, so
    // its beat arrives as the outgoing track is gone.
    phrasealign::Grid ns;
    ns.firstBeatSec = 0.272;
    ns.beatSec = 60.0 / 111.68;
    const auto p = phrasealign::planUnmatched(k120, ns, 0.0, 200.0, 0.28, 8, 34.0);
    ASSERT_TRUE(p.has_value());
    EXPECT_DOUBLE_EQ(176.0, p->fromFadeBeginSec);
    EXPECT_DOUBLE_EQ(192.0, p->fromFadeEndSec);
    EXPECT_NEAR(ns.beatTime(64) - 16.0, p->toStartSec, 1e-9);
    // No intro: starts at the first sound.
    const auto q = phrasealign::planUnmatched(k120, ns, 0.0, 200.0, 0.28, 8, -1.0);
    ASSERT_TRUE(q.has_value());
    EXPECT_DOUBLE_EQ(0.28, q->toStartSec);
}

TEST(PhraseAlignTest, UnmatchedMixWithAShortIntroIsAQuickSwitch) {
    phrasealign::Grid ns;
    ns.firstBeatSec = 0.272;
    ns.beatSec = 60.0 / 111.68;
    // Beat at about 6 s (a measured time, snapped to a bar), 0.28 s of
    // silence first: the intro holds a few whole bars. The fade shrinks to
    // that many bars, still ending on the outgoing phrase ending (192 s),
    // and the new beat comes in as it ends.
    const auto r = phrasealign::planUnmatched(k120, ns, 0.0, 200.0, 0.28, 8, 6.0);
    ASSERT_TRUE(r.has_value());
    const double entry = ns.beatTime(phrasealign::entryBeat(ns, 6.0, false));
    const int introBars = static_cast<int>(std::floor((entry - 0.28) / 2.0));
    EXPECT_GE(introBars, 2);
    EXPECT_LT(introBars, 8);
    EXPECT_EQ(introBars, r->bars);
    EXPECT_DOUBLE_EQ(192.0 - 2.0 * introBars, r->fromFadeBeginSec);
    EXPECT_DOUBLE_EQ(192.0, r->fromFadeEndSec);
    EXPECT_NEAR(entry - 2.0 * introBars, r->toStartSec, 1e-9);
    EXPECT_GE(r->toStartSec, 0.28);

    // The beat starts at once (like the Depeche Mode edit): a quick switch
    // of one bar at the phrase ending.
    const auto s = phrasealign::planUnmatched(k120, ns, 0.0, 200.0, 0.1, 8, 0.14, true);
    ASSERT_TRUE(s.has_value());
    EXPECT_EQ(phrasealign::kQuickSwitchBars, s->bars);
    EXPECT_DOUBLE_EQ(190.0, s->fromFadeBeginSec);
    EXPECT_DOUBLE_EQ(192.0, s->fromFadeEndSec);
    EXPECT_DOUBLE_EQ(0.1, s->toStartSec);

    // A long intro still gets the whole fade.
    const auto t = phrasealign::planUnmatched(k120, ns, 0.0, 200.0, 0.28, 8, 34.0);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(8, t->bars);
    EXPECT_DOUBLE_EQ(176.0, t->fromFadeBeginSec);
}

TEST(PhraseAlignTest, FadeNowWaitsForTheNextPhrase) {
    // 120 BPM from 0 s: phrases every 16 s. Pressed at 50 s: the next phrase
    // is at 64 s, so the 8-bar fade runs 64 -> 80 s.
    const double limit = phrasealign::fadeNowLimitSec(k120, 50.0, 8);
    EXPECT_DOUBLE_EQ(80.0, limit);
    const auto p = phrasealign::plan(k120, k120, 50.0, limit, 0.0, 8);
    ASSERT_TRUE(p.has_value());
    EXPECT_DOUBLE_EQ(64.0, p->fromFadeBeginSec);
    // Pressed 1 s before a phrase: too close to cue, so the one after.
    EXPECT_DOUBLE_EQ(96.0, phrasealign::fadeNowLimitSec(k120, 63.0, 8));
}

namespace {
// A drummer who speeds up: beat n is 0.5 s long at the start and gets
// 0.1 ms shorter every beat (120 -> about 133 BPM over 200 beats).
std::vector<double> bendingBeats(int count, double firstSec = 0.2) {
    std::vector<double> t;
    double now = firstSec;
    for (int n = 0; n < count; ++n) {
        t.push_back(now);
        now += 0.5 - 0.0001 * n;
    }
    return t;
}
} // namespace

TEST(PhraseAlignTest, BeatMapFindsItsBeats) {
    const auto times = bendingBeats(400);
    const Grid map = Grid::fromBeats(times);
    ASSERT_TRUE(map.isValid());
    ASSERT_TRUE(map.isMap());
    EXPECT_DOUBLE_EQ(times[0], map.firstBeatSec);
    for (int n : {0, 1, 32, 199, 399}) {
        EXPECT_NEAR(times[n], map.beatTime(n), 1e-9);
        EXPECT_NEAR(n, map.beatAt(times[n]), 1e-9);
    }
    // Half way between two beats, and past both ends (the nearest beat
    // length carries on).
    EXPECT_NEAR(100.5, map.beatAt(0.5 * (times[100] + times[101])), 1e-9);
    EXPECT_NEAR(times[399] + (times[399] - times[398]), map.beatTime(400), 1e-9);
    EXPECT_NEAR(-1.0, map.beatAt(times[0] - 0.5), 1e-9);
    // The local beat length follows the drummer.
    EXPECT_NEAR(0.5, map.beatSecAt(times[0]), 0.001);
    EXPECT_NEAR(0.5 - 0.0001 * 300, map.beatSecAt(times[300]), 0.001);
    // A false beat just before the first real one (The Smiths: 0.04 s, then
    // the real beats every 0.572 s) does not change the tempo at the start.
    std::vector<double> pickup{0.041};
    for (int n = 0; n < 40; ++n) {
        pickup.push_back(0.354 + 0.572 * n);
    }
    const Grid withPickup = Grid::fromBeats(pickup);
    EXPECT_NEAR(0.572, withPickup.beatSecAt(0.041), 1e-9);
    EXPECT_NEAR(0.572, withPickup.beatSecAt(3.0), 1e-9);
}

TEST(PhraseAlignTest, SteadyTempoIgnoresAWrongMapInTheFadeOut) {
    // Xymox "A Million Things": 120 BPM, then the beat map goes wrong where
    // the music stops (8 "beats" at 159 BPM, then 40 at 235 BPM).
    std::vector<double> times{1.0};
    auto add = [&times](int count, double bpm) {
        for (int n = 0; n < count; ++n) {
            times.push_back(times.back() + 60.0 / bpm);
        }
    };
    add(400, 120.0);
    add(8, 159.0);
    add(40, 235.0);
    const Grid map = Grid::fromBeats(times);
    const double end = times.back() - 2.0;
    EXPECT_GT(60.0 / map.beatSecAt(end), 200.0); // the plain one is fooled
    EXPECT_NEAR(0.5, map.steadyBeatSecAt(end), 1e-6);
    EXPECT_NEAR(0.5, map.steadyBeatSecAt(times[100]), 1e-6);
    // A real tempo change that holds (100 then 120 BPM for 200 beats each)
    // is followed.
    std::vector<double> change{0.0};
    for (int n = 0; n < 200; ++n) {
        change.push_back(change.back() + 0.6);
    }
    for (int n = 0; n < 200; ++n) {
        change.push_back(change.back() + 0.5);
    }
    const Grid changing = Grid::fromBeats(change);
    EXPECT_NEAR(0.5, changing.steadyBeatSecAt(change[380]), 1e-6);
    EXPECT_NEAR(0.6, changing.steadyBeatSecAt(change[50]), 1e-6);
    // At another speed, everything is scaled in time.
    const Grid fast = map.atSpeed(1.25);
    EXPECT_NEAR(times[64] / 1.25, fast.beatTime(64), 1e-9);
}

TEST(PhraseAlignTest, SteadyBeatMapPlansLikeASteadyGrid) {
    std::vector<double> times;
    for (int n = 0; n < 600; ++n) {
        times.push_back(0.3 + 0.5 * n);
    }
    const Grid map = Grid::fromBeats(times);
    const Grid steady{0.3, 0.5};
    const auto a = phrasealign::plan(steady, steady, 10.0, 200.0, 0.4, 8, 30.0, true);
    const auto b = phrasealign::plan(map, map, 10.0, 200.0, 0.4, 8, 30.0, true);
    ASSERT_TRUE(a.has_value());
    ASSERT_TRUE(b.has_value());
    EXPECT_NEAR(a->fromFadeBeginSec, b->fromFadeBeginSec, 1e-9);
    EXPECT_NEAR(a->fromFadeEndSec, b->fromFadeEndSec, 1e-9);
    EXPECT_NEAR(a->toStartSec, b->toStartSec, 1e-9);
}

TEST(PhraseAlignTest, BendingTrackFadesOnItsOwnPhrases) {
    const auto times = bendingBeats(400);
    const Grid from = Grid::fromBeats(times);
    // Must be over by beat 300: the last whole 8-bar fade starts on beat
    // 256 (a phrase start) and ends on beat 288 - exactly on the beats of
    // the map, not where a steady grid would put them.
    const auto p = phrasealign::plan(from, k120, 0.0, times[300], 0.0, 8);
    ASSERT_TRUE(p.has_value());
    EXPECT_NEAR(times[256], p->fromFadeBeginSec, 1e-9);
    EXPECT_NEAR(times[288], p->fromFadeEndSec, 1e-9);
    // The incoming track's intro is counted in its own bending beats too.
    const auto q = phrasealign::plan(k120, from, 0.0, 200.0, times[40], 8);
    ASSERT_TRUE(q.has_value());
    EXPECT_NEAR(times[64], q->toStartSec, 1e-9);
    // Fade Now: the next phrase start of the map.
    const double limit = phrasealign::fadeNowLimitSec(from, times[70], 8);
    EXPECT_NEAR(times[96 + 32], limit, 1e-9);
    // A marker between beats snaps to the nearest beat of the map.
    EXPECT_NEAR(150.0, phrasealign::entryBeat(from, times[150] + 0.1, true), 1e-9);
}

TEST(PhraseAlignTest, QuickSwitchCountsBendingBars) {
    const auto times = bendingBeats(400);
    const Grid from = Grid::fromBeats(times);
    const Grid to{0.0, 0.4};
    // The incoming beat starts after an intro of just over 2 bars of the
    // outgoing track: a 2-bar switch, on the outgoing beats.
    const auto p = phrasealign::plan(from, from, 0.0, times[300], 0.0, 8);
    ASSERT_TRUE(p.has_value());
    const double twoBars = times[288] - times[280];
    const auto s = phrasealign::planUnmatched(
            from, to, 0.0, times[300], 0.0, 8, twoBars + 0.05, true);
    ASSERT_TRUE(s.has_value());
    EXPECT_EQ(2, s->bars);
    EXPECT_NEAR(times[280], s->fromFadeBeginSec, 1e-9);
    EXPECT_NEAR(times[288], s->fromFadeEndSec, 1e-9);
}
