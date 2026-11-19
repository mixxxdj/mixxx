#include "analyzer/plugins/rbgrid/rbgridtracker.h"

#include <pffft.h>

#include <algorithm>
#include <cmath>
#include <numeric>

#include "analyzer/plugins/rbgrid/rbdownbeattrees.h"
#include "analyzer/plugins/rbgrid/rbgridmodel.h"

namespace rbgrid {

namespace {

// The reference implementation runs at 22050 Hz with a 1024-point FFT and a
// hop of 64 samples. Other sample rates scale FFT size/hop to keep the same
// time resolution.
constexpr double kRefSampleRate = 22050.0;
constexpr double kRefFps = kRefSampleRate / 64.0;
constexpr std::array<double, kBandCount + 1> kBandEdgesHz = {
        20, 60, 120, 250, 500, 1000, 2000, 4000, 8000, 11025};
constexpr std::array<float, kBandCount> kBandWeights = {
        1.0f, 1.0f, 0.8f, 0.5f, 0.4f, 0.3f, 0.3f, 0.3f, 0.2f};
// Beat timing uses a flatter weighting than tempo: rekordbox's beat positions
// follow the transient (hi-hat/snare click) more than the kick body.
constexpr std::array<float, kBandCount> kTimingBandWeights = {
        1.0f, 1.0f, 0.8f, 0.5f, 0.4f, 0.6f, 0.8f, 0.8f, 0.8f};
constexpr double kWhitenSec = 0.5;
constexpr double kIntegerSnapBpm = 0.01;
constexpr std::array<double, 9> kOctaveMultipliers = {
        1.0 / 3, 1.0 / 2, 2.0 / 3, 3.0 / 4, 1.0, 4.0 / 3, 3.0 / 2, 2.0, 3.0};
constexpr double kMinBpm = 55.0; // octave candidates outside are dropped
constexpr double kMaxBpm = 215.0;
constexpr double kMinTrackSec = 10.0;
constexpr double kOnsetWindowSec = 0.02; // +- window for the onset at a beat
constexpr int kHalfbandHalfLength = 63;  // taps -63..63
// decimator input history is trimmed once this much is no longer needed
constexpr std::size_t kHistoryTrim = 4096;

/// Per-candidate features used by the tempo-octave chooser.
struct CandidateFeatures {
    double bpm;
    double lb;    // log2(bpm / 120)
    double ac;    // onset autocorrelation at the beat period
    double ac2;   // ... at twice the beat period
    double ach;   // ... at half the beat period
    double sharp; // peak-to-mean of the onset function folded at the period
    // Per low/mid/high group, from the group onset function folded at 1, 2
    // and 4 periods: off-beat / on-beat ratio, peak-to-mean, contrast between
    // alternate beats, and spread of the 4 beats of a bar.
    std::array<double, 3> off, sh, alt, bar;
};

/// main onset, per-band onset (9), per-band energy (9), is-current, is-half
using PhaseFeatures = std::array<double, kPhaseFeatureCount>;

/// Standardised linear model: sum of (x - mean) / sd * weight.
template<std::size_t N>
double linearScore(const double* x,
        const double (&mean)[N],
        const double (&sd)[N],
        const double (&weight)[N]) {
    double s = 0;
    for (std::size_t i = 0; i < N; ++i) {
        s += (x[i] - mean[i]) / sd[i] * weight[i];
    }
    return s;
}

/// Index of the highest score(i) for i < n (first one on ties), -1 if none
/// is above -1e300 (e.g. all NaN).
template<typename Score>
int argmax(int n, Score score) {
    int best = -1;
    double bestScore = -1e300;
    for (int i = 0; i < n; ++i) {
        const double s = score(i);
        if (s > bestScore) {
            bestScore = s;
            best = i;
        }
    }
    return best;
}

/// Offset of the vertex of the parabola through (-1, a), (0, c), (1, d), or 0
/// if it does not open downwards.
double parabolicOffset(double a, double c, double d) {
    const double den = a - 2 * c + d;
    return den < 0 ? 0.5 * (a - d) / den : 0.0;
}

/// x - centred moving average (zero padded, numpy 'same' semantics), clamped >= 0.
std::vector<float> whiten(const std::vector<float>& o, int k) {
    const std::size_t n = o.size();
    std::vector<double> prefix(n + 1, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + o[i];
    }
    std::vector<float> out(n);
    const long hi = (k - 1) / 2;
    for (std::size_t i = 0; i < n; ++i) {
        const long end = std::min<long>(static_cast<long>(n) - 1, static_cast<long>(i) + hi);
        const long begin = std::max<long>(0, static_cast<long>(i) + hi - k + 1);
        const double mean = (prefix[end + 1] - prefix[begin]) / k;
        out[i] = static_cast<float>(std::max(0.0, o[i] - mean));
    }
    return out;
}

/// Mean of o folded at a fractional period into nb phase bins.
std::vector<double> fold(const std::vector<float>& o, double period, int nb) {
    std::vector<double> h(nb, 0.0);
    // n = floor(size / period) - 1 keeps every index + 1 inside o (period >= 1)
    const long n = static_cast<long>(static_cast<double>(o.size()) / period) - 1;
    if (n <= 0) {
        return h;
    }
    const double step = period / nb;
    const float* p = o.data();
    for (long k = 0; k < n; ++k) {
        const double base = k * period;
        for (int j = 0; j < nb; ++j) {
            const double idx = base + j * step;
            const std::size_t i0 = static_cast<std::size_t>(idx);
            const double f = idx - static_cast<double>(i0);
            h[j] += p[i0] * (1.0 - f) + p[i0 + 1] * f;
        }
    }
    for (auto& v : h) {
        v /= static_cast<double>(n);
    }
    return h;
}

double mean(const std::vector<double>& v) {
    return std::accumulate(v.begin(), v.end(), 0.0) / static_cast<double>(v.size());
}

class Analysis {
  public:
    Analysis(std::vector<float> odf, std::array<std::vector<float>, 3> groups, double fps)
            : m_odf(std::move(odf)),
              m_groupOdf(std::move(groups)),
              m_fps(fps) {
        // autocorrelation of the mean-removed ODF, normalised to lag 0
        const std::size_t n = m_odf.size();
        const double mu = std::accumulate(m_odf.begin(), m_odf.end(), 0.0) / n;
        std::vector<float> x(n);
        for (std::size_t i = 0; i < n; ++i) {
            x[i] = static_cast<float>(m_odf[i] - mu);
        }
        const std::size_t maxLag = std::min<std::size_t>(
                n - 1, static_cast<std::size_t>(m_fps * 60.0 / 40.0 * 2.0) + 4);
        m_ac.assign(maxLag + 1, 0.0);
        for (std::size_t l = 0; l <= maxLag; ++l) {
            const float* a = x.data();
            const float* b = x.data() + l;
            const std::size_t m = n - l;
            // float partial sums (vectorised), flushed to double every 1024
            double s = 0.0;
            std::size_t i = 0;
            while (i + 8 <= m) {
                float acc[8] = {};
                const std::size_t end = std::min(m - m % 8, i + 1024);
                for (; i < end; i += 8) {
                    for (int u = 0; u < 8; ++u) {
                        acc[u] += a[i + u] * b[i + u];
                    }
                }
                for (int u = 0; u < 8; ++u) {
                    s += acc[u];
                }
            }
            for (; i < m; ++i) {
                s += static_cast<double>(a[i]) * b[i];
            }
            m_ac[l] = s;
        }
        const double ac0 = m_ac[0] > 0 ? m_ac[0] : 1.0;
        for (auto& v : m_ac) {
            v /= ac0;
        }
    }

