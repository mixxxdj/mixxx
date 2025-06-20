#pragma once

#include <QList>
#include <QMenuBar>
#include <QObject>
#include <QScopedPointer>

#include "control/controlproxy.h"
#include "preferences/configobject.h"
#include "preferences/usersettings.h"

class QAction;

class VisibilityControlConnection : public QObject {
    Q_OBJECT
  public:
    VisibilityControlConnection(QObject* pParent, QAction* pAction,
                                const ConfigKey& key);

  public slots:
    void slotClearControl();
    void slotReconnectControl();

  private slots:
    void slotControlChanged();
    void slotActionToggled(bool toggle);

  private:
    ConfigKey m_key;
    QScopedPointer<ControlProxy> m_pControl;
    QAction* m_pAction;
};

class WMainMenuBar : public QMenuBar {
    Q_OBJECT
  public:
    WMainMenuBar(QWidget* pParent, UserSettingsPointer pConfig,
                 ConfigObject<ConfigValueKbd>* pKbdConfig);
#ifndef __APPLE__
    void hideMenuBar();
    void showMenuBar();
#endif

  public slots:
    void onLibraryScanStarted();
    void onLibraryScanFinished();
    void onRecordingStateChange(bool recording);
    void onBroadcastingStateChange(bool broadcasting);
    void onNewSkinAboutToLoad();
    void onNewSkinLoaded();
    void onDeveloperToolsHidden();
    void onDeveloperToolsShown();
    void onFullScreenStateChange(bool fullscreen);
    void onVinylControlDeckEnabledStateChange(int deck, bool enabled);
    void onNumberOfDecksChanged(int decks);
    void onKeywheelChange(int state);
#ifndef __APPLE__
    void slotToggleMenuBar();
#endif

  signals:
    void createCrate();
    void createPlaylist();
    void loadTrackToDeck(int deck);
    void reloadSkin();
    void rescanLibrary();
#ifdef __ENGINEPRIME__
    void exportLibrary();
#endif
    void searchInCurrentView();
    void searchInAllTracks();
    void showAbout();
    void showKeywheel(bool visible);
    void showPreferences();
    void toggleDeveloperTools(bool toggle);
    void toggleFullScreen(bool toggle);
    void toggleKeyboardShortcuts(bool toggle);
    void toggleBroadcasting(bool toggle);
    void toggleRecording(bool enabled);
    void toggleVinylControl(int deck);
    void visitUrl(const QString& url);
    void quit();

    void internalRecordingStateChange(bool recording);
    void internalBroadcastingStateChange(bool broadcasting);
    void internalFullScreenStateChange(bool fullscreen);
    void internalLibraryScanActive(bool active);
    void internalDeveloperToolsStateChange(bool visible);
    void internalKeywheelStateChanged(int state);
    void internalOnNewSkinLoaded();
    void internalOnNewSkinAboutToLoad();

  private slots:
#ifndef __APPLE__
    void slotAutoHideMenuBarToggled(bool autoHide);
#endif
    void slotDeveloperStatsExperiment(bool enable);
    void slotDeveloperStatsBase(bool enable);
    void slotDeveloperDebugger(bool toggle);
    void slotVisitUrl(const QUrl& url);

  private:
    void initialize();
#ifndef __APPLE__
    /// this ensures the menubar is shown when a menu hotkey is pressed
    /// while the menubar is hidden
    void connectMenuToSlotShowMenuBar(const QMenu* pMenu);
#endif
    void createVisibilityControl(QAction* pAction, const ConfigKey& key);

    UserSettingsPointer m_pConfig;
    QAction* m_pViewKeywheel;
    ConfigObject<ConfigValueKbd>* m_pKbdConfig;
    QList<QAction*> m_loadToDeckActions;
    QList<QAction*> m_vinylControlEnabledActions;
};
