#include "library/itunes/itunesfeature.h"

#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QStandardPaths>
#include <QtConcurrentRun>
#include <QtDebug>
#include <memory>
#include <utility>

#include "library/baseexternalplaylistmodel.h"
#include "library/baseexternaltrackmodel.h"
#include "library/basetrackcache.h"
#include "library/dao/settingsdao.h"
#include "library/itunes/itunesdao.h"
#include "library/itunes/itunesimporter.h"
#include "library/itunes/itunesplaylistmodel.h"
#include "library/itunes/itunesxmlimporter.h"
#include "library/library.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "library/treeitemmodel.h"
#include "moc_itunesfeature.cpp"
#include "util/sandbox.h"
#include "widget/wlibrarysidebar.h"

#ifdef __MACOS_ITUNES_LIBRARY__
#include "library/itunes/itunesmacosimporter.h"
#endif

namespace {

const QString kItdbPathKey = "mixxx.itunesfeature.itdbpath";

bool isMacOSImporterAvailable() {
#ifdef __MACOS_ITUNES_LIBRARY__
    // The iTunesLibrary framework is only available on macOS 10.13+
    // Note that this uses a Clang directive to check the macOS version at
    // runtime, which is the suggested approach for conditionally accessing
    // macOS frameworks from C++ depending on availability:
    // https://stackoverflow.com/a/57825758
    if (__builtin_available(macOS 10.13, *)) {
        return true;
    }
#endif
    return false;
}

} // anonymous namespace

ITunesFeature::ITunesFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig, QStringLiteral("itunes")),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_cancelImport(false) {
    QString tableName = "itunes_library";
    QString idColumn = "id";
    QStringList columns = {
            "id",
            "artist",
            "title",
            "album",
            "album_artist",
            "year",
            "genre",
            "grouping",
            "tracknumber",
            "location",
            "comment",
            "duration",
            "bitrate",
            "bpm",
            "rating"};
    QStringList searchColumns = {
            "artist",
            "album",
            "album_artist",
            "location",
            "grouping",
            "comment",
            "title",
            "genre"};

    m_trackSource = QSharedPointer<BaseTrackCache>::create(
            m_pLibrary->trackCollectionManager()->internalCollection(),
            std::move(tableName),
            std::move(idColumn),
            std::move(columns),
            std::move(searchColumns),
            false);
    m_pITunesTrackModel = new BaseExternalTrackModel(this,
            m_pLibrary->trackCollectionManager(),
            "mixxx.db.model.itunes",
            "itunes_library",
            m_trackSource);
    m_pITunesPlaylistModel = new ITunesPlaylistModel(this,
            m_pLibrary->trackCollectionManager(),
            m_trackSource);
    m_isActivated = false;
    m_title = tr("iTunes");

    m_database =
            QSqlDatabase::cloneDatabase(m_pLibrary->trackCollectionManager()
                                                ->internalCollection()
                                                ->database(),
                    "ITUNES_SCANNER");

    // Open the database connection in this thread.
    if (!m_database.open()) {
        qDebug() << "Failed to open database for iTunes scanner." << m_database.lastError();
    }
    connect(&m_future_watcher,
            &QFutureWatcher<TreeItem*>::finished,
            this,
            &ITunesFeature::onTrackCollectionLoaded);

    m_pITunesTrackModel->setSearch(""); // enable search.
}

ITunesFeature::~ITunesFeature() {
    m_database.close();
    m_cancelImport = true;
    m_future.waitForFinished();
    delete m_pITunesTrackModel;
    delete m_pITunesPlaylistModel;
}

std::unique_ptr<BaseSqlTableModel>
ITunesFeature::createPlaylistModelForPlaylist(const QString& playlist) {
    auto pModel = std::make_unique<BaseExternalPlaylistModel>(this,
            m_pLibrary->trackCollectionManager(),
            "mixxx.db.model.itunes_playlist",
            "itunes_playlists",
            "itunes_playlist_tracks",
            m_trackSource);
    pModel->setPlaylist(playlist);
    return pModel;
}

// static
bool ITunesFeature::isSupported() {
    // itunes db might just be elsewhere, don't rely on it being in its
    // normal place. And since we will load an itdb on any platform...
    return true;
}


