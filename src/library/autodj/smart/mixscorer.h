#pragma once

#include <QString>

#include "library/autodj/smart/trackfeatures.h"

/// Tunable weights for MixScorer. All values are starting points to be
/// tuned by ear; they are not measured optimums.
struct MixScoreWeights {
    enum class EnergyDirection {
        Build, ///< energy should rise gently (short sets, warm-ups)
        Hold,  ///< energy should stay level
        Wave,  ///< rises and dips are fine (long sets)
    };

    double key = 1.0;
    double tempo = 1.0;
    double energy = 0.6;
    /// How much a *measured* energy counts compared with a hand-rated one.
    /// Low because measured energy did not match the DJ's ears in testing.
    double measuredEnergyTrust = 0.25;
    /// Set shape (Build only): cost per energy point of opening with a
    /// high-energy track, or closing with a low-energy one.
    double setShape = 0.5;
    double bpmTolerancePct = 5.0;
    bool allowHalfDoubleTime = true;
    bool energyBoostExcusesKeyJump = true;
    EnergyDirection direction = EnergyDirection::Build;
};

/// The cost of playing one track after another. Lower is better.
/// The parts are kept so the GUI can explain *why* a track was chosen.
struct MixScore {
    double total = 0.0;
    double keyCost = 0.0;
    double tempoCost = 0.0;
    double energyCost = 0.0;
    QString reason;
};

class MixScorer {
  public:
    // Costs at or above these count as a "clash" in reports.
    static constexpr double kClashKeyCost = 3.0;
    static constexpr double kClashTempoCost = 5.0;

    explicit MixScorer(MixScoreWeights weights = MixScoreWeights());

    MixScore score(const TrackFeatures& from, const TrackFeatures& to) const;

    const MixScoreWeights& weights() const {
        return m_weights;
    }

    /// Harmonic cost on the Camelot wheel. 0 = same key.
    static double camelotCost(const TrackFeatures& from,
            const TrackFeatures& to,
            double energyDelta,
            bool boostExcusesKeyJump);

    /// Tempo cost. 0..1 inside the tolerance; beyond it a clash
    /// (kClashTempoCost and up). Uses the log BPM ratio, so it is symmetric.
    static double tempoCost(double bpmFrom,
            double bpmTo,
            double tolerancePct,
            bool allowHalfDoubleTime);

    /// Energy-flow cost, depends on the chosen EnergyDirection.
    double energyCost(double energyFrom, double energyTo) const;

    /// Set-shape cost of opening the set with this track (Build: calm first).
    double startCost(const TrackFeatures& track) const;
    /// Set-shape cost of closing the set with this track (Build: peak last).
    double endCost(const TrackFeatures& track) const;

    /// One line of a running order, e.g. " 3. 8A  124.0 BPM  energy 7 (rated)  Artist - Title".
    static QString trackLine(int position, const TrackFeatures& track);
    /// The same line without the position number.
    static QString trackText(const TrackFeatures& track);

    /// "key clash", "tempo clash", "key + tempo clash", or empty if smooth.
    static QString clashLabel(const MixScore& score);

    /// Key morph: the key a track plays in when pitched by `semitones`
    /// (key lock on, tempo unchanged). One semitone up = 7 steps round the
    /// Camelot wheel (8A -> 3A), the letter stays.
    static TrackFeatures shiftKey(const TrackFeatures& track, int semitones);

    /// Key morph: how many semitones (-maxShift..+maxShift) to pitch the
    /// incoming track so its key fits the outgoing one (no key clash).
    /// 0 = leave it alone: the keys already fit, a key is unknown, or no
    /// shift that small makes them fit. The smallest shift wins; with a tie,
    /// the better fit, then down (a lower voice sounds more natural than a
    /// higher one).
    static int keyMorphSemitones(const TrackFeatures& from, const TrackFeatures& to, int maxShift);

    /// Shortest distance round the Camelot wheel: 0..6.
    static int camelotDistance(int from, int to);

    /// Signed clockwise step from `from` to `to`: -5..+6 (+7 shows as -5).
    static int camelotStep(int from, int to);

  private:
    MixScoreWeights m_weights;
};
