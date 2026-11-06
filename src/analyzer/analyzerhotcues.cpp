#include "analyzer/analyzerhotcues.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

#include "analyzer/analyzertrack.h"
#include "track/track.h"

namespace mixxx::hotcues {

namespace {
constexpr int kWindow = 16;        // beats either side of a change
constexpr int kMinGap = 32;        // 8 bars between cues
constexpr double kMinChangeDb = 3; // ponytail: hand-picked, tune on labelled tracks
constexpr double kSoundDb = -50;

double mean(const std::vector<double>& v, int from, int to) {
    double sum = 0;
    for (int i = from; i < to; ++i) {
        sum += v[i];
    }
    return sum / (to - from);
}
} // namespace

std::vector<AutoCue> placeCues(const std::vector<double>& loudDb,
        const std::vector<double>& bassDb) {
    const int n = static_cast<int>(loudDb.size());
    const auto firstSound = std::find_if(loudDb.begin(), loudDb.end(), [](double db) {
        return db >= kSoundDb;
    });
    if (firstSound == loudDb.end()) {
        return {};
    }
    const int first = static_cast<int>(firstSound - loudDb.begin());

    // level after beat i minus before, from first sound
    std::vector<double> dLoud(n, 0);
    std::vector<double> dBass(n, 0);
    std::vector<double> score(n, 0);
    for (int i = first + kWindow; i + kWindow <= n; ++i) {
        dLoud[i] = mean(loudDb, i, i + kWindow) - mean(loudDb, i - kWindow, i);
        dBass[i] = mean(bassDb, i, i + kWindow) - mean(bassDb, i - kWindow, i);
        score[i] = std::abs(dLoud[i]) + std::abs(dBass[i]);
    }

    // bar phase: beat offset mod 4 where change peaks fall
    std::array<double, 4> votes{};
    for (int i = 2; i + 2 < n; ++i) {
        if (score[i] >= kMinChangeDb &&
                std::max({score[i - 2], score[i - 1], score[i + 1], score[i + 2]}) <= score[i]) {
            votes[i % 4] += score[i];
        }
    }
    const int phase = votes == std::array<double, 4>{}
            ? first % 4
            : static_cast<int>(std::max_element(votes.begin(), votes.end()) - votes.begin());

    const int start = first + (phase - first % 4 + 4) % 4;
    if (start >= n) {
        return {};
    }
    std::vector<AutoCue> cues{{start, QStringLiteral("Start")}};

    std::vector<int> candidates;
    for (int i = phase; i < n; i += 4) {
        if (score[i] >= kMinChangeDb) {
            candidates.push_back(i);
        }
    }
    std::stable_sort(candidates.begin(), candidates.end(), [&](int a, int b) {
        return score[a] > score[b];
    });
    for (const int beat : candidates) {
        if (static_cast<int>(cues.size()) == kMaxCues) {
            break;
        }
        const bool farEnough = std::all_of(cues.begin(), cues.end(), [&](const AutoCue& cue) {
            return std::abs(cue.beat - beat) >= kMinGap;
        });
        if (farEnough) {
            cues.push_back({beat,
                    dBass[beat] >= kMinChangeDb ? QStringLiteral("Drop")
                            : dBass[beat] <= -kMinChangeDb
                            ? QStringLiteral("Break")
                            : QStringLiteral("Phrase")});
        }
    }
    std::sort(cues.begin(), cues.end(), [](const AutoCue& a, const AutoCue& b) {
        return a.beat < b.beat;
    });
    return cues;
}

} // namespace mixxx::hotcues