QVariant ITunesFeature::title() {
    return m_title;
}

void ITunesFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClick()
    m_pSidebarWidget = pSidebarWidget;
    // send it to BaseExternalLibraryFeature for onRightClickChild()
    BaseExternalLibraryFeature::bindSidebarWidget(pSidebarWidget);
}

void ITunesFeature::activate() {
    activate(false);
    emit enableCoverArtDisplay(false);
}

void ITunesFeature::activate(bool forceReload) {
    //qDebug("ITunesFeature::activate()");
    if (!m_isActivated || forceReload) {

        //Delete all table entries of iTunes feature
        ScopedTransaction transaction(m_database);
        clearTable("itunes_playlist_tracks");
        clearTable("itunes_library");
        clearTable("itunes_playlists");
        transaction.commit();

        emit showTrackModel(m_pITunesTrackModel);

        SettingsDAO settings(m_pTrackCollection->database());
        QString dbSetting(settings.getValue(kItdbPathKey));
        // if a path exists in the database, use it
        if (!dbSetting.isEmpty() && QFile::exists(dbSetting)) {
            m_dbfile = dbSetting;
        } else {
            // No Path in settings, try the default
            m_dbfile = getiTunesMusicPath();
        }

        if (!isMacOSImporterUsed()) {
            mixxx::FileInfo fileInfo(m_dbfile);
            if (fileInfo.checkFileExists()) {
                // Users of Mixxx <1.12.0 didn't support sandboxing. If we are sandboxed
                // and using a custom iTunes path then we have to ask for access to this
                // file.
                Sandbox::askForAccess(&fileInfo);
            } else {
                // if the path we got between the default and the database doesn't
                // exist, ask for a new one and use/save it if it exists
                m_dbfile = showOpenDialog();
                mixxx::FileInfo fileInfo(m_dbfile);
                if (!fileInfo.checkFileExists()) {
                    return;
                }

                // The user has picked a new directory via a file dialog. This means the
                // system sandboxer (if we are sandboxed) has granted us permission to
                // this folder. Create a security bookmark while we have permission so
                // that we can access the folder on future runs. We need to canonicalize
                // the path so we first wrap the directory string with a QDir.
                Sandbox::createSecurityToken(&fileInfo);
                settings.setValue(kItdbPathKey, m_dbfile);
            }
        }
        m_isActivated =  true;
        // Let a worker thread do the XML parsing
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_future = QtConcurrent::run(&ITunesFeature::importLibrary, this);
#else
        m_future = QtConcurrent::run(this, &ITunesFeature::importLibrary);
#endif
        m_future_watcher.setFuture(m_future);
        m_title = tr("(loading) iTunes");
        // calls a slot in the sidebar model such that 'iTunes (isLoading)' is displayed.
        emit featureIsLoading(this, true);
    } else {
        emit showTrackModel(m_pITunesTrackModel);
    }
    emit enableCoverArtDisplay(false);
}

void ITunesFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "ITunesFeature::activateChild()" << index;
    TreeItem* treeItem = static_cast<TreeItem*>(index.internalPointer());
    const QString& playlistName = treeItem->getLabel();
    int playlistId = treeItem->getData().toInt();
    qDebug() << "Activating playlist" << playlistName << "with id" << playlistId;
    m_pITunesPlaylistModel->setPlaylistById(playlistId);
    emit showTrackModel(m_pITunesPlaylistModel);
    emit enableCoverArtDisplay(false);
}

TreeItemModel* ITunesFeature::sidebarModel() const {
    return m_pSidebarModel;
}

QString ITunesFeature::showOpenDialog() {
    return QFileDialog::getOpenFileName(nullptr,
            tr("Select your iTunes library"),
            QDir::homePath(),
            "iTunes XML (*.xml)");
}

bool ITunesFeature::isMacOSImporterUsed() {
    return isMacOSImporterAvailable() && m_dbfile.isEmpty();
}

