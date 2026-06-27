#include "qml/qmllibraryproxy.h"

#include <QAbstractItemModel>

#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_qmllibraryproxy.cpp"
#include "qml/qmllibrarytracklistmodel.h"
#include "qmltrackproxy.h"
#include "track/track.h"
#include "util/assert.h"

namespace mixxx {
namespace qml {

QmlLibraryProxy::QmlLibraryProxy(
        std::shared_ptr<Library> pLibrary, QObject* parent)
        : QObject(parent),
          m_pLibrary(pLibrary),
          m_pModelProperty(new QmlLibraryTrackListModel(
                  QList<QmlLibraryTrackListColumn*>{}, m_pLibrary->trackTableModel(), this)),
          m_pScanner(new QmlLibraryScannerProxy(
                  m_pLibrary->trackCollectionManager()->scanner(), this)) {
}

QmlLibraryScannerProxy::QmlLibraryScannerProxy(LibraryScanner* libraryScanner, QObject* parent)
        : QObject(parent),
          m_pLibraryScanner(libraryScanner),
          m_running(false),
          m_cancelling(false) {
    connect(libraryScanner,
            &LibraryScanner::progressLoading,
            this,
            &QmlLibraryScannerProxy::progress);
    connect(libraryScanner,
            &LibraryScanner::scanStarted,
            this,
            &QmlLibraryScannerProxy::started);
    connect(libraryScanner,
            &LibraryScanner::scanFinished,
            this,
            &QmlLibraryScannerProxy::finished);
    connect(this,
            &QmlLibraryScannerProxy::requestCancel,
            libraryScanner,
            &LibraryScanner::slotCancel);

    // Properties
    connect(libraryScanner,
            &LibraryScanner::scanStarted,
            this,
            [this]() {
                m_cancelling = false;
                m_running = true;
                emit stateChanged();
            });
    connect(libraryScanner,
            &LibraryScanner::scanFinished,
            this,
            [this]() {
                m_cancelling = false;
                m_running = false;
                emit stateChanged();
            });
}
void QmlLibraryProxy::analyze(const QmlTrackProxy* track) const {
    VERIFY_OR_DEBUG_ASSERT(track && track->internal()) {
        return;
    }
    emit s_pLibrary->analyzeTracks({track->internal()->getId()});
}

// static
QmlLibraryProxy* QmlLibraryProxy::create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine) {
    // The implementation of this method is mostly taken from the code example
    // that shows the replacement for `qmlRegisterSingletonInstance()` when
    // using `QML_SINGLETON`.
    // https://doc.qt.io/qt-6/qqmlengine.html#QML_SINGLETON

    // The instance has to exist before it is used. We cannot replace it.
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet";
        return nullptr;
    }
    return new QmlLibraryProxy(s_pLibrary, pQmlEngine);
}

QmlLibraryProxy::AddResult QmlLibraryProxy::addSource(
        const QString& newPath) {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet!";
        return QmlLibraryProxy::AddResult::InvalidOrMissingDirectory;
    }
    QDir directory(newPath);
    Sandbox::createSecurityTokenForDir(directory);
    return static_cast<QmlLibraryProxy::AddResult>(
            s_pLibrary->trackCollectionManager()->addDirectory(
                    mixxx::FileInfo(newPath)));
}

QmlLibraryProxy::RemoveResult QmlLibraryProxy::removeSource(
        const QString& oldPath, SourceRemovalType removalType) {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet!";
        return QmlLibraryProxy::RemoveResult::NotFound;
    }

    DirectoryDAO::RemoveResult result =
            s_pLibrary->trackCollectionManager()->removeDirectory(mixxx::FileInfo(oldPath));
    if (result != DirectoryDAO::RemoveResult::Ok) {
        return static_cast<QmlLibraryProxy::RemoveResult>(result);
    }

    switch (removalType) {
    case SourceRemovalType::KeepTracks:
        break;
    case SourceRemovalType::HideTracks:
        // Mark all tracks in this directory as deleted but DON'T purge them
        // in case the user re-adds them manually.
        s_pLibrary->trackCollectionManager()->hideAllTracks(oldPath);
        break;
    case SourceRemovalType::PurgeTracks:
        // The user requested that we purge all metadata.
        s_pLibrary->trackCollectionManager()->purgeAllTracks(oldPath);
        break;
    default:
        DEBUG_ASSERT(!"unreachable");
    }
    return static_cast<QmlLibraryProxy::RemoveResult>(result);
}

QmlLibraryProxy::RelocateResult QmlLibraryProxy::relinkSource(
        const QString& oldPath, const QString& newPath) {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet!";
        return QmlLibraryProxy::RelocateResult::SqlError;
    }
    return static_cast<QmlLibraryProxy::RelocateResult>(
            s_pLibrary->trackCollectionManager()->relocateDirectory(
                    oldPath, newPath));
}

// Static
qsizetype QmlLibraryProxy::sources_count(QQmlListProperty<QmlLibrarySource>* pList) {
    QmlLibraryProxy* pLibrary = static_cast<QmlLibraryProxy*>(pList->object);
    VERIFY_OR_DEBUG_ASSERT(pLibrary) {
        return 0;
    }
    return pLibrary->m_pLibrary->trackCollectionManager()
            ->internalCollection()
            ->getRootDirectories()
            .size();
}

// Static
QmlLibrarySource* QmlLibraryProxy::sources_at(
        QQmlListProperty<QmlLibrarySource>* pList, qsizetype index) {
    VERIFY_OR_DEBUG_ASSERT(pList && pList->object) {
        return nullptr;
    }
    QmlLibraryProxy* pLibrary = static_cast<QmlLibraryProxy*>(pList->object);
    VERIFY_OR_DEBUG_ASSERT(pLibrary) {
        return nullptr;
    }
    return make_qml_owned<QmlLibrarySource>(
            pLibrary->m_pLibrary->trackCollectionManager()
                    ->internalCollection()
                    ->getRootDirectories()
                    .at(index));
}

// Static
void QmlLibraryProxy::sources_clear(QQmlListProperty<QmlLibrarySource>*) {
    DEBUG_ASSERT(!"unsupported operation");
}

} // namespace qml
} // namespace mixxx
