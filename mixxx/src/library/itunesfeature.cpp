#include <QtDebug>

#include "library/itunesfeature.h"

#include "library/itunestrackmodel.h"
#include "library/itunesplaylistmodel.h"
#include "library/proxytrackmodel.h"

ITunesFeature::ITunesFeature(QObject* parent)
    : LibraryFeature(parent) {
    //Don't actually initialize these until the iTunes item in the sidebar is clicked.
    m_pITunesTrackModel = NULL;
    m_pITunesPlaylistModel = NULL;
    m_pTrackModelProxy = NULL;
    m_pPlaylistModelProxy = NULL;
}

ITunesFeature::~ITunesFeature() {

}

bool ITunesFeature::isSupported() {
    return (QFile::exists(MIXXX_ITUNES_DB_LOCATION));
}


QVariant ITunesFeature::title() {
    return tr("iTunes");
}

QIcon ITunesFeature::getIcon() {
    return QIcon(":/images/library/rhythmbox.png");
}

void ITunesFeature::activate() {
    qDebug("ITunesFeature::activate()");

    if (!m_pITunesTrackModel) {
        m_pITunesTrackModel = new ITunesTrackModel();
        m_pITunesPlaylistModel = new ITunesPlaylistModel(m_pITunesTrackModel);

        // Use a ProxyTrackModel for search/sorting of iTunes tracks
        m_pTrackModelProxy = new ProxyTrackModel(m_pITunesTrackModel);
        m_pTrackModelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_pTrackModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

        // Use a ProxyTrackModel for search/sorting of iTunes playlists
        m_pPlaylistModelProxy = new ProxyTrackModel(m_pITunesPlaylistModel);
        m_pPlaylistModelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_pPlaylistModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

        QStringList list;
        for (int i = 0; i < m_pITunesPlaylistModel->numPlaylists(); ++i) {
            list << m_pITunesPlaylistModel->playlistTitle(i);
        }
        m_childModel.setStringList(list);
    }

    emit(showTrackModel(m_pTrackModelProxy));
}

void ITunesFeature::activateChild(const QModelIndex& index) {
    qDebug() << "ITunesFeature::activateChild()" << index;
    QString playlist = index.data().toString();
    qDebug() << "Activating " << playlist;
    m_pITunesPlaylistModel->setPlaylist(playlist);
    emit(showTrackModel(m_pPlaylistModelProxy));
}

QAbstractItemModel* ITunesFeature::getChildModel() {
    return &m_childModel;
}

void ITunesFeature::onRightClick(const QPoint& globalPos) {
}

void ITunesFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
}

bool ITunesFeature::dropAccept(QUrl url) {
    return false;
}

bool ITunesFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool ITunesFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool ITunesFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}
