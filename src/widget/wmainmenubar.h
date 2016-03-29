#ifndef WIDGET_WMAINMENUBAR
#define WIDGET_WMAINMENUBAR

#include <QAction>
#include <QList>
#include <QMenuBar>
#include <QObject>
#include <QScopedPointer>
#include <QSignalMapper>

#include "configobject.h"
#include "controlobjectslave.h"
#include "preferences/usersettings.h"

class VisibilityControlConnection : public QObject {
    Q_OBJECT
  public:
    VisibilityControlConnection(QObject* pParent, QAction* pAction,
                                const ConfigKey& key);
    virtual ~VisibilityControlConnection();

  private slots:
    void slotClearControl();
    void slotReconnectControl();
    void slotControlChanged();
    void slotActionToggled(bool toggle);

  private:
    ConfigKey m_key;
    QScopedPointer<ControlObjectSlave> m_pControl;
    QAction* m_pAction;
};

class WMainMenuBar : public QMenuBar {
    Q_OBJECT
  public:
    WMainMenuBar(QWidget* pParent, UserSettingsPointer pSettings,
                 ConfigObject<ConfigValueKbd>* pKbdConfig);
    virtual ~WMainMenuBar();

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
    void visitUrl(QString url);
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
    void slotDeveloperDebugger(bool enable);
    void slotVisitUrl(const QString& url);

  private:
    void initialize();
    void createVisibilityControl(QAction* pAction, const ConfigKey& key);

    UserSettingsPointer m_pConfig;
    ConfigObject<ConfigValueKbd>* m_pKbdConfig;
    QSignalMapper m_loadToDeckMapper;
    QSignalMapper m_visitUrlMapper;
    QList<QAction*> m_loadToDeckActions;
    QSignalMapper m_vinylControlEnabledMapper;
    QList<QAction*> m_vinylControlEnabledActions;
};

#endif /* WIDGET_WMAINMENUBAR */
