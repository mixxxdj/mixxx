#pragma once

#include <complex>
#include <memory>
#include <vector>

struct fftwf_plan_s;

namespace mixxx {

/// The complex domain onset detection function (Duxbury et al., DAFx 2003;
/// Bello et al., IEEE SPL 2004) as computed by qm-dsp's DetectionFunction with
/// DF_COMPLEXSD (no adaptive whitening), using single precision FFTW and no
/// trigonometric functions. Per bin it sums |M1 - M e^(j dev)| over the
/// current spectrum (M, phi) and the two before it (1, 2), where
/// dev = phi - 2 phi1 + phi2:
/// - The phasor of dev is the product of the unit phasors p * conj(p1)^2 * p2,
///   since multiplying unit phasors adds their angles
///   (https://en.wikipedia.org/wiki/Euler%27s_formula).
/// - |M1 - M e^(j dev)| = sqrt(M1^2 + M^2 - 2 M1 M cos(dev)) by the law of
///   cosines (https://en.wikipedia.org/wiki/Law_of_cosines), and cos(dev) is
///   the real part of that phasor.
/// Like qm-dsp, the window is Hann windowed and FFT shifted (odd bins
/// negated), and an all zero bin has phase zero.
class ComplexSpectralDifference {
  public:
    explicit ComplexSpectralDifference(int windowSize);
    ~ComplexSpectralDifference();

    /// Returns the detection function value of the next windowSize samples.
    double process(const double* pWindow);

  private:
    struct FftwDeleter {
        void operator()(fftwf_plan_s* pPlan) const;
        void operator()(void* pBuffer) const;
    };

    double processBin(int bin, std::complex<double> value);

    const int m_windowSize;
    std::unique_ptr<float, FftwDeleter> m_pInput;
    std::unique_ptr<std::complex<float>, FftwDeleter> m_pSpectrum;
    std::unique_ptr<fftwf_plan_s, FftwDeleter> m_pPlan;
    std::vector<float> m_window;
    /// Per bin: the magnitude of the previous spectrum, and the unit phasors
    /// of the previous spectrum and the one before.
    std::vector<double> m_previousMagnitude;
    std::vector<std::complex<double>> m_previousPhasor;
    std::vector<std::complex<double>> m_olderPhasor;
};

} // namespace mixxx
