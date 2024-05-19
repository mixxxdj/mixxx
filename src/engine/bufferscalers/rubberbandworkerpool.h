#pragma once

#include "util/singleton.h"

class RubberBandWorker;
namespace RubberBand {
class RubberBandStretcher;
}

// RubberBandWorkerPool is a global pool manager for RubberBandWorkerPool. It
// allows a the Engine thread to use a pool of agnostic RubberBandWorker which
// can be distributed stretching job
class RubberBandWorkerPool : public Singleton<RubberBandWorkerPool> {
  public:
    ~RubberBandWorkerPool();

    /// @brief Submit a new stretching job
    /// @param stretcher the RubberBand::RubberBandStretcher instance to use.
    /// Must remain valid till waitReady() as returned
    /// @param input The samples buffer
    /// @param samples the samples count
    /// @param final whether or not this is the final buffer
    /// @return the worker on which the job as been schedule one, or null if
    /// none available
    RubberBandWorker* submit(RubberBand::RubberBandStretcher* stretcher,
            const float* const* input,
            size_t samples,
            bool final);

  protected:
    RubberBandWorkerPool();

  private:
    std::vector<std::unique_ptr<RubberBandWorker>> m_workers;

    friend class Singleton<RubberBandWorkerPool>;
};
