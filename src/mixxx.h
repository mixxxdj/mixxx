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

#include <QMainWindow>
#include <QString>

#include "preferences/configobject.h"
#include "preferences/usersettings.h"
#include "preferences/constants.h"
#include "track/track.h"
#include "util/cmdlineargs.h"
#include "util/timer.h"

class ControlPushButton;
class ControllerManager;
class DlgDeveloperTools;
class DlgPreferences;
class EffectsManager;
class EngineMaster;
class GuiTick;
class LaunchImage;
class Library;
class KeyboardEventFilter;
class PlayerManager;
class RecordingManager;
class SettingsManager;
class BroadcastManager;
class SkinLoader;
class SoundManager;
class VinylControlManager;
class WMainMenuBar;

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

    void initialize(QApplication *app, const CmdlineArgs& args);
    void finalize();

    // creates the menu_bar and inserts the file Menu
    void createMenuBar();
    void connectMenuBar();

    void setToolTipsCfg(mixxx::TooltipsPreference tt);
    inline mixxx::TooltipsPreference getToolTipsCfg() { return m_toolTipsCfg; }

    inline GuiTick* getGuiTick() { return m_pGuiTick; };

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

  protected:
    // Event filter to block certain events (eg. tooltips if tooltips are disabled)
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    virtual bool event(QEvent* e);

  private:
    // progresses the launch image progress bar
    // this must be called from the GUi thread only
    void launchProgress(int progress);
    void initializeWindow();
    void initializeKeyboard();
    void checkDirectRendering();
    bool confirmExit();
    int noSoundDlg(void);
    int noOutputDlg(bool* continueClicked);

    // Pointer to the root GUI widget
    QWidget* m_pWidgetParent;
    LaunchImage* m_pLaunchImage;

    SettingsManager* m_pSettingsManager;

    // The effects processing system
    EffectsManager* m_pEffectsManager;

    // The mixing engine.
    EngineMaster* m_pEngine;

    // The skin loader.
    // TODO(rryan): doesn't need to be a member variable
    SkinLoader* m_pSkinLoader;

    // The sound manager
    SoundManager* m_pSoundManager;

    // Keeps track of players
    PlayerManager* m_pPlayerManager;
    // RecordingManager
    RecordingManager* m_pRecordingManager;
#ifdef __BROADCAST__
    BroadcastManager* m_pBroadcastManager;
#endif
    ControllerManager* m_pControllerManager;

    GuiTick* m_pGuiTick;

    VinylControlManager* m_pVCManager;

    KeyboardEventFilter* m_pKeyboard;
    // The library management object
    Library* m_pLibrary;

    WMainMenuBar* m_pMenuBar;

    DlgDeveloperTools* m_pDeveloperToolsDlg;

    /** Pointer to preference dialog */
    DlgPreferences* m_pPrefDlg;

    ConfigObject<ConfigValueKbd>* m_pKbdConfig;
    ConfigObject<ConfigValueKbd>* m_pKbdConfigEmpty;

    mixxx::TooltipsPreference m_toolTipsCfg;
    // Timer that tracks how long Mixxx has been running.
    Timer m_runtime_timer;

    const CmdlineArgs& m_cmdLineArgs;

    ControlPushButton* m_pTouchShift;

    static const int kMicrophoneCount;
    static const int kAuxiliaryCount;
};

#endif
