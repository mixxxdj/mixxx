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
    m_dPitchThreshold = threshold; //variation above which we say we aren't steady
    m_dOldSteadyPitch = 1.0f;  //last-known steady pitch value
    m_dSteadyPitchTime = 0.0f; //last track location we had a steady pitch
    m_dLastSteadyDur = 0.0f;   //last known duration of steadiness
    m_dLastTime = 0.0f;        //track location of previous call
    m_iPlayDirection = 1;      //1=forward, -1=backward
}


void SteadyPitch::reset(double pitch, double time)
{
    m_dSteadyPitch = pitch;
    m_dSteadyPitchTime = time;
    m_dLastTime = time;
    if (m_dSteadyPitch >= 0) {
        m_iPlayDirection = 1;
    } else {
        m_iPlayDirection = -1;
    }
    m_dLastSteadyDur = 0.0;
}

bool SteadyPitch::directionChanged(double pitch)
{
    if (pitch >= 0) {
        return m_iPlayDirection != 1;
    } else {
        return m_iPlayDirection != -1;
    }
}

bool SteadyPitch::resyncDetected(double new_time)
{
    //did track location jump opposite to the play direction?
    if (m_iPlayDirection >= 0)
    {
        return new_time < m_dLastTime;
    }
    else
    {
        return new_time > m_dLastTime;
    }
}

double SteadyPitch::check(double pitch, double time, bool looping=false)
{
    //return length of time pitch has been steady, 0 if not steady
    if (directionChanged(pitch))
    {
        //we've reversed direction, reset
        reset(pitch, time);
        return 0.0;
    }
    
    if (resyncDetected(time))
    {
        m_dLastTime = time;
        if (looping)
        {
            //if looping, rereport the last value since we don't know where the 
            //loop actually is
            if (fabs(pitch - m_dSteadyPitch) < m_dPitchThreshold)
            {
                //fabricate an old time by take current time and applying
                //last known duration
                m_dSteadyPitchTime = time - (m_dLastSteadyDur * m_iPlayDirection);
                return m_dLastSteadyDur;
            }
        }
        reset(pitch, time);
        return 0.0;
    }
    m_dLastTime = time;

    if (fabs(pitch - m_dSteadyPitch) < m_dPitchThreshold)
    {
        if (fabs(time - m_dSteadyPitchTime) > 2.0) //fabs for both directions
        {
            m_dSteadyPitch = pitch;
            m_dOldSteadyPitch = m_dSteadyPitch; //this was a known-good value
            //update steady pitch time so it's between 1 and 2 seconds ago
            //(or ahead, if moving backward)
            m_dSteadyPitchTime += 1.0 * m_iPlayDirection;
        }
        m_dLastSteadyDur = fabs(time - m_dSteadyPitchTime);
        return m_dLastSteadyDur;
    }

    //else
    reset(pitch, time);
    return 0.0;
}

double SteadyPitch::steadyValue(void)
{
    return m_dOldSteadyPitch;
}
