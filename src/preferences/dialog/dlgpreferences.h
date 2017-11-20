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

#include "preferences/dialog/ui_dlgpreferencesdlg.h"
#include "preferences/usersettings.h"
#include "control/controlpushbutton.h"
#include "preferences/dlgpreferencepage.h"

class MixxxMainWindow;
class SoundManager;
class DlgPrefSound;
class DlgPrefController;
class DlgPrefControllers;
class DlgPrefLibrary;
class DlgPrefInterface;
class DlgPrefDeck;
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
class DlgPrefBroadcast;
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
                   UserSettingsPointer pConfig, Library *pLibrary);
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
    UserSettingsPointer m_pConfig;
    DlgPrefSound* m_soundPage;
    DlgPrefLibrary* m_libraryPage;
    DlgPrefControllers *m_controllersPage;
    DlgPrefInterface* m_interfacePage;
    DlgPrefDeck* m_deckPage;
    DlgPrefWaveform* m_waveformPage;
    DlgPrefAutoDJ* m_autoDjPage;
    DlgPrefEQ* m_equalizerPage;
    //TODO: Re-enable the effects pane when it does something useful.
    //DlgPrefEffects* m_effectsPage;
    DlgPrefCrossfader* m_crossfaderPage;
    DlgPrefRecord* m_recordingPage;
    DlgPrefKey* m_musicalKeyPage;
    DlgPrefBeats* m_beatgridPage;
    DlgPrefVinyl* m_vinylControlPage;
    DlgPrefNoVinyl* m_noVinylControlPage;
    DlgPrefBroadcast* m_broadcastingPage;
    DlgPrefReplayGain* m_replayGainPage;
#ifdef __MODPLUG__
    DlgPrefModplug* m_modplugPage;
#endif

    QTreeWidgetItem* m_pSoundButton;
    QTreeWidgetItem* m_pLibraryButton;
    QTreeWidgetItem* m_pInterfaceButton;
    QTreeWidgetItem* m_pDecksButton;
    QTreeWidgetItem* m_pWaveformButton;
    QTreeWidgetItem* m_pAutoDJButton;
    QTreeWidgetItem* m_pEqButton;
    //QTreeWidgetItem* m_pEffectsButton;
    QTreeWidgetItem* m_pCrossfaderButton;
    QTreeWidgetItem* m_pRecordingButton;
    QTreeWidgetItem* m_pBeatDetectionButton;
    QTreeWidgetItem* m_pKeyDetectionButton;
    QTreeWidgetItem* m_pVinylControlButton;
    QTreeWidgetItem* m_pBroadcastButton;
    QTreeWidgetItem* m_pReplayGainButton;
#ifdef __MODPLUG__
    QTreeWidgetItem* m_pModplugButton;
#endif
    QTreeWidgetItem* m_pControllerTreeItem;

    QSize m_pageSizeHint;

    ControlPushButton m_preferencesUpdated;
};

#endif
