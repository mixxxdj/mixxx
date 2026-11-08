#include "analyzer/analyzerbeats.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "analyzer/analyzertrack.h"
#include "analyzer/plugins/analyzerqueenmarybeats.h"
#include "analyzer/plugins/beattrackingrefiner.h"
#include "test/mixxxtest.h"
#include "track/beatfactory.h"
#include "track/track.h"

namespace {

std::vector<CSAMPLE> makePulseTrack(int sampleRate,
        double bpm,
        double duration,
        double offset,
        int accentPeriod,
        double drift = 0) {
    std::vector<CSAMPLE> audio(static_cast<size_t>(duration * sampleRate) * 2);
    const double period = 60.0 / bpm;
    for (int beat = 0;; ++beat) {
        const double base = beat * period;
        const double time = offset + base + drift * base * base / (2 * duration);
        if (time >= duration) {
            break;
        }
        const int start = static_cast<int>(std::round(time * sampleRate));
        const double amplitude = accentPeriod > 1 && beat % accentPeriod ? 0.25 : 0.8;
        for (int i = 0; i < sampleRate / 40 && start + i < duration * sampleRate; ++i) {
            const double t = static_cast<double>(i) / sampleRate;
            const auto value = static_cast<CSAMPLE>(
                    amplitude * std::exp(-t * 180) * std::cos(2 * M_PI * 110 * t));
            audio[(start + i) * 2] += value;
            audio[(start + i) * 2 + 1] += value;
        }
    }
    return audio;
}

class AnalyzerBeatAudioTest : public testing::TestWithParam<int> {};

TEST_P(AnalyzerBeatAudioTest, PulseTracks) {
    const int sampleRate = GetParam();
    for (const double bpm : {80., 96., 120., 140., 174.}) {
        for (const int accentPeriod : {1, 2, 3}) {
            SCOPED_TRACE(testing::Message()
                    << sampleRate << " Hz, " << bpm << " BPM, accentPeriod=" << accentPeriod);
            auto audio = makePulseTrack(sampleRate, bpm, 24., 0.2, accentPeriod);
            mixxx::AnalyzerQueenMaryBeats analyzer;
            ASSERT_TRUE(analyzer.initialize(mixxx::audio::SampleRate(sampleRate)));
            for (size_t i = 0; i < audio.size(); i += 8192) {
                ASSERT_TRUE(analyzer.processSamples(audio.data() + i,
                        static_cast<SINT>(std::min(size_t(8192), audio.size() - i))));
            }
            ASSERT_TRUE(analyzer.finalize());
            const auto beats = analyzer.getBeats();
            ASSERT_GT(beats.size(), 10);
            auto grid = BeatFactory::makePreferredBeats(
                    beats, {}, true, mixxx::audio::SampleRate(sampleRate));
            ASSERT_TRUE(grid);
            const double actualBpm = grid->getBpmInRange(mixxx::audio::kStartFramePos,
                                                 mixxx::audio::FramePos(24. * sampleRate))
                                             .value();
            const double phase =
                    std::remainder(grid->firstBeat().value() / sampleRate - 0.2, 60. / bpm);
            EXPECT_NEAR(bpm, actualBpm, 0.2);
            EXPECT_NEAR(0., phase, 0.015);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(SampleRates, AnalyzerBeatAudioTest, testing::Values(44100, 48000, 96000));

TEST(AnalyzerBeatAudioTest, HighFrequencySubdivisionsDoNotDoubleSlowTempo) {
    constexpr int sampleRate = 44100;
    auto audio = makePulseTrack(sampleRate, 80, 24, 0.2, 1);
    for (double time = 0.2 + 60. / 160; time < 24.; time += 60. / 80) {
        const int start = static_cast<int>(std::round(time * sampleRate));
        for (int i = 0; i < sampleRate / 40 && start + i < 24 * sampleRate; ++i) {
            const double t = static_cast<double>(i) / sampleRate;
            const auto value =
                    static_cast<CSAMPLE>(0.3 * std::exp(-180 * t) * std::cos(2 * M_PI * 8000 * t));
            audio[(start + i) * 2] += value;
            audio[(start + i) * 2 + 1] += value;
        }
    }
    mixxx::AnalyzerQueenMaryBeats analyzer;
    ASSERT_TRUE(analyzer.initialize(mixxx::audio::SampleRate(sampleRate)));
    ASSERT_TRUE(analyzer.processSamples(audio.data(), audio.size()));
    ASSERT_TRUE(analyzer.finalize());
    const auto grid = BeatFactory::makePreferredBeats(
            analyzer.getBeats(), {}, true, mixxx::audio::SampleRate(sampleRate));
    ASSERT_TRUE(grid);
    EXPECT_NEAR(80,
            grid->getBpmInRange(
                        mixxx::audio::kStartFramePos, mixxx::audio::FramePos(24 * sampleRate))
                    .value(),
            0.1);
}

TEST(BeatTrackingRefinerTest, HarmonicHypothesesRequireOnsetEvidence) {
    constexpr int rate = 44100;
    constexpr int hop = 512;
    constexpr double bpm = 150;
    const double period = 60. * rate / bpm;
    std::vector<double> onsets(static_cast<size_t>(24. * rate / hop));
    for (double position = 0.2 * rate; position < 24. * rate; position += period) {
        onsets[static_cast<size_t>(std::round(position / hop - 0.5))] = 1.;
    }
    for (double ratio : {0.5, 2. / 3.}) {
        SCOPED_TRACE(ratio);
        QVector<mixxx::audio::FramePos> beats;
        for (double position = 0.2 * rate; position < 24. * rate; position += period / ratio) {
            beats.push_back(mixxx::audio::FramePos(position));
        }
        const auto refined =
                mixxx::refineBeatTrackingTempo(beats, onsets, mixxx::audio::SampleRate(rate), hop);
        const auto grid =
                BeatFactory::makePreferredBeats(refined, {}, true, mixxx::audio::SampleRate(rate));
        ASSERT_TRUE(grid);
        EXPECT_NEAR(bpm, grid->getBpmInRange(refined.front(), refined.back()).value(), 0.1);
        std::fill(onsets.begin(), onsets.end(), 0.);
        EXPECT_EQ(beats,
                mixxx::refineBeatTrackingTempo(beats, onsets, mixxx::audio::SampleRate(rate), hop));
        for (double position = 0.2 * rate; position < 24. * rate; position += period) {
            onsets[static_cast<size_t>(std::round(position / hop - 0.5))] = 1.;
        }
    }
}

TEST(BeatTrackingRefinerTest, StrongOnsetsCorrectHalfBeatPhaseWithoutChangingTempo) {
    constexpr int rate = 44100;
    constexpr int hop = 512;
    constexpr double period = 0.5 * rate;
    std::vector<double> onsets(24 * rate / hop);
    QVector<mixxx::audio::FramePos> wrongPhase;
    for (double position = 0.2 * rate; position < 24 * rate; position += period) {
        onsets[static_cast<size_t>(std::round(position / hop - 0.5))] = 1;
        wrongPhase.push_back(mixxx::audio::FramePos(position + period / 2));
    }
    const auto corrected =
            mixxx::refineBeatTrackingTempo(wrongPhase, onsets, mixxx::audio::SampleRate(rate), hop);
    ASSERT_GT(corrected.size(), 20);
    for (const auto beat : corrected) {
        EXPECT_NEAR(0, std::remainder(beat.value() - 0.2 * rate, period), hop);
    }
    EXPECT_NEAR(period, corrected[2] - corrected[1], 1);
}

TEST(BeatTrackingRefinerTest, UncertainOrChangingTempoIsPreserved) {
    constexpr int rate = 44100;
    constexpr int hop = 512;
    QVector<mixxx::audio::FramePos> beats;
    for (int i = 0; i < 60; ++i) {
        beats.push_back(mixxx::audio::FramePos((0.2 + 0.4 * i) * rate));
    }
    std::minstd_rand random(42);
    std::vector<double> onsets(24 * rate / hop);
    for (auto& onset : onsets) {
        onset = static_cast<double>(random()) / random.max();
    }
    EXPECT_EQ(beats,
            mixxx::refineBeatTrackingTempo(beats, onsets, mixxx::audio::SampleRate(rate), hop));
    for (int i = 0; i < beats.size(); ++i) {
        beats[i] += 0.0008 * i * i * rate;
    }
    EXPECT_EQ(beats,
            mixxx::refineBeatTrackingTempo(beats, onsets, mixxx::audio::SampleRate(rate), hop));
}

TEST(AnalyzerBeatLifecycleTest, SilenceAndReinitializationDoNotReusePreviousBeats) {
    mixxx::AnalyzerQueenMaryBeats analyzer;
    const auto rate = mixxx::audio::SampleRate(44100);
    auto audio = makePulseTrack(44100, 120, 24, 0.2, 1);
    ASSERT_TRUE(analyzer.initialize(rate));
    ASSERT_TRUE(analyzer.processSamples(audio.data(), audio.size()));
    ASSERT_TRUE(analyzer.finalize());
    ASSERT_FALSE(analyzer.getBeats().isEmpty());
    ASSERT_TRUE(analyzer.initialize(rate));
    std::fill(audio.begin(), audio.end(), 0);
    ASSERT_TRUE(analyzer.processSamples(audio.data(), audio.size()));
    ASSERT_TRUE(analyzer.finalize());
    EXPECT_TRUE(analyzer.getBeats().isEmpty());
    EXPECT_FALSE(analyzer.finalize());
}

TEST(AnalyzerBeatAudioTest, InputChunkBoundariesDoNotChangeAnalysis) {
    const auto rate = mixxx::audio::SampleRate(44100);
    const auto audio = makePulseTrack(44100, 174, 24, 0.2, 2);
    mixxx::AnalyzerQueenMaryBeats whole;
    mixxx::AnalyzerQueenMaryBeats chunked;
    ASSERT_TRUE(whole.initialize(rate));
    ASSERT_TRUE(chunked.initialize(rate));
    ASSERT_TRUE(whole.processSamples(audio.data(), audio.size()));
    for (size_t offset = 0; offset < audio.size();) {
        const size_t count = std::min(size_t(1394), audio.size() - offset);
        ASSERT_TRUE(chunked.processSamples(audio.data() + offset, count));
        offset += count;
    }
    ASSERT_TRUE(whole.finalize());
    ASSERT_TRUE(chunked.finalize());
    EXPECT_EQ(whole.getBeats(), chunked.getBeats());
}

class AnalyzerBeatProtectionTest : public MixxxTest {};

TEST_F(AnalyzerBeatProtectionTest, AutoUsesFullAudioAndCachesEitherResolvedGridType) {
    BeatDetectionSettings settings(config());
    settings.setTempoMode(mixxx::BeatGridMode::Auto);
    settings.setFastAnalysis(true);
    settings.setReanalyzeWhenSettingsChange(true);
    for (double drift : {0.0, 0.1}) {
        SCOPED_TRACE(drift);
        auto track = Track::newTemporary();
        const auto rate = mixxx::audio::SampleRate(44100);
        track->setAudioProperties(mixxx::audio::ChannelCount::stereo(),
                rate,
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(128));
        const auto audio = makePulseTrack(44100, 120, 128, 0.2, 1, drift);
        AnalyzerBeats analyzer(config());
        ASSERT_TRUE(analyzer.initialize(
                AnalyzerTrack(track), rate, mixxx::audio::ChannelCount::stereo(), 128 * 44100));
        for (size_t i = 0; i < audio.size(); i += 8192) {
            ASSERT_TRUE(analyzer.processSamples(
                    audio.data() + i, std::min(size_t(8192), audio.size() - i)));
        }
        analyzer.storeResults(track);
        analyzer.cleanup();
        ASSERT_TRUE(track->getBeats());
        EXPECT_EQ(drift == 0, track->getBeats()->hasConstantTempo());
        EXPECT_TRUE(track->getBeats()->getSubVersion().contains("tempo_mode=auto:3"));
        EXPECT_FALSE(track->getBeats()->getSubVersion().contains("fast_analysis"));
        EXPECT_FALSE(analyzer.initialize(
                AnalyzerTrack(track), rate, mixxx::audio::ChannelCount::stereo(), 128 * 44100));
    }
}

TEST_F(AnalyzerBeatProtectionTest, LockedAndManualGridsArePreserved) {
    auto track = Track::newTemporary();
    track->setAudioProperties(mixxx::audio::ChannelCount::stereo(),
            mixxx::audio::SampleRate(44100),
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(24));
    const auto grid = mixxx::Beats::fromConstTempo(
            track->getSampleRate(), mixxx::audio::FramePos(4410), mixxx::Bpm(87));
    track->trySetBeats(grid);
    track->setBpmLocked(true);
    AnalyzerBeats analyzer(config(), true);
    EXPECT_FALSE(analyzer.initialize(AnalyzerTrack(track),
            track->getSampleRate(),
            mixxx::audio::ChannelCount::stereo(),
            24 * 44100));
    EXPECT_EQ(grid, track->getBeats());
    track->setBpmLocked(false);
    EXPECT_FALSE(analyzer.initialize(AnalyzerTrack(track),
            track->getSampleRate(),
            mixxx::audio::ChannelCount::stereo(),
            24 * 44100));
    EXPECT_EQ(grid, track->getBeats());
}

TEST_F(AnalyzerBeatProtectionTest, CorrectedTempoIsStoredWithAnalysisVersion) {
    BeatDetectionSettings(config()).setTempoMode(mixxx::BeatGridMode::Constant);
    auto track = Track::newTemporary();
    track->setAudioProperties(mixxx::audio::ChannelCount::stereo(),
            mixxx::audio::SampleRate(44100),
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(24));
    AnalyzerBeats analyzer(config());
    ASSERT_TRUE(analyzer.initialize(AnalyzerTrack(track),
            track->getSampleRate(),
            mixxx::audio::ChannelCount::stereo(),
            24 * 44100));
    const auto audio = makePulseTrack(44100, 174, 24, 0.2, 1);
    for (size_t i = 0; i < audio.size(); i += 8192) {
        ASSERT_TRUE(analyzer.processSamples(
                audio.data() + i, static_cast<SINT>(std::min(size_t(8192), audio.size() - i))));
    }
    analyzer.storeResults(track);
    analyzer.cleanup();
    ASSERT_TRUE(track->getBeats());
    EXPECT_NEAR(174, track->getBpm(), 0.1);
    EXPECT_TRUE(track->getBeats()->getSubVersion().contains("beat_tracker_version=3"));
    EXPECT_TRUE(track->getBeats()->getSubVersion().contains("audio_grid_evidence=5"));
    EXPECT_FALSE(analyzer.initialize(AnalyzerTrack(track),
            track->getSampleRate(),
            mixxx::audio::ChannelCount::stereo(),
            24 * 44100));
}

TEST_F(AnalyzerBeatProtectionTest, FastAnalysisProcessesPartialFinalBlock) {
    BeatDetectionSettings settings(config());
    settings.setTempoMode(mixxx::BeatGridMode::Constant);
    settings.setFastAnalysis(true);
    auto track = Track::newTemporary();
    track->setAudioProperties(mixxx::audio::ChannelCount::stereo(),
            mixxx::audio::SampleRate(44100),
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(64));
    AnalyzerBeats analyzer(config());
    ASSERT_TRUE(analyzer.initialize(AnalyzerTrack(track),
            track->getSampleRate(),
            mixxx::audio::ChannelCount::stereo(),
            64 * 44100));
    const auto audio = makePulseTrack(44100, 174, 64, 0.2, 1);
    ASSERT_TRUE(analyzer.processSamples(audio.data(), audio.size()));
    analyzer.storeResults(track);
    analyzer.cleanup();
    ASSERT_TRUE(track->getBeats());
    EXPECT_NEAR(174, track->getBpm(), 0.1);
}

} // namespace
