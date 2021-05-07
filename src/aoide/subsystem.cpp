#include "aoide/subsystem.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QProcessEnvironment>
#include <QUrl>

#include "aoide/trackexport.h"
#include "aoide/web/listcollectionstask.h"
#include "library/trackloader.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace {

const mixxx::Logger kLogger("aoide Subsystem");

const QString kExecutableName = QStringLiteral(
#if defined(__WINDOWS__)
        "aoide.exe"
#else
        "aoide"
#endif
);

const QString kDatabaseFileName = QStringLiteral("aoide.sqlite");

const QString kProcessEnvLogLevel = QStringLiteral("RUST_LOG");
const QString kProcessEnvEndpointIp = QStringLiteral("ENDPOINT_IP");
const QString kProcessEnvEndpointPort = QStringLiteral("ENDPOINT_PORT");
const QString kProcessEnvDatabaseUrl = QStringLiteral("DATABASE_URL");

// The shutdown is delayed until all pending write requests have
// been finished. This timeout controls how long to wait for those
// pending write requests.
const int kProcessShutdownTimeoutMillis = 10000;

const QString kThreadName = QStringLiteral("aoide");

const QThread::Priority kThreadPriority = QThread::LowPriority;

const QString kDefaultCollectionKind = QStringLiteral("mixxx.org");

} // anonymous namespace

namespace aoide {

namespace {

void startProcess(
        QProcess& process,
        const Settings& settings) {
    auto command = settings.command();
    if (command.isEmpty()) {
        // Try to load the executable from the settings folder first
        command = QDir(settings.getSettingsPath()).filePath(kExecutableName);
        if (!QFileInfo::exists(command)) {
            // ...otherwise try to load the executable from the application folder
            command = QDir(QCoreApplication::applicationDirPath()).filePath(kExecutableName);
        }
    }
    QFileInfo cmdFile(command);
    if (!cmdFile.exists()) {
        kLogger.info()
                << "Executable file not found"
                << command;
        command = kExecutableName;
    }
    QProcessEnvironment processEnvironment = process.processEnvironment();
    QString logLevel = processEnvironment.value(kProcessEnvLogLevel);
    if (logLevel.isEmpty()) {
        if (kLogger.traceEnabled()) {
            logLevel = QStringLiteral("trace");
        } else if (kLogger.debugEnabled()) {
            logLevel = QStringLiteral("debug");
        } else if (kLogger.infoEnabled()) {
            logLevel = QStringLiteral("info");
        } else {
            logLevel = QStringLiteral("warning");
        }
        processEnvironment.insert(
                kProcessEnvLogLevel,
                logLevel);
    }
    kLogger.info() << kProcessEnvLogLevel << '=' << logLevel;
    QUrl databaseUrl = QUrl(processEnvironment.value(kProcessEnvDatabaseUrl));
    if (databaseUrl.isEmpty()) {
        QString database = settings.database();
        if (database.isEmpty()) {
            database = QDir(settings.getSettingsPath()).filePath(kDatabaseFileName);
        } else {
            kLogger.info() << "Using database file" << database << "from settings";
        }
        databaseUrl = QUrl::fromLocalFile(database);
        processEnvironment.insert(
                kProcessEnvDatabaseUrl,
                databaseUrl.toString(QUrl::None));
    }
    kLogger.info() << kProcessEnvDatabaseUrl << '=' << databaseUrl;
    auto dbFile = QFileInfo(databaseUrl.toLocalFile());
    if (dbFile.exists()) {
        kLogger.info()
                << "Using existing database file"
                << dbFile.absoluteFilePath();
    } else {
        kLogger.info()
                << "Creating new database file"
                << dbFile.filePath();
    }
    QString endpointIp = processEnvironment.value(kProcessEnvEndpointIp);
    if (endpointIp.isEmpty()) {
        endpointIp = settings.host();
        kLogger.info() << "Using endpoint host IP" << endpointIp << "from settings";
        processEnvironment.insert(
                kProcessEnvEndpointIp,
                endpointIp);
    }
    QString endpointPort = processEnvironment.value(kProcessEnvEndpointPort);
    if (endpointPort.isEmpty()) {
        endpointPort = settings.port();
        kLogger.info() << "Using endpoint port" << endpointPort << "from settings";
        processEnvironment.insert(
                kProcessEnvEndpointPort,
                endpointPort);
    }
#ifdef MIXXX_BUILD_DEBUG
    processEnvironment.insert(
            QStringLiteral("RUST_BACKTRACE"),
            QStringLiteral("1"));
#endif
    process.setProcessEnvironment(processEnvironment);
    kLogger.info()
            << "Starting process"
            << command
            << "with environment"
            << processEnvironment.toStringList();
    process.start(command, QStringList{});
}

bool findCollectionByUid(
        const QVector<json::CollectionEntity>& allCollections,
        const QString& collectionUid,
        std::optional<json::CollectionEntity>* result = nullptr) {
    for (const auto& collection : allCollections) {
        if (collectionUid == collection.header().uid()) {
            if (result) {
                *result = collection;
            }
            return true;
        }
    }
    if (result) {
        *result = std::nullopt;
    }
    return false;
}

} // anonymous namespace

Subsystem::Subsystem(
        UserSettingsPointer userSettings,
        mixxx::TrackLoader* trackLoader,
        QObject* parent)
        : QObject(parent),
          m_settings(std::move(userSettings)),
          m_trackLoader(trackLoader) {
    DEBUG_ASSERT(!isConnected());
}

Subsystem::~Subsystem() {
    DEBUG_ASSERT(m_process.state() == QProcess::NotRunning);
}

void Subsystem::startUp() {
    connect(&m_process,
            &QProcess::readyReadStandardOutput,
            this,
            &Subsystem::onReadyReadStandardOutputFromProcess);
    connect(&m_process,
            &QProcess::readyReadStandardError,
            this,
            &Subsystem::onReadyReadStandardErrorFromProcess);
    startProcess(m_process, m_settings);
}

void Subsystem::onReadyReadStandardOutputFromProcess() {
    VERIFY_OR_DEBUG_ASSERT(!isConnected()) {
        kLogger.warning()
                << "Received unexpected output from process:"
                << QString::fromLocal8Bit(m_process.readAllStandardOutput());
        return;
    }
    const auto lines = QString::fromLocal8Bit(
            m_process.readAllStandardOutput())
                               .split(
                                       QChar('\n'),
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                                       Qt::SkipEmptyParts);
#else
                                       QString::SkipEmptyParts);
#endif
    for (auto&& line : lines) {
        auto endpointAddress = line.trimmed();
        if (!endpointAddress.isEmpty()) {
            kLogger.info() << "Received endpoint address" << endpointAddress;
            connectProcess(endpointAddress);
            DEBUG_ASSERT(isConnected());
            startThread();
            emit connected();
            return;
        }
    }
}

