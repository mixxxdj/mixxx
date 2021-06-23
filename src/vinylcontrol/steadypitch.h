#pragma once

#include <QTime>

#include "vinylcontrol.h"

class SteadyPitch {
    public:
        SteadyPitch(double threshold, bool assumeSteady);
        void reset(double pitch, double time);
        double check(double pitch, double time);
        double steadyValue(void) const;
        bool directionChanged(double pitch);
        bool resyncDetected(double new_time);
    private:
        const bool m_bAssumeSteady;
        double m_dSteadyPitch;
        double m_dOldSteadyPitch;
        double m_dSteadyPitchTime;
        double m_dLastSteadyDur;
        double m_dLastTime;
        double m_dPitchThreshold;
        int m_iPlayDirection;
};
