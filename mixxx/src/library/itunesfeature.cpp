#include <QtDebug>

#include "library/itunesfeature.h"

#include "library/itunestrackmodel.h"
#include "library/itunesplaylistmodel.h"
#include "library/proxytrackmodel.h"

ITunesFeature::ITunesFeature(QObject* parent)
    : LibraryFeature(parent) {
    m_pITunesTrackModel = new ITunesTrackModel();
    m_pITunesPlaylistModel = new ITunesPlaylistModel(m_pITunesTrackModel);
    m_pTrackModelProxy = new ProxyTrackModel(m_pITunesTrackModel);
    m_pTrackModelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pTrackModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    QStringList list;
    for (int i = 0; i < m_pITunesPlaylistModel->numPlaylists(); ++i) {
        list << m_pITunesPlaylistModel->playlistTitle(i);
    }
    m_childModel.setStringList(list);
}

ITunesFeature::~ITunesFeature() {

}

QVariant ITunesFeature::title() {
    return tr("ITunes");
}

QIcon ITunesFeature::getIcon() {
    return QIcon(":/images/library/rhythmbox.png");
}

void ITunesFeature::activate() {
    qDebug("ITunesFeature::activate()");
    emit(showTrackModel(m_pTrackModelProxy));
}

void ITunesFeature::activateChild(const QModelIndex& index) {
    qDebug() << "ITunesFeature::activateChild()" << index;
    QString playlist = index.data().toString();
    qDebug() << "Activating " << playlist;
    m_pITunesPlaylistModel->setPlaylist(playlist);
    emit(showTrackModel(m_pITunesPlaylistModel));
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
