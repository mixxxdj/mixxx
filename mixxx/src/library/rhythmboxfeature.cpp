#include <QtDebug>

#include "library/proxytrackmodel.h"
#include "library/rhythmboxtrackmodel.h"
#include "library/rhythmboxplaylistmodel.h"
#include "library/rhythmboxfeature.h"

RhythmboxFeature::RhythmboxFeature(QObject* parent)
    : LibraryFeature(parent) {
    m_pRhythmboxTrackModel = new RhythmboxTrackModel();
    m_pTrackModelProxy = new ProxyTrackModel(m_pRhythmboxTrackModel);
    m_pTrackModelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pTrackModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_pRhythmboxPlaylistModel = new RhythmboxPlaylistModel(m_pRhythmboxTrackModel);
    m_pPlaylistModelProxy = new ProxyTrackModel(m_pRhythmboxPlaylistModel);
    m_pPlaylistModelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pPlaylistModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
}

RhythmboxFeature::~RhythmboxFeature() {

}

QVariant RhythmboxFeature::title() {
    return tr("Rhythmbox");
}

QIcon RhythmboxFeature::getIcon() {
    return QIcon(":/images/library/rhythmbox.png");
}

int RhythmboxFeature::numChildren() {
    return m_pRhythmboxPlaylistModel->numPlaylists();
}

QVariant RhythmboxFeature::child(int n) {
    return QVariant(m_pRhythmboxPlaylistModel->playlistTitle(n));
}

void RhythmboxFeature::activate() {
    qDebug("RhythmboxFeature::activate()");
    emit(showTrackModel(m_pTrackModelProxy));
}

void RhythmboxFeature::activateChild(int n) {
    qDebug("RhythmboxFeature::activateChild(%d)", n);
    QString playlist = m_pRhythmboxPlaylistModel->playlistTitle(n);
    qDebug() << "Activating " << playlist;
    m_pRhythmboxPlaylistModel->setPlaylist(playlist);
    emit(showTrackModel(m_pPlaylistModelProxy));
}

void RhythmboxFeature::onRightClick(const QPoint& globalPos, QModelIndex index) {
}
void RhythmboxFeature::onClick(QModelIndex index) {
}
bool RhythmboxFeature::dropAccept(const QModelIndex& index, QUrl url) {
    return false;
}
bool RhythmboxFeature::dragMoveAccept(const QModelIndex& index, QUrl url) {
    return false;
}


