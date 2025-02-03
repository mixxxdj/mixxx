#include "widget/wmainmenubar.h"

#ifndef __APPLE__
#include <QApplication>
#include <QWindow>
#endif
#include <QUrl>

#include "config.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "moc_wmainmenubar.cpp"
#include "util/cmdlineargs.h"
#include "util/desktophelper.h"
#include "util/experiment.h"
#include "vinylcontrol/defs_vinylcontrol.h"

namespace {

constexpr int kMaxLoadToDeckActions = 4;
const QString kSkinGroup = QStringLiteral("[Skin]");

QString buildWhatsThis(const QString& title, const QString& text) {
    QString preparedTitle = title;
    return QString("%1\n\n%2").arg(preparedTitle.remove("&"), text);
}

#ifdef __VINYLCONTROL__
QString vinylControlDefaultKeyBinding(int deck) {
    // More bindings need to be defined if you increment
    // kMaximumVinylControlInputs.
    DEBUG_ASSERT(deck < kMaximumVinylControlInputs);
    switch (deck) {
        case 0: return QObject::tr("Ctrl+t");
        case 1: return QObject::tr("Ctrl+y");
        case 2: return QObject::tr("Ctrl+u");
        case 3: return QObject::tr("Ctrl+i");
        default: return QString();
    }
}
#endif // __VINYLCONTROL__

QString loadToDeckDefaultKeyBinding(int deck) {
    switch (deck) {
        case 0: return QObject::tr("Ctrl+o");
        case 1: return QObject::tr("Ctrl+Shift+O");
        default: return QString();
    }
}

QString showPreferencesKeyBinding() {
#ifdef __APPLE__
    return QObject::tr("Ctrl+,");
#else
    return QObject::tr("Ctrl+P");
#endif
}

QUrl documentationUrl(
        const QString& resourcePath, const QString& fileName, const QString& docUrl) {
    QDir resourceDir(resourcePath);
    // Documentation PDFs are included on Windows and Linux only,
    // so on macOS this always returns the web URL.
#if defined(MIXXX_INSTALL_DOCDIR_RELATIVE_TO_DATADIR)
    if (!resourceDir.exists(fileName)) {
        resourceDir.cd(MIXXX_INSTALL_DOCDIR_RELATIVE_TO_DATADIR);
    }
#endif
    if (resourceDir.exists(fileName)) {
        return QUrl::fromLocalFile(resourceDir.absoluteFilePath(fileName));
    } else {
        return QUrl(docUrl);
    }
}
}  // namespace

WMainMenuBar::WMainMenuBar(QWidget* pParent, UserSettingsPointer pConfig,
                           ConfigObject<ConfigValueKbd>* pKbdConfig)
        : QMenuBar(pParent),
          m_pConfig(pConfig),
          m_pKbdConfig(pKbdConfig) {
    setObjectName(QStringLiteral("MainMenu"));
    initialize();
}

void WMainMenuBar::initialize() {
    // FILE MENU
    QMenu* pFileMenu = new QMenu(tr("&File"), this);
#ifndef __APPLE__
    connectMenuToSlotShowMenuBar(pFileMenu);
#endif

    QString loadTrackText = tr("Load Track to Deck &%1");
    QString loadTrackStatusText = tr("Loads a track in deck %1");
    QString openText = tr("Open");
    for (unsigned int deck = 0; deck < kMaxLoadToDeckActions; ++deck) {
        QString playerLoadStatusText = loadTrackStatusText.arg(QString::number(deck + 1));
        QAction* pFileLoadSongToPlayer = new QAction(
            loadTrackText.arg(QString::number(deck + 1)), this);

        QString binding = m_pKbdConfig->getValue(
                ConfigKey("[KeyboardShortcuts]", QString("FileMenu_LoadDeck%1").arg(deck + 1)),
                loadToDeckDefaultKeyBinding(deck));
        if (!binding.isEmpty()) {
            pFileLoadSongToPlayer->setShortcut(QKeySequence(binding));
            pFileLoadSongToPlayer->setShortcutContext(Qt::ApplicationShortcut);
        }
        pFileLoadSongToPlayer->setStatusTip(playerLoadStatusText);
        pFileLoadSongToPlayer->setWhatsThis(
            buildWhatsThis(openText, playerLoadStatusText));
        // Visibility of load to deck actions is set in
        // WMainMenuBar::onNumberOfDecksChanged.
        pFileLoadSongToPlayer->setVisible(false);
        connect(pFileLoadSongToPlayer, &QAction::triggered,
                this, [this, deck] { emit loadTrackToDeck(deck + 1); });

        pFileMenu->addAction(pFileLoadSongToPlayer);
        m_loadToDeckActions.push_back(pFileLoadSongToPlayer);
    }

    pFileMenu->addSeparator();

    QString quitTitle = tr("&Exit");
    QString quitText = tr("Quits Mixxx");
    auto* pFileQuit = new QAction(quitTitle, this);
    pFileQuit->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(ConfigKey("[KeyboardShortcuts]", "FileMenu_Quit"),
                                                  tr("Ctrl+q"))));
    pFileQuit->setShortcutContext(Qt::ApplicationShortcut);
    pFileQuit->setStatusTip(quitText);
    pFileQuit->setWhatsThis(buildWhatsThis(quitTitle, quitText));
    pFileQuit->setMenuRole(QAction::QuitRole);
    connect(pFileQuit, &QAction::triggered, this, &WMainMenuBar::quit);
    pFileMenu->addAction(pFileQuit);

    addMenu(pFileMenu);

    // LIBRARY MENU
    QMenu* pLibraryMenu = new QMenu(tr("&Library"), this);