void Subsystem::onReadyReadStandardErrorFromProcess() {
    m_bufferedStandardErrorFromProcess += m_process.readAllStandardError();
    // Forward stderr from process into log file. Always submit a complete
    // chunk of lines that ends with the last newline character.
    const int lastNewlinePos = m_bufferedStandardErrorFromProcess.lastIndexOf('\n');
    if (lastNewlinePos >= 0) {
        DEBUG_ASSERT(lastNewlinePos < m_bufferedStandardErrorFromProcess.size());
        const auto leftSize = lastNewlinePos + 1;
        mixxx::Logging::writeMessage(m_bufferedStandardErrorFromProcess.left(leftSize));
        m_bufferedStandardErrorFromProcess = m_bufferedStandardErrorFromProcess.chopped(leftSize);
    }
    // The remaining data is buffered until more data is received
}

void Subsystem::connectProcess(QString endpointAddr) {
    DEBUG_ASSERT(!m_gateway);
    m_gateway = new Gateway(
            m_settings.baseUrl(std::move(endpointAddr)));
    m_gateway->moveToThread(&m_thread);
    connect(&m_thread, &QThread::finished, m_gateway, &QObject::deleteLater);

    DEBUG_ASSERT(!m_trackReplacementScheduler);
    m_trackReplacementScheduler =
            new TrackReplacementScheduler(m_gateway, m_trackLoader);
    m_trackReplacementScheduler->moveToThread(&m_thread);
    connect(&m_thread,
            &QThread::finished,
            m_trackReplacementScheduler,
            &QObject::deleteLater);
}

void Subsystem::invokeShutdown() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_process.state() == QProcess::NotRunning) {
        DEBUG_ASSERT(!m_gateway);
        return;
    }
    if (m_gateway) {
        m_gateway->invokeShutdown();
        return;
    }
    kLogger.warning() << "Unable to shut down the process gracefully";
    slotGatewayShuttingDown();
}

void Subsystem::slotGatewayShuttingDown() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_process.state() != QProcess::NotRunning) {
        if (!m_process.waitForFinished(kProcessShutdownTimeoutMillis)) {
            kLogger.warning() << "Killing child process";
        }
        m_process.close();
    }
    DEBUG_ASSERT(m_process.state() == QProcess::NotRunning);
    stopThread();
    emit disconnected();
}

