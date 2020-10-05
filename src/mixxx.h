#pragma once

#include <QMainWindow>
#include <QSharedPointer>
#include <QString>
#include <memory>

#include "preferences/configobject.h"
#include "preferences/constants.h"
#include "preferences/usersettings.h"
#include "soundio/sounddeviceerror.h"
#include "track/track_decl.h"
#include "util/cmdlineargs.h"
#include "util/db/dbconnectionpool.h"
#include "util/parented_ptr.h"
#include "util/timer.h"

class BroadcastManager;
class ChannelHandleFactory;
class ControllerManager;
class ControlPushButton;
class DlgDeveloperTools;
class DlgPreferences;
class EffectsManager;
class EngineMaster;
class GuiTick;
class KeyboardEventFilter;
class LaunchImage;
class Library;
class PlayerManager;
class RecordingManager;
class SettingsManager;
class SkinLoader;
class SoundManager;
class TrackCollectionManager;
class VinylControlManager;
class VisualsManager;
class WMainMenuBar;

/// This Class is the base class for Mixxx.
/// It sets up the main window providing a menubar.
/// For the main view, an instance of class MixxxView is
/// created which creates your view.
class MixxxMainWindow : public QMainWindow {
    Q_OBJECT
  public:
    MixxxMainWindow(QApplication *app, const CmdlineArgs& args);
    ~MixxxMainWindow() override;

    void finalize();

    /// creates the menu_bar and inserts the file Menu
    void createMenuBar();
    void connectMenuBar();
    void setInhibitScreensaver(mixxx::ScreenSaverPreference inhibit);
    mixxx::ScreenSaverPreference getInhibitScreensaver();

    void setToolTipsCfg(mixxx::TooltipsPreference tt);
    inline mixxx::TooltipsPreference getToolTipsCfg() { return m_toolTipsCfg; }

    inline GuiTick* getGuiTick() { return m_pGuiTick; };

  public slots:
    void rebootMixxxView();

    void slotFileLoadSongPlayer(int deck);
    /// toggle keyboard on-off
    void slotOptionsKeyboard(bool toggle);
    /// show the preferences dialog
    void slotOptionsPreferences();
    /// show the about dialog
    void slotHelpAbout();
    /// toggle full screen mode
    void slotViewFullScreen(bool toggle);
    /// open the developer tools dialog.
    void slotDeveloperTools(bool enable);
    void slotDeveloperToolsClosed();

    void slotUpdateWindowTitle(TrackPointer pTrack);
    void slotChangedPlayingDeck(int deck);

    /// warn the user when inputs are not configured.
    void slotNoMicrophoneInputConfigured();
    void slotNoAuxiliaryInputConfigured();
    void slotNoDeckPassthroughInputConfigured();
    void slotNoVinylControlInputConfigured();

  signals:
    void skinLoaded();
    /// used to uncheck the menu when the dialog of developer tools is closed
    void developerToolsDlgClosed(int r);
    void closeDeveloperToolsDlgChecked(int r);
    void fullScreenChanged(bool fullscreen);

  protected:
    /// Event filter to block certain events (eg. tooltips if tooltips are disabled)
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent* e) override;

  private:
    void initialize(QApplication *app, const CmdlineArgs& args);

    /// progresses the launch image progress bar
    /// this must be called from the GUI thread only
    void launchProgress(int progress);

    void initializeWindow();
    void initializeKeyboard();
    void checkDirectRendering();

    bool initializeDatabase();

    /// Load skin to a QWidget that we set as the central widget.
    bool loadConfiguredSkin();

    bool confirmExit();
    QDialog::DialogCode soundDeviceErrorDlg(
            const QString &title, const QString &text, bool* retryClicked);
    QDialog::DialogCode soundDeviceBusyDlg(bool* retryClicked);
    QDialog::DialogCode soundDeviceErrorMsgDlg(
            SoundDeviceError err, bool* retryClicked);
    QDialog::DialogCode noOutputDlg(bool* continueClicked);

    QWidget* m_pCentralWidget;
    LaunchImage* m_pLaunchImage;

    std::unique_ptr<SettingsManager> m_pSettingsManager;

    /// The effects processing system
    EffectsManager* m_pEffectsManager;

    /// The mixing engine
    EngineMaster* m_pEngine;

    SkinLoader* m_pSkinLoader; // TODO(rryan): doesn't need to be a member variable

    SoundManager* m_pSoundManager;

    PlayerManager* m_pPlayerManager;
    RecordingManager* m_pRecordingManager;
#ifdef __BROADCAST__
    BroadcastManager* m_pBroadcastManager;
#endif
    ControllerManager* m_pControllerManager;

    GuiTick* m_pGuiTick;
    VisualsManager* m_pVisualsManager;

    VinylControlManager* m_pVCManager;

    KeyboardEventFilter* m_pKeyboard;

    mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    TrackCollectionManager* m_pTrackCollectionManager;

    Library* m_pLibrary;

    parented_ptr<WMainMenuBar> m_pMenuBar;

    DlgDeveloperTools* m_pDeveloperToolsDlg;

    DlgPreferences* m_pPrefDlg;

    ConfigObject<ConfigValueKbd>* m_pKbdConfig;
    ConfigObject<ConfigValueKbd>* m_pKbdConfigEmpty;

    mixxx::TooltipsPreference m_toolTipsCfg;
    /// tracks how long Mixxx has been running
    Timer m_runtime_timer;

    const CmdlineArgs& m_cmdLineArgs;

    ControlPushButton* m_pTouchShift;
    mixxx::ScreenSaverPreference m_inhibitScreensaver;

    QSet<ControlObject*> m_skinCreatedControls;

    static const int kMicrophoneCount;
    static const int kAuxiliaryCount;
};