#ifndef __APPLE__
    connectMenuToSlotShowMenuBar(pLibraryMenu);
#endif

    QString rescanTitle = tr("&Rescan Library");
    QString rescanText = tr("Rescans library folders for changes to tracks.");
    auto* pLibraryRescan = new QAction(rescanTitle, this);
    pLibraryRescan->setShortcut(QKeySequence(m_pKbdConfig->getValue(
            ConfigKey("[KeyboardShortcuts]", "LibraryMenu_Rescan"),
            tr("Ctrl+Shift+L"))));
    pLibraryRescan->setStatusTip(rescanText);
    pLibraryRescan->setWhatsThis(buildWhatsThis(rescanTitle, rescanText));
    pLibraryRescan->setCheckable(false);
    connect(pLibraryRescan, &QAction::triggered, this, &WMainMenuBar::rescanLibrary);
    // Disable the action when a scan is active.
    connect(this, &WMainMenuBar::internalLibraryScanActive, pLibraryRescan, &QAction::setDisabled);
    pLibraryMenu->addAction(pLibraryRescan);

#ifdef __ENGINEPRIME__
    QString exportTitle = tr("E&xport Library to Engine Prime");
    QString exportText = tr("Export the library to the Engine Prime format");
    auto* pLibraryExport = new QAction(exportTitle, this);
    pLibraryExport->setStatusTip(exportText);
    pLibraryExport->setWhatsThis(buildWhatsThis(exportTitle, exportText));
    pLibraryExport->setCheckable(false);
    connect(pLibraryExport, &QAction::triggered, this, &WMainMenuBar::exportLibrary);
    pLibraryMenu->addAction(pLibraryExport);
#endif

    pLibraryMenu->addSeparator();

    QString createPlaylistTitle = tr("Create &New Playlist");
    QString createPlaylistText = tr("Create a new playlist");
    auto* pLibraryCreatePlaylist = new QAction(createPlaylistTitle, this);
    pLibraryCreatePlaylist->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(
                ConfigKey("[KeyboardShortcuts]", "LibraryMenu_NewPlaylist"),
                tr("Ctrl+n"))));
    pLibraryCreatePlaylist->setShortcutContext(Qt::ApplicationShortcut);
    pLibraryCreatePlaylist->setStatusTip(createPlaylistText);
    pLibraryCreatePlaylist->setWhatsThis(buildWhatsThis(createPlaylistTitle, createPlaylistText));
    connect(pLibraryCreatePlaylist, &QAction::triggered, this, &WMainMenuBar::createPlaylist);
    pLibraryMenu->addAction(pLibraryCreatePlaylist);

    QString createCrateTitle = tr("Create New &Crate");
    QString createCrateText = tr("Create a new crate");
    auto* pLibraryCreateCrate = new QAction(createCrateTitle, this);
    pLibraryCreateCrate->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(ConfigKey("[KeyboardShortcuts]",
                                                  "LibraryMenu_NewCrate"),
                                                  tr("Ctrl+Shift+N"))));
    pLibraryCreateCrate->setShortcutContext(Qt::ApplicationShortcut);
    pLibraryCreateCrate->setStatusTip(createCrateText);
    pLibraryCreateCrate->setWhatsThis(buildWhatsThis(createCrateTitle, createCrateText));
    connect(pLibraryCreateCrate, &QAction::triggered, this, &WMainMenuBar::createCrate);
    pLibraryMenu->addAction(pLibraryCreateCrate);

    addMenu(pLibraryMenu);

#if defined(__APPLE__)
    // Note: On macOS 10.11 ff. we have to deal with "automagic" menu items,
    // when ever a menu "View" is present. QT (as of 5.12.3) does not handle this for us.
    // Add an invisible suffix to the View item string so it doesn't string-equal "View" ,
    // and the magic menu items won't get injected.
    // https://github.com/mixxxdj/mixxx/issues/8442
    QMenu* pViewMenu = new QMenu(tr("&View") + QStringLiteral("\u200C"), this);
#else
    QMenu* pViewMenu = new QMenu(tr("&View"), this);
    connectMenuToSlotShowMenuBar(pViewMenu);
#endif

#ifndef __APPLE__
    // Show menu bar
    QString showMenuBarTitle = tr("Auto-hide menu bar");
    QString showMenuBarText = tr("Auto-hide the main menu bar when it's not used.");
    auto* pViewAutoHideMenuBar = new QAction(showMenuBarTitle, this);
    pViewAutoHideMenuBar->setCheckable(true);
    pViewAutoHideMenuBar->setStatusTip(showMenuBarText);
    pViewAutoHideMenuBar->setWhatsThis(buildWhatsThis(showMenuBarTitle, showMenuBarText));
    connect(pViewAutoHideMenuBar,
            &QAction::triggered,
            this,
            &WMainMenuBar::slotAutoHideMenuBarToggled);
    // update checked state from config each time the menu is shown
    connect(pViewMenu,
            &QMenu::aboutToShow,
            this,
            [this, pViewAutoHideMenuBar]() {
                bool autoHide = m_pConfig->getValue<bool>(
                        ConfigKey("[Config]", "hide_menubar"), false);
                pViewAutoHideMenuBar->setChecked(autoHide);
            });
    pViewMenu->addAction(pViewAutoHideMenuBar);

    pViewMenu->addSeparator();
