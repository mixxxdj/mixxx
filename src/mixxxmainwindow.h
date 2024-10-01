#pragma once

#include <QMainWindow>
#include <QString>
#include <memory>

#include "preferences/constants.h"
#include "soundio/sounddevicestatus.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class ControlObject;
class DlgDeveloperTools;
class DlgPreferences;
class DlgKeywheel;
class GuiTick;
class LaunchImage;
class VisualsManager;
class WMainMenuBar;
struct LibraryScanResultSummary;

namespace mixxx {

class CoreServices;

namespace skin {
class SkinLoader;
}

#ifdef __ENGINEPRIME__
class LibraryExporter;
#endif

} // namespace mixxx

/// This Class is the base class for Mixxx.
/// It sets up the main window providing a menubar.
/// For the main view, an instance of class MixxxView is
/// created which creates your view.
class MixxxMainWindow : public QMainWindow {
    Q_OBJECT
  public:
    MixxxMainWindow(std::shared_ptr<mixxx::CoreServices> pCoreServices);
    ~MixxxMainWindow() override;

#ifdef MIXXX_USE_QOPENGL
    void initializeQOpenGL();
#endif
    /// Initialize main window after creation. Should only be called once.
    void initialize();
    /// creates the menu_bar and inserts the file Menu
    void createMenuBar();
    void connectMenuBar();
    void setInhibitScreensaver(mixxx::preferences::ScreenSaver inhibit);
    mixxx::preferences::ScreenSaver getInhibitScreensaver();

    inline GuiTick* getGuiTick() { return m_pGuiTick; };

  public slots:
    void rebootMixxxView();

    void slotFileLoadSongPlayer(int deck);
    /// show the preferences dialog
    void slotOptionsPreferences();
    /// show the about dialog
    void slotHelpAbout();
    /// show popup with library scan results
    void slotLibraryScanSummaryDlg(const LibraryScanResultSummary& result);
    /// show keywheel
    void slotShowKeywheel(bool toggle);
    /// toggle full screen mode
    void slotViewFullScreen(bool toggle);
    /// open the developer tools dialog.
    void slotDeveloperTools(bool enable);
    void slotDeveloperToolsClosed();

    void slotUpdateWindowTitle(TrackPointer pTrack);

    /// warn the user when inputs are not configured.
    void slotNoMicrophoneInputConfigured();
    void slotNoAuxiliaryInputConfigured();
    void slotNoDeckPassthroughInputConfigured();
    void slotNoVinylControlInputConfigured();
#ifndef __APPLE__
    /// Update whether the menubar is toggled pressing the Alt key and show/hide
    /// it accordingly
    void slotUpdateMenuBarAltKeyConnection();
#endif

    void initializationProgressUpdate(int progress, const QString& serviceName);

  private slots:
    void slotTooltipModeChanged(mixxx::preferences::Tooltips tt);

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

  private:
    void initializeWindow();
    void checkDirectRendering();
    // EveOSC
    void oscEnable();
    // EveOSC

    /// Load skin to a QWidget that we set as the central widget.
    bool loadConfiguredSkin();
    void tryParseAndSetDefaultStyleSheet();

    bool confirmExit();
#ifndef __APPLE__
    void alwaysHideMenuBarDlg();
#endif

    QDialog::DialogCode soundDeviceErrorDlg(
            const QString &title, const QString &text, bool* retryClicked);
    QDialog::DialogCode soundDeviceBusyDlg(bool* retryClicked);
    QDialog::DialogCode soundDeviceErrorMsgDlg(
            SoundDeviceStatus status, bool* retryClicked);
    QDialog::DialogCode noOutputDlg(bool* continueClicked);

    std::shared_ptr<mixxx::CoreServices> m_pCoreServices;

    QWidget* m_pCentralWidget;
    LaunchImage* m_pLaunchImage;
#ifndef __APPLE__
    Qt::WindowStates m_prevState;
#endif

    parented_ptr<QMessageBox> m_noVinylInputDialog;
    parented_ptr<QMessageBox> m_noPassthroughInputDialog;
    parented_ptr<QMessageBox> m_noMicInputDialog;
    parented_ptr<QMessageBox> m_noAuxInputDialog;

    std::shared_ptr<mixxx::skin::SkinLoader> m_pSkinLoader;
    GuiTick* m_pGuiTick;
    VisualsManager* m_pVisualsManager;

    parented_ptr<WMainMenuBar> m_pMenuBar;
#ifdef __LINUX__
    const bool m_supportsGlobalMenuBar;
#endif
    bool m_inRebootMixxxView;

    DlgDeveloperTools* m_pDeveloperToolsDlg;

    DlgPreferences* m_pPrefDlg;
    parented_ptr<DlgKeywheel> m_pKeywheel;

#ifdef __ENGINEPRIME__
    // Library exporter
    std::unique_ptr<mixxx::LibraryExporter> m_pLibraryExporter;
#endif

    mixxx::preferences::Tooltips m_toolTipsCfg;

    mixxx::preferences::ScreenSaver m_inhibitScreensaver;

    QSet<ControlObject*> m_skinCreatedControls;
};