    double autocorr(double lag) const {
        if (lag <= 0) {
            return m_ac[0];
        }
        const std::size_t i0 = static_cast<std::size_t>(lag);
        if (i0 + 1 >= m_ac.size()) {
            return 0.0;
        }
        const double f = lag - static_cast<double>(i0);
        return m_ac[i0] * (1.0 - f) + m_ac[i0 + 1] * f;
    }

    double periodOf(double bpm) const {
        return 60.0 * m_fps / bpm;
    }

    double sharp(double bpm) const {
        const double per = periodOf(bpm);
        const auto h = fold(m_odf, per, std::max(8, static_cast<int>(std::lround(per / 2))));
        const double m = mean(h);
        return (*std::max_element(h.begin(), h.end()) - m) / (m + 1e-9);
    }

    double coarseTempo() const {
        const int lagMin = static_cast<int>(m_fps * 60.0 / 260.0);
        const int lagMax = static_cast<int>(m_fps * 60.0 / 40.0);
        double best = -1e9;
        double bestBpm = 0.0;
        for (int l = lagMin; l < lagMax; ++l) {
            const double bpm = 60.0 * m_fps / l;
            if (bpm < 60.0 || bpm > 200.0) {
                continue;
            }
            const double s = autocorr(l) + 0.5 * autocorr(2.0 * l) + 0.5 * autocorr(l / 2.0);
            if (s > best) {
                best = s;
                bestBpm = bpm;
            }
        }
        return bestBpm;
    }

    double fineTempo(double b0, double span) const {
        const double st = span > 0.003 ? 0.02 : 0.004;
        double b1 = b0;
        double best = -1e9;
        for (double b = b0 * (1 - span); b <= b0 * (1 + span) + st / 2; b += st) {
            const double s = sharp(b);
            if (s > best) {
                best = s;
                b1 = b;
            }
        }
        std::vector<double> g, s;
        for (double b = b1 - st; b <= b1 + st + 1e-4; b += 0.001) {
            g.push_back(b);
            s.push_back(sharp(b));
        }
        const std::size_t i = std::max_element(s.begin(), s.end()) - s.begin();
        if (i > 0 && i + 1 < s.size()) {
            return g[i] + parabolicOffset(s[i - 1], s[i], s[i + 1]) * 0.001;
        }
        return g[i];
    }

    CandidateFeatures features(double b) const {
        CandidateFeatures f;
        const double per = periodOf(b);
        constexpr int nb = 32;
        f.bpm = b;
        f.lb = std::log2(b / 120.0);
        f.ac = autocorr(per);
        f.ac2 = autocorr(2 * per);
        f.ach = autocorr(per / 2);
        const auto h1 = fold(m_odf, per, nb);
        const std::size_t pk = std::max_element(h1.begin(), h1.end()) - h1.begin();
        const double m1 = mean(h1);
        f.sharp = (h1[pk] - m1) / (m1 + 1e-9);
        for (int gi = 0; gi < 3; ++gi) {
            const auto h = fold(m_groupOdf[gi], per, nb);
            const double m = mean(h) + 1e-9;
            f.off[gi] = h[(pk + nb / 2) % nb] / (h[pk] + 1e-9);
            f.sh[gi] = (h[pk] - m) / m;
            const auto h2 = fold(m_groupOdf[gi], 2 * per, 2 * nb);
            const double a = h2[pk];
            const double c = h2[pk + nb];
            f.alt[gi] = std::abs(a - c) / (a + c + 1e-9);
            const auto h4 = fold(m_groupOdf[gi], 4 * per, 4 * nb);
            const double v[4] = {h4[pk], h4[pk + nb], h4[pk + 2 * nb], h4[pk + 3 * nb]};
            const double vm = (v[0] + v[1] + v[2] + v[3]) / 4;
            double var = 0;
            for (double x : v) {
                var += (x - vm) * (x - vm);
            }
            f.bar[gi] = std::sqrt(var / 4) / (vm + 1e-9);
        }
        return f;
    }

