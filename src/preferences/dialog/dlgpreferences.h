#pragma once

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
    DlgPrefVinyl* m_vinylControlPage;
    DlgPrefNoVinyl* m_noVinylControlPage;
    DlgPrefInterface* m_interfacePage;
    DlgPrefWaveform* m_waveformPage;
    DlgPrefDeck* m_deckPage;
    DlgPrefColors* m_colorsPage;
    DlgPrefEQ* m_equalizerPage;
    DlgPrefCrossfader* m_crossfaderPage;
    DlgPrefEffects* m_effectsPage;
    DlgPrefAutoDJ* m_autoDjPage;
    DlgPrefBroadcast* m_broadcastingPage;
    DlgPrefRecord* m_recordingPage;
    DlgPrefBeats* m_beatgridPage;
    DlgPrefKey* m_musicalKeyPage;
    DlgPrefReplayGain* m_replayGainPage;
#ifdef __LILV__
    DlgPrefLV2* m_lv2Page;
#endif /* __LILV__ */
#ifdef __MODPLUG__
    DlgPrefModplug* m_modplugPage;
#endif

    QTreeWidgetItem* m_pSoundButton;
    QTreeWidgetItem* m_pLibraryButton;
    QTreeWidgetItem* m_pControllerTreeItem;
    QTreeWidgetItem* m_pVinylControlButton;
    QTreeWidgetItem* m_pInterfaceButton;
    QTreeWidgetItem* m_pWaveformButton;
    QTreeWidgetItem* m_pDecksButton;
    QTreeWidgetItem* m_pColorsButton;
    QTreeWidgetItem* m_pEqButton;
#ifdef __LILV__
    QTreeWidgetItem* m_pLV2Button;
#endif /* __LILV__ */
    QTreeWidgetItem* m_pEffectsButton;
    QTreeWidgetItem* m_pCrossfaderButton;
    //QTreeWidgetItem* m_pEffectsButton;
    QTreeWidgetItem* m_pAutoDJButton;
    QTreeWidgetItem* m_pBroadcastButton;
    QTreeWidgetItem* m_pRecordingButton;
    QTreeWidgetItem* m_pBeatDetectionButton;
    QTreeWidgetItem* m_pKeyDetectionButton;
    QTreeWidgetItem* m_pReplayGainButton;
#ifdef __MODPLUG__
    QTreeWidgetItem* m_pModplugButton;
#endif

    QSize m_pageSizeHint;
};
