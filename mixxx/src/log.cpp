//
// C++ Implementation: log
//
// Description: 
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "log.h"
#include <qtextstream.h>
#include "controlobject.h"
#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "track.h"
#include "trackinfoobject.h"
#include "trackplaylistlist.h"

Log::Log(QString qFilename, Track *pTrack) : m_qFile(qFilename)
{
    if (QFile::exists(qFilename))
        qFatal("Log file exists: %s",qFilename.latin1());
     
    if (!m_qFile.open(IO_WriteOnly))
        qFatal("Could not write to file: %s",qFilename.latin1());
    
    qDebug("Logging to %s",qFilename.latin1());
            
    m_qTime.start();

    //
    // Set up what to log
    //
    ControlObjectThreadMain *p;
    
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "playposition")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPlayPosition(double)));
    
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "virtualplayposition")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogVirtualPlayPosition(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "rateSearch")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogSearchRate(double)));
    
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "play")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPlay(double)));
    
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "play")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogCuePlay(double)));
    
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","NextTask")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogNextTask(double)));
    
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","PrevTask")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPrevTask(double)));

    
    // Track change
    connect(pTrack, SIGNAL(newTrackPlayer1(TrackInfoObject *)), this, SLOT(slotLogTrack(TrackInfoObject *)));
    
    // Playlist load
    connect(pTrack, SIGNAL(activePlaylist(TrackPlaylist *)), this, SLOT(slotLogPlaylist(TrackPlaylist *)));

}
        
Log::~Log()
{
    m_qFile.close();
}

void Log::slotLogEvent(QString type, QString arg)
{
    QTextStream stream(&m_qFile);
    stream << m_qTime.elapsed() << " " << type << ", " << arg << "\n";  
    m_qFile.flush();  
}

void Log::slotLogPlayPosition(double v)
{
    slotLogEvent("pos", QString("%1").arg(v));
}

void Log::slotLogVirtualPlayPosition(double v)
{
    slotLogEvent("vpos", QString("%1").arg(v));
}

void Log::slotLogSearchRate(double v)
{
    slotLogEvent("searchr", QString("%1").arg(v));
}

void Log::slotLogPlay(double v)
{
    slotLogEvent("play", QString("%1").arg(v));
}

void Log::slotLogCuePlay(double v)
{
    slotLogEvent("cueplay", QString("%1").arg(v));
}

void Log::slotLogTrack(TrackInfoObject *p)
{
    QString title = p->getFilename();
    slotLogEvent("track", title);
}

void Log::slotLogPlaylist(TrackPlaylist *p)
{
    QString name = p->getName();
    slotLogEvent("plist", name);
}

void Log::slotLogNextTask(double v)
{
    slotLogEvent("finish", QString("%1").arg(v));
}

void Log::slotLogPrevTask(double v)
{
	    slotLogEvent("prevtask", QString("%1").arg(v));
}


void Log::slotLogCondition(int c)
{
    slotLogEvent("condition", QString("%1").arg(c));
}

void Log::slotLogConditionNo(int cn)
{
	    slotLogEvent("condno", QString("%1").arg(cn));
}

