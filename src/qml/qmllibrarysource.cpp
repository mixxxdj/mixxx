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
#include "qmlconfigproxy.h"
#include "qmllibraryproxy.h"
#include "qmlplaylistproxy.h"
#include "track/track.h"

AllTrackLibraryFeature::AllTrackLibraryFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : LibraryFeature(pLibrary, pConfig, QStringLiteral("tracks")),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_pLibraryTableModel(pLibrary->trackTableModel()) {
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);
    m_pSidebarModel->setRootItem(std::move(pRootItem));
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
          m_creatable(false) {
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
void QmlLibraryPlaylistSource::create(const QString& name) const {
    // TODO new interface to allow name
    m_pLibraryFeature->slotCreatePlaylist();
}

QList<QmlPlaylistProxy*> QmlLibraryPlaylistSource::list() {
    QList<QmlPlaylistProxy*> list;
    auto& playlistDao = m_pLibraryFeature->dao();
    for (const auto& [id, name] : playlistDao.getPlaylists(PlaylistDAO::PLHT_NOT_HIDDEN)) {
        auto* pCrate = new QmlPlaylistProxy(this, playlistDao, id, name);
        QQmlEngine::setObjectOwnership(pCrate, QQmlEngine::JavaScriptOwnership);
        list.append(pCrate);
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

void QmlLibraryCrateSource::create(const QString& name) const {
    // TODO new interface to allow name
    m_pLibraryFeature->slotCreateCrate();
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
        auto* pCrate = new QmlCrateProxy(this, trackCollectionManager, crate);
        QQmlEngine::setObjectOwnership(pCrate, QQmlEngine::JavaScriptOwnership);
        list.append(pCrate);
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