void ITunesFeature::onRightClick(const QPoint& globalPos) {
    BaseExternalLibraryFeature::onRightClick(globalPos);
    QMenu menu(m_pSidebarWidget);
    QAction useDefault(tr("Use Default Library"), &menu);
    QAction chooseNew(tr("Choose Library..."), &menu);
    menu.addAction(&useDefault);
    menu.addAction(&chooseNew);
    QAction *chosen(menu.exec(globalPos));
    if (chosen == &useDefault) {
        SettingsDAO settings(m_database);
        settings.setValue(kItdbPathKey, QString());
        activate(true); // clears tables before parsing
    } else if (chosen == &chooseNew) {
        SettingsDAO settings(m_database);
        QString dbfile = showOpenDialog();

        mixxx::FileInfo dbFileInfo(dbfile);
        if (!dbFileInfo.checkFileExists()) {
            return;
        }
        // The user has picked a new directory via a file dialog. This means the
        // system sandboxer (if we are sandboxed) has granted us permission to
        // this folder. Create a security bookmark while we have permission so
        // that we can access the folder on future runs. We need to canonicalize
        // the path so we first wrap the directory string with a QDir.
        Sandbox::createSecurityToken(&dbFileInfo);

        settings.setValue(kItdbPathKey, dbfile);
        activate(true); // clears tables before parsing
    }
}

QString ITunesFeature::getiTunesMusicPath() {
    QString musicFolder;
    if (isMacOSImporterAvailable()) {
        qDebug() << "Using empty iTunes music path (which we interpret as "
                    "using ITunesMacOSImporter)";
        return "";
    }
#if defined(__APPLE__)
    musicFolder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
                  + "/iTunes/iTunes Music Library.xml";
#elif defined(__WINDOWS__)
    musicFolder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
                  + "\\iTunes\\iTunes Music Library.xml";
#else
    musicFolder = "";
#endif
    qDebug() << "ITunesLibrary=[" << musicFolder << "]";
    return musicFolder;
}

std::unique_ptr<ITunesImporter> ITunesFeature::makeImporter() {
    std::unique_ptr<ITunesDAO> dao = std::make_unique<ITunesDAO>();
    dao->initialize(m_database);
#ifdef __MACOS_ITUNES_LIBRARY__
    if (isMacOSImporterUsed()) {
        qDebug() << "Using ITunesMacOSImporter to read default iTunes library";
        return std::make_unique<ITunesMacOSImporter>(this, std::move(dao));
    }
#endif
    qDebug() << "Using ITunesXMLImporter to read iTunes library from " << m_dbfile;
    return std::make_unique<ITunesXMLImporter>(this, m_dbfile, std::move(dao));
}

// This method is executed in a separate thread
// via QtConcurrent::run
TreeItem* ITunesFeature::importLibrary() {
    //Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    qDebug() << "ITunesFeature::importLibrary() ";

    ScopedTransaction transaction(m_database);

    std::unique_ptr<ITunesImporter> importer = makeImporter();
    ITunesImport iTunesImport = importer->importLibrary();

    // Even if an error occurred, commit the transaction. The file may have been
    // half-parsed.
    transaction.commit();

    return iTunesImport.playlistRoot.release();
}

void ITunesFeature::clearTable(const QString& table_name) {
    QSqlQuery query(m_database);
    query.prepare("delete from "+table_name);
    bool success = query.exec();

    if (!success) {
        qDebug() << "Could not delete remove old entries from table "
                 << table_name << " : " << query.lastError();
    } else {
        qDebug() << "iTunes table entries of '"
                 << table_name <<"' have been cleared.";
    }
}

void ITunesFeature::onTrackCollectionLoaded() {
    std::unique_ptr<TreeItem> root(m_future.result());
    if (root) {
        m_pSidebarModel->setRootItem(std::move(root));

        // Tell the rhythmbox track source that it should re-build its index.
        m_trackSource->buildIndex();

        //m_pITunesTrackModel->select();
        emit showTrackModel(m_pITunesTrackModel);
        qDebug() << "Itunes library loaded: success";
    } else {
        QMessageBox::warning(
                nullptr,
                tr("Error Loading iTunes Library"),
                tr("There was an error loading your iTunes library. Check the logs for details."));
    }
    // calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
    m_title = tr("iTunes");
    emit featureLoadingFinished(this);
    activate();
}