    /// Beat phase in seconds, about [0, period) (may be slightly negative).
    double phaseSec(double bpm, const std::vector<float>& timingOdf) const {
        const double per = periodOf(bpm);
        const int nb = static_cast<int>(std::lround(per)) * 4;
        const auto h0 = fold(timingOdf, per, nb);
        std::vector<double> h(nb);
        for (int i = 0; i < nb; ++i) {
            double s = 0;
            for (int d = -2; d <= 2; ++d) {
                s += h0[(i + d + nb) % nb];
            }
            h[i] = s / 5;
        }
        const int i = static_cast<int>(std::max_element(h.begin(), h.end()) - h.begin());
        const double a = h[(i - 1 + nb) % nb];
        const double c = h[i];
        const double d = h[(i + 1) % nb];
        return (i + parabolicOffset(a, c, d)) * per / nb / m_fps;
    }

  private:
    std::vector<float> m_odf;
    std::array<std::vector<float>, 3> m_groupOdf;
    double m_fps;
    std::vector<double> m_ac;
};

double octaveScore(const CandidateFeatures& c, const std::vector<CandidateFeatures>& all) {
    auto maxOf = [&](double CandidateFeatures::* p) {
        double m = -1e300;
        for (const auto& x : all) {
            m = std::max(m, x.*p);
        }
        return m + 1e-9;
    };
    const double v[kOctaveFeatureCount] = {
            c.lb,
            c.lb * c.lb,
            std::max(0.0, std::log2(85.0 / c.bpm)),
            std::max(0.0, std::log2(c.bpm / 175.0)),
            c.ac / maxOf(&CandidateFeatures::ac),
            c.ac2 / maxOf(&CandidateFeatures::ac2),
            c.ach / maxOf(&CandidateFeatures::ach),
            c.sharp / maxOf(&CandidateFeatures::sharp),
            c.off[0],
            c.sh[0],
            c.alt[0],
            c.bar[0],
            c.off[1],
            c.sh[1],
            c.alt[1],
            c.bar[1],
            c.off[2],
            c.sh[2],
            c.alt[2],
            c.bar[2],
    };
    return linearScore(v, kOctaveMean, kOctaveStd, kOctaveWeight);
}

/// Circular linear interpolation into a phase histogram; pos in bins.
double sampleCircular(const std::vector<double>& h, double pos) {
    const int nb = static_cast<int>(h.size());
    double p = std::fmod(pos, nb);
    if (p < 0) {
        p += nb;
    }
    const int i0 = static_cast<int>(p) % nb;
    const double f = p - std::floor(p);
    return h[i0] * (1 - f) + h[(i0 + 1) % nb] * f;
}

/// Features for the beat phase candidates phi + j/4 period (j = 0..3), used to
/// pick the beat (kick) rather than the off-beat like rekordbox does: folded
/// onset strength (all bands and per band) at the candidate, folded per-band
/// energy just after it, and whether it is the strongest/opposite candidate.
std::vector<PhaseFeatures> phaseFeatures(const std::vector<float>& odf,
        const std::vector<std::array<float, kBandCount>>& logE,
        double fps,
        double bpm,
        double phiSec) {
    constexpr int nb = 64;
    const double per = 60.0 * fps / bpm;
    auto normFold = [&](const std::vector<float>& x, bool zscore) {
        auto h = fold(x, per, nb);
        const double m = mean(h);
        double var = 0;
        for (double v : h) {
            var += (v - m) * (v - m);
        }
        const double sd = std::sqrt(var / nb);
        for (auto& v : h) {
            v = zscore ? (v - m) / (sd + 1e-9) : v / (m + 1e-9);
        }
        return h;
    };
    const std::size_t n = logE.size();
    std::array<std::vector<double>, kBandCount> hFlux, hEnergy;
    std::vector<float> x(n);
    for (int band = 0; band < kBandCount; ++band) {
        for (std::size_t i = 0; i < n; ++i) {
            x[i] = i > 0 ? std::max(0.0f, logE[i][band] - logE[i - 1][band]) : 0.0f;
        }
        hFlux[band] = normFold(x, false);
        for (std::size_t i = 0; i < n; ++i) {
            x[i] = logE[i][band];
        }
        hEnergy[band] = normFold(x, true);
    }
    const auto hMain = normFold(odf, false);
    const double binsPerSec = nb / (per / fps);
    std::vector<PhaseFeatures> out(4);
    for (int j = 0; j < 4; ++j) {
        const double pos = (phiSec + j * per / fps / 4) * binsPerSec;
        const double posE = pos + 0.02 * binsPerSec;
        PhaseFeatures& f = out[j];
        f[0] = sampleCircular(hMain, pos);
        for (int band = 0; band < kBandCount; ++band) {
            f[1 + band] = sampleCircular(hFlux[band], pos);
            f[10 + band] = sampleCircular(hEnergy[band], posE);
        }
        f[19] = j == 0 ? 1.0 : 0.0;
        f[20] = j == 2 ? 1.0 : 0.0;
    }
    return out;
}

double percentile(std::vector<float> v, double q) {
    if (v.empty()) {
        return 0.0;
    }
    const double pos = q / 100.0 * (v.size() - 1);
    const std::size_t i = static_cast<std::size_t>(pos);
    const double f = pos - i;
    std::nth_element(v.begin(), v.begin() + i, v.end());
    if (i + 1 >= v.size()) {
        return v[i];
    }
    const float next = *std::min_element(v.begin() + i + 1, v.end());
    return v[i] * (1 - f) + next * f;
}

int mod4(long x) {
    return static_cast<int>(((x % 4) + 4) % 4);
}

/// Per-beat downbeat score (log-odds that a beat is rekordbox's beat "1"),
/// averaged per bar position. Features are beat-synchronous band energy
/// changes and per-band onsets around each beat.
std::array<double, 4> perBeatDownbeatScores(
        const std::vector<std::array<float, kBandCount>>& logE,
        double fps,
        double firstBeatSec,
        double beatSec) {
    std::array<double, 4> out{};
    const long n = static_cast<long>(logE.size());
    const long beatCount = static_cast<long>((n / fps - firstBeatSec) / beatSec) - 1;
    if (beatCount < 16) {
        return out;
    }
    std::vector<long> beatFrame(beatCount + 1);
    for (long k = 0; k <= beatCount; ++k) {
        beatFrame[k] = std::clamp(std::lround((firstBeatSec + beatSec * k) * fps), 0L, n);
    }
    // per-beat mean log energy and per-band onset
    std::vector<std::array<double, kBandCount>> beatEnergy(beatCount), beatOnset(beatCount);
    const long w = std::max(1L, static_cast<long>(kOnsetWindowSec * fps));
    for (long k = 0; k < beatCount; ++k) {
        const long a = beatFrame[k];
        const long e = beatFrame[k + 1];
        for (int b = 0; b < kBandCount; ++b) {
            double s = 0;
            for (long i = a; i < e; ++i) {
                s += logE[i][b];
            }
            beatEnergy[k][b] = s / std::max(1L, e - a);
            float m = 0;
            for (long i = std::max(1L, a - w); i < std::min(n, a + w + 1); ++i) {
                m = std::max(m, logE[i][b] - logE[i - 1][b]);
            }
            beatOnset[k][b] = m;
        }
    }
    for (int b = 0; b < kBandCount; ++b) {
        double mu = 0, mo = 0;
        for (long k = 0; k < beatCount; ++k) {
            mu += beatEnergy[k][b];
            mo += beatOnset[k][b];
        }
        mu /= beatCount;
        mo /= beatCount;
        double var = 0;
        for (long k = 0; k < beatCount; ++k) {
            var += (beatEnergy[k][b] - mu) * (beatEnergy[k][b] - mu);
        }
        const double sd = std::sqrt(var / beatCount) + 1e-6;
        for (long k = 0; k < beatCount; ++k) {
            beatEnergy[k][b] = (beatEnergy[k][b] - mu) / sd;
            beatOnset[k][b] /= mo + 1e-6;
        }
    }
    std::vector<std::array<double, kBandCount>> energyPrefix(beatCount + 1);
    energyPrefix[0].fill(0.0);
    for (long k = 0; k < beatCount; ++k) {
        for (int b = 0; b < kBandCount; ++b) {
            energyPrefix[k + 1][b] = energyPrefix[k][b] + beatEnergy[k][b];
        }
    }
    auto at = [beatCount](long i) { return std::clamp(i, 0L, beatCount - 1); };
    auto win = [&](long k, long a, long e, int b) {
        const long lo = std::clamp(k + a, 0L, beatCount);
        const long hi = std::clamp(k + e, 0L, beatCount);
        return (energyPrefix[hi][b] - energyPrefix[lo][b]) / std::max(1L, hi - lo);
    };
    static_assert(8 * kBandCount + 2 == kDownbeatTreeFeatureCount);
    std::array<double, kDownbeatTreeFeatureCount> f;
    std::array<double, 4> sum{};
    std::array<int, 4> cnt{};
    for (long k = 0; k < beatCount; ++k) {
        for (int b = 0; b < kBandCount; ++b) {
            f[b] = beatEnergy[k][b] - beatEnergy[at(k - 1)][b];
            f[kBandCount + b] = beatEnergy[k][b] - beatEnergy[at(k + 1)][b];
            f[2 * kBandCount + b] = win(k, 0, 4, b) - win(k, -4, 0, b);
            f[3 * kBandCount + b] = win(k, 0, 8, b) - win(k, -8, 0, b);
            f[4 * kBandCount + b] = win(k, 0, 16, b) - win(k, -16, 0, b);
            f[5 * kBandCount + b] = beatOnset[k][b];
            f[6 * kBandCount + b] = beatOnset[k][b] -
                    (beatOnset[at(k - 1)][b] + beatOnset[at(k + 1)][b]) / 2;
            f[7 * kBandCount + b] = beatOnset[k][b] -
                    (beatOnset[at(k - 2)][b] + beatOnset[at(k + 2)][b]) / 2;
        }
        f[8 * kBandCount] = static_cast<double>(k) / beatCount;
        f[8 * kBandCount + 1] = std::min<long>(k, 64) / 64.0;
        double score = kDownbeatTreeBaseline;
        for (int t = 0; t < kDownbeatTreeCount; ++t) {
            const DownbeatTreeNode* tree = kDownbeatTreeNodes + kDownbeatTreeStart[t];
            int i = 0;
            while (tree[i].feature >= 0) {
                i = static_cast<float>(f[tree[i].feature]) <= tree[i].threshold
                        ? tree[i].left
                        : tree[i].right;
            }
            score += tree[i].value;
        }
        sum[k % 4] += score;
        ++cnt[k % 4];
    }
    for (int r = 0; r < 4; ++r) {
        out[r] = sum[r] / std::max(1, cnt[r]);
    }
    return out;
}

/// Bar phase (0..3): which beat, counted from the first beat, rekordbox would
/// call "1". Linear model over per-phase features: onset strength per band
/// group, section-change novelty, beat loudness, where the music/kick starts
/// and the per-beat downbeat score.
int chooseBarPhase(const std::vector<std::array<float, kBandCount>>& logE,
        const std::array<std::vector<float>, 3>& groupOdf,
        double fps,
        double firstBeatSec,
        double beatSec) {
    const long n = static_cast<long>(logE.size());
    const long beatCount = static_cast<long>((n / fps - firstBeatSec) / beatSec);
    if (beatCount < 8) {
        return 0;
    }
    std::vector<double> lin(n), loud(n);
    std::vector<float> lowFlux(n, 0.0f);
    for (long i = 0; i < n; ++i) {
        double s = 0;
        for (int b = 0; b < kBandCount; ++b) {
            s += std::expm1(static_cast<double>(logE[i][b]));
        }
        lin[i] = s;
        loud[i] = std::log1p(s);
        if (i > 0) {
            const float lo = logE[i][0] + logE[i][1] + logE[i][2];
            const float lp = logE[i - 1][0] + logE[i - 1][1] + logE[i - 1][2];
            lowFlux[i] = std::max(0.0f, lo - lp);
        }
    }
    std::vector<long> beatFrame(beatCount);
    for (long k = 0; k < beatCount; ++k) {
        beatFrame[k] = std::lround((firstBeatSec + beatSec * k) * fps);
    }
    // feature columns, in the order of kDownbeatMean/Std/Weight
    enum Feature {
        kLow,
        kMid,
        kHigh,
        kNovTop,
        kNovPos,
        kStart,
        kFirst,
        kLoud,
        kNov16,
        kKick,
        kBeat,
    };
    static_assert(kBeat + 1 == kDownbeatFeatureCount);
    double feat[4][kDownbeatFeatureCount] = {};
    auto normalizeColumn = [&feat](int col, bool subtractMean) {
        double m = 0;
        for (int r = 0; r < 4; ++r) {
            m += feat[r][col] / 4;
        }
        for (int r = 0; r < 4; ++r) {
            feat[r][col] = subtractMean ? feat[r][col] - m : feat[r][col] / (m + 1e-9);
        }
    };
    // onset strength per band group (low/mid/high) on each bar position
    double beatsPerPosition[4] = {};
    for (long k = 0; k < beatCount; ++k) {
        beatsPerPosition[k % 4] += 1;
    }
    const long w = std::max(1L, static_cast<long>(kOnsetWindowSec * fps));
    for (int g = 0; g < 3; ++g) {
        for (long k = 0; k < beatCount; ++k) {
            const long a = std::max(0L, beatFrame[k] - w);
            const long e = std::min(n, beatFrame[k] + w + 1);
            float m = 0;
            for (long i = a; i < e; ++i) {
                m = std::max(m, groupOdf[g][i]);
            }
            feat[k % 4][kLow + g] += m;
        }
        for (int r = 0; r < 4; ++r) {
            feat[r][kLow + g] /= std::max(1.0, beatsPerPosition[r]);
        }
        normalizeColumn(kLow + g, false);
    }
    // loudness per beat
    const long span = static_cast<long>(beatSec * fps);
    std::vector<double> beatLoudness(beatCount, 0.0);
    for (long k = 0; k < beatCount; ++k) {
        // beatFrame[k] < n - span + 1, so the range is never empty
        const long e = std::min(n, beatFrame[k] + span);
        double s = 0;
        for (long i = beatFrame[k]; i < e; ++i) {
            s += loud[i];
        }
        beatLoudness[k] = s / (e - beatFrame[k]);
    }
    auto meanRange = [&](long a, long e) {
        double s = 0;
        for (long i = a; i < e; ++i) {
            s += beatLoudness[i];
        }
        return s / (e - a);
    };
    // loudness change across the beat over 4 and 16 beats (section changes)
    std::vector<double> nov(beatCount, 0.0), nov16(beatCount, 0.0);
    for (long k = 4; k < beatCount - 4; ++k) {
        nov[k] = meanRange(k, k + 4) - meanRange(k - 4, k);
    }
    for (long k = 16; k < beatCount - 16; ++k) {
        nov16[k] = std::abs(meanRange(k, k + 16) - meanRange(k - 16, k));
    }
    for (int r = 0; r < 4; ++r) {
        std::vector<double> a;
        double pos = 0, l = 0, mx = 0;
        long c = 0;
        for (long k = r; k < beatCount; k += 4) {
            a.push_back(std::abs(nov[k]));
            pos += std::max(nov[k], 0.0);
            l += beatLoudness[k];
            mx = std::max(mx, nov16[k]);
            ++c;
        }
        std::sort(a.begin(), a.end());
        const std::size_t top = std::max<std::size_t>(1, a.size() / 10);
        double t = 0;
        for (std::size_t i = a.size() - top; i < a.size(); ++i) {
            t += a[i];
        }
        feat[r][kNovTop] = t / top;
        feat[r][kNovPos] = pos / std::max(1L, c);
        feat[r][kLoud] = l / std::max(1L, c);
        feat[r][kNov16] = mx;
    }
    normalizeColumn(kNovTop, false);
    normalizeColumn(kNovPos, false);
    normalizeColumn(kLoud, true);
    normalizeColumn(kNov16, false);
    // first frame above a threshold, or 0 if there is none
    auto firstAbove = [n](const auto& v, double threshold) {
        long i = 0;
        while (i < n && !(v[i] > threshold)) {
            ++i;
        }
        return i == n ? 0L : i;
    };
    auto barPosition = [&](long frame) {
        return mod4(std::lround((frame / fps - firstBeatSec) / beatSec));
    };
    // first significant sound, the first beat and the first kick onset
    std::vector<float> linF(lin.begin(), lin.end());
    feat[barPosition(firstAbove(lin, percentile(linF, 50) * 0.05))][kStart] = 1;
    feat[0][kFirst] = 1;
    feat[barPosition(firstAbove(lowFlux, percentile(lowFlux, 99) / 8))][kKick] = 1;
    // per-beat model, centred over the four bar positions
    const std::array<double, 4> beatScores =
            perBeatDownbeatScores(logE, fps, firstBeatSec, beatSec);
    const double beatMean = std::accumulate(beatScores.begin(), beatScores.end(), 0.0) / 4;
    for (int r = 0; r < 4; ++r) {
        feat[r][kBeat] = beatScores[r] - beatMean;
    }
    return std::max(0, argmax(4, [&](int r) {
        return linearScore(feat[r], kDownbeatMean, kDownbeatStd, kDownbeatWeight);
    }));
}

} // namespace

/// Streaming 2:1 decimator: zero-phase half-band FIR, output sample j is
/// centred on input sample 2j (no delay), input before the start is silence.
class HalfbandDecimator {
  public:
    HalfbandDecimator();
    /// Push input samples; appends the outputs that became complete.
    void push(const float* pIn, std::size_t count, std::vector<float>* pOut);
    /// Emit the remaining outputs (input after the end is silence).
    void flush(std::vector<float>* pOut);

