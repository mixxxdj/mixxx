/***************************************************************************
                          dlgpreferences.h  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
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

#ifndef DLGPREFERENCES_H
#define DLGPREFERENCES_H

 #include <QDialog>

#include <qevent.h>
#include <QtGui>
#include "ui_dlgpreferencesdlg.h"
#include "configobject.h"

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class MixxxApp;
class MixxxView;
class PlayerProxy;
class SoundManager;
class Track;
class DlgPrefSound;
class DlgPrefMidi;
class DlgPrefPlaylist;
class DlgPrefControls;
class DlgPrefEQ;
class DlgPrefCrossfader;
class DlgPrefRecord;
class DlgPrefBPM;
class DlgPrefVinyl;
class DlgPrefShoutcast;
class PowerMate;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPreferences : public QDialog, public Ui::DlgPreferencesDlg
{
    Q_OBJECT
public:
    DlgPreferences(MixxxApp *mixxx, MixxxView *view,
                   SoundManager *soundman,
                   Track *track, ConfigObject<ConfigValue> *config);
    ~DlgPreferences();
    void createIcons();
public slots:
    void slotUpdate();
    void slotApply();
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void showVinylControlPage();
signals:
    void closeDlg();
    void showDlg();
protected:
    bool eventFilter(QObject *, QEvent *);
private:
    DlgPrefSound *wsound;
    DlgPrefMidi *wmidi;
    DlgPrefPlaylist *wplaylist;
    DlgPrefControls *wcontrols;
    DlgPrefEQ *weq;
    DlgPrefCrossfader *wcrossfader;
    DlgPrefRecord *wrecord;
    DlgPrefBPM *wbpm;
    DlgPrefVinyl *wvinylcontrol;
    DlgPrefShoutcast *wshoutcast;

    ConfigObject<ConfigValue> *config;
    MixxxApp *m_pMixxx;
};

#endif
