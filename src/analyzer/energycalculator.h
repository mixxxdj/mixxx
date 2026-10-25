#pragma once

#include <cstdint>
#include <vector>

#include "library/autodj/smart/phrasealign.h"

/// Auto DJ 2.0 plus Video Mixing: computes a 1..10 "energy" score for a whole track.
///
/// Pure C++ with no Mixxx or Qt dependencies, so it can be unit-tested with
/// synthetic audio. AnalyzerEnergy feeds it the decoded samples.
///
/// Three ingredients, each scaled to 0..1:
///  - loudness:   how loud the loud parts are (90th percentile of 1-second
///                RMS levels, before ReplayGain)
///  - brightness: share of energy above ~2.5 kHz (hats, synths, noise)
///  - busyness:   onsets per second (how often a new hit starts)
/// energy = 1 + 9 * (0.45 * loudness + 0.30 * brightness + 0.25 * busyness)
///
/// The weights and ranges are a starting point, to be tuned by ear.
class EnergyCalculator {
  public:
    /// Bump when the formula changes, so tracks get re-analysed.
    /// v2: adds bodyStartSec / bodyEndSec.
    /// v3: body end uses the stricter kBodyEndDropDb.
    /// v4: same numbers; re-run so the analyzer sets the Intro End and
    ///     Outro Start markers on tracks analysed before that existed.
    /// v5: body start uses the bass band too (finds loud intros without
    ///     drums).
    static constexpr int kVersion = 5;

    struct Result {
        double energy = 0.0;       ///< 1..10
        double loudness01 = 0.0;   ///< 0..1 part of the score
        double brightness01 = 0.0; ///< 0..1 part of the score
        double busyness01 = 0.0;   ///< 0..1 part of the score
        double loudnessDb = 0.0;   ///< raw: p90 of 1 s RMS, dBFS
        double brightRatio = 0.0;  ///< raw: high-band / full-band energy
        double onsetsPerSec = 0.0; ///< raw: onsets per second of sound
        double bassRatio = 0.0;    ///< raw: low-band / full-band energy (kept for tuning)
        /// The "body" of the track: from where it first gets going (the beat
        /// kicks in after a quiet intro) to where it starts to fade out.
        /// Seconds from the start of the track. Found by loudness: the body
        /// starts where the level first gets within kBodyStartDropDb of the
        /// track's p90, and ends where it last stays within kBodyEndDropDb.
        double bodyStartSec = 0.0;
        double bodyEndSec = 0.0;
    };

    /// How far below the track's usual loud level still counts as "body".
    /// Start: generous, so a slightly quieter first verse still counts.
    /// End: strict, so a mix is over before the track's own fade-out has
    /// taken the energy away (the dancers must never lose the beat).
    static constexpr double kBodyStartDropDb = 6.0;
    static constexpr double kBodyEndDropDb = 3.0;

    EnergyCalculator(double sampleRate, int channelCount);

    /// Feeds interleaved samples. `sampleCount` counts samples, not frames.
    void process(const float* pInterleaved, std::int64_t sampleCount);

    /// Returns false if there was too little non-silent audio (< 5 s).
    bool finish(Result* pResult) const;

    /// Beat grid check: does the beat grid (steady, or a beat map that
    /// bends with the music; seconds of the track) stay on the music from
    /// the start of the body to its end?
    ///
    /// For the bass hits and for the treble hits (hats, snares) separately,
    /// it finds where in the beat the hits fall (the "phase") in 16-beat
    /// windows at the start, middle and end of the body. With a good grid
    /// that place stays the same all through the track; with a wrong tempo
    /// or a drummer who drifts it wanders. Where the hits fall does not
    /// matter (an off-beat bass line is fine), only that it stays put.
    ///
    /// Returns how far it wanders, in beats (0 = rock steady, 0.5 = half a
    /// beat off somewhere), using whichever band shows the beat most
    /// clearly. -1 = cannot tell (body too short or no clear beat), which
    /// counts as OK. Call after process().
    double gridDriftBeats(const phrasealign::Grid& grid,
            double bodyStartSec,
            double bodyEndSec) const;

    /// More drift than this and Auto DJ does not beatmatch the track (it
    /// fades plainly instead). 0.15 beats = 75 ms at 120 BPM: a flam you
    /// can hear. On 5 real tracks: good grids 0.04..0.13, a grid 0.05% off
    /// 0.15..0.33, bigger errors 0.2..0.5.
    static constexpr double kGridMaxDriftBeats = 0.15;
    /// Bump when gridDriftBeats() changes, so every track is checked again.
    /// v2: only the first tick of a double kick counted. Worse on the
    ///     DJ's library (89 -> 99 flagged, steady tracks failed), so
    /// v3: back to counting every hit.
    /// v5: in each region the one window that does not fit (a breakdown,
    ///     an intro before the bass) is left out; the other three must
    ///     agree. One odd window flagged songs whose grid was fine.
    static constexpr int kGridCheckVersion = 5;
    /// The same check made on a beat map (a tempo that bends, followed beat
    /// by beat). Its own number, so a track that gets a beat map is checked
    /// again even if its BPM and first beat stay the same.
    static constexpr int kGridCheckMapVersion = 6;
    /// Results of the previous versions (steady grid, beat map) still count
    /// until the track is analysed again, so no track that was flagged is
    /// suddenly beatmatched without being checked.
    static constexpr int kGridCheckOldVersion = 3;
    static constexpr int kGridCheckOldMapVersion = 4;

    /// Maps the three 0..1 parts to the 1..10 score.
    static double combine(double loudness01, double brightness01, double busyness01);

  private:
    struct OnePole {
        double a = 0.0;
        double y = 0.0;
        double step(double x) {
            y += a * (x - y);
            return y;
        }
    };

    void endBlock();

    const double m_sampleRate;
    const int m_channelCount;
    const int m_blockFrames; // 20 ms

    // Two cascaded one-pole low-pass filters per band (12 dB/octave).
    OnePole m_low1, m_low2;   // < 150 Hz
    OnePole m_high1, m_high2; // subtracted to get > 2.5 kHz

    // Current block accumulators.
    int m_framesInBlock = 0;
    double m_blockSumFull = 0.0;
    double m_blockSumLow = 0.0;
    double m_blockSumHigh = 0.0;

    // Per-block results.
    std::vector<float> m_blockDb;    // full-band level of each 20 ms block
    std::vector<float> m_blockLowDb; // bass-band level of each 20 ms block
    std::vector<float> m_blockHighDb; // treble-band level of each 20 ms block

    // Totals over non-silent blocks.
    double m_totalFull = 0.0;
    double m_totalLow = 0.0;
    double m_totalHigh = 0.0;
};