#endif

    // Skin Settings Menu
    QString mayNotBeSupported = tr("May not be supported on all skins.");
    QString showSkinSettingsTitle = tr("Show Skin Settings Menu");
    QString showSkinSettingsText = tr("Show the Skin Settings Menu of the currently selected Skin") +
            " " + mayNotBeSupported;
    auto* pViewShowSkinSettings = new QAction(showSkinSettingsTitle, this);
    pViewShowSkinSettings->setCheckable(true);
    pViewShowSkinSettings->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(
            ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowSkinSettings"),
            tr("Ctrl+1", "Menubar|View|Show Skin Settings"))));
    pViewShowSkinSettings->setStatusTip(showSkinSettingsText);
    pViewShowSkinSettings->setWhatsThis(buildWhatsThis(showSkinSettingsTitle, showSkinSettingsText));
    createVisibilityControl(pViewShowSkinSettings,
            ConfigKey(kSkinGroup, QStringLiteral("show_settings")));
    pViewMenu->addAction(pViewShowSkinSettings);

    // Microphone Section
    QString showMicrophoneTitle = tr("Show Microphone Section");
    QString showMicrophoneText = tr("Show the microphone section of the Mixxx interface.") +
            " " + mayNotBeSupported;
    auto* pViewShowMicrophone = new QAction(showMicrophoneTitle, this);
    pViewShowMicrophone->setCheckable(true);
    pViewShowMicrophone->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(
            ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowMicrophone"),
            tr("Ctrl+2", "Menubar|View|Show Microphone Section"))));
    pViewShowMicrophone->setStatusTip(showMicrophoneText);
    pViewShowMicrophone->setWhatsThis(buildWhatsThis(showMicrophoneTitle, showMicrophoneText));
    createVisibilityControl(pViewShowMicrophone,
            ConfigKey(kSkinGroup, QStringLiteral("show_microphones")));
    pViewMenu->addAction(pViewShowMicrophone);

#ifdef __VINYLCONTROL__
    QString showVinylControlTitle = tr("Show Vinyl Control Section");
    QString showVinylControlText = tr("Show the vinyl control section of the Mixxx interface.") +
            " " + mayNotBeSupported;
    auto* pViewVinylControl = new QAction(showVinylControlTitle, this);
    pViewVinylControl->setCheckable(true);
    pViewVinylControl->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(
            ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowVinylControl"),
            tr("Ctrl+3", "Menubar|View|Show Vinyl Control Section"))));
    pViewVinylControl->setStatusTip(showVinylControlText);
    pViewVinylControl->setWhatsThis(buildWhatsThis(showVinylControlTitle, showVinylControlText));
    createVisibilityControl(pViewVinylControl,
            ConfigKey(kSkinGroup, QStringLiteral("show_vinylcontrol")));
    pViewMenu->addAction(pViewVinylControl);
#endif

    QString showPreviewDeckTitle = tr("Show Preview Deck");
    QString showPreviewDeckText = tr("Show the preview deck in the Mixxx interface.") +
            " " + mayNotBeSupported;
    auto* pViewShowPreviewDeck = new QAction(showPreviewDeckTitle, this);
    pViewShowPreviewDeck->setCheckable(true);
    pViewShowPreviewDeck->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(
                ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowPreviewDeck"),
                tr("Ctrl+4", "Menubar|View|Show Preview Deck"))));
    pViewShowPreviewDeck->setStatusTip(showPreviewDeckText);
    pViewShowPreviewDeck->setWhatsThis(buildWhatsThis(showPreviewDeckTitle, showPreviewDeckText));
    createVisibilityControl(pViewShowPreviewDeck,
            ConfigKey(kSkinGroup, QStringLiteral("show_preview_decks")));
    pViewMenu->addAction(pViewShowPreviewDeck);


    QString showCoverArtTitle = tr("Show Cover Art");
    QString showCoverArtText = tr("Show cover art in the Mixxx interface.") +
            " " + mayNotBeSupported;
    auto* pViewShowCoverArt = new QAction(showCoverArtTitle, this);
    pViewShowCoverArt->setCheckable(true);
    pViewShowCoverArt->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(
                ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowCoverArt"),
                tr("Ctrl+6", "Menubar|View|Show Cover Art"))));
    pViewShowCoverArt->setStatusTip(showCoverArtText);
    pViewShowCoverArt->setWhatsThis(buildWhatsThis(showCoverArtTitle, showCoverArtText));
    createVisibilityControl(pViewShowCoverArt,
            ConfigKey(kSkinGroup, QStringLiteral("show_library_coverart")));
    pViewMenu->addAction(pViewShowCoverArt);

    //: menu title
    QString keywheelTitle = tr("Show Keywheel");
    //: tooltip text
    QString keywheelText = tr("Show keywheel");
    m_pViewKeywheel = new QAction(keywheelTitle, this);
    m_pViewKeywheel->setCheckable(true);
    m_pViewKeywheel->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "ViewMenu_ShowKeywheel"),
                    tr("F12", "Menubar|View|Show Keywheel"))));
    m_pViewKeywheel->setShortcutContext(Qt::ApplicationShortcut);
    m_pViewKeywheel->setStatusTip(keywheelText);
    m_pViewKeywheel->setWhatsThis(buildWhatsThis(keywheelTitle, keywheelText));
    connect(m_pViewKeywheel, &QAction::triggered, this, &WMainMenuBar::showKeywheel);
    pViewMenu->addAction(m_pViewKeywheel);

    QString maximizeLibraryTitle = tr("Maximize Library");
    QString maximizeLibraryText = tr("Maximize the track library to take up all the available screen space.") +
            " " + mayNotBeSupported;
    auto* pViewMaximizeLibrary = new QAction(maximizeLibraryTitle, this);
    pViewMaximizeLibrary->setCheckable(true);
    pViewMaximizeLibrary->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(
                ConfigKey("[KeyboardShortcuts]", "ViewMenu_MaximizeLibrary"),
                tr("Space", "Menubar|View|Maximize Library"))));
    pViewMaximizeLibrary->setStatusTip(maximizeLibraryText);
    pViewMaximizeLibrary->setWhatsThis(buildWhatsThis(maximizeLibraryTitle, maximizeLibraryText));
    createVisibilityControl(pViewMaximizeLibrary,
            ConfigKey(kSkinGroup, QStringLiteral("show_maximized_library")));
    pViewMenu->addAction(pViewMaximizeLibrary);

    pViewMenu->addSeparator();

    QString fullScreenTitle = tr("&Full Screen");
    QString fullScreenText = tr("Display Mixxx using the full screen");
    auto* pViewFullScreen = new QAction(fullScreenTitle, this);
    QList<QKeySequence> shortcuts;
    // We use F11 _AND_ the OS shortcut only on Linux and Windows because on
    // newer macOS versions there might be issues with getting F11 working.
    // https://github.com/mixxxdj/mixxx/pull/3011#issuecomment-678678328
