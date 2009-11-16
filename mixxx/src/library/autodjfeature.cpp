// autodjfeature.cpp
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/autodjfeature.h"
#include "library/playlisttablemodel.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"

AutoDJFeature::AutoDJFeature(QObject* parent,
                                         TrackCollection* pTrackCollection)
    : LibraryFeature(parent),
      m_pTrackCollection(pTrackCollection),
      m_playlistDao(pTrackCollection->getPlaylistDAO()) {
      //m_pAutoDJTableModel(new PlaylistTableModel(this, pTrackCollection)),
      //m_pAutoDJTableModelProxy(new ProxyTrackModel(m_pAutoDJTableModel, false)) {
    //m_pAutoDJTableModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    
}

AutoDJFeature::~AutoDJFeature() {
    // TODO(XXX) delete these
    //delete m_pAutoDJTableModel;
}

QVariant AutoDJFeature::title() {
    return tr("Auto DJ");
}

QIcon AutoDJFeature::getIcon() {
    return QIcon();
}

QAbstractItemModel* AutoDJFeature::getChildModel() {
    return &m_childModel;
}

void AutoDJFeature::activate() {
    qDebug() << "AutoDJFeature::activate()";
    //emit(showTrackModel(m_pAutoDJTableModelProxy));
    emit(switchToView("Auto DJ"));
}

void AutoDJFeature::activateChild(const QModelIndex& index) {

}

void AutoDJFeature::onRightClick(const QPoint& globalPos) {
}

void AutoDJFeature::onRightClickChild(const QPoint& globalPos,
                                            QModelIndex index) {
}

bool AutoDJFeature::dropAccept(QUrl url) {

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
    
    int playlistId = m_playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
    m_playlistDao.appendTrackToPlaylist(trackId, playlistId);
    return true;
    
}

bool AutoDJFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool AutoDJFeature::dragMoveAccept(QUrl url) {
    qDebug() << "dragMoveAccept";
    return true;
}

bool AutoDJFeature::dragMoveAcceptChild(const QModelIndex& index,
                                              QUrl url) {
    return false;
}
