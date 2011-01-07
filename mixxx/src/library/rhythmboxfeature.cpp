#include <QMessageBox>
#include <QtDebug>
#include <QStringList>

#include "library/proxytrackmodel.h"
#include "library/rhythmboxtrackmodel.h"
#include "library/rhythmboxplaylistmodel.h"
#include "library/rhythmboxfeature.h"
#include "treeitem.h"

RhythmboxFeature::RhythmboxFeature(QObject* parent)
    : LibraryFeature(parent) {

    m_pRhythmboxTrackModel = NULL;
    m_pTrackModelProxy = NULL;
    m_pRhythmboxPlaylistModel = NULL;
    m_pPlaylistModelProxy = NULL;

}

RhythmboxFeature::~RhythmboxFeature() {

}

bool RhythmboxFeature::isSupported() {
    return (QFile::exists(MIXXX_RHYTHMBOX_DB_LOCATION) ||
            QFile::exists(MIXXX_RHYTHMBOX_DB_LOCATION_ALT));
}

QVariant RhythmboxFeature::title() {
    return tr("Rhythmbox");
}

QIcon RhythmboxFeature::getIcon() {
    return QIcon(":/images/library/ic_library_rhythmbox.png");
}

TreeItemModel* RhythmboxFeature::getChildModel() {
    return &m_childModel;
}

void RhythmboxFeature::activate() {
    //qDebug("RhythmboxFeature::activate()");

    if (!m_pRhythmboxTrackModel) {
        if (QMessageBox::question(
            NULL,
            tr("Load Rhythmbox Library?"),
            tr("Would you like to load your Rhythmbox library?"),
            QMessageBox::Ok,
            QMessageBox::Cancel)
            == QMessageBox::Cancel) {
            return;
        }

        m_pRhythmboxTrackModel = new RhythmboxTrackModel();
        m_pTrackModelProxy = new ProxyTrackModel(m_pRhythmboxTrackModel);
        m_pTrackModelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_pTrackModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

        m_pRhythmboxPlaylistModel = new RhythmboxPlaylistModel(m_pRhythmboxTrackModel);
        m_pPlaylistModelProxy = new ProxyTrackModel(m_pRhythmboxPlaylistModel);
        m_pPlaylistModelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_pPlaylistModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

        TreeItem *rootItem = new TreeItem("$root","$root", this);
        for (int i = 0; i < m_pRhythmboxPlaylistModel->numPlaylists(); ++i) {
            QString playlist_name = m_pRhythmboxPlaylistModel->playlistTitle(i);
            TreeItem *item = new TreeItem(playlist_name, playlist_name, this,rootItem);
            
            rootItem->appendChild(item);
        }
        m_childModel.setRootItem(rootItem);
    }
    emit(showTrackModel(m_pTrackModelProxy));
}

void RhythmboxFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "RhythmboxFeature::activateChild()" << index;
    QString playlist = index.data().toString();
    qDebug() << "Activating " << playlist;
    m_pRhythmboxPlaylistModel->setPlaylist(playlist);
    emit(showTrackModel(m_pPlaylistModelProxy));
}

void RhythmboxFeature::onRightClick(const QPoint& globalPos) {
}

void RhythmboxFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
}

bool RhythmboxFeature::dropAccept(QUrl url) {
    return false;
}

bool RhythmboxFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool RhythmboxFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool RhythmboxFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}