#ifndef __APPLE__
    shortcuts << QKeySequence("F11");
#endif
    QKeySequence osShortcut = QKeySequence::FullScreen;
    // Note(ronso0) Only add the OS shortcut if it's not empty and not F11.
    // In some Linux distros the window managers doesn't pass the OS fullscreen
    // key sequence to Mixxx for some reason.
    // Both adding an empty key sequence or the same sequence twice can render
    // the fullscreen shortcut nonfunctional.
    // https://github.com/mixxxdj/mixxx/issues/10005  PR #3011
    if (!osShortcut.isEmpty() && !shortcuts.contains(osShortcut)) {
        shortcuts << osShortcut;
    }

    pViewFullScreen->setShortcuts(shortcuts);
    pViewFullScreen->setShortcutContext(Qt::ApplicationShortcut);
    pViewFullScreen->setCheckable(true);
    pViewFullScreen->setChecked(false);
    pViewFullScreen->setStatusTip(fullScreenText);
    pViewFullScreen->setWhatsThis(buildWhatsThis(fullScreenTitle, fullScreenText));
    connect(pViewFullScreen, &QAction::triggered, this, &WMainMenuBar::toggleFullScreen);
    connect(this,
            &WMainMenuBar::internalFullScreenStateChange,
            pViewFullScreen,
            &QAction::setChecked);
    pViewMenu->addAction(pViewFullScreen);

    addMenu(pViewMenu);

    // OPTIONS MENU
    QMenu* pOptionsMenu = new QMenu(tr("&Options"), this);
#ifndef __APPLE__
    connectMenuToSlotShowMenuBar(pOptionsMenu);
#endif

#ifdef __VINYLCONTROL__
    QMenu* pVinylControlMenu = new QMenu(tr("&Vinyl Control"), this);
    QString vinylControlText = tr(
            "Use timecoded vinyls on external turntables to control Mixxx");

    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        QString vinylControlTitle = tr("Enable Vinyl Control &%1").arg(i + 1);
        auto* vc_checkbox = new QAction(vinylControlTitle, this);
        m_vinylControlEnabledActions.push_back(vc_checkbox);

        QString binding = m_pKbdConfig->getValue(
            ConfigKey("[KeyboardShortcuts]",
                    QString("OptionsMenu_EnableVinyl%1").arg(i + 1)),
            vinylControlDefaultKeyBinding(i));
        if (!binding.isEmpty()) {
            vc_checkbox->setShortcut(QKeySequence(binding));
            vc_checkbox->setShortcutContext(Qt::ApplicationShortcut);
        }

        // Either check or uncheck the vinyl control menu item depending on what
        // it was saved as.
        vc_checkbox->setCheckable(true);
        vc_checkbox->setChecked(false);
        // The visibility of these actions is set in
        // WMainMenuBar::onNumberOfDecksChanged.
        vc_checkbox->setVisible(false);
        vc_checkbox->setStatusTip(vinylControlText);
        vc_checkbox->setWhatsThis(buildWhatsThis(vinylControlTitle,
                                                 vinylControlText));
        connect(vc_checkbox, &QAction::triggered,
                this, [this, i] { emit toggleVinylControl(i); });
        pVinylControlMenu->addAction(vc_checkbox);
    }
    pOptionsMenu->addMenu(pVinylControlMenu);
    pOptionsMenu->addSeparator();
#endif

    QString recordTitle = tr("&Record Mix");
    QString recordText = tr("Record your mix to a file");
    auto* pOptionsRecord = new QAction(recordTitle, this);
    pOptionsRecord->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(
                ConfigKey("[KeyboardShortcuts]", "OptionsMenu_RecordMix"),
                tr("Ctrl+R"))));
    pOptionsRecord->setShortcutContext(Qt::ApplicationShortcut);
    pOptionsRecord->setCheckable(true);
    pOptionsRecord->setStatusTip(recordText);
    pOptionsRecord->setWhatsThis(buildWhatsThis(recordTitle, recordText));
    connect(pOptionsRecord, &QAction::triggered, this, &WMainMenuBar::toggleRecording);
    connect(this,
            &WMainMenuBar::internalRecordingStateChange,
            pOptionsRecord,
            &QAction::setChecked);
    pOptionsMenu->addAction(pOptionsRecord);

