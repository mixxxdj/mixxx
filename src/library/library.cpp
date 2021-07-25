#include "library/library.h"

#include <QDir>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPointer>
#include <QTranslator>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "database/mixxxdb.h"
#include "library/analysisfeature.h"
#include "library/autodj/autodjfeature.h"
#include "library/banshee/bansheefeature.h"
#include "library/browse/browsefeature.h"
#ifdef __ENGINEPRIME__
#include "library/export/libraryexporter.h"
#endif
#include "library/externaltrackcollection.h"
#include "library/itunes/itunesfeature.h"
#include "library/library_preferences.h"
#include "library/librarycontrol.h"
#include "library/libraryfeature.h"
#include "library/librarytablemodel.h"
#include "library/mixxxlibraryfeature.h"
#include "library/recording/recordingfeature.h"
#include "library/rekordbox/rekordboxfeature.h"
#include "library/rhythmbox/rhythmboxfeature.h"
#include "library/serato/seratofeature.h"
#include "library/sidebarmodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackmodel.h"
#include "library/trackset/crate/cratefeature.h"
#include "library/trackset/playlistfeature.h"
#include "library/trackset/setlogfeature.h"
#include "library/traktor/traktorfeature.h"
#include "mixer/playermanager.h"
#include "moc_library.cpp"
#include "recording/recordingmanager.h"
#include "util/assert.h"
#include "util/db/dbconnectionpooled.h"
#include "util/logger.h"
#include "util/sandbox.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wsearchlineedit.h"
#include "widget/wtracktableview.h"

namespace {

const mixxx::Logger kLogger("Library");

} // anonymous namespace

//static
const QString Library::kConfigGroup("[Library]");

// This is the name which we use to register the WTrackTableView with the
// WLibrary
const QString Library::m_sTrackViewName = QString("WTrackTableView");

// The default row height of the library.
const int Library::kDefaultRowHeightPx = 20;