  private:
    std::size_t inputCount() const {
        return m_base + m_x.size();
    }
    float compute(std::size_t j) const;
    void produce(std::vector<float>* pOut);
    std::vector<float> m_taps; // h[1], h[3], ... (odd taps; h[0] = 0.5)
    std::vector<float> m_x;    // input history
    std::vector<float> m_odd;  // scratch: odd input samples of a block
    std::size_t m_base = 0;    // absolute index of m_x[0]
    std::size_t m_next = 0;    // next output index
};

HalfbandDecimator::HalfbandDecimator() {
    // Kaiser-windowed sinc, cutoff fs/4.
    constexpr double beta = 8.0;
    auto bessel0 = [](double x) {
        double sum = 1, term = 1;
        for (int k = 1; k < 30; ++k) {
            term *= (x / (2 * k)) * (x / (2 * k));
            sum += term;
        }
        return sum;
    };
    double total = 0.5;
    for (int m = 1; m <= kHalfbandHalfLength; m += 2) {
        const double r = static_cast<double>(m) / (kHalfbandHalfLength + 1);
        const double w = bessel0(beta * std::sqrt(1 - r * r)) / bessel0(beta);
        const double h = std::sin(M_PI * m / 2) / (M_PI * m) * w;
        m_taps.push_back(static_cast<float>(h));
        total += 2 * h;
    }
    for (auto& t : m_taps) {
        t = static_cast<float>(t / total);
    }
    m_x.reserve(kHistoryTrim);
}

/// One output near the start or end of the input, where taps fall outside it.
float HalfbandDecimator::compute(std::size_t j) const {
    const std::ptrdiff_t c = static_cast<std::ptrdiff_t>(2 * j);
    const std::size_t nt = m_taps.size();
    auto at = [&](std::ptrdiff_t i) -> float {
        if (i < 0 || static_cast<std::size_t>(i) >= inputCount()) {
            return 0.0f;
        }
        return m_x[i - m_base];
    };
    float y = 0.5f * at(c);
    for (std::size_t t = 0; t < nt; ++t) {
        const std::ptrdiff_t m = static_cast<std::ptrdiff_t>(2 * t + 1);
        y += m_taps[t] * (at(c - m) + at(c + m));
    }
    return y;
}

void HalfbandDecimator::push(const float* pIn, std::size_t count, std::vector<float>* pOut) {
    m_x.insert(m_x.end(), pIn, pIn + count);
    produce(pOut);
}

void HalfbandDecimator::produce(std::vector<float>* pOut) {
    // outputs j < jEnd are complete
    const std::size_t count = inputCount();
    const std::size_t jEnd = count > kHalfbandHalfLength
            ? (count - kHalfbandHalfLength + 1) / 2
            : 0;
    // start-up outputs that need samples before the start of the input
    while (m_next < jEnd && 2 * m_next < m_base + kHalfbandHalfLength) {
        pOut->push_back(compute(m_next++));
    }
    if (m_next < jEnd) {
        // Block: y[j] = x[2j]/2 + sum_t h_t (x[2j-2t-1] + x[2j+2t+1]),
        // vectorised over j (same summation order as compute()).
        const std::size_t nOut = jEnd - m_next;
        const std::size_t nt = m_taps.size();
        const float* x = m_x.data(); // x[i - m_base] is input sample i
        // odd samples o[i] = x[2(i + oFirst) + 1] for every tap of the block
        const std::size_t oFirst = m_next - nt;
        m_odd.resize(nOut + 2 * nt - 1);
        for (std::size_t i = 0; i < m_odd.size(); ++i) {
            m_odd[i] = x[2 * (oFirst + i) + 1 - m_base];
        }
        const std::size_t outStart = pOut->size();
        pOut->resize(outStart + nOut);
        float* __restrict y = pOut->data() + outStart;
        for (std::size_t j = 0; j < nOut; ++j) {
            y[j] = 0.5f * x[2 * (m_next + j) - m_base];
        }
        const float* o = m_odd.data();
        for (std::size_t t = 0; t < nt; ++t) {
            const float h = m_taps[t];
            const float* lo = o + nt - t - 1; // x[2j-2t-1]
            const float* hi = o + nt + t;     // x[2j+2t+1]
            for (std::size_t j = 0; j < nOut; ++j) {
                y[j] += h * (lo[j] + hi[j]);
            }
        }
        m_next = jEnd;
    }
    // keep the history needed by the next output
    const std::ptrdiff_t keepFrom = static_cast<std::ptrdiff_t>(2 * m_next) - kHalfbandHalfLength;
    if (keepFrom > static_cast<std::ptrdiff_t>(m_base + kHistoryTrim)) {
        const std::size_t drop = keepFrom - m_base;
        m_x.erase(m_x.begin(), m_x.begin() + drop);
        m_base += drop;
    }
}

void HalfbandDecimator::flush(std::vector<float>* pOut) {
    while (2 * m_next < inputCount()) {
        pOut->push_back(compute(m_next++));
    }
}

void Tracker::PffftDeleter::operator()(PFFFT_Setup* p) const {
    pffft_destroy_setup(p);
}

void Tracker::PffftDeleter::operator()(float* p) const {
    pffft_aligned_free(p);
}

Tracker::Tracker(double inputSampleRate) {
    // Decimate 2:1 while the result stays at or above ~22 kHz: the analysis
    // only uses content below 11025 Hz, and the reference ran at 22050 Hz.
    double sampleRate = inputSampleRate;
    while (sampleRate / 2 >= kRefSampleRate * 0.95) {
        sampleRate /= 2;
        m_decimators.emplace_back();
    }
    // FFT size: power of two closest to the reference window duration.
    const double wantN = 1024.0 * sampleRate / kRefSampleRate;
    m_fftSize = 1 << static_cast<int>(std::lround(std::log2(wantN)));
    m_hop = std::max(1, static_cast<int>(std::lround(sampleRate / kRefFps)));
    m_fps = sampleRate / m_hop;
    // Band energies scale with N^2; normalise to the reference FFT size.
    m_energyScale = std::pow(1024.0 / m_fftSize, 2.0);
    m_window.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i) {
        // numpy.hanning (symmetric)
        m_window[i] = static_cast<float>(
                0.5 - 0.5 * std::cos(2.0 * M_PI * i / (m_fftSize - 1)));
    }
    // Bands are contiguous ranges of bins 1..N/2-1 (0 Hz and Nyquist excluded).
    const int half = m_fftSize / 2;
    m_bandBegin.fill(half);
    m_bandEnd.fill(half);
    for (int k = 1; k < half; ++k) {
        const double f = k * sampleRate / m_fftSize;
        for (int b = 0; b < kBandCount; ++b) {
            if (f >= kBandEdgesHz[b] && f < kBandEdgesHz[b + 1]) {
                if (m_bandBegin[b] == half) {
                    m_bandBegin[b] = k;
                }
                m_bandEnd[b] = k + 1;
            }
        }
    }
    // Real FFT of size N (PFFFT, SIMD). N is a power of two >= 512 for any rate
    // >= 8 kHz, which PFFFT supports; without a setup the grid stays invalid.
    m_pFftSetup.reset(pffft_new_setup(m_fftSize, PFFFT_REAL));
    const auto alloc = [this] {
        return std::unique_ptr<float, PffftDeleter>(static_cast<float*>(
                pffft_aligned_malloc(m_fftSize * sizeof(float))));
    };
    m_pFftIn = alloc();
    m_pFftOut = alloc();
    m_pFftWork = alloc();
    // Frames are centred: start with half a window of silence.
    m_pending.assign(m_fftSize / 2, 0.0f);
    m_pending.reserve(m_fftSize * 4);
}

