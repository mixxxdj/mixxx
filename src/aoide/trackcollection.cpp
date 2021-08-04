#include "aoide/trackcollection.h"

#include <QElapsedTimer>

#include "aoide/activecollectionagent.h"
#include "aoide/libraryfeature.h"
#include "aoide/subsystem.h"
#include "aoide/web/purgecollectedtrackstask.h"
#include "aoide/web/relocatecollectedtrackstask.h"
#include "aoide/web/replacecollectedtrackstask.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackloader.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide TrackCollection");

constexpr int kDisconnectSleepMillis = 50;
constexpr int kDisconnectTimeoutMillis = 10000;

} // anonymous namespace

namespace aoide {

TrackCollection::TrackCollection(
        SyncMode syncMode,
        TrackCollectionManager* trackCollectionManager,
        UserSettingsPointer userSettings)
        : ExternalTrackCollection(trackCollectionManager),
          m_syncMode(syncMode),
          m_trackLoader(new mixxx::TrackLoader(trackCollectionManager, this)),
          m_subsystem(new Subsystem(userSettings, m_trackLoader, this)),
          m_activeCollectionAgent(new ActiveCollectionAgent(
                  m_subsystem, trackCollectionManager, this)),
          m_connectionState(ConnectionState::Disconnected) {
    connect(m_subsystem,
            &Subsystem::connected,
            this,
            &TrackCollection::onSubsystemConnected);
    connect(m_subsystem,
            &Subsystem::disconnected,
            this,
            &TrackCollection::onSubsystemDisconnected);
    connect(m_subsystem,
            &Subsystem::collectionsChanged,
            this,
            &TrackCollection::onSubsystemCollectionsChanged);
}

void TrackCollection::establishConnection() {
    VERIFY_OR_DEBUG_ASSERT(
            m_connectionState == ConnectionState::Disconnected) {
        return;
    }
    m_connectionState = ConnectionState::Connecting;
    m_subsystem->startUp();
}

void TrackCollection::finishPendingTasksAndDisconnect() {
    if (m_connectionState == ConnectionState::Disconnected) {
        return;
    }
    m_connectionState = ConnectionState::Disconnecting;
    m_subsystem->invokeShutdown();
    // TODO: The polling should be done by the TrackCollectionManager
    // instead of every TrackCollection individually.
    QElapsedTimer disconnectTimer;
    disconnectTimer.start();
    while (m_connectionState != ConnectionState::Disconnected &&
            !disconnectTimer.hasExpired(kDisconnectTimeoutMillis)) {
        QThread::msleep(kDisconnectSleepMillis);
        QCoreApplication::processEvents();
    }
    if (m_connectionState == ConnectionState::Disconnected) {
        kLogger.info() << "Disconnected after" << disconnectTimer.elapsed() << "ms";
    } else {
        kLogger.warning() << "Disconnecting timed out after" << disconnectTimer.elapsed() << "ms";
    }
}

void TrackCollection::onSubsystemConnected() {
    if (m_connectionState == ConnectionState::Disconnecting) {
        // Disconnect requested while connecting
        return;
    }
    DEBUG_ASSERT(m_connectionState == ConnectionState::Connecting);
    // An active collection is required until fully connected!
    onSubsystemCollectionsChanged(Subsystem::ACTIVE_COLLECTION);
}

void TrackCollection::onSubsystemDisconnected() {
    DEBUG_ASSERT(m_connectionState == ConnectionState::Disconnecting);
    m_connectionState = ConnectionState::Disconnected;
    emit connectionStateChanged(m_connectionState);
}

void TrackCollection::onSubsystemCollectionsChanged(int flags) {
    Q_UNUSED(flags)
    if (m_subsystem->isConnected() && m_subsystem->activeCollection()) {
        if (m_connectionState == ConnectionState::Connecting) {
            m_connectionState = ConnectionState::Connected;
            emit connectionStateChanged(m_connectionState);
        }
    } else {
        if (m_connectionState == ConnectionState::Connected) {
            m_connectionState = ConnectionState::Connecting;
            emit connectionStateChanged(m_connectionState);
        }
    }
}

ExternalTrackCollection::ConnectionState TrackCollection::connectionState() const {
    DEBUG_ASSERT(m_subsystem->isConnected() ||
            m_connectionState != ConnectionState::Connected);
    return m_connectionState;
}

QString TrackCollection::name() const {
    return tr("aoide");
}

QString TrackCollection::description() const {
    return tr("aoide Music Library");
}

void TrackCollection::relocateDirectory(
        const QString& oldRootDir,
        const QString& newRootDir) {
    if (m_syncMode != SyncMode::ReadWrite) {
        return;
    }
    kLogger.debug()
            << "Relocating directory:"
            << oldRootDir
            << "->"
            << newRootDir;
    auto* task = m_subsystem->relocateAllTracks(oldRootDir, newRootDir);
    task->invokeStart();
}

void TrackCollection::purgeTracks(
        const QList<QString>& trackLocations) {
    if (m_syncMode != SyncMode::ReadWrite) {
        return;
    }
    kLogger.debug()
            << "Purging tracks:"
            << trackLocations;
    auto* task = m_subsystem->purgeTracks(trackLocations);
    task->invokeStart();
}

void TrackCollection::purgeAllTracks(
        const QDir& rootDir) {
    if (m_syncMode != SyncMode::ReadWrite) {
        return;
    }
    kLogger.debug()
            << "Purging all tracks:"
            << rootDir;
    auto* task = m_subsystem->purgeAllTracks(rootDir);
    task->invokeStart();
}

void TrackCollection::updateTracks(
        const QList<TrackRef>& updatedTracks) {
    if (m_syncMode != SyncMode::ReadWrite) {
        return;
    }
    kLogger.debug()
            << "Updating tracks:"
            << updatedTracks;
    m_subsystem->scheduleReplaceTracks(updatedTracks);
}

void TrackCollection::saveTrack(
        const Track& track,
        ChangeHint changeHint) {
    Q_UNUSED(changeHint)
    DEBUG_ASSERT(track.getId().isValid());
    DEBUG_ASSERT(track.getDateAdded().isValid());
    if (m_syncMode != SyncMode::ReadWrite) {
        return;
    }
    kLogger.debug()
            << "Saving track:"
            << track.getId()
            << track.getFileInfo();
    auto* task = m_subsystem->replaceTrack(track);
    task->invokeStart();
}

::LibraryFeature* TrackCollection::newLibraryFeature(
        Library* library,
        UserSettingsPointer userSettings) {
    return new LibraryFeature(
            library,
            userSettings,
            m_subsystem);
}

} // namespace aoide
