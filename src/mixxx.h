/***************************************************************************
                          mixxx.h  -  description
                             -------------------
    begin                : Mon Feb 18 09:48:17 CET 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MIXXX_H
#define MIXXX_H

#include <QAction>
#include <QList>
#include <QMainWindow>
#include <QString>
#include <QDir>

// REMOVE ME
#include <QtDebug>
#include <QResizeEvent>

class EngineMaster;
class Library;
class LibraryScanner;
class ControllerManager;
class MixxxKeyboard;
class PlayerManager;
class RecordingManager;
class ShoutcastManager;
class SkinLoader;
class EffectsManager;
class VinylControlManager;
class GuiTick;
class DlgPreferences;
class SoundManager;
class ControlPushButton;
class DlgDeveloperTools;

#include "configobject.h"
#include "util/cmdlineargs.h"
#include "util/timer.h"

class ControlObjectSlave;
class ControlObjectThread;
class QTranslator;

// This Class is the base class for Mixxx. It sets up the main
// window and providing a menubar.
// For the main view, an instance of class MixxxView is
// created which creates your view.
class MixxxMainWindow : public QMainWindow {
    Q_OBJECT

  public:
    // Construtor. files is a list of command line arguments
    MixxxMainWindow(QApplication *app, const CmdlineArgs& args);
    virtual ~MixxxMainWindow();
    // initializes all QActions of the application
    void initActions();
    // initMenuBar creates the menu_bar and inserts the menuitems
    void initMenuBar();

    void setToolTipsCfg(int tt);
    inline int getToolTipsCgf() { return m_toolTipsCfg; }
    void rebootMixxxView();

    inline GuiTick* getGuiTick() { return m_pGuiTick; };

  public slots:

    //void slotQuitFullScreen();
    void slotFileLoadSongPlayer(int deck);
    // Opens a file in player 1
    void slotFileLoadSongPlayer1();
    // Opens a file in player 2
    void slotFileLoadSongPlayer2();
    // exits the application
    void slotFileQuit();

    // toggle vinyl control - Don't #ifdef this because MOC is dumb
    void slotControlVinylControl(int);
    void slotCheckboxVinylControl(int);
    void slotControlPassthrough(int);
    void slotControlAuxiliary(int);
    // toogle keyboard on-off
    void slotOptionsKeyboard(bool toggle);
    // Preference dialog
    void slotOptionsPreferences();
    // shows an about dlg
    void slotHelpAbout();
    // visits support section of website
    void slotHelpSupport();
    // Visits a feedback form
    void slotHelpFeedback();
    // Open the manual.
    void slotHelpManual();
    // Visits translation interface on launchpad.net
    void slotHelpTranslation();
    // Scan or rescan the music library directory
    void slotScanLibrary();
    // Enables the "Rescan Library" menu item. This gets disabled when a scan is running.
    void slotEnableRescanLibraryAction();
    //Updates the checkboxes for Recording and Livebroadcasting when connection drops, or lame is not available
    void slotOptionsMenuShow();
    // toogle on-screen widget visibility
    void slotViewShowSamplers(bool);
    void slotViewShowVinylControl(bool);
    void slotViewShowMicrophone(bool);
    void slotViewShowPreviewDeck(bool);
    void slotViewShowCoverArt(bool);
    // toogle full screen mode
    void slotViewFullScreen(bool toggle);
    // Reload the skin.
    void slotDeveloperReloadSkin(bool toggle);
    // Open the developer tools dialog.
    void slotDeveloperTools();
    void slotDeveloperToolsClosed();
    void slotDeveloperStatsExperiment();
    void slotDeveloperStatsBase();
    // toogle the script debugger
    void slotDeveloperDebugger(bool toggle);

    void slotToCenterOfPrimaryScreen();

    void onNewSkinLoaded();

    // Activated when the number of decks changed, so we can update the UI.
    void slotNumDecksChanged(double);

    // Activated when the talkover button is pushed on a microphone so we
    // can alert the user if a mic is not configured.
    void slotTalkoverChanged(int);

  signals:
    void newSkinLoaded();
    void libraryScanStarted();
    void libraryScanFinished();

  protected:
    // Event filter to block certain events (eg. tooltips if tooltips are disabled)
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    virtual bool event(QEvent* e);

  private:
    void logBuildDetails();
    void initializeWindow();
    void initializeKeyboard();
    void initializeTranslations(QApplication* pApp);
    bool loadTranslations(const QLocale& systemLocale, QString userLocale,
                          const QString& translation, const QString& prefix,
                          const QString& translationPath, QTranslator* pTranslator);
    void checkDirectRendering();
    bool confirmExit();

    // Pointer to the root GUI widget
    QWidget* m_pWidgetParent;

    // The effects processing system
    EffectsManager* m_pEffectsManager;

    // The mixing engine.
    EngineMaster* m_pEngine;

    // The skin loader
    SkinLoader* m_pSkinLoader;

    // The sound manager
    SoundManager *m_pSoundManager;

    // Keeps track of players
    PlayerManager* m_pPlayerManager;
    // RecordingManager
    RecordingManager* m_pRecordingManager;
#ifdef __SHOUTCAST__
    ShoutcastManager* m_pShoutcastManager;
#endif
    ControllerManager* m_pControllerManager;

    ConfigObject<ConfigValue>* m_pConfig;

    GuiTick* m_pGuiTick;

    VinylControlManager* m_pVCManager;

    MixxxKeyboard* m_pKeyboard;
    // Library scanner object
    LibraryScanner* m_pLibraryScanner;
    // The library management object
    Library* m_pLibrary;

    // file_menu contains all items of the menubar entry "File"
    QMenu* m_pFileMenu;
    // edit_menu contains all items of the menubar entry "Edit"
    QMenu* m_pEditMenu;
    // library menu
    QMenu* m_pLibraryMenu;
    // options_menu contains all items of the menubar entry "Options"
    QMenu* m_pOptionsMenu;
    // view_menu contains all items of the menubar entry "View"
    QMenu* m_pViewMenu;
    // view_menu contains all items of the menubar entry "Help"
    QMenu* m_pHelpMenu;
    // Developer options.
    QMenu* m_pDeveloperMenu;

    QAction* m_pFileLoadSongPlayer1;
    QAction* m_pFileLoadSongPlayer2;
    QAction* m_pFileQuit;
    QAction* m_pPlaylistsNew;
    QAction* m_pCratesNew;
    QAction* m_pLibraryRescan;
#ifdef __VINYLCONTROL__
    QMenu* m_pVinylControlMenu;
    QList<QAction*> m_pOptionsVinylControl;
#endif
    QAction* m_pOptionsRecord;
    QAction* m_pOptionsKeyboard;

    QAction* m_pOptionsPreferences;
#ifdef __SHOUTCAST__
    QAction* m_pOptionsShoutcast;
#endif
    QAction* m_pViewShowSamplers;
    QAction* m_pViewVinylControl;
    QAction* m_pViewShowMicrophone;
    QAction* m_pViewShowPreviewDeck;
    QAction* m_pViewShowCoverArt;
    QAction* m_pViewFullScreen;
    QAction* m_pHelpAboutApp;
    QAction* m_pHelpSupport;
    QAction* m_pHelpFeedback;
    QAction* m_pHelpTranslation;
    QAction* m_pHelpManual;

    QAction* m_pDeveloperReloadSkin;
    QAction* m_pDeveloperTools;
    QAction* m_pDeveloperStatsExperiment;
    QAction* m_pDeveloperStatsBase;
    DlgDeveloperTools* m_pDeveloperToolsDlg;
    QAction* m_pDeveloperDebugger;

    int m_iNoPlaylists;

    /** Pointer to preference dialog */
    DlgPreferences* m_pPrefDlg;

    int noSoundDlg(void);
    int noOutputDlg(bool* continueClicked);
    // Fullscreen patch
    QPoint m_winpos;
    bool m_NativeMenuBarSupport;

    ConfigObject<ConfigValueKbd>* m_pKbdConfig;
    ConfigObject<ConfigValueKbd>* m_pKbdConfigEmpty;

    int m_toolTipsCfg; //0=OFF, 1=ON, 2=ON (only in Library)
    // Timer that tracks how long Mixxx has been running.
    Timer m_runtime_timer;

    const CmdlineArgs& m_cmdLineArgs;

    ControlPushButton* m_pTouchShift;
    QList<ControlObjectSlave*> m_pVinylControlEnabled;
    QList<ControlObjectSlave*> m_pPassthroughEnabled;
    QList<ControlObjectSlave*> m_pAuxiliaryPassthrough;
    ControlObjectThread* m_pNumDecks;
    int m_iNumConfiguredDecks;
    QList<ControlObjectSlave*> m_micTalkoverControls;
    QSignalMapper* m_VCControlMapper;
    QSignalMapper* m_VCCheckboxMapper;
    QSignalMapper* m_PassthroughMapper;
    QSignalMapper* m_AuxiliaryMapper;
    QSignalMapper* m_TalkoverMapper;

    static const int kMicrophoneCount;
    static const int kAuxiliaryCount;
};

#endif
