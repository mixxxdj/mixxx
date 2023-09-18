#pragma once

#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QRect>
#include <QStringList>
#include <memory>

#include "control/controlpushbutton.h"
#include "preferences/constants.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgpreferencesdlg.h"
#include "preferences/settingsmanager.h"
#include "preferences/usersettings.h"

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
class DlgPrefEffects;
class DlgPrefMixer;
class DlgPrefAutoDJ;
#ifdef __BROADCAST__
class DlgPrefBroadcast;
#endif // __BROADCAST__
class DlgPrefRecord;
class DlgPrefBeats;
class DlgPrefRhythm;
class DlgPrefKey;
class DlgPrefReplayGain;
class LV2Backend;
class ControllerManager;
class EffectsManager;
class PlayerManager;
class Library;
class VinylControlManager;
#ifdef __MODPLUG__
class DlgPrefModplug;
#endif // __MODPLUG__

namespace mixxx {
class ScreensaverManager;
namespace skin {
class SkinLoader;
}
} // namespace mixxx

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

    DlgPreferences(
            std::shared_ptr<mixxx::ScreensaverManager> pScreensaverManager,
            std::shared_ptr<mixxx::skin::SkinLoader> pSkinLoader,
            std::shared_ptr<SoundManager> pSoundManager,
            std::shared_ptr<ControllerManager> pControllerManager,
            std::shared_ptr<VinylControlManager> pVCManager,
            std::shared_ptr<EffectsManager> pEffectsManager,
            std::shared_ptr<SettingsManager> pSettingsManager,
            std::shared_ptr<Library> pLibrary);
    virtual ~DlgPreferences();

    void addPageWidget(PreferencesPage page,
            const QString& pageTitle,
            const QString& iconFile);
    void removePageWidget(DlgPreferencePage* pWidget);
    void expandTreeItem(QTreeWidgetItem* pItem);
    void switchToPage(const QString& pageTitle, DlgPreferencePage* pPage);

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

  signals:
    void reloadUserInterface();
    void tooltipModeChanged(mixxx::TooltipsPreference tooltipMode);

  protected:
    bool eventFilter(QObject*, QEvent*);
    void moveEvent(QMoveEvent* e);
    void resizeEvent(QResizeEvent* e);

  private:
    DlgPreferencePage* currentPage();
    void fixSliderStyle();
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
