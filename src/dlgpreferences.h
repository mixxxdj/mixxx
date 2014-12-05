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
#include <QRect>
#include <QStringList>

#include "ui_dlgpreferencesdlg.h"
#include "configobject.h"
#include "controlpushbutton.h"
#include "preferences/dlgpreferencepage.h"

class MixxxMainWindow;
class SoundManager;
class DlgPrefSound;
class DlgPrefController;
class DlgPrefControllers;
class DlgPrefLibrary;
class DlgPrefControls;
class DlgPrefWaveform;
class DlgPrefAutoDJ;
class DlgPrefEQ;
class DlgPrefEffects;
class DlgPrefCrossfader;
class DlgPrefRecord;
class DlgPrefKey;
class DlgPrefBeats;
class DlgPrefVinyl;
class DlgPrefNoVinyl;
class DlgPrefShoutcast;
class DlgPrefReplayGain;
class ControllerManager;
class EffectsManager;
class SkinLoader;
class PlayerManager;
class Library;
class VinylControlManager;
#ifdef __MODPLUG__
class DlgPrefModplug;
#endif

class DlgPreferences : public QDialog, public Ui::DlgPreferencesDlg {
    Q_OBJECT
  public:
    DlgPreferences(MixxxMainWindow* mixxx, SkinLoader* pSkinLoader, SoundManager* soundman,
                   PlayerManager* pPlayerManager, ControllerManager* controllers,
                   VinylControlManager* pVCManager, EffectsManager* pEffectsManager,
                   ConfigObject<ConfigValue>* pConfig, Library *pLibrary);
    virtual ~DlgPreferences();

    void addPageWidget(DlgPreferencePage* pWidget);
    void removePageWidget(DlgPreferencePage* pWidget);
    void expandTreeItem(QTreeWidgetItem* pItem);
    void switchToPage(DlgPreferencePage* pWidget);

  public slots:
    void changePage(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void showSoundHardwarePage();
    void slotButtonPressed(QAbstractButton* pButton);
  signals:
    void closeDlg();
    void showDlg();

    // Emitted just after the user clicks Apply or OK.
    void applyPreferences();
    // Emitted if the user clicks Cancel
    void cancelPreferences();
    // Emitted if the user clicks Reset to Defaults.
    void resetToDefaults();

  protected:
    bool eventFilter(QObject*, QEvent*);
    void moveEvent(QMoveEvent* e);
    void resizeEvent(QResizeEvent* e);

  private:
    DlgPreferencePage* currentPage();
    void createIcons();
    void onShow();
    void onHide();
    QRect getDefaultGeometry();

    QStringList m_geometry;
    ConfigObject<ConfigValue>* m_pConfig;
    DlgPrefSound* m_wsound;
    DlgPrefLibrary* m_wlibrary;
    DlgPrefControllers *m_wcontrollers;
    DlgPrefControls* m_wcontrols;
    DlgPrefWaveform* m_wwaveform;
    DlgPrefAutoDJ* m_wautodj;
    DlgPrefEQ* m_weq;
    DlgPrefEffects* m_weffects;
    DlgPrefCrossfader* m_wcrossfader;
    DlgPrefRecord* m_wrecord;
    DlgPrefKey* m_wkey;
    DlgPrefBeats* m_wbeats;
    DlgPrefVinyl* m_wvinylcontrol;
    DlgPrefNoVinyl* m_wnovinylcontrol;
    DlgPrefShoutcast* m_wshoutcast;
    DlgPrefReplayGain* m_wreplaygain;
#ifdef __MODPLUG__
    DlgPrefModplug* m_wmodplug;
#endif

    QTreeWidgetItem* m_pSoundButton;
    QTreeWidgetItem* m_pLibraryButton;
    QTreeWidgetItem* m_pControlsButton;
    QTreeWidgetItem* m_pWaveformButton;
    QTreeWidgetItem* m_pAutoDJButton;
    QTreeWidgetItem* m_pEqButton;
    QTreeWidgetItem* m_pEffectsButton;
    QTreeWidgetItem* m_pCrossfaderButton;
    QTreeWidgetItem* m_pRecordingButton;
    QTreeWidgetItem* m_pBeatDetectionButton;
    QTreeWidgetItem* m_pKeyDetectionButton;
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
