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
class SkinLoader;
class VinylControlManager;
class CmdlineArgs;

class DlgPreferences;
class SoundManager;

#include "configobject.h"

/**
  * This Class is the base class for Mixxx. It sets up the main
  * window and providing a menubar.
  * For the main view, an instance of class MixxxView is
  * created which creates your view.
  */
class MixxxApp : public QMainWindow {
    Q_OBJECT

  public:
    /** Construtor. files is a list of command line arguments */
    MixxxApp(QApplication *app, const CmdlineArgs& args);
    virtual ~MixxxApp();
    /** initializes all QActions of the application */
    void initActions();
    /** initMenuBar creates the menu_bar and inserts the menuitems */
    void initMenuBar();

    void resizeEvent(QResizeEvent *e) { qDebug() << "resize" << e->size();}

    void rebootMixxxView();

  public slots:

    //void slotQuitFullScreen();
    void slotFileLoadSongPlayer(int deck);
    /** Opens a file in player 1 */
    void slotFileLoadSongPlayer1();
    /** Opens a file in player 2 */
    void slotFileLoadSongPlayer2();
    /** exits the application */
    void slotFileQuit();

    /** toggle vinyl control - Don't #ifdef this because MOC is dumb**/
    void slotControlVinylControl(double toggle);
    void slotCheckboxVinylControl(bool toggle);
    void slotControlVinylControl2(double toggle);
    void slotCheckboxVinylControl2(bool toggle);
    /** toggle recording - Don't #ifdef this because MOC is dumb**/
    void slotOptionsRecord(bool toggle);
    /** toogle keyboard on-off */
    void slotOptionsKeyboard(bool toggle);
    /** toogle full screen mode */
    void slotOptionsFullScreen(bool toggle);
    /** Preference dialog */
    void slotOptionsPreferences();
    /** shows an about dlg*/
    void slotHelpAbout();
    /** visits support section of website*/
    void slotHelpSupport();
    // Visits a feedback form
    void slotHelpFeedback();
    // Open the manual.
    void slotHelpManual();
    // Visits translation interface on launchpad.net
    void slotHelpTranslation();
    /** Scan or rescan the music library directory */
    void slotScanLibrary();
    /** Enables the "Rescan Library" menu item. This gets disabled when a scan is running.*/
    void slotEnableRescanLibraryAction();
    /**Updates the checkboxes for Recording and Livebroadcasting when connection drops, or lame is not available **/
    void slotOptionsMenuShow();
    /** toggles Livebroadcasting **/
    void slotOptionsShoutcast(bool value);

    void slotToCenterOfPrimaryScreen();

  protected:
    /** Event filter to block certain events (eg. tooltips if tooltips are disabled) */
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);

  private:
    void checkDirectRendering();
    bool confirmExit();

    // Pointer to the root GUI widget
    QWidget* m_pView;
    QWidget* m_pWidgetParent;

    // The mixing engine.
    EngineMaster *m_pEngine;

    // The skin loader
    SkinLoader* m_pSkinLoader;

    // The sound manager
    SoundManager *m_pSoundManager;

    // Keeps track of players
    PlayerManager* m_pPlayerManager;
    // RecordingManager
    RecordingManager* m_pRecordingManager;
    ControllerManager *m_pControllerManager;

    ConfigObject<ConfigValue> *m_pConfig;

    VinylControlManager *m_pVCManager;

    MixxxKeyboard* m_pKeyboard;
    /** Library scanner object */
    LibraryScanner* m_pLibraryScanner;
    // The library management object
    Library* m_pLibrary;

    /** file_menu contains all items of the menubar entry "File" */
    QMenu *m_pFileMenu;
    /** edit_menu contains all items of the menubar entry "Edit" */
    QMenu *m_pEditMenu;
    /** library menu */
    QMenu *m_pLibraryMenu;
    /** options_menu contains all items of the menubar entry "Options" */
    QMenu *m_pOptionsMenu;
    /** view_menu contains all items of the menubar entry "View" */
    QMenu *m_pViewMenu;
    /** view_menu contains all items of the menubar entry "Help" */
    QMenu *m_pHelpMenu;

    QAction *m_pFileLoadSongPlayer1;
    QAction *m_pFileLoadSongPlayer2;
    QAction *m_pFileQuit;
    QAction *m_pPlaylistsNew;
    QAction *m_pCratesNew;
    QAction *m_pLibraryRescan;
