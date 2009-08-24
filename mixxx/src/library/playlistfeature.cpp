#include <QtDebug>

#include "library/playlisttablemodel.h"
#include "library/playlistfeature.h"

PlaylistFeature::PlaylistFeature(QObject* parent, TrackCollection* pTrackCollection)
    : LibraryFeature(parent) {
    playlists.push_back("My Rox");
    playlists.push_back("Trance");
    playlists.push_back("Technoize");
    
    m_pTrackCollection = pTrackCollection;
    m_pPlaylistTableModel = new PlaylistTableModel(NULL, pTrackCollection, 1);
}

PlaylistFeature::~PlaylistFeature() {
    delete m_pPlaylistTableModel;
}

QVariant PlaylistFeature::title() {
    return "Playlists";
}

QIcon PlaylistFeature::getIcon() {
    return QIcon(":/images/library/rhythmbox.png");
}

int PlaylistFeature::numChildren() {
    return playlists.size();
}

QVariant PlaylistFeature::child(int n) {
    return QVariant(playlists[n]);
}

void PlaylistFeature::activate() {
    qDebug("PlaylistFeature::activate()");
}

void PlaylistFeature::activateChild(int n) {
    qDebug("PlaylistFeature::activateChild(%d)", n);
    //qDebug() << "Activating " << playlists[n];

    //FIXME: Finish this code by implementing these functions:
    //m_pTrackCollection->getPlaylistByPosition(n);
    //pPlaylistTableModel->setPlaylist(playlist_id);
    emit(showTrackModel(m_pPlaylistTableModel));
}

void PlaylistFeature::onRightClick(QModelIndex index) {
}
void PlaylistFeature::onClick(QModelIndex index) {
}