#ifdef __BROADCAST__
    QString broadcastingTitle = tr("Enable Live &Broadcasting");
    QString broadcastingText = tr("Stream your mixes to a shoutcast or icecast server");
    auto* pOptionsBroadcasting = new QAction(broadcastingTitle, this);
    pOptionsBroadcasting->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]",
                              "OptionsMenu_EnableLiveBroadcasting"),
                    tr("Ctrl+L"))));
    pOptionsBroadcasting->setShortcutContext(Qt::ApplicationShortcut);
    pOptionsBroadcasting->setCheckable(true);
    pOptionsBroadcasting->setStatusTip(broadcastingText);
    pOptionsBroadcasting->setWhatsThis(buildWhatsThis(broadcastingTitle, broadcastingText));

    connect(pOptionsBroadcasting, &QAction::triggered, this, &WMainMenuBar::toggleBroadcasting);
    connect(this,
            &WMainMenuBar::internalBroadcastingStateChange,
            pOptionsBroadcasting,
            &QAction::setChecked);
    pOptionsMenu->addAction(pOptionsBroadcasting);
#endif

    pOptionsMenu->addSeparator();

    QString keyboardShortcutTitle = tr("Enable &Keyboard Shortcuts");
    QString keyboardShortcutText = tr("Toggles keyboard shortcuts on or off");
    bool keyboardShortcutsEnabled = m_pConfig->getValueString(
        ConfigKey("[Keyboard]", "Enabled")) == "1";
    auto* pOptionsKeyboard = new QAction(keyboardShortcutTitle, this);
    pOptionsKeyboard->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(
                ConfigKey("[KeyboardShortcuts]", "OptionsMenu_EnableShortcuts"),
                tr("Ctrl+`"))));
    pOptionsKeyboard->setShortcutContext(Qt::ApplicationShortcut);
    pOptionsKeyboard->setCheckable(true);
    pOptionsKeyboard->setChecked(keyboardShortcutsEnabled);
    pOptionsKeyboard->setStatusTip(keyboardShortcutText);
    pOptionsKeyboard->setWhatsThis(buildWhatsThis(keyboardShortcutTitle, keyboardShortcutText));
    connect(pOptionsKeyboard, &QAction::triggered, this, &WMainMenuBar::toggleKeyboardShortcuts);

    pOptionsMenu->addAction(pOptionsKeyboard);

    pOptionsMenu->addSeparator();

    QString preferencesTitle = tr("&Preferences");
    QString preferencesText = tr("Change Mixxx settings (e.g. playback, MIDI, controls)");
    auto* pOptionsPreferences = new QAction(preferencesTitle, this);
    pOptionsPreferences->setShortcut(
        QKeySequence(m_pKbdConfig->getValue(
                ConfigKey("[KeyboardShortcuts]", "OptionsMenu_Preferences"),
                showPreferencesKeyBinding())));
    pOptionsPreferences->setShortcutContext(Qt::ApplicationShortcut);
    pOptionsPreferences->setStatusTip(preferencesText);
    pOptionsPreferences->setWhatsThis(buildWhatsThis(preferencesTitle, preferencesText));
    pOptionsPreferences->setMenuRole(QAction::PreferencesRole);
    connect(pOptionsPreferences, &QAction::triggered, this, &WMainMenuBar::showPreferences);
    pOptionsMenu->addAction(pOptionsPreferences);

    addMenu(pOptionsMenu);

    // DEVELOPER MENU
    if (CmdlineArgs::Instance().getDeveloper()) {
        QMenu* pDeveloperMenu = new QMenu(tr("&Developer"), this);
#ifndef __APPLE__
        connectMenuToSlotShowMenuBar(pDeveloperMenu);
#endif

        QString reloadSkinTitle = tr("&Reload Skin");
        QString reloadSkinText = tr("Reload the skin");
        auto* pDeveloperReloadSkin = new QAction(reloadSkinTitle, this);
        pDeveloperReloadSkin->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "OptionsMenu_ReloadSkin"),
                    tr("Ctrl+Shift+R"))));
        pDeveloperReloadSkin->setShortcutContext(Qt::ApplicationShortcut);
        pDeveloperReloadSkin->setStatusTip(reloadSkinText);
        pDeveloperReloadSkin->setWhatsThis(buildWhatsThis(reloadSkinTitle, reloadSkinText));
        connect(pDeveloperReloadSkin, &QAction::triggered, this, &WMainMenuBar::reloadSkin);
        pDeveloperMenu->addAction(pDeveloperReloadSkin);

        QString developerToolsTitle = tr("Developer &Tools");
        QString developerToolsText = tr("Opens the developer tools dialog");
        auto* pDeveloperTools = new QAction(developerToolsTitle, this);
        pDeveloperTools->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "OptionsMenu_DeveloperTools"),
                    tr("Ctrl+Shift+T"))));
        pDeveloperTools->setShortcutContext(Qt::ApplicationShortcut);
        pDeveloperTools->setCheckable(true);
        pDeveloperTools->setChecked(false);
        pDeveloperTools->setStatusTip(developerToolsText);
        pDeveloperTools->setWhatsThis(buildWhatsThis(developerToolsTitle, developerToolsText));
        connect(pDeveloperTools, &QAction::triggered, this, &WMainMenuBar::toggleDeveloperTools);
        connect(this,
                &WMainMenuBar::internalDeveloperToolsStateChange,
                pDeveloperTools,
                &QAction::setChecked);
        pDeveloperMenu->addAction(pDeveloperTools);

        QString enableExperimentTitle = tr("Stats: &Experiment Bucket");
        QString enableExperimentToolsText = tr(
            "Enables experiment mode. Collects stats in the EXPERIMENT tracking bucket.");
        auto* pDeveloperStatsExperiment = new QAction(enableExperimentTitle, this);
        pDeveloperStatsExperiment->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "OptionsMenu_DeveloperStatsExperiment"),
                    tr("Ctrl+Shift+E"))));
        pDeveloperStatsExperiment->setShortcutContext(Qt::ApplicationShortcut);
        pDeveloperStatsExperiment->setStatusTip(enableExperimentToolsText);
        pDeveloperStatsExperiment->setWhatsThis(buildWhatsThis(
            enableExperimentTitle, enableExperimentToolsText));
        pDeveloperStatsExperiment->setCheckable(true);
        pDeveloperStatsExperiment->setChecked(Experiment::isExperiment());
        connect(pDeveloperStatsExperiment,
                &QAction::triggered,
                this,
                &WMainMenuBar::slotDeveloperStatsExperiment);
        pDeveloperMenu->addAction(pDeveloperStatsExperiment);

        QString enableBaseTitle = tr("Stats: &Base Bucket");
        QString enableBaseToolsText = tr(
            "Enables base mode. Collects stats in the BASE tracking bucket.");
        auto* pDeveloperStatsBase = new QAction(enableBaseTitle, this);
        pDeveloperStatsBase->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "OptionsMenu_DeveloperStatsBase"),
                    tr("Ctrl+Shift+B"))));
        pDeveloperStatsBase->setShortcutContext(Qt::ApplicationShortcut);
        pDeveloperStatsBase->setStatusTip(enableBaseToolsText);
        pDeveloperStatsBase->setWhatsThis(buildWhatsThis(
            enableBaseTitle, enableBaseToolsText));
        pDeveloperStatsBase->setCheckable(true);
        pDeveloperStatsBase->setChecked(Experiment::isBase());
        connect(pDeveloperStatsBase,
                &QAction::triggered,
                this,
                &WMainMenuBar::slotDeveloperStatsBase);
        pDeveloperMenu->addAction(pDeveloperStatsBase);

        // "D" cannot be used with Alt here as it is already by the Developer menu
        QString scriptDebuggerTitle = tr("Deb&ugger Enabled");
        QString scriptDebuggerText = tr("Enables the debugger during skin parsing");
        bool scriptDebuggerEnabled = m_pConfig->getValueString(
            ConfigKey("[ScriptDebugger]", "Enabled")) == "1";
        auto* pDeveloperDebugger = new QAction(scriptDebuggerTitle, this);
        pDeveloperDebugger->setShortcut(
            QKeySequence(m_pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "DeveloperMenu_EnableDebugger"),
                    tr("Ctrl+Shift+D"))));
        pDeveloperDebugger->setShortcutContext(Qt::ApplicationShortcut);
        pDeveloperDebugger->setWhatsThis(buildWhatsThis(keyboardShortcutTitle, keyboardShortcutText));
        pDeveloperDebugger->setCheckable(true);
        pDeveloperDebugger->setStatusTip(scriptDebuggerText);
        pDeveloperDebugger->setChecked(scriptDebuggerEnabled);
        connect(pDeveloperDebugger,
                &QAction::triggered,
                this,
                &WMainMenuBar::slotDeveloperDebugger);
        pDeveloperMenu->addAction(pDeveloperDebugger);

        addMenu(pDeveloperMenu);
    }

    addSeparator();

    // HELP MENU
    QMenu* pHelpMenu = new QMenu(tr("&Help"), this);
