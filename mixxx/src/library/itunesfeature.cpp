#include <QtDebug>

#include "library/itunesfeature.h"

#include "library/itunestrackmodel.h"
//#include "library/rhythmboxplaylistmodel.h"
#include "library/proxytrackmodel.h"

ITunesFeature::ITunesFeature(QObject* parent)
    : LibraryFeature(parent) {
    m_pITunesTrackModel = new ITunesTrackModel();
    m_pTrackModelProxy = new ProxyTrackModel(m_pITunesTrackModel);
    m_pTrackModelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pTrackModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    //m_pITunesPlaylistModel = new ITunesPlaylistModel(m_pITunesTrackModel);
}

ITunesFeature::~ITunesFeature() {

}

QVariant ITunesFeature::title() {
    return tr("ITunes");
}

QIcon ITunesFeature::getIcon() {
    return QIcon(":/images/library/rhythmbox.png");
}

int ITunesFeature::numChildren() {
    //return m_pITunesPlaylistModel->numPlaylists();
    return 0;
}

QVariant ITunesFeature::child(int n) {
    //return QVariant(m_pITunesPlaylistModel->playlistTitle(n));
}

void ITunesFeature::activate() {
    qDebug("ITunesFeature::activate()");
    emit(showTrackModel(m_pTrackModelProxy));
}

void ITunesFeature::activateChild(int n) {
    /*
    qDebug("ITunesFeature::activateChild(%d)", n);
    QString playlist = m_pITunesPlaylistModel->playlistTitle(n);
    qDebug() << "Activating " << playlist;
    m_pITunesPlaylistModel->setPlaylist(playlist);
    emit(showTrackModel(m_pITunesPlaylistModel));
    */
}

void ITunesFeature::onRightClick(const QPoint& globalPos, QModelIndex index) {
}

void ITunesFeature::onClick(QModelIndex index) {
}

bool ITunesFeature::dropAccept(const QModelIndex& index, QUrl url) {
    return false;
}
bool ITunesFeature::dragMoveAccept(const QModelIndex& index, QUrl url) {
    return false;
}
