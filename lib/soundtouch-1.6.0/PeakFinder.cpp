////////////////////////////////////////////////////////////////////////////////
///
/// Peak detection routine. 
///
/// The routine detects highest value on an array of values and calculates the 
/// precise peak location as a mass-center of the 'hump' around the peak value.
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
/// SoundTouch WWW: http://www.surina.net/soundtouch
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2010-08-24 18:53:56 +0300 (Tue, 24 Aug 2010) $
// File revision : $Revision: 4 $
//
// $Id: PeakFinder.cpp 91 2010-08-24 15:53:56Z oparviai $
//
////////////////////////////////////////////////////////////////////////////////
//
// License :
//
//  SoundTouch audio processing library
//  Copyright (c) Olli Parviainen
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <assert.h>

#include "PeakFinder.h"

using namespace soundtouch;

#define max(x, y) (((x) > (y)) ? (x) : (y))


PeakFinder::PeakFinder()
{
    minPos = maxPos = 0;
}


// Finds 'ground level' of a peak hump by starting from 'peakpos' and proceeding
// to direction defined by 'direction' until next 'hump' after minimum value will 
// begin
int PeakFinder::findGround(const float *data, int peakpos, int direction) const
{
    float refvalue;
    int lowpos;
    int pos;
    int climb_count;
    float delta;

    climb_count = 0;
    refvalue = data[peakpos];
    lowpos = peakpos;

    pos = peakpos;

    while ((pos > minPos) && (pos < maxPos))
    {
        int prevpos;

        prevpos = pos;
        pos += direction;

        // calculate derivate
        delta = data[pos] - data[prevpos];
        if (delta <= 0)
        {
            // going downhill, ok
            if (climb_count)
            {
                climb_count --;  // decrease climb count
            }

            // check if new minimum found
            if (data[pos] < refvalue)
            {
                // new minimum found
                lowpos = pos;
                refvalue = data[pos];
            }
        }
        else
        {
            // going uphill, increase climbing counter
            climb_count ++;
            if (climb_count > 5) break;    // we've been climbing too long => it's next uphill => quit
        }
    }
    return lowpos;
}


// Find offset where the value crosses the given level, when starting from 'peakpos' and
// proceeds to direction defined in 'direction'
int PeakFinder::findCrossingLevel(const float *data, float level, int peakpos, int direction) const
{
    float peaklevel;
    int pos;

    peaklevel = data[peakpos];
    assert(peaklevel >= level);
    pos = peakpos;
    while ((pos >= minPos) && (pos < maxPos))
    {
        if (data[pos + direction] < level) return pos;   // crossing found
        pos += direction;
    }
    return -1;  // not found
}


// Calculates the center of mass location of 'data' array items between 'firstPos' and 'lastPos'
double PeakFinder::calcMassCenter(const float *data, int firstPos, int lastPos) const
{
    int i;
    float sum;
    float wsum;

    sum = 0;
    wsum = 0;
    for (i = firstPos; i <= lastPos; i ++)
    {
        sum += (float)i * data[i];
        wsum += data[i];
    }

    if (wsum < 1e-6) return 0;
    return sum / wsum;
}



/// get exact center of peak near given position by calculating local mass of center
double PeakFinder::getPeakCenter(const float *data, int peakpos) const
{
    float peakLevel;            // peak level
    int crosspos1, crosspos2;   // position where the peak 'hump' crosses cutting level
    float cutLevel;             // cutting value
    float groundLevel;          // ground level of the peak
    int gp1, gp2;               // bottom positions of the peak 'hump'

    // find ground positions.
    gp1 = findGround(data, peakpos, -1);
    gp2 = findGround(data, peakpos, 1);

    groundLevel = max(data[gp1], data[gp2]);
    peakLevel = data[peakpos];

    if (groundLevel < 1e-9) return 0;                // ground level too small => detection failed
    if ((peakLevel / groundLevel) < 1.3) return 0;   // peak less than 30% of the ground level => no good peak detected

    // calculate 70%-level of the peak
    cutLevel = 0.70f * peakLevel + 0.30f * groundLevel;
    // find mid-level crossings
    crosspos1 = findCrossingLevel(data, cutLevel, peakpos, -1);
    crosspos2 = findCrossingLevel(data, cutLevel, peakpos, 1);

    if ((crosspos1 < 0) || (crosspos2 < 0)) return 0;   // no crossing, no peak..

    // calculate mass center of the peak surroundings
    return calcMassCenter(data, crosspos1, crosspos2);
}



double PeakFinder::detectPeak(const float *data, int aminPos, int amaxPos) 
{

    int i;
    int peakpos;                // position of peak level
    double highPeak, peak;

    this->minPos = aminPos;
    this->maxPos = amaxPos;

    // find absolute peak
    peakpos = minPos;
    peak = data[minPos];
    for (i = minPos + 1; i < maxPos; i ++)
    {
        if (data[i] > peak) 
        {
            peak = data[i];
            peakpos = i;
        }
    }
    
    // Calculate exact location of the highest peak mass center
    highPeak = getPeakCenter(data, peakpos);
    peak = highPeak;

    // Now check if the highest peak were in fact harmonic of the true base beat peak 
    // - sometimes the highest peak can be Nth harmonic of the true base peak yet 
    // just a slightly higher than the true base
    for (i = 2; i < 10; i ++)
    {
        double peaktmp, tmp;
        int i1,i2;

        peakpos = (int)(highPeak / (double)i + 0.5f);
        if (peakpos < minPos) break;

        // calculate mass-center of possible base peak
        peaktmp = getPeakCenter(data, peakpos);

        // now compare to highest detected peak
        i1 = (int)(highPeak + 0.5);
        i2 = (int)(peaktmp + 0.5);
        tmp = 2 * (data[i2] - data[i1]) / (data[i2] + data[i1]);
        if (fabs(tmp) < 0.1)
        {
            // The highest peak is harmonic of almost as high base peak,
            // thus use the base peak instead
            peak = peaktmp;
        }
    }

    return peak;
}


