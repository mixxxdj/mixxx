/***************************************************************************
                          dlgbpmtap.h  -  description
                             -------------------
    begin                : Wed Jul 11 2007
    copyright            : (C) 2007 by Micah Lee
    email                : mtl@clemson.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//#ifdef __EXPERIMENTAL_BPM__
#ifndef DLGBPMTAP_H
#define DLGBPMTAP_H

#include "ui_dlgbpmtapdlg.h"
#include <QEvent>
#include <QtGui>
#include "configobject.h"
#include "bpmreceiver.h"
#include "track.h"

class MixxxApp;
class TrackInfoObject;

/**
  *@author Micah Lee
  */
class DlgBpmTap : public QDialog, public Ui::DlgBpmTapDlg, public BpmReceiver
{
    Q_OBJECT
public:
    DlgBpmTap(QWidget *mixxx, TrackInfoObject *tio, TrackPlaylist *playlist);
    ~DlgBpmTap();

public slots:
    void slotTapBPM();
    void slotDetectBPM();
    void slotLoadDialog();
    void slotOK();
    void slotNext();
    void slotPrev();
    void slotUpdate();
    void slotApply();
    void slotUpdateMinBpm(int i);
    void slotUpdateMaxBpm(int i);
    void slotBpmChanged(const QString & bpm);

signals:
    void closeDlg();
    void aboutToShow();

public:
    // Inherited methods from BpmReceiver.
    void setProgress(TrackInfoObject *tio, int progress);
    void setComplete(TrackInfoObject *tio, bool failed, float returnBpm);

protected:
    bool eventFilter(QObject *, QEvent *);
    void loadTrackInfo();

private:
    MixxxApp *m_pMixxx;
    TrackInfoObject *m_CurrentTrack;
    TrackPlaylist *m_TrackPlaylist;
    QTime *m_Time;
    int m_TapCount;
};

#endif
//#endif //__EXPERIMENTAL_BPM__
