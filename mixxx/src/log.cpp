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
    
    //p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "playposition")));
    //connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPlayPosition(double)));
    
    //p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "virtualplayposition")));
    //connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogVirtualPlayPosition(double)));

    //p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "rateSearch")));
    //connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogSearchRate(double)));
    
    
    //p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "play")));
    //connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogCuePlay(double)));
    
    //p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","NextTask")));
    //connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogNextTask(double)));
    
    //p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","PrevTask")));
    //connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPrevTask(double)));

    // Playlist load
    //connect(pTrack, SIGNAL(activePlaylist(TrackPlaylist *)), this, SLOT(slotLogPlaylist(TrackPlaylist *)));
    
    // Track change
    connect(pTrack, SIGNAL(newTrackPlayer1(TrackInfoObject *)), this, SLOT(slotLogTrack1(TrackInfoObject *)));
    connect(pTrack, SIGNAL(newTrackPlayer2(TrackInfoObject *)), this, SLOT(slotLogTrack2(TrackInfoObject *)));
    
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "play")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPlay1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "play")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPlay2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "loop")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogLoop1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "loop")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogLoop2(double)));
    
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "cue_set")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogCueSet1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "cue_set")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogCueSet2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "cue_preview")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogCuePreview1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "cue_preview")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogCuePreview2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "volume")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogVolume1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "volume")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogVolume2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "pregain")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogGain1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "pregain")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogGain2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "temporalShapeRate")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogTemporalShape1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "temporalShapeRate")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogTemporalShape2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "temporalPhaseRate")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogTemporalPhase1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "temporalPhaseRate")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogTemporalPhase2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "wheel")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPhase1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "wheel")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPhase2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "beatsync")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogSync1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "beatsync")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogSync2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "rate")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPitch1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "rate")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPitch2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "rate_perm_down_small")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPitchMinus1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "rate_perm_down_small")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPitchMinus2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "rate_perm_up_small")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPitchPlus1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "rate_perm_up_small")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogPitchPlus2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "pfl")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogHeadphone1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "pfl")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogHeadphone2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "filterLow")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogLow1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "filterLow")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogLow2(double)));
    
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "filterMid")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogMid1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "filterMid")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogMid2(double)));

    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]", "filterHigh")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogHigh1(double)));
    p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]", "filterHigh")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotLogHigh2(double)));
}
        
Log::~Log()
{
    m_qFile.close();
}

void Log::slotLogEvent(QString type, QString arg)
{
    QTextStream stream(&m_qFile);
    stream << m_qTime.elapsed() << " " << type << ", " << arg << "\n";  
//    m_qFile.flush();  
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

void Log::slotLogCuePlay(double v)
{
    slotLogEvent("cueplay", QString("%1").arg(v));
}

void Log::slotLogTrack1(TrackInfoObject *p)
{
    QString title = p->getFilename();
    slotLogEvent("track1", title);
}

void Log::slotLogTrack2(TrackInfoObject *p)
{
    QString title = p->getFilename();
    slotLogEvent("track2", title);
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

void Log::slotLogPlay1(double v)
{
    slotLogEvent("play1", QString("%1").arg(v));
}

void Log::slotLogPlay2(double v)
{
    slotLogEvent("play2", QString("%1").arg(v));
}
    
void Log::slotLogLoop1(double v)
{
    slotLogEvent("loop1", QString("%1").arg(v));
}

void Log::slotLogLoop2(double v)
{
    slotLogEvent("loop2", QString("%1").arg(v));
}

void Log::slotLogCuePreview1(double v)
{
    slotLogEvent("cuepre1", QString("%1").arg(v));
}

void Log::slotLogCuePreview2(double v)
{
    slotLogEvent("cuepre2", QString("%1").arg(v));
}

void Log::slotLogCueSet1(double v)
{
    slotLogEvent("cueset1", QString("%1").arg(v));
}

void Log::slotLogCueSet2(double v)
{
    slotLogEvent("cueset2", QString("%1").arg(v));
}

void Log::slotLogVolume1(double v)
{
    slotLogEvent("vol1", QString("%1").arg(v));
}

void Log::slotLogVolume2(double v)
{
    slotLogEvent("vol2", QString("%1").arg(v));
}

void Log::slotLogGain1(double v)
{
    slotLogEvent("gain1", QString("%1").arg(v));
}

void Log::slotLogGain2(double v)
{
    slotLogEvent("gain2", QString("%1").arg(v));
}

void Log::slotLogTemporalShape1(double v)
{
    slotLogEvent("tmpshape1", QString("%1").arg(v));
}

void Log::slotLogTemporalShape2(double v)
{
    slotLogEvent("tmpshape2", QString("%1").arg(v));
}

void Log::slotLogTemporalPhase1(double v)
{
    slotLogEvent("tmpphase1", QString("%1").arg(v));
}

void Log::slotLogTemporalPhase2(double v)
{
    slotLogEvent("tmpphase2", QString("%1").arg(v));
}

void Log::slotLogPhase1(double v)
{
    slotLogEvent("phase1", QString("%1").arg(v));
}

void Log::slotLogPhase2(double v)
{
    slotLogEvent("phase2", QString("%1").arg(v));
}

void Log::slotLogSync1(double v)
{
    slotLogEvent("sync1", QString("%1").arg(v));
}

void Log::slotLogSync2(double v)
{
    slotLogEvent("sync2", QString("%1").arg(v));
}

void Log::slotLogPitch1(double v)
{
    slotLogEvent("pitch1", QString("%1").arg(v));
}

void Log::slotLogPitch2(double v)
{
    slotLogEvent("pitch2", QString("%1").arg(v));
}

void Log::slotLogPitchMinus1(double v)
{
    slotLogEvent("p-1", QString("%1").arg(v));
}

void Log::slotLogPitchMinus2(double v)
{
    slotLogEvent("p-2", QString("%1").arg(v));
}

void Log::slotLogPitchPlus1(double v)
{
    slotLogEvent("p+1", QString("%1").arg(v));
}

void Log::slotLogPitchPlus2(double v)
{
    slotLogEvent("p+2", QString("%1").arg(v));
}

void Log::slotLogHeadphone1(double v)
{
    slotLogEvent("head1", QString("%1").arg(v));
}

void Log::slotLogHeadphone2(double v)
{
    slotLogEvent("head2", QString("%1").arg(v));
}

void Log::slotLogLow1(double v)
{
    slotLogEvent("low1", QString("%1").arg(v));
}

void Log::slotLogLow2(double v)
{
    slotLogEvent("low2", QString("%1").arg(v));
}

void Log::slotLogMid1(double v)
{
    slotLogEvent("mid1", QString("%1").arg(v));
}

void Log::slotLogMid2(double v)
{
    slotLogEvent("mid2", QString("%1").arg(v));
}

void Log::slotLogHigh1(double v)
{
    slotLogEvent("high1", QString("%1").arg(v));
}

void Log::slotLogHigh2(double v)
{
    slotLogEvent("high2", QString("%1").arg(v));
}



