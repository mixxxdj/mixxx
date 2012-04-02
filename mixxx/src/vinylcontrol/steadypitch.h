#ifndef __STEADYPITCH_H__
#define __STEADYPITCH_H__

#include "vinylcontrol.h"
#include <time.h>
#include <QTime>


class SteadyPitch
{
    public:
        SteadyPitch(double threshold);
        void reset(double pitch, double time);
        double check(double pitch, double time, bool looping);
        double steadyValue(void);
        bool directionChanged(double pitch);
        bool resyncDetected(double new_time);
    private:
        double m_dSteadyPitch;
        double m_dOldSteadyPitch;
        double m_dSteadyPitchTime;
        double m_dLastSteadyDur;
        double m_dLastTime;
        double m_dPitchThreshold;
        int m_iPlayDirection;
};

#endif