Tracker::~Tracker() = default;

void Tracker::processStereo(const float* pInterleaved, std::size_t frameCount) {
    m_mono.resize(frameCount);
    for (std::size_t i = 0; i < frameCount; ++i) {
        m_mono[i] = 0.5f * (pInterleaved[2 * i] + pInterleaved[2 * i + 1]);
    }
    const float* p = m_mono.data();
    std::size_t n = frameCount;
    for (std::size_t st = 0; st < m_decimators.size(); ++st) {
        auto& out = m_stageOut[st % 2];
        out.clear();
        m_decimators[st].push(p, n, &out);
        p = out.data();
        n = out.size();
    }
    pushDecimated(p, n);
}

void Tracker::pushDecimated(const float* pIn, std::size_t count) {
    if (!m_pFftSetup) {
        return;
    }
    m_pending.insert(m_pending.end(), pIn, pIn + count);
    while (m_pending.size() - m_readPos >= static_cast<std::size_t>(m_fftSize)) {
        processFrame();
    }
}

void Tracker::processFrame() {
    const float* __restrict x = m_pending.data() + m_readPos;
    const float* __restrict win = m_window.data();
    float* __restrict in = m_pFftIn.get();
    for (int i = 0; i < m_fftSize; ++i) {
        in[i] = x[i] * win[i];
    }
    // Ordered real FFT: out[0] = DC, out[1] = Nyquist, then (re, im) per bin.
    pffft_transform_ordered(
            m_pFftSetup.get(), in, m_pFftOut.get(), m_pFftWork.get(), PFFFT_FORWARD);
    const float* __restrict z = m_pFftOut.get();
    std::array<float, kBandCount> logE;
    for (int b = 0; b < kBandCount; ++b) {
        double e = 0; // sum of |X[k]|^2 over the band
        for (int k = m_bandBegin[b]; k < m_bandEnd[b]; ++k) {
            e += z[2 * k] * z[2 * k] + z[2 * k + 1] * z[2 * k + 1];
        }
        logE[b] = static_cast<float>(std::log1p(1000.0 * e * m_energyScale));
    }
    float o = 0.0f;
    float onsetTiming = 0.0f;
    std::array<float, 3> g{};
    for (int b = 0; b < kBandCount; ++b) {
        const float d = m_logE.empty() ? 0.0f : std::max(0.0f, logE[b] - m_logE.back()[b]);
        o += d * kBandWeights[b];
        onsetTiming += d * kTimingBandWeights[b];
        g[b / 3] += d; // low/mid/high groups of 3 bands
    }
    m_logE.push_back(logE);
    m_odf.push_back(o);
    m_timingOdf.push_back(onsetTiming);
    for (int i = 0; i < 3; ++i) {
        m_groupOdf[i].push_back(g[i]);
    }
    m_readPos += m_hop;
    if (m_readPos >= static_cast<std::size_t>(m_fftSize) * 4) {
        m_pending.erase(m_pending.begin(), m_pending.begin() + m_readPos);
        m_readPos = 0;
    }
}