#ifndef __APPLE__
    connectMenuToSlotShowMenuBar(pHelpMenu);
#endif

    QString externalLinkSuffix;
#ifndef __APPLE__
    // According to Apple's Human Interface Guidelines devs are encouraged
    // to not use custom icons in menus.
    // https://developer.apple.com/design/human-interface-guidelines/macos/menus/menu-anatomy/
    externalLinkSuffix = QChar(' ') + QChar(0x2197); // north-east arrow
#endif

    // Community Support
    QString supportTitle = tr("&Community Support") + externalLinkSuffix;
    QString supportText = tr("Get help with Mixxx");
    auto* pHelpSupport = new QAction(supportTitle, this);
    pHelpSupport->setStatusTip(supportText);
    pHelpSupport->setWhatsThis(buildWhatsThis(supportTitle, supportText));
    connect(pHelpSupport, &QAction::triggered, this, [this] {
        slotVisitUrl(QUrl(MIXXX_SUPPORT_URL));
    });
    pHelpMenu->addAction(pHelpSupport);

    // User Manual
    QUrl manualUrl = documentationUrl(m_pConfig->getResourcePath(),
            MIXXX_MANUAL_FILENAME,
            MIXXX_MANUAL_URL);
    QString manualSuffix = manualUrl.isLocalFile() ? QString() : externalLinkSuffix;

    QString manualTitle = tr("&User Manual") + manualSuffix;
    QString manualText = tr("Read the Mixxx user manual.");
    auto* pHelpManual = new QAction(manualTitle, this);
    pHelpManual->setStatusTip(manualText);
    pHelpManual->setWhatsThis(buildWhatsThis(manualTitle, manualText));
    connect(pHelpManual, &QAction::triggered, this, [this, manualUrl] {
        slotVisitUrl(manualUrl);
    });
    pHelpMenu->addAction(pHelpManual);

    // Keyboard Shortcuts
    QUrl keyboardShortcutsUrl = documentationUrl(m_pConfig->getResourcePath(),
            MIXXX_KBD_SHORTCUTS_FILENAME,
            MIXXX_MANUAL_SHORTCUTS_URL);
    QString keyboardShortcutsSuffix =
            keyboardShortcutsUrl.isLocalFile() ? QString() : externalLinkSuffix;

    QString shortcutsTitle = tr("&Keyboard Shortcuts") + keyboardShortcutsSuffix;
    QString shortcutsText = tr("Speed up your workflow with keyboard shortcuts.");
    auto* pHelpKbdShortcuts = new QAction(shortcutsTitle, this);
    pHelpKbdShortcuts->setStatusTip(shortcutsText);
    pHelpKbdShortcuts->setWhatsThis(buildWhatsThis(shortcutsTitle, shortcutsText));
    connect(pHelpKbdShortcuts,
            &QAction::triggered,
            this,
            [this, keyboardShortcutsUrl] {
                slotVisitUrl(keyboardShortcutsUrl);
            });
    pHelpMenu->addAction(pHelpKbdShortcuts);

    // User Settings Directory
    const QString& settingsDirPath = m_pConfig->getSettingsPath();
    QString settingsDirTitle = tr("&Settings directory");
    QString settingsDirText = tr("Open the Mixxx user settings directory.");
    auto* pHelpSettingsDir = new QAction(settingsDirTitle, this);
    pHelpSettingsDir->setMenuRole(QAction::NoRole);
    pHelpSettingsDir->setStatusTip(settingsDirText);
    pHelpSettingsDir->setWhatsThis(buildWhatsThis(settingsDirTitle, settingsDirText));
    connect(pHelpSettingsDir, &QAction::triggered, this, [this, settingsDirPath] {
        slotVisitUrl(QUrl::fromLocalFile(settingsDirPath));
    });
    pHelpMenu->addAction(pHelpSettingsDir);

    // Translate This Application
    QString translateTitle = tr("&Translate This Application") + externalLinkSuffix;
    QString translateText = tr("Help translate this application into your language.");
    auto* pHelpTranslation = new QAction(translateTitle, this);
    pHelpTranslation->setStatusTip(translateText);
    pHelpTranslation->setWhatsThis(buildWhatsThis(translateTitle, translateText));
    connect(pHelpTranslation, &QAction::triggered, this, [this] {
        slotVisitUrl(QUrl(MIXXX_TRANSLATION_URL));
    });
    pHelpMenu->addAction(pHelpTranslation);

    pHelpMenu->addSeparator();

    QString aboutTitle = tr("&About");
    QString aboutText = tr("About the application");
    auto* pHelpAboutApp = new QAction(aboutTitle, this);
    pHelpAboutApp->setStatusTip(aboutText);
    pHelpAboutApp->setWhatsThis(buildWhatsThis(aboutTitle, aboutText));
    pHelpAboutApp->setMenuRole(QAction::AboutRole);
    connect(pHelpAboutApp, &QAction::triggered, this, &WMainMenuBar::showAbout);

    pHelpMenu->addAction(pHelpAboutApp);
    addMenu(pHelpMenu);

