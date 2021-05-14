#pragma once

#include <vector>
#include <functional>

#include "util/types.h"

namespace mixxx {

// This is used for downmixing a stereo buffer into mono and framing it into
// overlapping windows as is typically necessary when taking a short-time
// Fourier transform.
class DownmixAndOverlapHelper {
  public:
    DownmixAndOverlapHelper() = default;

    typedef std::function<bool(double* pBuffer, size_t frames)> WindowReadyCallback;

    bool initialize(
            size_t windowSize,
            size_t stepSize,
            const WindowReadyCallback& callback);

    bool processStereoSamples(
            const CSAMPLE* pInput,
            size_t inputStereoSamples);

    bool finalize();

  private:
    bool processInner(const CSAMPLE* pInput, size_t numInputFrames);

    std::vector<double> m_buffer;
    // The window size in frames.
    size_t m_windowSize = 0;
    // The number of frames to step the window forward on each output.
    size_t m_stepSize = 0;
    size_t m_bufferWritePosition = 0;
    WindowReadyCallback m_callback;
};

} // namespace mixxx
