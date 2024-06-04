#pragma once

#include <rubberband/RubberBandStretcher.h>

#include <QRunnable>
#include <QSemaphore>
#include <atomic>

#include "audio/types.h"

using RubberBand::RubberBandStretcher;

class RubberBandTask : public RubberBandStretcher, public QRunnable {
  public:
    RubberBandTask(size_t sampleRate,
            size_t channels,
            Options options = DefaultOptions);

    /// @brief Submit a new stretching task
    /// @param stretcher the RubberBand::RubberBandStretcher instance to use.
    /// Must remain valid till waitReady() as returned
    /// @param input The samples buffer
    /// @param samples the samples count
    /// @param final whether or not this is the final buffer
    void set(const float* const* input,
            size_t samples,
            bool isFinal);

    // Wait for the current task to complete.
    void waitReady();

    void run();

  private:
    // Whether or not the scheduled job as completed
    QSemaphore m_completedSema;

    const float* const* m_input;
    size_t m_samples;
    bool m_isFinal;
};
