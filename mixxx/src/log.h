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
    
    void slotLogPlay1(double v);
    void slotLogPlay2(double v);
    void slotLogLoop1(double v);
    void slotLogLoop2(double v);
    void slotLogCuePreview1(double v);
    void slotLogCuePreview2(double v);
    void slotLogCueSet1(double v);
    void slotLogCueSet2(double v);
    void slotLogVolume1(double v);
    void slotLogVolume2(double v);
    void slotLogGain1(double v);
    void slotLogGain2(double v);
    void slotLogTemporalShape1(double v);
    void slotLogTemporalShape2(double v);
    void slotLogTemporalPhase1(double v);
    void slotLogTemporalPhase2(double v);
    void slotLogPhase1(double v);
    void slotLogPhase2(double v);
    void slotLogSync1(double v);
    void slotLogSync2(double v);
    void slotLogPitch1(double v);
    void slotLogPitch2(double v);
    void slotLogPitchMinus1(double v);
    void slotLogPitchMinus2(double v);
    void slotLogPitchPlus1(double v);
    void slotLogPitchPlus2(double v);
    void slotLogHeadphone1(double v);
    void slotLogHeadphone2(double v);
    void slotLogLow1(double v);
    void slotLogLow2(double v);
    void slotLogMid1(double v);
    void slotLogMid2(double v);
    void slotLogHigh1(double v);
    void slotLogHigh2(double v);
    void slotLogTrack1(TrackInfoObject *p);
    void slotLogTrack2(TrackInfoObject *p);
    
    
    
    void slotLogPlayPosition(double v);
    void slotLogVirtualPlayPosition(double v);
    void slotLogSearchRate(double v);
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
