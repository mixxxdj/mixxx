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

    // TODO(XXX) Populate m_childModel with playlist names.
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
    /*
    qDebug("ITunesFeature::activateChild(%d)", n);
    QString playlist = index.data();
    qDebug() << "Activating " << playlist;
    m_pITunesPlaylistModel->setPlaylist(playlist);
    emit(showTrackModel(m_pITunesPlaylistModel));
    */
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
