#pragma once

#include <rubberband/RubberBandStretcher.h>

#include <QMutex>
#include <QRunnable>
#include <QWaitCondition>
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
    // Used to schedule the thread
    QMutex m_waitLock;
    QWaitCondition m_waitCondition;
    // Whether or not the scheduled job as completed
    bool m_completed;

    const float* const* m_input;
    size_t m_samples;
    bool m_isFinal;
};
