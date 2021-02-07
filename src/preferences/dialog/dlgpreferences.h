#pragma once

#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QRect>
#include <QStringList>
#include <memory>

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
            std::shared_ptr<SkinLoader> pSkinLoader,
            std::shared_ptr<SoundManager> pSoundManager,
            std::shared_ptr<PlayerManager> pPlayerManager,
            std::shared_ptr<ControllerManager> pControllerManager,
            std::shared_ptr<VinylControlManager> pVCManager,
            LV2Backend* pLV2Backend,
            std::shared_ptr<EffectsManager> pEffectsManager,
            std::shared_ptr<SettingsManager> pSettingsManager,
            std::shared_ptr<Library> pLibrary);
    virtual ~DlgPreferences();

    void addPageWidget(PreferencesPage page,
            const QString& pageTitle,
            const QString& iconFile);
    void removePageWidget(DlgPreferencePage* pWidget);
    void expandTreeItem(QTreeWidgetItem* pItem);
    void switchToPage(DlgPreferencePage* pPage);

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
    QList<PreferencesPage> m_allPages;
    void onShow();
    void onHide();
    QRect getDefaultGeometry();

    QAbstractButton* m_pApplyButton;
    QAbstractButton* m_pAcceptButton;

    QStringList m_geometry;
    UserSettingsPointer m_pConfig;
    PreferencesPage m_soundPage;
    DlgPrefControllers* m_pControllersDlg;

    QSize m_pageSizeHint;

    QDir m_iconsPath;
};