Library::Library(
        QObject* parent,
        UserSettingsPointer pConfig,
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        TrackCollectionManager* pTrackCollectionManager,
        PlayerManager* pPlayerManager,
        RecordingManager* pRecordingManager)
        : QObject(parent),
          m_pConfig(pConfig),
          m_pDbConnectionPool(std::move(pDbConnectionPool)),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_pSidebarModel(make_parented<SidebarModel>(this)),
          m_pLibraryControl(make_parented<LibraryControl>(this)),
          m_pMixxxLibraryFeature(nullptr),
          m_pPlaylistFeature(nullptr),
          m_pCrateFeature(nullptr),
          m_pAnalysisFeature(nullptr) {
    qRegisterMetaType<Library::RemovalType>("Library::RemovalType");

    m_pKeyNotation.reset(new ControlObject(ConfigKey(kConfigGroup, "key_notation")));

    connect(m_pTrackCollectionManager,
            &TrackCollectionManager::libraryScanFinished,
            this,
            &Library::slotRefreshLibraryModels);

    // TODO(rryan) -- turn this construction / adding of features into a static
    // method or something -- CreateDefaultLibrary
    m_pMixxxLibraryFeature = new MixxxLibraryFeature(
            this,
            m_pConfig);
    addFeature(m_pMixxxLibraryFeature);
#ifdef __ENGINEPRIME__
    connect(m_pMixxxLibraryFeature,
            &MixxxLibraryFeature::exportLibrary,
            this,
            &Library::exportLibrary,
            Qt::DirectConnection /* signal-to-signal */);
#endif

    addFeature(new AutoDJFeature(this, m_pConfig, pPlayerManager));
    m_pPlaylistFeature = new PlaylistFeature(this, UserSettingsPointer(m_pConfig));
    addFeature(m_pPlaylistFeature);

    m_pCrateFeature = new CrateFeature(this, m_pConfig);
    addFeature(m_pCrateFeature);
#ifdef __ENGINEPRIME__
    connect(m_pCrateFeature,
            &CrateFeature::exportAllCrates,
            this,
            &Library::exportLibrary, // signal-to-signal
            Qt::DirectConnection);
    connect(m_pCrateFeature,
            &CrateFeature::exportCrate,
            this,
            &Library::exportCrate, // signal-to-signal
            Qt::DirectConnection);
#endif

    BrowseFeature* browseFeature = new BrowseFeature(
            this, m_pConfig, pRecordingManager);
    connect(browseFeature,
            &BrowseFeature::scanLibrary,
            m_pTrackCollectionManager,
            &TrackCollectionManager::startLibraryScan);
    connect(m_pTrackCollectionManager,
            &TrackCollectionManager::libraryScanStarted,
            browseFeature,
            &BrowseFeature::slotLibraryScanStarted);
    connect(m_pTrackCollectionManager,
            &TrackCollectionManager::libraryScanFinished,
            browseFeature,
            &BrowseFeature::slotLibraryScanFinished);
    addFeature(browseFeature);

    addFeature(new RecordingFeature(this, m_pConfig, pRecordingManager));
    addFeature(new SetlogFeature(this, UserSettingsPointer(m_pConfig)));

    m_pAnalysisFeature = new AnalysisFeature(this, m_pConfig);
    connect(m_pPlaylistFeature,
            &PlaylistFeature::analyzeTracks,
            m_pAnalysisFeature,
            &AnalysisFeature::analyzeTracks);
    connect(m_pCrateFeature,
            &CrateFeature::analyzeTracks,
            m_pAnalysisFeature,
            &AnalysisFeature::analyzeTracks);
    addFeature(m_pAnalysisFeature);
    // Suspend a batch analysis while an ad-hoc analysis of
    // loaded tracks is in progress and resume it afterwards.
    connect(pPlayerManager,
            &PlayerManager::trackAnalyzerProgress,
            this,
            &Library::onPlayerManagerTrackAnalyzerProgress);
    connect(pPlayerManager,
            &PlayerManager::trackAnalyzerIdle,
            this,
            &Library::onPlayerManagerTrackAnalyzerIdle);

    // iTunes and Rhythmbox should be last until we no longer have an obnoxious
    // messagebox popup when you select them. (This forces you to reach for your
    // mouse or keyboard if you're using MIDI control and you scroll through them...)
    if (RhythmboxFeature::isSupported() &&
            m_pConfig->getValue(
                    ConfigKey(kConfigGroup, "ShowRhythmboxLibrary"), true)) {
        addFeature(new RhythmboxFeature(this, m_pConfig));
    }
    if (m_pConfig->getValue(
                ConfigKey(kConfigGroup, "ShowBansheeLibrary"), true)) {
        BansheeFeature::prepareDbPath(m_pConfig);
        if (BansheeFeature::isSupported()) {
            addFeature(new BansheeFeature(this, m_pConfig));
        }
    }
    if (ITunesFeature::isSupported() &&
            m_pConfig->getValue(
                    ConfigKey(kConfigGroup, "ShowITunesLibrary"), true)) {
        addFeature(new ITunesFeature(this, m_pConfig));
    }
    if (TraktorFeature::isSupported() &&
            m_pConfig->getValue(
                    ConfigKey(kConfigGroup, "ShowTraktorLibrary"), true)) {
        addFeature(new TraktorFeature(this, m_pConfig));
    }

    // TODO(XXX) Rekordbox feature added persistently as the only way to enable it to
    // dynamically appear/disappear when correctly prepared removable devices
    // are mounted/unmounted would be to have some form of timed thread to check
    // periodically. Not ideal performance wise.
    if (m_pConfig->getValue(
                ConfigKey(kConfigGroup, "ShowRekordboxLibrary"), true)) {
        addFeature(new RekordboxFeature(this, m_pConfig));
    }

    if (m_pConfig->getValue(
                ConfigKey(kConfigGroup, "ShowSeratoLibrary"), true)) {
        addFeature(new SeratoFeature(this, m_pConfig));
    }

    for (const auto& externalTrackCollection : m_pTrackCollectionManager->externalCollections()) {
        auto* feature = externalTrackCollection->newLibraryFeature(this, m_pConfig);
        if (feature) {
            kLogger.info() << "Adding library feature for"
                           << externalTrackCollection->name();
            addFeature(feature);
        } else {
            kLogger.info() << "Library feature for"
                           << externalTrackCollection->name()
                           << "is not available";
        }
    }

    // On startup we need to check if all of the user's library folders are
    // accessible to us. If the user is using a database from <1.12.0 with
    // sandboxing then we will need them to give us permission.
    const auto rootDirs = m_pTrackCollectionManager->internalCollection()->loadRootDirs();
    for (mixxx::FileInfo dirInfo : rootDirs) {
        if (!dirInfo.exists() || !dirInfo.isDir()) {
            kLogger.warning()
                    << "Skipping access check for missing or invalid directory"
                    << dirInfo;
            continue;
        }
        if (Sandbox::askForAccess(&dirInfo)) {
            kLogger.info()
                    << "Access to directory"
                    << dirInfo
                    << "from sandbox granted";
        } else {
            kLogger.warning()
                    << "Access to directory"
                    << dirInfo
                    << "from sandbox denied";
        }
    }

    m_iTrackTableRowHeight = m_pConfig->getValue(
            ConfigKey(kConfigGroup, "RowHeight"), kDefaultRowHeightPx);
    QString fontStr =
            m_pConfig->getValueString(ConfigKey(kConfigGroup, "Font"));
    if (!fontStr.isEmpty()) {
        m_trackTableFont.fromString(fontStr);
    } else {
        m_trackTableFont = QApplication::font();
    }

    m_editMetadataSelectedClick = m_pConfig->getValue(
            ConfigKey(kConfigGroup, "EditMetadataSelectedClick"),
            PREF_LIBRARY_EDIT_METADATA_DEFAULT);
}