void Subsystem::startThread() {
    kLogger.info() << "Starting thread";
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    m_thread.setObjectName(kThreadName);
    m_thread.start(kThreadPriority);
    connect(m_gateway,
            &Gateway::shuttingDown,
            this,
            &Subsystem::slotGatewayShuttingDown);
    connect(m_trackReplacementScheduler,
            &TrackReplacementScheduler::progress,
            this,
            &Subsystem::replacingTracksProgress); // signal/signal pass-through
    invokeRefreshCollections();
}

void Subsystem::stopThread() {
    kLogger.info() << "Stopping thread";
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    m_trackReplacementScheduler->invokeCancel();
    m_thread.quit();
    m_thread.wait();
    m_gateway = nullptr;
}

CreateCollectionTask* Subsystem::createCollection(json::Collection collection) {
    return m_gateway->createCollection(std::move(collection));
}

UpdateCollectionTask* Subsystem::updateCollection(
        json::CollectionEntity collectionEntity) {
    return m_gateway->updateCollection(std::move(collectionEntity));
}

DeleteCollectionTask* Subsystem::deleteCollection(QString collectionUid) {
    return m_gateway->deleteCollection(std::move(collectionUid));
}

void Subsystem::selectActiveCollection(const QString& collectionUid) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    QString activeCollectionUidBefore;
    if (m_activeCollection) {
        activeCollectionUidBefore = m_activeCollection->header().uid();
    }
    if (collectionUid.isEmpty()) {
        m_activeCollection = std::nullopt;
    } else {
        findCollectionByUid(m_allCollections, collectionUid, &m_activeCollection);
    }
    QString activeCollectionUidAfter;
    if (m_activeCollection) {
        activeCollectionUidAfter = m_activeCollection->header().uid();
        // Only overwrite the settings if a different collection
        // has actually been selected!
        m_settings.setCollectionUid(activeCollectionUidAfter);
        kLogger.info() << "Selected active collection:"
                       << *m_activeCollection;
    } else {
        kLogger.info() << "No active collection";
    }
    if (activeCollectionUidBefore != activeCollectionUidAfter) {
        emit collectionsChanged(CollectionsChangedFlags::ACTIVE_COLLECTION);
    }
}

SearchCollectedTracksTask* Subsystem::searchTracks(
        const QJsonObject& baseQuery,
        const TrackSearchOverlayFilter& overlayFilter,
        const QStringList& searchTerms,
        const Pagination& pagination) {
    // Accesses mutable member variables -> not thread-safe
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    return m_gateway->searchTracks(
            m_activeCollection->header().uid(),
            baseQuery,
            overlayFilter,
            searchTerms,
            pagination);
}

ResolveCollectedTracksTask* Subsystem::resolveTracksByUrl(QList<QUrl> trackUrls) {
    // Accesses mutable member variables -> not thread-safe
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!activeCollection()) {
        kLogger.warning() << "No active collection:"
                          << "Cannot resolve track URLs";
        return nullptr;
    }
    return m_gateway->resolveTracksByUrl(
            m_activeCollection->header().uid(), std::move(trackUrls));
}

void Subsystem::invokeRefreshCollections() {
    auto* task = m_gateway->listCollections(kDefaultCollectionKind);
    connect(task,
            &ListCollectionsTask::succeeded,
            this,
            &Subsystem::slotListCollectionsSucceeded);
    task->invokeStart();
}

ReplaceCollectedTracksTask* Subsystem::replaceTrack(
        const Track& track) {
    // Accesses mutable member variables -> not thread-safe
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!activeCollection()) {
        kLogger.warning() << "No active collection:"
                          << "Cannot replace track" << track.getFileInfo();
        return nullptr;
    }
    auto collectionUid = m_activeCollection->header().uid();
    auto jsonTrack =
            exportTrack(
                    m_activeCollection->body().mediaSourceConfig(),
                    track.getFileInfo(),
                    track.getRecord(),
                    track.getCuePoints());
    return m_gateway->replaceTracks(
            std::move(collectionUid),
            QList<json::Track>{jsonTrack});
}

RelocateCollectedTracksTask* Subsystem::relocateTracks(
        const QList<QPair<QString, QString>>& trackRelocations) {
    // Accesses mutable member variables -> not thread-safe
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!activeCollection()) {
        kLogger.warning() << "No active collection:"
                          << "Cannot relocate tracks" << trackRelocations;
        return nullptr;
    }
    return m_gateway->relocateTracks(
            m_activeCollection->header().uid(), trackRelocations);
}

