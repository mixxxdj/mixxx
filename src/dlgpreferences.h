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
#include <QEvent>
#include <QtGui>

#include "ui_dlgpreferencesdlg.h"
#include "configobject.h"
#include "controlpushbutton.h"

class MixxxApp;
class SoundManager;
class DlgPrefSound;
class DlgPrefController;
class DlgPrefControllers;
class DlgPrefPlaylist;
class DlgPrefControls;
class DlgPrefEQ;
class DlgPrefCrossfader;
class DlgPrefRecord;
class DlgPrefBeats;
class DlgPrefVinyl;
class DlgPrefNoVinyl;
class DlgPrefShoutcast;
class DlgPrefReplayGain;
class ControllerManager;
class SkinLoader;
class PlayerManager;
class VinylControlManager;
#ifdef __MODPLUG__
class DlgPrefModplug;
#endif

class DlgPreferences : public QDialog, public Ui::DlgPreferencesDlg {
    Q_OBJECT
  public:
    DlgPreferences(MixxxApp* mixxx, SkinLoader* pSkinLoader, SoundManager* soundman,
                   PlayerManager* pPlayerManager, ControllerManager* controllers,
                   VinylControlManager* pVCManager, ConfigObject<ConfigValue>* config);
    virtual ~DlgPreferences();

    int addPageWidget(QWidget* pWidget);
    void removePageWidget(QWidget* pWidget);
    void switchToPage(QWidget* pWidget);

  public slots:
    void changePage(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void showSoundHardwarePage();

  signals:
    void closeDlg();
    void showDlg();

  protected:
    bool eventFilter(QObject*, QEvent*);

  private:
    void createIcons();
    void onShow();
    void onHide();

    DlgPrefSound* m_wsound;
    DlgPrefPlaylist* m_wplaylist;
    DlgPrefControllers *m_wcontrollers;
    DlgPrefControls* m_wcontrols;
    DlgPrefEQ* m_weq;
    DlgPrefCrossfader* m_wcrossfader;
    DlgPrefRecord* m_wrecord;
    DlgPrefBeats* m_wbeats;
    DlgPrefVinyl* m_wvinylcontrol;
    DlgPrefNoVinyl* m_wnovinylcontrol;
    DlgPrefShoutcast* m_wshoutcast;
    DlgPrefReplayGain* m_wreplaygain;
#ifdef __MODPLUG__
    DlgPrefModplug* m_wmodplug;
#endif

    QTreeWidgetItem* m_pSoundButton;
    QTreeWidgetItem* m_pPlaylistButton;
    QTreeWidgetItem* m_pControlsButton;
    QTreeWidgetItem* m_pEqButton;
    QTreeWidgetItem* m_pCrossfaderButton;
    QTreeWidgetItem* m_pRecordingButton;
    QTreeWidgetItem* m_pBPMdetectButton;
    QTreeWidgetItem* m_pAnalysersButton;
    QTreeWidgetItem* m_pVinylControlButton;
    QTreeWidgetItem* m_pShoutcastButton;
    QTreeWidgetItem* m_pReplayGainButton;
#ifdef __MODPLUG__
    QTreeWidgetItem* m_pModplugButton;
#endif
    QTreeWidgetItem* m_pControllerTreeItem;

    QSize m_pageSizeHint;

    ControlPushButton m_preferencesUpdated;
};

#endif