Library::~Library() {
    // Empty but required due to forward declarations in header file!
}

TrackCollectionManager* Library::trackCollectionManager() const {
    // Cannot be implemented inline due to forward declarations
    return m_pTrackCollectionManager;
}

void Library::stopPendingTasks() {
    if (m_pAnalysisFeature) {
        m_pAnalysisFeature->stopAnalysis();
        m_pAnalysisFeature = nullptr;
    }
}

void Library::bindSearchboxWidget(WSearchLineEdit* pSearchboxWidget) {
    connect(pSearchboxWidget,
            &WSearchLineEdit::search,
            this,
            &Library::search);
    connect(this,
            &Library::disableSearch,
            pSearchboxWidget,
            &WSearchLineEdit::slotDisableSearch);
    connect(this,
            &Library::restoreSearch,
            pSearchboxWidget,
            &WSearchLineEdit::slotRestoreSearch);
    connect(this,
            &Library::setTrackTableFont,
            pSearchboxWidget,
            &WSearchLineEdit::slotSetFont);
    emit setTrackTableFont(m_trackTableFont);
    m_pLibraryControl->bindSearchboxWidget(pSearchboxWidget);
}

void Library::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    m_pLibraryControl->bindSidebarWidget(pSidebarWidget);

    // Setup the sources view
    pSidebarWidget->setModel(m_pSidebarModel);
    connect(m_pSidebarModel,
            &SidebarModel::selectIndex,
            pSidebarWidget,
            &WLibrarySidebar::selectIndex);
    connect(pSidebarWidget,
            &WLibrarySidebar::pressed,
            m_pSidebarModel,
            &SidebarModel::pressed);
    connect(pSidebarWidget,
            &WLibrarySidebar::clicked,
            m_pSidebarModel,
            &SidebarModel::clicked);
    // Lazy model: Let triangle symbol increment the model
    connect(pSidebarWidget,
            &WLibrarySidebar::expanded,
            m_pSidebarModel,
            &SidebarModel::doubleClicked);

    connect(pSidebarWidget,
            &WLibrarySidebar::rightClicked,
            m_pSidebarModel,
            &SidebarModel::rightClicked);

    pSidebarWidget->slotSetFont(m_trackTableFont);
    connect(this,
            &Library::setTrackTableFont,
            pSidebarWidget,
            &WLibrarySidebar::slotSetFont);

    for (const auto& feature : qAsConst(m_features)) {
        feature->bindSidebarWidget(pSidebarWidget);
    }
}

void Library::bindLibraryWidget(
        WLibrary* pLibraryWidget, KeyboardEventFilter* pKeyboard) {
    WTrackTableView* pTrackTableView = new WTrackTableView(pLibraryWidget,
            m_pConfig,
            this,
            pLibraryWidget->getTrackTableBackgroundColorOpacity(),
            true);
    pTrackTableView->installEventFilter(pKeyboard);
    connect(this,
            &Library::showTrackModel,
            pTrackTableView,
            &WTrackTableView::loadTrackModel);
    connect(pTrackTableView,
            &WTrackTableView::loadTrack,
            this,
            &Library::slotLoadTrack);
    connect(pTrackTableView,
            &WTrackTableView::loadTrackToPlayer,
            this,
            &Library::slotLoadTrackToPlayer);
    pLibraryWidget->registerView(m_sTrackViewName, pTrackTableView);

    connect(this,
            &Library::switchToView,
            pLibraryWidget,
            &WLibrary::switchToView);

    connect(pTrackTableView,
            &WTrackTableView::trackSelected,
            this,
            &Library::trackSelected);

    connect(this,
            &Library::setTrackTableFont,
            pTrackTableView,
            &WTrackTableView::setTrackTableFont);
    connect(this,
            &Library::setTrackTableRowHeight,
            pTrackTableView,
            &WTrackTableView::setTrackTableRowHeight);
    connect(this,
            &Library::setSelectedClick,
            pTrackTableView,
            &WTrackTableView::setSelectedClick);

    m_pLibraryControl->bindLibraryWidget(pLibraryWidget, pKeyboard);

    for (const auto& feature : qAsConst(m_features)) {
        feature->bindLibraryWidget(pLibraryWidget, pKeyboard);
    }

    // Set the current font and row height on all the WTrackTableViews that were
    // just connected to us.
    emit setTrackTableFont(m_trackTableFont);
    emit setTrackTableRowHeight(m_iTrackTableRowHeight);
    emit setSelectedClick(m_editMetadataSelectedClick);
}

