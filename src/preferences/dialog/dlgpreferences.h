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
#include "preferences/settingsmanager.h"

class MixxxMainWindow;
class SoundManager;
class DlgPrefSound;
class DlgPrefLibrary;
class DlgPrefController;
class DlgPrefControllers;
class DlgPrefVinyl;
class DlgPrefNoVinyl;
class DlgPrefInterface;
class DlgPrefWaveform;
class DlgPrefDeck;
class DlgPrefColors;
class DlgPrefEQ;
class DlgPrefEffects;
class DlgPrefCrossfader;
class DlgPrefAutoDJ;
class DlgPrefBroadcast;
class DlgPrefRecord;
class DlgPrefBeats;
class DlgPrefKey;
class DlgPrefReplayGain;
#ifdef __LILV__
class DlgPrefLV2;
#endif /* __LILV__ */
class LV2Backend;
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
    struct PreferencesPage {
        PreferencesPage() {
        }
        PreferencesPage(DlgPreferencePage* pDlg, QTreeWidgetItem* pTreeItem)
                : pDlg(pDlg), pTreeItem(pTreeItem) {
        }

        DlgPreferencePage* pDlg;
        QTreeWidgetItem* pTreeItem;
    };

DlgPreferences(MixxxMainWindow* mixxx,
            SkinLoader* pSkinLoader,
            SoundManager* soundman,
            PlayerManager* pPlayerManager,
            ControllerManager* controllers,
            VinylControlManager* pVCManager,
            LV2Backend* pLV2Backend,
            EffectsManager* pEffectsManager,
            SettingsManager* pSettingsManager,
            Library* pLibrary);
    virtual ~DlgPreferences();

    void addPageWidget(PreferencesPage page);
    void removePageWidget(DlgPreferencePage* pWidget);
    void expandTreeItem(QTreeWidgetItem* pItem);
    void switchToPage(DlgPreferencePage* pPage);

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
    QList<PreferencesPage> m_allPages;
    QTreeWidgetItem* createTreeItem(QString text, QIcon icon);
    void onShow();
    void onHide();
    QRect getDefaultGeometry();

    QAbstractButton* m_pApplyButton;
    QAbstractButton* m_pAcceptButton;

    QStringList m_geometry;
    UserSettingsPointer m_pConfig;
    PreferencesPage m_soundPage;
    DlgPrefControllers* m_pControllersDlg;
    DlgPrefColors* m_colorsPage;
    QTreeWidgetItem* m_pColorsButton;

    QSize m_pageSizeHint;
};

#endif
