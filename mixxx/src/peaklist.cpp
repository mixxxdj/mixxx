/***************************************************************************
                          peaklist.cpp  -  description
                             -------------------
    begin                : Wed Jul 9 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "peaklist.h"

#include <stdio.h>
#include <QtDebug>
//#include <fstream.h>
#include <string>
#include <iostream>
//Added by qt3to4:
#include <Q3ValueList>

PeakList::PeakList(int iIdxSize, float * pBuffer) : Q3ValueList<PeakType>()
{
    m_iIdxSize = iIdxSize;
    m_pBuffer = pBuffer;
}

PeakList::~PeakList()
{
}

void PeakList::update(int idx, int len)
{
    iterator it;

    //
    // Delete peaks in range
    it = getFirstInRange(idx, len);

    if (it!=end())
    {
        // If buffer is wrapped, and we are at the beginning
        if (idx+len>m_iIdxSize && it==begin())
        {
            while (it!=end() && (*it).i<(idx+len)%m_iIdxSize)
                it = remove(it);
        }
        else
        {
            while (it!=end() && (*it).i<idx+len && (*it).i>=idx)
                it = remove(it);
            // Check if buffer is wrapped
            if (idx+len>m_iIdxSize && it==end())
            {
                it = begin();
                while (it!=end() && (*it).i<(idx+len)%m_iIdxSize)
                    it = remove(it);
            }
        }
    }



    //qDebug() << "from " << idx << ", len " << len << ", START " << (*it).i << ", no " << getNoInRange(idx;

    //
    // Add new peaks to the peak list
    if (idx+len<=m_iIdxSize)
    {
        if (it==end())
            it = getFirstInRange(idx,len,true);
        // The buffer is not wrapped
        for (int i=idx+len-1; i>=idx; --i)
            it = insertIfPeak(i, it);
    }
    else
    {
        // The buffer is wrapped
        it = begin();
        int i;
        for (i=(idx+len-1+m_iIdxSize)%m_iIdxSize; i>=0; --i)
        {
            it = insertIfPeak(i, it);
        }
        it = end();
        for (i=m_iIdxSize-1; i>=idx; --i)
        {
            it = insertIfPeak(i, it);
        }
    }
}

PeakList::iterator PeakList::insertIfPeak(int idx, PeakList::iterator it)
{
//    qDebug() << "check idx " << idx;
    if (m_pBuffer[idx]>m_pBuffer[(idx-1+m_iIdxSize)%m_iIdxSize] && m_pBuffer[idx]>m_pBuffer[(idx+1)%m_iIdxSize])
    {
//        qDebug() << "peak";
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
        if (it!=end())
            it = insert(it, p);
        else
            it = append(p);
    }
    return it;
}

PeakList::iterator PeakList::getFirstInRange(int idx, int len, bool returnElementAfterRange)
{
    // Find first peak in list which is bigger than or equal than idx
    iterator it = begin();
    iterator itbegin = begin();
    iterator itend = end();
    // Return if the list is empty...
    if (it==itend)
        return it;

    // If range is not wrapped...
    if (idx+len<m_iIdxSize)
    {
        while (it!=itend && (*it).i<idx)
            ++it;

        if (!returnElementAfterRange && it!=itend && (*it).i>idx+len)
            it = end();
    }
    else // Range is wrapped...
    {
        // Search for a valid point in the end of the list (start of range)
        while (it!=itend && (*it).i<idx)
            ++it;

        // If we're at the end of the list check if the first point in the list is in range
        if (it==itend && (*itbegin).i<(idx+len)%m_iIdxSize)
            it = itbegin;
    }
    return it;
}

/*
   PeakList::iterator PeakList::getLastInRange(int idx, int len)
   {
    iterator it = end();

    // Only search further if the list is not empty...
    if (begin()!=end())
    {
        while (it!=begin() && (*it).i>=idx+len)
           --it;

        // If we're at the start of the list, and the buffer range is wrapped
        if (it==begin() && idx+len>m_iIdxSize)
        {
            if ((*end()).i+m_iIdxSize>=idx && (*end()).i+m_iIdxSize<idx+len)
                it = begin();
        }
    }
    return it;


    // Find last peak in list which is bigger than or equal to idx+len
    iterator it = end();
    --it;

    // If the buffer range is not wrapped...
    if (idx+len<=m_iIdxSize)
    {
        while (it!=end() && (*it).i>idx+len)
            --it;
    }
    else
    {
        qDebug() << "--";

        // The range is wrapped
        it = begin();
        bool found = false;
        while (it!=end() && (*it).i<(idx+len)%m_iIdxSize)
        {
            qDebug() << "-0";
            found = true;
 ++it;
        }
        if (found)
        {
            qDebug() << "-1";
            return --it;
        }

        it = --end();
        if (!((*it).i<idx+len && (*it).i>=idx))
        {
            qDebug() << "-2";
        }
    }
    return it;*/
//}

int PeakList::getNoInRange(int idx, int len)
{
    // Get first peak in range
    iterator it = getFirstInRange(idx, len);

    int no = 0;

    if (idx+len>m_iIdxSize)
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
        while ((*it).i<idx+len && it!=end())
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
            //qDebug() << "it1 " << (*it).i;
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
                //qDebug() << "it2 " << (*it).i;
                if (m_pBuffer[(*it).i]>m_pBuffer[(*itmax).i])
                    itmax =it;
                ++it;
            }
        }
    }
    return itmax;
}

void PeakList::print()
{
    iterator it = begin();
    while (it!=end())
    {
        std::cout << (*it).i << " ";
        ++it;
    }
    std::cout << "\n";

    //qDebug() << "size " << size();
    if (size()>1)
    {
        iterator it2 = begin();
        ++it2;
        while (it2!=end())
        {
            iterator itback = it2;
            --itback;
            if ((*itback).i>(*it2).i)
            {
                qDebug() << "************error";
            }
            ++it2;
        }
    }
    qDebug() << "---";
}







