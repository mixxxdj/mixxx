/***************************************************************************
                          steadypitch.cpp
                             -------------------
    begin                : Halloween, 2011
    copyright            : (C) 2011 Owen Williams
    email                : owilliams@mixxx.org
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>
#include <limits.h>
#include "vinylcontrolxwax.h"
#include "steadypitch.h"
#include <math.h>

SteadyPitch::SteadyPitch(double threshold)
{
    m_dPitchThreshold = threshold;
    m_dOldSteadyPitch = 1.0f;
    m_dSteadyPitchTime =  0.0f;
}


void SteadyPitch::reset(double pitch, double time)
{
    m_dSteadyPitch = pitch;
    m_dSteadyPitchTime = time;
}

double SteadyPitch::check(double pitch, double time, bool looping=false)
{
    //return length of time pitch has been steady, 0 if not steady

    if (time < m_dSteadyPitchTime) //bad values, often happens during resync
    {
        if (looping)
        {
            //if looping, fake it since we don't know where the loop
            //actually is
            if (fabs(pitch - m_dSteadyPitch) < m_dPitchThreshold)
            {
                m_dSteadyPitchTime = time - 2.0;
                return 2.0;
            }
        }
        else
        {
            reset(pitch, time);
            return 0.0;
        }
    }

    if (fabs(pitch - m_dSteadyPitch) < m_dPitchThreshold)
    {
        if (time - m_dSteadyPitchTime > 2.0)
        {
            m_dSteadyPitch = pitch;
            m_dOldSteadyPitch = m_dSteadyPitch; //this was a known-good value
            m_dSteadyPitchTime += 1.0;
        }
        return time - m_dSteadyPitchTime;
    }

    //else
    reset(pitch, time);
    return 0.0;
}

double SteadyPitch::steadyValue(void)
{
    return m_dOldSteadyPitch;
}