void Library::addFeature(LibraryFeature* feature) {
    VERIFY_OR_DEBUG_ASSERT(feature) {
        return;
    }
    m_features.push_back(feature);
    m_pSidebarModel->addLibraryFeature(feature);
    connect(feature,
            &LibraryFeature::showTrackModel,
            this,
            &Library::slotShowTrackModel);
    connect(feature,
            &LibraryFeature::switchToView,
            this,
            &Library::slotSwitchToView);
    connect(feature,
            &LibraryFeature::loadTrack,
            this,
            &Library::slotLoadTrack);
    connect(feature,
            &LibraryFeature::loadTrackToPlayer,
            this,
            &Library::slotLoadTrackToPlayer);
    connect(feature,
            &LibraryFeature::restoreSearch,
            this,
            &Library::restoreSearch); // forward signal
    connect(feature,
            &LibraryFeature::disableSearch,
            this,
            &Library::disableSearch); // forward signal
    connect(feature,
            &LibraryFeature::enableCoverArtDisplay,
            this,
            &Library::enableCoverArtDisplay);
    connect(feature,
            &LibraryFeature::trackSelected,
            this,
            &Library::trackSelected);
}

void Library::onPlayerManagerTrackAnalyzerProgress(
        TrackId /*trackId*/, AnalyzerProgress /*analyzerProgress*/) {
    if (m_pAnalysisFeature) {
        m_pAnalysisFeature->suspendAnalysis();
    }
}

void Library::onPlayerManagerTrackAnalyzerIdle() {
    if (m_pAnalysisFeature) {
        m_pAnalysisFeature->resumeAnalysis();
    }
}

void Library::slotShowTrackModel(QAbstractItemModel* model) {
    //qDebug() << "Library::slotShowTrackModel" << model;
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model);
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }
    emit showTrackModel(model);
    emit switchToView(m_sTrackViewName);
    emit restoreSearch(trackModel->currentSearch());
}

void Library::slotSwitchToView(const QString& view) {
    //qDebug() << "Library::slotSwitchToView" << view;
    emit switchToView(view);
}

void Library::slotLoadTrack(TrackPointer pTrack) {
    emit loadTrack(pTrack);
}

void Library::slotLoadLocationToPlayer(const QString& location, const QString& group) {
    auto trackRef = TrackRef::fromFilePath(location);
    TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(trackRef);
    if (pTrack) {
        emit loadTrackToPlayer(pTrack, group);
    }
}

void Library::slotLoadTrackToPlayer(
        TrackPointer pTrack, const QString& group, bool play) {
    emit loadTrackToPlayer(pTrack, group, play);
}

void Library::slotRefreshLibraryModels() {
    m_pMixxxLibraryFeature->refreshLibraryModels();
    m_pAnalysisFeature->refreshLibraryModels();
}

void Library::slotCreatePlaylist() {
    m_pPlaylistFeature->slotCreatePlaylist();
}

void Library::slotCreateCrate() {
    m_pCrateFeature->slotCreateCrate();
}

void Library::onSkinLoadFinished() {
    // Enable the default selection when a new skin is loaded.
    m_pSidebarModel->activateDefaultSelection();
}