#ifdef __VINYLCONTROL__
    QMenu *m_pVinylControlMenu;
    QAction *m_pOptionsVinylControl;
    QAction *m_pOptionsVinylControl2;
#endif
    QAction *m_pOptionsRecord;
    QAction *m_pOptionsKeyboard;
    QAction *m_pOptionsFullScreen;
    QAction *m_pOptionsPreferences;
#ifdef __SHOUTCAST__
    QAction *m_pOptionsShoutcast;
#endif
    QAction *m_pHelpAboutApp;
    QAction *m_pHelpSupport;
    QAction *m_pHelpFeedback;
    QAction *m_pHelpTranslation;
    QAction *m_pHelpManual;

    int m_iNoPlaylists;

    /** Pointer to preference dialog */
    DlgPreferences *m_pPrefDlg;

    int noSoundDlg(void);
    int noOutputDlg(bool *continueClicked);
    // Fullscreen patch
    QPoint m_winpos;
    bool m_NativeMenuBarSupport;

    ConfigObject<ConfigValueKbd>* m_pKbdConfig;
    ConfigObject<ConfigValueKbd>* m_pKbdConfigEmpty;
};

//A structure to store the parsed command-line arguments
class CmdlineArgs
{
public:
    static CmdlineArgs& Instance() {
        static CmdlineArgs cla;
        return cla;
    }
    bool Parse(int &argc, char **argv) {
        for (int i = 0; i < argc; ++i) {
            if (   argv[i] == QString("-h")
                || argv[i] == QString("--h")
                || argv[i] == QString("--help")) {
                return false; // Display Help Message
            }

            if (argv[i]==QString("-f").toLower() || argv[i]==QString("--f") || argv[i]==QString("--fullScreen"))
            {
                m_startInFullscreen = true;
            } else if (argv[i] == QString("--locale") && i+1 < argc) {
                m_locale = argv[i+1];
            } else if (argv[i] == QString("--settingsPath") && i+1 < argc) {
                m_settingsPath = QString::fromLocal8Bit(argv[i+1]);
                if (!m_settingsPath.endsWith("/")) {
                    m_settingsPath.append("/");
                }
            } else if (argv[i] == QString("--resourcePath") && i+1 < argc) {
                m_resourcePath = QString::fromLocal8Bit(argv[i+1]);
            } else if (argv[i] == QString("--pluginPath") && i+1 < argc) {
                m_pluginPath = QString::fromLocal8Bit(argv[i+1]);
            } else if (QString::fromLocal8Bit(argv[i]).contains("--midiDebug", Qt::CaseInsensitive)) {
                m_midiDebug = true;
            } else {
                m_musicFiles += QString::fromLocal8Bit(argv[i]);
            }
        }
        return true;
    }
    const QList<QString>& getMusicFiles() const { return m_musicFiles; };
    bool getStartInFullscreen() const { return m_startInFullscreen; };
    bool getMidiDebug() const { return m_midiDebug; };
    const QString& getLocale() const { return m_locale; };
    const QString& getSettingsPath() const { return m_settingsPath; };
    const QString& getResourcePath() const { return m_resourcePath; };
    const QString& getPluginPath() const { return m_pluginPath; };

private:
    CmdlineArgs() :
        m_startInFullscreen(false), //Initialize vars
        m_midiDebug(false),
        m_settingsPath(QDir::homePath().append("/").append(SETTINGS_PATH)) {
    }
    ~CmdlineArgs() { };
    QList<QString> m_musicFiles;    /* List of files to load into players at startup */
    bool m_startInFullscreen;       /* Start in fullscreen mode */
    bool m_midiDebug;
    QString m_locale;
    QString m_settingsPath;
    QString m_resourcePath;
    QString m_pluginPath;
};


#endif
