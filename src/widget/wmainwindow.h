#ifndef WIDGET_WMAINWINDOW_H
#define WIDGET_WMAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include "preferences/configobject.h"
#include "preferences/usersettings.h"
#include "preferences/constants.h"
#include "skin/skinloader.h"
#include "track/track.h"
#include "util/cmdlineargs.h"
#include "util/timer.h"
#include "soundio/sounddeviceerror.h"

namespace mixxx {
class CoreServices;
}  // namespace mixxx
class ControlPushButton;
class DlgDeveloperTools;
class DlgPreferences;
class LaunchImage;
class KeyboardEventFilter;
class WMainMenuBar;

class WMainWindow : public QMainWindow {
    Q_OBJECT
  public:
    WMainWindow(QApplication* app, std::shared_ptr<mixxx::CoreServices> pCoreServices);
    ~WMainWindow() override;

    void initialize(QApplication *app);

    // creates the menu_bar and inserts the file Menu
    void createMenuBar();
    void connectMenuBar();

    void setInhibitScreensaver(mixxx::ScreenSaverPreference inhibit);
    mixxx::ScreenSaverPreference getInhibitScreensaver();

    void setToolTipsCfg(mixxx::TooltipsPreference tt);
    inline mixxx::TooltipsPreference getToolTipsCfg() { return m_toolTipsCfg; }

  public slots:
    void rebootMixxxView();

    void slotFileLoadSongPlayer(int deck);
    // toogle keyboard on-off
    void slotOptionsKeyboard(bool toggle);
    // Preference dialog
    void slotOptionsPreferences();
    // shows an about dlg
    void slotHelpAbout();
    // toogle full screen mode
    void slotViewFullScreen(bool toggle);
    // Open the developer tools dialog.
    void slotDeveloperTools(bool enable);
    void slotDeveloperToolsClosed();

    void slotUpdateWindowTitle(TrackPointer pTrack);
    void slotChangedPlayingDeck(int deck);

    // Warn the user when inputs are not configured.
    void slotNoMicrophoneInputConfigured();
    void slotNoDeckPassthroughInputConfigured();
    void slotNoVinylControlInputConfigured();

  signals:
    void newSkinLoaded();
    // used to uncheck the menu when the dialog of develeoper tools is closed
    void developerToolsDlgClosed(int r);
    void closeDeveloperToolsDlgChecked(int r);
    void fullScreenChanged(bool fullscreen);
    void launchProgress(int progress);

  protected:
    // Event filter to block certain events (eg. tooltips if tooltips are disabled)
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    virtual bool event(QEvent* e);

  private:
    void initializeWindow();
    void initializeKeyboard();
    void checkDirectRendering();
    bool confirmExit();
    QDialog::DialogCode soundDeviceErrorDlg(
            const QString &title, const QString &text, bool* retryClicked);
    QDialog::DialogCode soundDeviceBusyDlg(bool* retryClicked);
    QDialog::DialogCode soundDeviceErrorMsgDlg(
            SoundDeviceError err, bool* retryClicked);
    QDialog::DialogCode noOutputDlg(bool* continueClicked);

    LaunchImage* m_pLaunchImage;

    // Pointer to the Mixxx session. Not owned by WMainWindow.
    std::shared_ptr<mixxx::CoreServices> m_pCore;

    // The skin loader.
    // TODO(rryan): doesn't need to be a member variable
    SkinLoader m_skinLoader;

    KeyboardEventFilter* m_pKeyboard;

    WMainMenuBar* m_pMenuBar;

    DlgDeveloperTools* m_pDeveloperToolsDlg;
    DlgPreferences* m_pPrefDlg;

    ConfigObject<ConfigValueKbd>* m_pKbdConfig;
    ConfigObject<ConfigValueKbd>* m_pKbdConfigEmpty;

    mixxx::TooltipsPreference m_toolTipsCfg;

    ControlPushButton* m_pTouchShift;
    mixxx::ScreenSaverPreference m_inhibitScreensaver;
};

#endif
