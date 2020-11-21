#pragma once

#include <QAction>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QScopedPointer>
#include <QWidget>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "preferences/configobject.h"
#include "preferences/usersettings.h"

namespace {
typedef std::function<void(QMenu*, QAction*, bool)> FnAddMenu;
}

class VisibilityControlConnection : public QObject {
    Q_OBJECT
  public:
    VisibilityControlConnection(QObject* pParent,
            QAction* pAction,
            const ConfigKey& key,
            ControlObject* COFeature);
    double value();
    bool valid();

  public slots:
    void slotClearControl();
  private slots:
    void slotReconnectControl();

  private slots:
    void slotControlChanged();
    void slotActionToggled(bool toggle);

  private:
    ConfigKey m_key;
    ControlObject* m_pCOFeature;
    QScopedPointer<ControlProxy> m_pControl;
    QAction* m_pAction;
};

class WMainMenu : public QWidget {
    Q_OBJECT
  public:
    WMainMenu(QWidget* pParent,
            UserSettingsPointer pConfig,
            ConfigObject<ConfigValueKbd>* pKbdConfig);
    void createMenu(FnAddMenu fnAddMenu, bool isMainMenu = false);
    QMenuBar* createMainMenuBar(QWidget* parent, bool native = true);
    /// Should the mainMenuBar be visible
    bool shouldBeVisible();
    /// Cleanup connections and remove controlproxies
    void finalize();

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

  signals:
    void createCrate();
    void createPlaylist();
    void loadTrackToDeck(int deck);
    void reloadSkin();
    void rescanLibrary();
    void showAbout();
    void showPreferences();
    void toggleDeveloperTools(bool toggle);
    void toggleFullScreen(bool toggle);
    void toggleKeyboardShortcuts(bool toggle);
    void toggleBroadcasting(bool toggle);
    void toggleRecording(bool enabled);
    void toggleVinylControl(int deck);
    void toggleMenubarVisible(bool visible);
    void visitUrl(const QString& url);
    void quit();

    void internalRecordingStateChange(bool recording);
    void internalBroadcastingStateChange(bool broadcasting);
    void internalFullScreenStateChange(bool fullscreen);
    void internalLibraryScanActive(bool active);
    void internalDeveloperToolsStateChange(bool visible);
    void internalOnNewSkinLoaded();
    void internalOnNewSkinAboutToLoad();

  private slots:
    void slotDeveloperStatsExperiment(bool enable);
    void slotDeveloperStatsBase(bool enable);
    void slotDeveloperDebugger(bool toggle);
    void slotVisitUrl(const QString& url);

  private:
    void resetFeatureFlags();
    void initialize();
    VisibilityControlConnection* createVisibilityControl(QAction* pAction,
            const ConfigKey& key,
            ControlObject* m_feature = nullptr);

    UserSettingsPointer m_pConfig;
    ConfigObject<ConfigValueKbd>* m_pKbdConfig;
    QList<QAction*> m_loadToDeckActions;
    QList<QAction*> m_vinylControlEnabledActions;
    QList<VisibilityControlConnection*> m_visibilityConnections;
    QAction* m_pFileQuit;
    QAction* m_pLibraryRescan;
    QAction* m_pLibraryCreatePlaylist;
    QAction* m_pLibraryCreateCrate;
    QAction* m_pViewShowSkinSettings;
    QAction* m_pViewShowMicrophone;
    QAction* m_pViewShowCoverArt;
    QAction* m_pViewShowPreviewDeck;
    QAction* m_pViewShowMenuBar;
    QAction* m_pViewVinylControl;
    QAction* m_pViewFullScreen;
    QAction* m_pViewMaximizeLibrary;
    QAction* m_pOptionsRecord;
    QAction* m_pOptionsBroadcasting;
    QAction* m_pOptionsKeyboard;
    QAction* m_pOptionsPreferences;
    QAction* m_pDeveloperReloadSkin;
    QAction* m_pDeveloperTools;
    QAction* m_pDeveloperStatsExperiment;
    QAction* m_pDeveloperStatsBase;
    QAction* m_pDeveloperDebugger;
    QAction* m_pHelpSupport;
    QAction* m_pHelpManual;
    QAction* m_pHelpShortcuts;
    QAction* m_pHelpFeedback;
    QAction* m_pHelpTranslation;
    QAction* m_pHelpAboutApp;

    ControlObject* m_pFeatureCOHideMenubar;
    ControlObject* m_pFeatureCOSkinSettings;

    unsigned int m_lastNumPlayers;
    VisibilityControlConnection* m_pMenubarConnection;
};