void Library::slotRequestAddDir(const QString& dir) {
    // We only call this method if the user has picked a new directory via a
    // file dialog. This means the system sandboxer (if we are sandboxed) has
    // granted us permission to this folder. Create a security bookmark while we
    // have permission so that we can access the folder on future runs. We need
    // to canonicalize the path so we first wrap the directory string with a
    // QDir.
    QDir directory(dir);
    Sandbox::createSecurityTokenForDir(directory);

    if (!m_pTrackCollectionManager->addDirectory(mixxx::FileInfo(dir))) {
        QMessageBox::information(nullptr,
                tr("Add Directory to Library"),
                tr("Could not add the directory to your library. Either this "
                   "directory is already in your library or you are currently "
                   "rescanning your library."));
    }
    // set at least one directory in the config file so that it will be possible
    // to downgrade from 1.12
    if (m_pConfig->getValueString(PREF_LEGACY_LIBRARY_DIR).length() < 1) {
        m_pConfig->set(PREF_LEGACY_LIBRARY_DIR, dir);
    }
}

void Library::slotRequestRemoveDir(const QString& dir, RemovalType removalType) {
    // Remove the directory from the directory list.
    if (!m_pTrackCollectionManager->removeDirectory(mixxx::FileInfo(dir))) {
        return;
    }

    switch (removalType) {
    case RemovalType::KeepTracks:
        break;
    case RemovalType::HideTracks:
        // Mark all tracks in this directory as deleted but DON'T purge them
        // in case the user re-adds them manually.
        m_pTrackCollectionManager->hideAllTracks(dir);
        break;
    case RemovalType::PurgeTracks:
        // The user requested that we purge all metadata.
        m_pTrackCollectionManager->purgeAllTracks(dir);
        break;
    default:
        DEBUG_ASSERT(!"unreachable");
    }

    // Also update the config file if necessary so that downgrading is still
    // possible.
    QString confDir = m_pConfig->getValueString(PREF_LEGACY_LIBRARY_DIR);

    if (QDir(dir) == QDir(confDir)) {
        const QList<mixxx::FileInfo> dirList =
                m_pTrackCollectionManager->internalCollection()->loadRootDirs();
        if (dirList.isEmpty()) {
            // Save empty string so that an old version of mixxx knows it has to
            // ask for a new directory.
            m_pConfig->set(PREF_LEGACY_LIBRARY_DIR, QString());
        } else {
            m_pConfig->set(PREF_LEGACY_LIBRARY_DIR, dirList.first().location());
        }
    }
}

void Library::slotRequestRelocateDir(const QString& oldDir, const QString& newDir) {
    m_pTrackCollectionManager->relocateDirectory(oldDir, newDir);

    // also update the config file if necessary so that downgrading is still
    // possible
    QString conDir = m_pConfig->getValueString(PREF_LEGACY_LIBRARY_DIR);
    if (oldDir == conDir) {
        m_pConfig->set(PREF_LEGACY_LIBRARY_DIR, newDir);
    }
}

void Library::setFont(const QFont& font) {
    QFontMetrics currMetrics(m_trackTableFont);
    QFontMetrics newMetrics(font);
    double currFontHeight = currMetrics.height();
    double newFontHeight = newMetrics.height();

    m_trackTableFont = font;
    emit setTrackTableFont(font);

    // adapt the previous font height/row height ratio
    int scaledRowHeight = static_cast<int>(std::round(
            (newFontHeight / currFontHeight) * m_iTrackTableRowHeight));
    setRowHeight(scaledRowHeight);
}

void Library::setRowHeight(int rowHeight) {
    m_iTrackTableRowHeight = rowHeight;
    emit setTrackTableRowHeight(rowHeight);
}

void Library::setEditMedatataSelectedClick(bool enabled) {
    m_editMetadataSelectedClick = enabled;
    emit setSelectedClick(enabled);
}

void Library::searchTracksInCollection(const QString& query) {
    VERIFY_OR_DEBUG_ASSERT(m_pMixxxLibraryFeature) {
        return;
    }
    m_pMixxxLibraryFeature->searchAndActivate(query);
    emit switchToView(m_sTrackViewName);
    m_pSidebarModel->activateDefaultSelection();
}

#ifdef __ENGINEPRIME__
std::unique_ptr<mixxx::LibraryExporter> Library::makeLibraryExporter(
        QWidget* parent) {
    return std::make_unique<mixxx::LibraryExporter>(
            parent, m_pConfig, m_pTrackCollectionManager);
}
#endif

LibraryTableModel* Library::trackTableModel() const {
    VERIFY_OR_DEBUG_ASSERT(m_pMixxxLibraryFeature) {
        return nullptr;
    }

    return m_pMixxxLibraryFeature->trackTableModel();
}
