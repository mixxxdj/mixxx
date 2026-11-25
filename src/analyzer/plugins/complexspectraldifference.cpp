#include "analyzer/plugins/complexspectraldifference.h"

#include <fftw3.h>

#include <algorithm>
#include <cmath>
#include <mutex>
#include <numbers>

namespace mixxx {

namespace {

// Creating and destroying FFTW plans is not thread-safe, and analyzers run in
// several threads. Executing a plan is.
std::mutex s_fftwPlannerMutex;

} // namespace

ComplexSpectralDifference::ComplexSpectralDifference(int windowSize)
        : m_windowSize(windowSize),
          m_pInput(fftwf_alloc_real(windowSize)),
          // std::complex<float> has the layout of fftwf_complex.
          m_pSpectrum(reinterpret_cast<std::complex<float>*>(
                  fftwf_alloc_complex(windowSize / 2 + 1))),
          m_window(windowSize),
          // The history starts with zero magnitudes and phases, as in qm-dsp.
          m_previousMagnitude(windowSize / 2 + 1, 0.0),
          m_previousPhasor(windowSize / 2 + 1, 1.0),
          m_olderPhasor(windowSize / 2 + 1, 1.0) {
    {
        const std::lock_guard lock(s_fftwPlannerMutex);
        m_pPlan.reset(fftwf_plan_dft_r2c_1d(windowSize,
                m_pInput.get(),
                reinterpret_cast<fftwf_complex*>(m_pSpectrum.get()),
                FFTW_ESTIMATE));
    }
    for (int i = 0; i < windowSize; ++i) {
        m_window[i] = static_cast<float>(
                0.5 - 0.5 * std::cos(2 * std::numbers::pi * i / windowSize));
    }
}

ComplexSpectralDifference::~ComplexSpectralDifference() = default;

void ComplexSpectralDifference::FftwDeleter::operator()(fftwf_plan_s* pPlan) const {
    const std::lock_guard lock(s_fftwPlannerMutex);
    fftwf_destroy_plan(pPlan);
}

void ComplexSpectralDifference::FftwDeleter::operator()(void* pBuffer) const {
    fftwf_free(pBuffer);
}

double ComplexSpectralDifference::process(const double* pWindow) {
    float* pInput = m_pInput.get();
    for (int i = 0; i < m_windowSize; ++i) {
        pInput[i] = static_cast<float>(pWindow[i]) * m_window[i];
    }
    fftwf_execute(m_pPlan.get());
    const std::complex<float>* pSpectrum = m_pSpectrum.get();
    double sum = 0;
    for (int bin = 0; bin <= m_windowSize / 2; ++bin) {
        sum += processBin(bin, pSpectrum[bin]);
    }
    return sum;
}

/// Returns |M1 - M e^(j (phi - 2 phi1 + phi2))| of one bin and moves its
/// history on.
double ComplexSpectralDifference::processBin(int bin, std::complex<double> value) {
    const double magnitude = std::sqrt(std::norm(value));
    // A zero bin has phase zero (atan2(0, 0)). qm-dsp's FFT shift by half a
    // window negates the odd bins.
    std::complex<double> phasor = 1.0;
    if (magnitude > 0) {
        phasor = value * (((bin & 1) ? -1.0 : 1.0) / magnitude);
    }
    const std::complex<double> previous = m_previousPhasor[bin];
    const double cosDeviation =
            (phasor * std::conj(previous * previous) * m_olderPhasor[bin]).real();
    const double previousMagnitude = m_previousMagnitude[bin];
    m_olderPhasor[bin] = previous;
    m_previousPhasor[bin] = phasor;
    m_previousMagnitude[bin] = magnitude;
    // Law of cosines; rounding can make the square slightly negative.
    return std::sqrt(std::max(0.0,
            previousMagnitude * previousMagnitude + magnitude * magnitude -
                    2 * previousMagnitude * magnitude * cosDeviation));
}

} // namespace mixxx
