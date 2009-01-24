/***************************************************************************
bpmscheme.h  -  Preset holding information for BPM detection
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

#ifndef BPMSCHEME_H
#define BPMSCHEME_H

#include <qobject.h>
#include <qmutex.h>

class QString;
class QDomElement;
class QDomDocument;
class QDomNode;
class TrackInfoObject;
class ControlObjectThread;


/**
  * Class for storing a BPM detection scheme
  *
  *@author Micah Lee
  */

class BpmScheme : public QObject
{
public:
    BpmScheme(const QString & name, int minBpm, int maxBpm, bool entire);
    BpmScheme();
    ~BpmScheme();
    
    void writeXML( QDomDocument & doc, QDomElement & header);

    int getMinBpm();
    int getMaxBpm();

    QString getName();
    QString getComment();

    bool getAnalyzeEntireSong();

    void setMinBpm(const int minBpm);
    void setMaxBpm(const int maxBpm);
    
    void setName(const QString & name);
    void setComment(const QString & comment);

    void setAnalyzeEntireSong(const bool entire);
    

protected:
    int m_MinBpm;
    int m_MaxBpm;
    bool m_AnalyzeEntireSong;
    QString m_Name;
    QString m_Comment;

    /** Mutex protecting access to object */
    QMutex m_qMutex;
};

#endif