#ifndef __APPLE__
    // Watch focus changes to hide the menubar as soon as all menus are closed,
    // e.g. when an action was triggered or when all menus are closed by pressing
    // Escape or clicking anywhere else
    connect(qApp,
            &QApplication::focusWindowChanged,
            this,
            [this]() {
                if (!isNativeMenuBar() && height() > 0 && !activeAction()) {
                    hideMenuBar();
                }
            });
    // ... and when the focus widget changes (main window or dialogs)
    connect(qApp,
            // This would work, too, but unfortunately this is also emitted on
            // leaveEvent of WStarRating in the library.
            // &QApplication::focusObjectChanged,
            &QApplication::focusChanged,
            this,
            [this]() {
                if (!isNativeMenuBar() && height() > 0 && !activeAction()) {
                    hideMenuBar();
                }
            });
#endif
}

void WMainMenuBar::onKeywheelChange(int state) {
    Q_UNUSED(state);
    m_pViewKeywheel->setChecked(false);
}

void WMainMenuBar::onLibraryScanStarted() {
    emit internalLibraryScanActive(true);
}

void WMainMenuBar::onLibraryScanFinished() {
    emit internalLibraryScanActive(false);
}

void WMainMenuBar::onNewSkinLoaded() {
    emit internalOnNewSkinLoaded();
}

void WMainMenuBar::onNewSkinAboutToLoad() {
    emit internalOnNewSkinAboutToLoad();
}

void WMainMenuBar::onRecordingStateChange(bool recording) {
    emit internalRecordingStateChange(recording);
}

void WMainMenuBar::onBroadcastingStateChange(bool broadcasting) {
    emit internalBroadcastingStateChange(broadcasting);
}

void WMainMenuBar::onDeveloperToolsShown() {
    emit internalDeveloperToolsStateChange(true);
}

void WMainMenuBar::onDeveloperToolsHidden() {
    emit internalDeveloperToolsStateChange(false);
}

void WMainMenuBar::onFullScreenStateChange(bool fullscreen) {
#ifndef __APPLE__
    // always try to hide the menubar when we switched
    hideMenuBar();
#endif
    emit internalFullScreenStateChange(fullscreen);
}

#ifndef __APPLE__
void WMainMenuBar::connectMenuToSlotShowMenuBar(const QMenu* pMenu) {
    // If a menu hotkey like Alt+F(ile) is pressed while the menubar is hidden,
    // show the menubar and open the requested menu. See showMenuBar() for details.

    // NOTE(ronso0) Test with xfwm4 and other window managers if you change this
    // code or think you found alternative ways to toggle the menubar!
    // In Gnome for example, when highlighting (pressed Alt only) or activating
    // menus (Alt combos), the menubar receives focusIn events (even while it is
    // hidden), and focusOut events respectively when closing menus. Hence we
    // could simply show/hide the menubar in QMenuBar::focusInEvent() and focusoutEvent().
    // However, with xfwm4 for example, menus are activated directly, i.e. the
    // menubar doesn't receive focus change events.
    connect(pMenu,
            &QMenu::aboutToShow,
            this,
            [this]() {
                if (!isNativeMenuBar() && height() <= 0) {
                    showMenuBar();
                }
            });
}

