#pragma once

#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QRect>
#include <QStringList>

#include "control/controlpushbutton.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgpreferencesdlg.h"
#include "preferences/settingsmanager.h"
#include "preferences/usersettings.h"

class MixxxMainWindow;
class SoundManager;
class DlgPrefSound;
class DlgPrefLibrary;
class DlgPrefController;
class DlgPrefControllers;
#ifdef __VINYLCONTROL__
class DlgPrefVinyl;
#endif // __VINYLCONTROL__
class DlgPrefInterface;
class DlgPrefWaveform;
class DlgPrefDeck;
class DlgPrefColors;
class DlgPrefEQ;
class DlgPrefEffects;
class DlgPrefCrossfader;
class DlgPrefAutoDJ;
#ifdef __BROADCAST__
class DlgPrefBroadcast;
#endif // __BROADCAST__
class DlgPrefRecord;
class DlgPrefBeats;
class DlgPrefKey;
class DlgPrefReplayGain;
#ifdef __LILV__
class DlgPrefLV2;
#endif // __LILV__
class LV2Backend;
class ControllerManager;
class EffectsManager;
class SkinLoader;
class PlayerManager;
class Library;
class VinylControlManager;
#ifdef __MODPLUG__
class DlgPrefModplug;
#endif // __MODPLUG__

class DlgPreferences : public QDialog, public Ui::DlgPreferencesDlg {
    Q_OBJECT
  public:
    DlgPreferences(MixxxMainWindow* pMixxx,
            SkinLoader* pSkinLoader,
            SoundManager* pSoundManager,
            PlayerManager* pPlayerManager,
            ControllerManager* pControllerManager,
            VinylControlManager* pVCManager,
            LV2Backend* pLV2Backend,
            EffectsManager* pEffectsManager,
            SettingsManager* pSettingsManager,
            Library* pLibrary);
    virtual ~DlgPreferences();

    void addPageWidget(DlgPreferencePage* pWidget,
            QTreeWidgetItem* pTreeItem,
            const QString& pageTitle,
            const QString& iconFile);
    void removePageWidget(DlgPreferencePage* pWidget);
    void expandTreeItem(QTreeWidgetItem* pItem);
    void switchToPage(DlgPreferencePage* pWidget);

  public slots:
    void changePage(QTreeWidgetItem* pCurrent, QTreeWidgetItem* pPrevious);
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
    void onShow();
    void onHide();
    QRect getDefaultGeometry();

    QStringList m_geometry;
    UserSettingsPointer m_pConfig;
    DlgPrefSound* m_soundPage;
    DlgPrefLibrary* m_libraryPage;
    DlgPrefControllers *m_controllersPage;
#ifdef __VINYLCONTROL__
    DlgPrefVinyl* m_vinylControlPage;
#endif // __VINYLCONTROL__
    DlgPrefInterface* m_interfacePage;
    DlgPrefWaveform* m_waveformPage;
    DlgPrefDeck* m_deckPage;
    DlgPrefColors* m_colorsPage;
    DlgPrefEQ* m_equalizerPage;
    DlgPrefCrossfader* m_crossfaderPage;
    DlgPrefEffects* m_effectsPage;
    DlgPrefAutoDJ* m_autoDjPage;
#ifdef __BROADCAST__
    DlgPrefBroadcast* m_broadcastingPage;
#endif // __BROADCAST__
    DlgPrefRecord* m_recordingPage;
    DlgPrefBeats* m_beatgridPage;
    DlgPrefKey* m_musicalKeyPage;
    DlgPrefReplayGain* m_replayGainPage;
#ifdef __LILV__
    DlgPrefLV2* m_lv2Page;
#endif // __LILV__
#ifdef __MODPLUG__
    DlgPrefModplug* m_modplugPage;
#endif // __MODPLUG__

    QTreeWidgetItem* m_pSoundButton;
    QTreeWidgetItem* m_pLibraryButton;
    QTreeWidgetItem* m_pControllersRootButton;
#ifdef __VINYLCONTROL__
    QTreeWidgetItem* m_pVinylControlButton;
#endif // __VINYLCONTROL__
    QTreeWidgetItem* m_pInterfaceButton;
    QTreeWidgetItem* m_pWaveformButton;
    QTreeWidgetItem* m_pDecksButton;
    QTreeWidgetItem* m_pColorsButton;
    QTreeWidgetItem* m_pEqButton;
#ifdef __LILV__
    QTreeWidgetItem* m_pLV2Button;
#endif // __LILV__
    QTreeWidgetItem* m_pEffectsButton;
    QTreeWidgetItem* m_pCrossfaderButton;
    QTreeWidgetItem* m_pAutoDJButton;
#ifdef __BROADCAST__
    QTreeWidgetItem* m_pBroadcastButton;
#endif // __BROADCAST__
    QTreeWidgetItem* m_pRecordingButton;
    QTreeWidgetItem* m_pBeatDetectionButton;
    QTreeWidgetItem* m_pKeyDetectionButton;
    QTreeWidgetItem* m_pReplayGainButton;
#ifdef __MODPLUG__
    QTreeWidgetItem* m_pModplugButton;
#endif // __MODPLUG__

    QSize m_pageSizeHint;

    QDir m_iconsPath;
};
