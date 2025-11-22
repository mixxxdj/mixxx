#include "qml/qmllibrarysource.h"

#include <qalgorithms.h>
#include <qlist.h>
#include <qqmlengine.h>

#include <QAbstractListModel>
#include <QVariant>
#include <QtDebug>
#include <memory>

#include "library/browse/browsefeature.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/cratefeature.h"
#include "library/trackset/crate/cratesummary.h"
#include "library/trackset/playlistfeature.h"
#include "library/treeitemmodel.h"
#include "moc_qmllibrarysource.cpp"
#include "qml_owned_ptr.h"
#include "qmlconfigproxy.h"
#include "qmllibraryproxy.h"
#include "qmlplaylistproxy.h"
#include "track/track.h"

AllTrackLibraryFeature::AllTrackLibraryFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : LibraryFeature(pLibrary, pConfig, QStringLiteral("")),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_pLibraryTableModel(pLibrary->trackTableModel()) {
    m_pSidebarModel->setRootItem(TreeItem::newRoot(this));
}

void AllTrackLibraryFeature::activate() {
    emit showTrackModel(m_pLibraryTableModel);
}

namespace mixxx {
namespace qml {

QmlLibrarySource::QmlLibrarySource(
        QObject* parent, const QList<QmlLibraryTrackListColumn*>& columns)
        : QObject(parent),
          m_columns(columns),
          m_capabilities(0) {
}

void QmlLibrarySource::slotShowTrackModel(QAbstractItemModel* pModel) {
    emit requestTrackModel(std::make_shared<QmlLibraryTrackListModel>(columns(), pModel));
}

QmlLibraryAllTrackSource::QmlLibraryAllTrackSource(
        QObject* parent, const QList<QmlLibraryTrackListColumn*>& columns)
        : QmlLibrarySource(parent, columns),
          m_pLibraryFeature(std::make_unique<AllTrackLibraryFeature>(
                  QmlLibraryProxy::get(), QmlConfigProxy::get())) {
    connect(m_pLibraryFeature.get(),
            &LibraryFeature::showTrackModel,
            this,
            &QmlLibrarySource::slotShowTrackModel);
}
QmlLibraryPlaylistSource::QmlLibraryPlaylistSource(
        QObject* parent, const QList<QmlLibraryTrackListColumn*>& columns)
        : QmlLibrarySource(parent, columns),
          m_pLibraryFeature(std::make_unique<PlaylistFeature>(
                  QmlLibraryProxy::get(), QmlConfigProxy::get())) {
    connect(m_pLibraryFeature.get(),
            &LibraryFeature::showTrackModel,
            this,
            &QmlLibrarySource::slotShowTrackModel);
}
QmlLibraryPlaylistSource::PlaylistCreateResult QmlLibraryPlaylistSource::create(
        const QString& name) const {
    auto& dao = m_pLibraryFeature->dao();
    int existingId = dao.getPlaylistIdFromName(name);

    if (existingId != kInvalidPlaylistId) {
        return PlaylistCreateResult::ConflictName;
    } else if (name.isEmpty()) {
        return PlaylistCreateResult::InvalidName;
    }

    int playlistId = dao.createPlaylist(name);

    if (playlistId == kInvalidPlaylistId) {
        return PlaylistCreateResult::Unknown;
    }

    return PlaylistCreateResult::Ok;
}
QmlPlaylistProxy* QmlLibraryPlaylistSource::get(const QString& name) {
    auto& playlistDao = m_pLibraryFeature->dao();
    int existingId = playlistDao.getPlaylistIdFromName(name);

    if (existingId == kInvalidPlaylistId) {
        return nullptr;
    }

    return make_qml_owned<QmlPlaylistProxy>(this, playlistDao, existingId, name);
}

QList<QmlPlaylistProxy*> QmlLibraryPlaylistSource::list() {
    QList<QmlPlaylistProxy*> list;
    auto& playlistDao = m_pLibraryFeature->dao();
    for (const auto& [id, name] : playlistDao.getPlaylists(PlaylistDAO::PLHT_NOT_HIDDEN)) {
        list.append(make_qml_owned<QmlPlaylistProxy>(this, playlistDao, id, name));
    }
    return list;
}

QmlLibraryCrateSource::QmlLibraryCrateSource(
        QObject* parent, const QList<QmlLibraryTrackListColumn*>& columns)
        : QmlLibrarySource(parent, columns),
          m_pLibraryFeature(std::make_unique<CrateFeature>(
                  QmlLibraryProxy::get(), QmlConfigProxy::get())) {
    connect(m_pLibraryFeature.get(),
            &LibraryFeature::showTrackModel,
            this,
            &QmlLibrarySource::slotShowTrackModel);
}

QmlLibraryCrateSource::CrateCreateResult QmlLibraryCrateSource::create(const QString& name) const {
    const auto& pTrackCollectionManager = m_pLibraryFeature->trackCollection();
    if (name.isEmpty()) {
        return CrateCreateResult::InvalidName;
    }
    if (pTrackCollectionManager->crates().readCrateByName(name)) {
        return CrateCreateResult::ConflictName;
    }
    Crate newCrate;
    newCrate.setName(std::move(name));
    CrateId newCrateId;
    if (pTrackCollectionManager->insertCrate(newCrate, &newCrateId)) {
        DEBUG_ASSERT(newCrateId.isValid());
        newCrate.setId(newCrateId);
        qDebug() << "Created new crate" << newCrate;
    } else {
        DEBUG_ASSERT(!newCrateId.isValid());
        qWarning() << "Failed to create new crate"
                   << "->" << newCrate.getName();
        return CrateCreateResult::Unknown;
    }
    return CrateCreateResult::Ok;
}

QList<QmlCrateProxy*> QmlLibraryCrateSource::list(const QList<QmlTrackProxy*>& tracks) {
    QList<QmlCrateProxy*> list;
    auto& crateDao = m_pLibraryFeature;
    auto* trackCollectionManager = QmlLibraryProxy::get()
                                           ->trackCollectionManager()
                                           ->internalCollection();

    QList<TrackId> trackIds;

    for (const auto& track : tracks) {
        trackIds.append(track->internal()->getId());
    }

    CrateSummarySelectResult allCrates(
            trackCollectionManager
                    ->crates()
                    .selectCratesWithTrackCount({trackIds}));

    CrateSummary crate;
    while (allCrates.populateNext(&crate)) {
        list.append(make_qml_owned<QmlCrateProxy>(this, trackCollectionManager, crate));
    }
    return list;
}

QmlLibraryExplorerSource::QmlLibraryExplorerSource(
        QObject* parent, const QList<QmlLibraryTrackListColumn*>& columns)
        : QmlLibrarySource(parent, columns),
          m_pLibraryFeature(std::make_unique<BrowseFeature>(
                  QmlLibraryProxy::get(),
                  QmlConfigProxy::get(),
                  // TODO acquire recording manager from singleton implemented
                  nullptr)) {
    connect(m_pLibraryFeature.get(),
            &LibraryFeature::showTrackModel,
            this,
            &QmlLibrarySource::slotShowTrackModel);
}

} // namespace qml
} // namespace mixxx