namespace {

constexpr int kBlock = 512;     // frames per power value
constexpr double kBassHz = 120; // two one-pole lowpasses

bool hasHotcues(const Track& track) {
    const auto cues = track.getCuePoints();
    return std::any_of(cues.begin(), cues.end(), [](const CuePointer& pCue) {
        return pCue->getType() == mixxx::CueType::HotCue;
    });
}

mixxx::RgbColor colorFor(const QString& label) {
    if (label == QStringLiteral("Start")) {
        return mixxx::RgbColor(0x32BE44); // mixxx green
    }
    if (label == QStringLiteral("Drop")) {
        return mixxx::RgbColor(0xC50A08); // mixxx red
    }
    if (label == QStringLiteral("Break")) {
        return mixxx::RgbColor(0x0044FF); // mixxx blue
    }
    return mixxx::RgbColor(0xF8D200); // mixxx yellow
}

double toDb(double meanSquare) {
    return 10 * std::log10(meanSquare + 1e-10);
}

} // namespace

AnalyzerHotcues::AnalyzerHotcues(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_channels(0),
          m_lowpass(0),
          m_low1(0),
          m_low2(0),
          m_sum(0),
          m_bassSum(0),
          m_blockFrames(0) {
}

bool AnalyzerHotcues::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount,
        SINT frameLength) {
    if (frameLength <= 0 ||
            !m_pConfig->getValue(ConfigKey("[Library]", "AutoHotcues"), true) ||
            hasHotcues(*track.getTrack())) {
        return false;
    }
    m_channels = channelCount;
    m_lowpass = 1 - std::exp(-2 * std::numbers::pi * kBassHz / sampleRate.value());
    m_low1 = m_low2 = m_sum = m_bassSum = 0;
    m_blockFrames = 0;
    m_power.clear();
    m_bassPower.clear();
    m_power.reserve(frameLength / kBlock + 1);
    m_bassPower.reserve(frameLength / kBlock + 1);
    return true;
}

bool AnalyzerHotcues::processSamples(const CSAMPLE* pIn, SINT count) {
    const SINT frames = count / m_channels;
    for (SINT f = 0; f < frames; ++f) {
        double x = 0;
        for (int c = 0; c < m_channels; ++c) {
            x += pIn[f * m_channels + c];
        }
        x /= m_channels;
        m_low1 += m_lowpass * (x - m_low1);
        m_low2 += m_lowpass * (m_low1 - m_low2);
        m_sum += x * x;
        m_bassSum += m_low2 * m_low2;
        if (++m_blockFrames == kBlock) {
            m_power.push_back(static_cast<float>(m_sum / kBlock));
            m_bassPower.push_back(static_cast<float>(m_bassSum / kBlock));
            m_sum = m_bassSum = 0;
            m_blockFrames = 0;
        }
    }
    return true;
}

void AnalyzerHotcues::storeResults(TrackPointer pTrack) {
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    // user may have set cues during analysis
    if (!pBeats || m_power.empty() || hasHotcues(*pTrack)) {
        return;
    }
    const auto blocks = static_cast<int>(m_power.size());
    std::vector<double> loudDb;
    std::vector<double> bassDb;
    std::vector<mixxx::audio::FramePos> positions;
    for (auto it = pBeats->iteratorFrom(mixxx::audio::kStartFramePos);; ++it) {
        const int from = static_cast<int>(it->value() / kBlock);
        const int to = std::max(from + 1, static_cast<int>(std::next(it)->value() / kBlock));
        if (to > blocks) {
            break;
        }
        double power = 0;
        double bass = 0;
        for (int b = from; b < to; ++b) {
            power += m_power[b];
            bass += m_bassPower[b];
        }
        loudDb.push_back(toDb(power / (to - from)));
        bassDb.push_back(toDb(bass / (to - from)));
        positions.push_back(*it);
    }

    const auto cues = mixxx::hotcues::placeCues(loudDb, bassDb);
    for (int i = 0; i < static_cast<int>(cues.size()); ++i) {
        CuePointer pCue = pTrack->createAndAddCue(mixxx::CueType::HotCue,
                i,
                positions[cues[i].beat],
                mixxx::audio::kInvalidFramePos,
                colorFor(cues[i].label));
        pCue->setLabel(cues[i].label);
    }
    qDebug() << "AnalyzerHotcues: set" << cues.size() << "hot cues on" << pTrack->getLocation();
}
