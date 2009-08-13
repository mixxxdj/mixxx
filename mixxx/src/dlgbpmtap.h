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

#ifndef DLGBPMTAP_H
#define DLGBPMTAP_H

#include "ui_dlgbpmtapdlg.h"
#include <QEvent>
#include <QtGui>
#include <qlist.h>

#include "configobject.h"
#include "bpm/bpmreceiver.h"

class MixxxApp;
class TrackInfoObject;
class TrackPlaylist;
class BpmScheme;
class AnalyserQueue;

/**
  *@author Micah Lee
  */
class DlgBpmTap : public QDialog, public Ui::DlgBpmTapDlg, public BpmReceiver
{
    Q_OBJECT
public:
    DlgBpmTap(QWidget *, TrackInfoObject *tio, TrackPlaylist *playlist, ConfigObject<ConfigValue> *_config);
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
    void slotTitleChanged(const QString & title);
    void slotArtistChanged(const QString & artist);
    void slotCommentChanged();
    void slotBpmSchemeChanged(int ndx);
    void slotComplete(TrackInfoObject *tio);

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
    
    // Private methods for loading and saving the BPM schemes
    // to and from the file system.
    void loadBpmSchemes();
    
    // Method for filling in the list of BPM schemes on the dialog
    void populateBpmSchemeList();

private:
    MixxxApp *m_pMixxx;
    AnalyserQueue *m_pAnalyserQueue;
    TrackInfoObject *m_CurrentTrack;
    TrackPlaylist *m_TrackPlaylist;
    QTime *m_Time;
    int m_TapCount;
    QList<BpmScheme*> m_BpmSchemes;
    int m_DefaultScheme;
    
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
};

#endif
