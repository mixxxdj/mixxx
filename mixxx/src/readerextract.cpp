/***************************************************************************
                          readerextract.cpp  -  description
                             -------------------
    begin                : Tue Mar 18 2003
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

#include "readerextract.h"
#ifdef __VISUALS__
#include "visual/visualchannel.h"
#endif

ReaderExtract::ReaderExtract(ReaderExtract *_input, QString qsVisualDataType)
{
    m_qsVisualDataType = qsVisualDataType;
    input = _input;
    m_pVisualBuffer = 0;
}

ReaderExtract::~ReaderExtract()
{
}

void ReaderExtract::addVisual(VisualChannel *pVisualChannel)
{
#ifdef __VISUALS__
    m_pVisualBuffer = pVisualChannel->add(this);
#endif
}

QString ReaderExtract::getVisualDataType()
{
    return m_qsVisualDataType;
}