void WMainMenuBar::slotToggleMenuBar() {
    if (isNativeMenuBar()) {
        return;
    }

    if (height() > 0) {
        hideMenuBar();
    } else {
        showMenuBar();
    }
}

void WMainMenuBar::showMenuBar() {
    if (isNativeMenuBar()) {
        return;
    }
    // Note: the resulting resizeEvent() calls QMenuBarPrivate::updateGeometries
    // which resets currentAction() to nullptr. So in case the menubar is shown
    // in response to a specific menu hotkey (Alt-F), or pressing Alt opens the
    // first menu (File) (depends on OS / window manager), the current action
    // will be reset to null and menus can't be switched with Left/Right keys
    // anymore, i.e. we'd be stuck in the triggered menu.
    // Workaround: reselect the active action after unhiding. It can be none,
    // user-requested (hotkey) or auto-selected (first menu's first action).
    QAction* pAct = activeAction();
    setMinimumHeight(sizeHint().height());
    // If there was a menu selected before, reselect that.
    // Note: with nullptr this would be a no-op. Though, even if no menu is
    // selected, hovering a menu would instantly open that (no click required).
    if (pAct) {
        setActiveAction(pAct);
    }
    // TODO Alternatively, activate the first menu?
    // setActiveAction(pAct ? pAct : actions().first());
}

void WMainMenuBar::hideMenuBar() {
    if (isNativeMenuBar()) {
        return;
    }
    if (m_pConfig->getValue<bool>(ConfigKey("[Config]", "hide_menubar"), false)) {
        // don't use setHidden(true) because Alt hotkeys wouldn't work anymore
        setFixedHeight(0);
    }
}

void WMainMenuBar::slotAutoHideMenuBarToggled(bool autoHide) {
    m_pConfig->setValue(ConfigKey("[Config]", "hide_menubar"), autoHide ? 1 : 0);
    // Just in case it was hidden after toggling the menu action
    if (!autoHide) {
        showMenuBar();
    }
}
#endif

void WMainMenuBar::onVinylControlDeckEnabledStateChange(int deck, bool enabled) {
    VERIFY_OR_DEBUG_ASSERT(deck >= 0 && deck < m_vinylControlEnabledActions.size()) {
        return;
    }
    m_vinylControlEnabledActions.at(deck)->setChecked(enabled);
}

void WMainMenuBar::slotDeveloperStatsBase(bool enable) {
    if (enable) {
        Experiment::setBase();
    } else {
        Experiment::disable();
    }
}

void WMainMenuBar::slotDeveloperStatsExperiment(bool enable) {
    if (enable) {
        Experiment::setExperiment();
    } else {
        Experiment::disable();
    }
}

void WMainMenuBar::slotDeveloperDebugger(bool toggle) {
    m_pConfig->set(ConfigKey("[ScriptDebugger]","Enabled"),
                   ConfigValue(toggle ? 1 : 0));
}

void WMainMenuBar::slotVisitUrl(const QUrl& url) {
    mixxx::DesktopHelper::openUrl(url);
}

void WMainMenuBar::createVisibilityControl(QAction* pAction,
                                           const ConfigKey& key) {
    auto* pConnection = new VisibilityControlConnection(this, pAction, key);
    connect(this,
            &WMainMenuBar::internalOnNewSkinLoaded,
            pConnection,
            &VisibilityControlConnection::slotReconnectControl);
#ifdef __LINUX__
    // reconnect when menu bar was recreated after toggling fullscreen
    // so all hotkeys and menu actions continue to work
    connect(this,
            &WMainMenuBar::internalFullScreenStateChange,
            pConnection,
            &VisibilityControlConnection::slotReconnectControl);
#endif
    connect(this,
            &WMainMenuBar::internalOnNewSkinAboutToLoad,
            pConnection,
            &VisibilityControlConnection::slotClearControl);
}

void WMainMenuBar::onNumberOfDecksChanged(int decks) {
    int deck = 0;
    for (QAction* pVinylControlEnabled : std::as_const(m_vinylControlEnabledActions)) {
        pVinylControlEnabled->setVisible(deck++ < decks);
    }
    deck = 0;
    for (QAction* pLoadToDeck : std::as_const(m_loadToDeckActions)) {
        pLoadToDeck->setVisible(deck++ < decks);
    }
}

VisibilityControlConnection::VisibilityControlConnection(
    QObject* pParent, QAction* pAction, const ConfigKey& key)
        : QObject(pParent),
          m_key(key),
          m_pAction(pAction) {
    connect(m_pAction, &QAction::triggered, this, &VisibilityControlConnection::slotActionToggled);
}

void VisibilityControlConnection::slotClearControl() {
    m_pControl.reset();
    m_pAction->setEnabled(false);
}

void VisibilityControlConnection::slotReconnectControl() {
    m_pControl.reset(new ControlProxy(m_key, this, ControlFlag::NoAssertIfMissing));
    m_pControl->connectValueChanged(this, &VisibilityControlConnection::slotControlChanged);
    m_pAction->setEnabled(m_pControl->valid());
    slotControlChanged();
}

void VisibilityControlConnection::slotControlChanged() {
    if (m_pControl) {
        m_pAction->setChecked(m_pControl->toBool());
    }
}

void VisibilityControlConnection::slotActionToggled(bool toggle) {
    if (m_pControl) {
        m_pControl->set(toggle ? 1.0 : 0.0);
    }
}