RelocateCollectedTracksTask* Subsystem::relocateAllTracks(
        const QDir& oldRootDir, const QDir& newRootDir) {
    // Accesses mutable member variables -> not thread-safe
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!activeCollection()) {
        kLogger.warning() << "No active collection:"
                          << "Cannot relocate all tracks from" << oldRootDir
                          << "to" << newRootDir;
        return nullptr;
    }
    return m_gateway->relocateAllTracks(
            m_activeCollection->header().uid(), oldRootDir, newRootDir);
}

PurgeTracksTask* Subsystem::purgeTracks(const QStringList& trackLocations) {
    // Accesses mutable member variables -> not thread-safe
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!activeCollection()) {
        kLogger.warning() << "No active collection:"
                          << "Cannot purge tracks" << trackLocations;
        return nullptr;
    }
    return m_gateway->purgeTracks(
            m_activeCollection->header().uid(), trackLocations);
}
PurgeTracksTask* Subsystem::purgeAllTracks(const QDir& rootDir) {
    // Accesses mutable member variables -> not thread-safe
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!activeCollection()) {
        kLogger.warning() << "No active collection:"
                          << "Cannot purge tracks" << rootDir;
        return nullptr;
    }
    return m_gateway->purgeAllTracks(
            m_activeCollection->header().uid(), rootDir);
}

void Subsystem::scheduleReplaceTracks(QList<TrackRef> trackRefs) {
    // Accesses mutable member variables -> not thread-safe
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!activeCollection()) {
        kLogger.warning() << "No active collection:"
                          << "Cannot replace tracks" << trackRefs;
        return;
    }
    if (trackRefs.isEmpty()) {
        return;
    }
    m_trackReplacementScheduler->scheduleReplaceTracks(
            m_activeCollection->header().uid(),
            m_activeCollection->body().mediaSourceConfig(),
            std::move(trackRefs));
}

ListCollectedPlaylistsTask* Subsystem::listCollectedPlaylists(
        const QString& kind) {
    if (!activeCollection()) {
        kLogger.warning()
                << "Cannot list collected playlists without an active collection";
        return nullptr;
    }
    return m_gateway->listCollectedPlaylists(
            m_activeCollection->header().uid(),
            kind);
}

CreateCollectedPlaylistTask* Subsystem::createPlaylist(
        const json::Playlist& playlist) {
    if (!activeCollection()) {
        kLogger.warning()
                << "Cannot create a playlist without an active collection";
        return nullptr;
    }
    return m_gateway->createPlaylist(
            m_activeCollection->header().uid(),
            playlist);
}

DeletePlaylistTask* Subsystem::deletePlaylist(QString playlistUid) {
    return m_gateway->deletePlaylist(std::move(playlistUid));
}

PlaylistAppendTrackEntriesByUrlTask* Subsystem::playlistAppendTrackEntriesByUrl(
        json::EntityHeader playlistEntityHeader,
        QList<QUrl> trackUrls) {
    // Accesses mutable member variables -> not thread-safe
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!activeCollection()) {
        kLogger.warning() << "No active collection:"
                          << "Cannot add tracks to playlist by URL"
                          << playlistEntityHeader << trackUrls;
        return nullptr;
    }
    return m_gateway->playlistAppendTrackEntriesByUrl(m_activeCollection->header().uid(),
            std::move(playlistEntityHeader),
            std::move(trackUrls));
}

void Subsystem::slotListCollectionsSucceeded(
        QVector<json::CollectionEntity> result) {
    auto* task = qobject_cast<ListCollectionsTask*>(sender());
    VERIFY_OR_DEBUG_ASSERT(task) {
        return;
    }
    task->deleteLater();
    m_allCollections = std::move(result);
    int changedFlags = CollectionsChangedFlags::ALL_COLLECTIONS;
    if (activeCollection()) {
        if (!findCollectionByUid(m_allCollections,
                    m_activeCollection->header().uid(),
                    &m_activeCollection)) {
            // active collection has been reset
            kLogger.info()
                    << "Deselected active collection";
            changedFlags |= CollectionsChangedFlags::ACTIVE_COLLECTION;
        }
    } else {
        auto settingsCollectionUid = m_settings.collectionUid();
        for (auto&& collection : qAsConst(m_allCollections)) {
            if (collection.header().uid() == settingsCollectionUid) {
                m_activeCollection = collection;
                kLogger.info()
                        << "Reselected active collection:"
                        << m_activeCollection;
                changedFlags |= CollectionsChangedFlags::ACTIVE_COLLECTION;
                break; // exit loop
            }
        }
    }
    emit collectionsChanged(changedFlags);
}

} // namespace aoide
