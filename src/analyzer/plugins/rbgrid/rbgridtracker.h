#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

struct PFFFT_Setup;

namespace rbgrid {

constexpr int kBandCount = 9; // log-spaced bands, 20 Hz .. 11025 Hz

/// Result of a constant-tempo grid analysis.
struct Grid {
    double bpm = 0.0;          ///< rounded to 0.01 like rekordbox
    double firstBeatSec = 0.0; ///< first beat in [0, 60/bpm)
    /// First downbeat ("1" in rekordbox) at or after firstBeatSec, i.e.
    /// firstBeatSec + barPhase * 60/bpm. Use it as the grid anchor so that
    /// bars counted from the anchor match rekordbox.
    double downbeatSec = 0.0;
    int barPhase = 0; ///< beats from firstBeatSec to downbeatSec, 0..3
    bool valid = false;
};

class HalfbandDecimator;

/// Constant-tempo beat grid detector tuned to reproduce rekordbox's grids.
///
/// Pipeline: 2:1 half-band decimation down to ~22 kHz -> 9-band log-energy
/// spectral flux (~2.9 ms hop) -> autocorrelation tempo salience -> precise
/// tempo by maximising the sharpness of the onset function folded at the beat
/// period over the whole track -> tempo octave (1/3x..3x) chosen by a linear
/// model trained on rekordbox grids -> phase from the folded onset histogram,
/// one of four quarter-beat offsets chosen by a linear model, downbeat by a linear model
/// over bar features and per-beat boosted trees. BPM is rounded to 0.01, or to
/// an integer within 0.01.
class Tracker {
  public:
    explicit Tracker(double inputSampleRate);
    ~Tracker();

    /// Interleaved stereo samples, frameCount frames.
    void processStereo(const float* pInterleaved, std::size_t frameCount);

    Grid finalize();

  private:
    void pushDecimated(const float* pIn, std::size_t count);
    void processFrame();

    std::vector<HalfbandDecimator> m_decimators; // 2:1 stages down to ~22 kHz
    std::vector<float> m_mono;
    std::array<std::vector<float>, 2> m_stageOut; // decimator stage outputs

    int m_fftSize;
    int m_hop;
    double m_fps;
    double m_energyScale;
    std::vector<float> m_window;
    std::vector<float> m_pending; // decimated mono samples not yet consumed
    std::size_t m_readPos = 0;    // start of the next frame in m_pending
    struct PffftDeleter {
        void operator()(PFFFT_Setup* p) const;
        void operator()(float* p) const;
    };
    std::unique_ptr<PFFFT_Setup, PffftDeleter> m_pFftSetup;
    // SIMD-aligned FFT buffers
    std::unique_ptr<float, PffftDeleter> m_pFftIn;
    std::unique_ptr<float, PffftDeleter> m_pFftOut;
    std::unique_ptr<float, PffftDeleter> m_pFftWork;
    // FFT bin range per band
    std::array<int, kBandCount> m_bandBegin{};
    std::array<int, kBandCount> m_bandEnd{};
    // Onset detection functions, one value per hop.
    std::vector<float> m_odf;
    std::vector<float> m_timingOdf;
    std::array<std::vector<float>, 3> m_groupOdf;      // low/mid/high groups of 3 bands
    std::vector<std::array<float, kBandCount>> m_logE; // log band energies
};

} // namespace rbgrid
