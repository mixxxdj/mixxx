#pragma once

#include <rubberband/RubberBandStretcher.h>

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <atomic>

#include "audio/types.h"

class RubberBandWorkerPool;
class RubberBandWorker : public QThread {
    Q_OBJECT
  public:
    // Wait for the current job to complete.
    void waitReady();

  protected:
    RubberBandWorker();

    void run() override;
    void schedule(RubberBand::RubberBandStretcher* stretcher,
            const float* const* input,
            size_t samples,
            bool final);
    void stop();

  private:
    struct Job {
        RubberBand::RubberBandStretcher* instance;
        const float* const* input;
        size_t samples;
        bool final;
    };
    /// Contains the scheduled job. May be dangling if completed=true
    Job m_currentJob;
    // Used to schedule the thread
    QMutex m_waitLock;
    QWaitCondition m_waitCondition;
    // Whether or not the scheduled job as completed
    bool m_completed;

    // Used by RubberBandWorkerPool to manage thread availability.
    // RubberBandWorker only clears the flag, once the job is completed
    std::atomic_flag m_assigned;

    friend RubberBandWorkerPool;
};
