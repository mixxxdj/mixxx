/***************************************************************************
                          peaklist.h  -  description
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

#ifndef PEAKLIST_H
#define PEAKLIST_H

#include <QList>

/** Struct used to store peaks as index (i) and interpolated correction (corr) */
typedef struct PeakType {
    int i;
    float corr;
} PeakType;

/**
  *@author Tue & Ken Haste Andersen
  */

class PeakList : public QList<PeakType> {
public: 
    PeakList(int iIdxSize, float *pBuffer);
    ~PeakList();
    /** Update the peak list with content from buffer index idx of length len */
    void update(int idx, int len);
    /** Return list iterator to first peak in the given range from idx to idx+len */
    PeakList::iterator getFirstInRange(int idx, int len, bool returnElementAfterRange=false);
    /** Return list iterator to last peak in the given range from idx to idx+len */
    //PeakList::iterator getLastInRange(int idx, int len);
    /** Return number of peaks in the given range from idx to idx+len */
    int getNoInRange(int idx, int len);
    /** Returns distance between two peaks */
    float getDistance(PeakList::iterator it1, PeakList::iterator it2);
    /** Returns distance between an index and a peak */
    float getDistance(int idx1, PeakList::iterator it2);
    /** Returns distance between an peak and an index */
    float getDistance(PeakList::iterator it1, int idx2);

    /** Return list iterator to the maximum peak in the given range from idx to idx+len */
    PeakList::iterator getMaxInRange(int idx, int len);

    void print();

private:
    /** Insert a peak into the list before iterator it, but only if idx is a valid peak in the m_pBuffer. */
    PeakList::iterator insertIfPeak(int idx, PeakList::iterator it);

    /** Total number of indexs in m_pBuffer */
    int m_iIdxSize;
    /** Pointer to buffer containing values */
    float *m_pBuffer;
};

#endif
