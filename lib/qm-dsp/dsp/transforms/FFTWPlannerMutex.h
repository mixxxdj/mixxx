/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef QM_DSP_FFTW_PLANNER_MUTEX_H
#define QM_DSP_FFTW_PLANNER_MUTEX_H

#include <mutex>

// Creating and destroying FFTW plans is not thread-safe, and analyzers run in
// several threads. Executing a plan is. Every user of the FFTW planner locks
// this one mutex.
inline std::mutex& fftwPlannerMutex()
{
    static std::mutex mutex;
    return mutex;
}

#endif
