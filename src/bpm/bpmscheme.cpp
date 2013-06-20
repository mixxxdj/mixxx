/***************************************************************************
   bpmscheme.cpp  -  Preset holding information for BPM detection
   -------------------
   begin                : Sat, Aug 25., 2007
   copyright            : (C) 2007 by Micah Lee
   email                : snipexv@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "bpmscheme.h"
#include "qstring.h"
#include "xmlparse.h"
#include <qdom.h>

BpmScheme::BpmScheme(const QString & name, int minBpm, int maxBpm, bool entire)
:m_MinBpm(minBpm), m_MaxBpm(maxBpm), m_AnalyzeEntireSong(entire), m_Name(name)
{
}

BpmScheme::BpmScheme()
{
}

BpmScheme::~BpmScheme()
{
}

void BpmScheme::writeXML( QDomDocument & doc, QDomElement & header)
{
    XmlParse::addElement(doc, header, "Name", m_Name);
    XmlParse::addElement(doc, header, "MinBpm", QString("%1").arg(m_MinBpm));
    XmlParse::addElement(doc, header, "MaxBpm", QString("%1").arg(m_MaxBpm));
    XmlParse::addElement(doc, header, "AnalyzeEntireSong", QString("%1").arg((int)m_AnalyzeEntireSong));
    XmlParse::addElement(doc, header, "Comment", m_Comment);
    
}

int BpmScheme::getMinBpm()
{
    m_qMutex.lock();
    int min = m_MinBpm;
    m_qMutex.unlock();
    return min;
}

int BpmScheme::getMaxBpm()
{
    m_qMutex.lock();
    int max = m_MaxBpm;
    m_qMutex.unlock();
    return max;
}

QString BpmScheme::getName()
{
    m_qMutex.lock();
    QString name = m_Name;
    m_qMutex.unlock();
    return name;
}

QString BpmScheme::getComment()
{
    m_qMutex.lock();
    QString comment = m_Comment;
    m_qMutex.unlock();
    return comment;
}

bool BpmScheme::getAnalyzeEntireSong()
{
    m_qMutex.lock();
    bool entire = m_AnalyzeEntireSong;
    m_qMutex.unlock();
    return entire;
}

void BpmScheme::setMinBpm(const int minBpm)
{
    m_qMutex.lock();
    m_MinBpm = minBpm;
    m_qMutex.unlock();
}

void BpmScheme::setMaxBpm(const int maxBpm)
{
    m_qMutex.lock();
    m_MaxBpm = maxBpm;
    m_qMutex.unlock();
}

void BpmScheme::setName(const QString & name)
{
    m_qMutex.lock();
    m_Name = name;
    m_qMutex.unlock();
}

void BpmScheme::setComment(const QString & comment)
{
    m_qMutex.lock();
    m_Comment = comment;
    m_qMutex.unlock();
}

void BpmScheme::setAnalyzeEntireSong(const bool entire)
{
    m_qMutex.lock();
    m_AnalyzeEntireSong = entire;
    m_qMutex.unlock();
}