Grid Tracker::finalize() {
    // Drain the decimators, then flush with a full window of trailing
    // silence (matches the reference).
    std::vector<float> in, out;
    for (auto& d : m_decimators) {
        out.clear();
        d.push(in.data(), in.size(), &out);
        d.flush(&out);
        std::swap(in, out);
    }
    pushDecimated(in.data(), in.size());
    const std::vector<float> zeros(m_fftSize, 0.0f);
    pushDecimated(zeros.data(), zeros.size());

    Grid grid;
    const int whitenLen = static_cast<int>(m_fps * kWhitenSec);
    if (m_odf.size() < static_cast<std::size_t>(m_fps * kMinTrackSec) ||
            *std::max_element(m_odf.begin(), m_odf.end()) <= 0) { // too short or silent
        return grid;
    }
    std::array<std::vector<float>, 3> groups;
    for (int i = 0; i < 3; ++i) {
        groups[i] = whiten(m_groupOdf[i], whitenLen);
    }
    const std::vector<float> odf = whiten(m_odf, whitenLen);
    const Analysis analysis(odf, groups, m_fps);

    double b0 = analysis.coarseTempo();
    if (b0 <= 0) {
        return grid;
    }
    b0 = analysis.fineTempo(b0, 0.012);
    std::vector<CandidateFeatures> candidates;
    for (double m : kOctaveMultipliers) {
        const double b = b0 * m;
        if (b >= kMinBpm && b <= kMaxBpm) {
            candidates.push_back(analysis.features(b));
        }
    }
    const int best = argmax(static_cast<int>(candidates.size()),
            [&](int i) { return octaveScore(candidates[i], candidates); });
    if (best < 0) {
        return grid;
    }
    const double raw = analysis.fineTempo(candidates[best].bpm, 0.004);
    // rekordbox reports 0.01 BPM resolution; tempos within 0.01 of an integer
    // come out as that integer (measured on ~950 rekordbox grids).
    const double bpm = std::abs(raw - std::round(raw)) < kIntegerSnapBpm
            ? std::round(raw)
            : std::round(raw * 100.0) / 100.0;
    const double period = 60.0 / bpm;
    const double phi = analysis.phaseSec(bpm, whiten(m_timingOdf, whitenLen));
    const std::vector<PhaseFeatures> phaseCandidates = phaseFeatures(odf, m_logE, m_fps, bpm, phi);
    const int bestJ = std::max(0, argmax(4, [&](int j) {
        return linearScore(phaseCandidates[j].data(), kPhaseMean, kPhaseStd, kPhaseWeight);
    }));
    double t0 = std::fmod(phi + bestJ * period / 4 + kPhaseBiasMs / 1000.0, period);
    if (t0 < 0) {
        t0 += period;
    }
    grid.bpm = bpm;
    grid.firstBeatSec = t0;
    grid.barPhase = chooseBarPhase(m_logE, m_groupOdf, m_fps, t0, period);
    grid.downbeatSec = t0 + grid.barPhase * period;
    grid.valid = true;
    return grid;
}

} // namespace rbgrid
