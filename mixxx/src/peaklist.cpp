/***************************************************************************
                          peaklist.cpp  -  description
                             -------------------
    begin                : Wed Jul 9 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/*******
********************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "peaklist.h"

PeakList::PeakList(int iIdxSize, float *pBuffer) : QValueList<PeakType>()
{
    m_iIdxSize = iIdxSize;
    m_pBuffer = pBuffer;
}

PeakList::~PeakList()
{
}

void PeakList::update(int idx, int len)
{
    iterator it, itStart;

    //
    // Delete peaks in range
    it = getFirstInRange(idx, len);
    while (it!=end() && (*it).i<=idx+len && (*it).i>=idx)
        it = remove(it);
    // Check if buffer is wrapped
    if (idx+len>m_iIdxSize)
    {
        it = begin();
        while (it!=end() && (*it).i<=(idx+len)%m_iIdxSize)
            it = remove(it);
    }

    //
    // Add new peaks to the peak list
    if (idx+len<m_iIdxSize)
    {
        // The buffer is not wrapped
        for (int i=idx+len; i>=idx; --i)
            it = insertIfPeak(i, it);
    }
    else
    {
        // The buffer is wrapped
        int i;
		for (i=(idx+len)%m_iIdxSize; i>=0; --i)
            it = insertIfPeak(i, it);
        it = end();
        for (i=m_iIdxSize; i>=idx; --i)
            it = insertIfPeak(i, it);
    }
}

PeakList::iterator PeakList::insertIfPeak(int idx, PeakList::iterator it)
{
    if (m_pBuffer[idx]>m_pBuffer[(idx-1+m_iIdxSize)%m_iIdxSize] && m_pBuffer[idx]>m_pBuffer[(idx+1)%m_iIdxSize])
    {        
        PeakType p;
        p.i = idx;

        // Perform second order interpolation of current hfc peak
        float t  = m_pBuffer[idx];
        float t1 = m_pBuffer[(idx-1+m_iIdxSize)%m_iIdxSize];
        float t2 = m_pBuffer[(idx+1)%m_iIdxSize];

        if ((t1-2.0*t+t2) != 0.)
            p.corr = (0.5*(t1-t2))/(t1-2.*t+t2);
        else
            p.corr = 0.;

        // Insert peak in peaks list
        if (it!=0)
            it = insert(it, p);
        else
            it = append(p);
    }
    return it;
}

PeakList::iterator PeakList::getFirstInRange(int idx, int len)
{
    // Find first peak in list which is bigger than or equal than idx
    iterator it = begin();

    // If the buffer range is not wrapped...
    if (idx+len<m_iIdxSize)
    {
        while (it!=end() && (*it).i<idx)
            ++it;
    }
    else
    {
        // The range is wrapped
        while (it!=end() && (*it).i<idx)
            ++it;
        if (it!=end())
            return it;

        if ((*begin()).i+m_iIdxSize<idx+len)
            it = begin();
    }
    return it;
}

PeakList::iterator PeakList::getLastInRange(int idx, int len)
{
    // Find last peak in list which is bigger than or equal to idx+len
    iterator it = end();
    --it;

    // If the buffer range is not wrapped...
    if (idx+len<m_iIdxSize)
    {
        while (it!=end() && (*it).i>idx+len)
            --it;
    }
    else
    {
        // The range is wrapped
        it = begin();
        while (it!=end() && (*it).i<(idx+len)%m_iIdxSize)
            ++it;
        if (it!=end())
            return it;

        it = end()--;
        if (!((*it).i<idx+len && (*it).i>=idx))
            it = end();
    }
    return it;
}

int PeakList::getNoInRange(int idx, int len)
{
    // Get first peak in range
    iterator it = getFirstInRange(idx, len);

    int no = 0;

    if (idx+len>=m_iIdxSize)
    {
        // Wrapped buffer
        while ((*it).i<m_iIdxSize && it!=end())
        {
            ++it;
            ++no;
        }
        it = begin();
        while ((*it).i<idx && it!=end())
        {
            ++it;
            ++no;
        }
    }
    else
    {
        // Unwrapped buffer
        while ((*it).i<=idx+len && it!=end())
        {
            ++it;
            ++no;
        }
    }
    return no;
}

float PeakList::getDistance(PeakList::iterator it1, PeakList::iterator it2)
{
    if ((*it2).i>(*it1).i)
        return ((float)(*it2).i+(*it2).corr)-((float)(*it1).i+(*it1).corr);
    else
        return ((float)(*it2).i+(*it2).corr+(float)m_iIdxSize)-((float)(*it1).i+(*it1).corr);
}

float PeakList::getDistance(int idx1, PeakList::iterator it2)
{
    if ((*it2).i>idx1)
        return ((float)(*it2).i+(*it2).corr)-(float)idx1;
    else
        return ((float)(*it2).i+(*it2).corr+(float)m_iIdxSize)-(float)idx1;
}

float PeakList::getDistance(PeakList::iterator it1, int idx2)
{
    if (idx2>(*it1).i)
        return (float)idx2-((float)(*it1).i+(*it1).corr);
    else
        return (float)(idx2+m_iIdxSize)-((float)(*it1).i+(*it1).corr);
}



PeakList::iterator PeakList::getMaxInRange(int idx, int len)
{
    iterator it = getFirstInRange(idx,len);
    iterator itmax = it;

    if (it!=end())
    {
        while (it!=end() && (*it).i<idx+len)
        {
            if (m_pBuffer[(*it).i]>m_pBuffer[(*itmax).i])
                itmax =it;
            ++it;
        }

        // If the range is wrapped...
        if (idx+len>m_iIdxSize)
        {
            it = begin();        
            while (it!=end() && (*it).i<(idx+len)%m_iIdxSize)
            {
                if (m_pBuffer[(*it).i]>m_pBuffer[(*itmax).i])
                    itmax =it;
                ++it;
            }
        }
    }
    return itmax;    
}

/*
bool PeakList::circularValidIndex(int idx, int start, int end, int len)
{
    if (start<end)
    {
        if (idx<=end && idx>=start)
            return true;
    }
    else if (start>end)
    {
        if (!(idx>end && idx<start))
            return true;
    }
    else if (start==end)
        return true;

    return false;
}
*/






