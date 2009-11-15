// playqueuefeature.cpp
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/playqueuefeature.h"

#include "library/playlisttablemodel.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"

PlayQueueFeature::PlayQueueFeature(QObject* parent,
                                         TrackCollection* pTrackCollection)
    : LibraryFeature(parent),
      m_pTrackCollection(pTrackCollection),
      m_playlistDao(pTrackCollection->getPlaylistDAO()),
      m_pPlayQueueTableModel(new PlaylistTableModel(this, pTrackCollection)),
      m_pPlayQueueTableModelProxy(new ProxyTrackModel(m_pPlayQueueTableModel, false)) {
    m_pPlayQueueTableModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    
    int playlistId = m_playlistDao.getPlaylistIdFromName("Play Queue");
    if (playlistId < 0) {
        m_playlistDao.createPlaylist("Play Queue");
        playlistId = m_playlistDao.getPlaylistIdFromName("Play Queue");
    }
    m_pPlayQueueTableModel->setPlaylist(playlistId);
}

PlayQueueFeature::~PlayQueueFeature() {
    // TODO(XXX) delete these
    //delete m_pPlayQueueTableModel;
}

QVariant PlayQueueFeature::title() {
    return tr("Play Queue");
}

QIcon PlayQueueFeature::getIcon() {
    return QIcon();
}

QAbstractItemModel* PlayQueueFeature::getChildModel() {
    return &m_childModel;
}

void PlayQueueFeature::activate() {
    qDebug() << "PlayQueueFeature::activate()";
    emit(showTrackModel(m_pPlayQueueTableModelProxy));
}

void PlayQueueFeature::activateChild(const QModelIndex& index) {

}

void PlayQueueFeature::onRightClick(const QPoint& globalPos) {
}

void PlayQueueFeature::onRightClickChild(const QPoint& globalPos,
                                            QModelIndex index) {
}

bool PlayQueueFeature::dropAccept(QUrl url) {

    //TODO: Filter by supported formats regex and reject anything that doesn't match.
    
    TrackDAO &trackDao = m_pTrackCollection->getTrackDAO();
    
    //If a track is dropped onto a playlist's name, but the track isn't in the library,
    //then add the track to the library before adding it to the playlist.
    QString location = url.toLocalFile();
    if (!trackDao.trackExistsInDatabase(location))
    {
        trackDao.addTrack(location);
    }
    //Get id of track
    int trackId = trackDao.getTrackId(location);
    
    int playlistId = m_playlistDao.getPlaylistIdFromName("Play Queue");
    m_playlistDao.appendTrackToPlaylist(trackId, playlistId);
    return true;
    
}

bool PlayQueueFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool PlayQueueFeature::dragMoveAccept(QUrl url) {
    qDebug() << "dragMoveAccept";
    return true;
}

bool PlayQueueFeature::dragMoveAcceptChild(const QModelIndex& index,
                                              QUrl url) {
    return false;
}
