//
// C++ Interface: log
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LOG_H
#define LOG_H

#include <qfile.h>
#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>

/**
@author Tue Haste Andersen
*/

class Track;
class TrackInfoObject;
class TrackPlaylist;

class Log : public QObject
{
    Q_OBJECT
public:
    Log(QString qFilename, Track *pTrack);
    ~Log();

public slots:    
    void slotLogEvent(QString type, QString arg);
    void slotLogPlayPosition(double v);
    void slotLogVirtualPlayPosition(double v);
    void slotLogSearchRate(double v);
    void slotLogPlay(double v);
    void slotLogTrack(TrackInfoObject *p);
    void slotLogPlaylist(TrackPlaylist *p);
    void slotLogNextTask(double v);
    void slotLogPrevTask(double v);
    void slotLogCondition(int c);
    void slotLogConditionNo(int cn); 
    void slotLogCuePlay(double v);
    
protected:
    QFile m_qFile;
    QTime m_qTime;
};

#endif
