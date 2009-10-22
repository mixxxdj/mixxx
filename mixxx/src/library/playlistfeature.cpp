#include <QtDebug>
#include <QMenu>
#include <QInputDialog>

#include "library/trackcollection.h"
#include "library/playlisttablemodel.h"
#include "library/playlistfeature.h"

PlaylistFeature::PlaylistFeature(QObject* parent, TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pTrackCollection(pTrackCollection) {
    m_pPlaylistTableModel = new PlaylistTableModel(NULL, pTrackCollection, 1);

    m_pCreatePlaylistAction = new QAction(tr("New Playlist"),this);
    connect(m_pCreatePlaylistAction, SIGNAL(triggered()), this, SLOT(slotCreatePlaylist()));

    m_pDeletePlaylistAction = new QAction(tr("Remove"),this);
    connect(m_pDeletePlaylistAction, SIGNAL(triggered()), this, SLOT(slotDeletePlaylist()));
}

PlaylistFeature::~PlaylistFeature() {
    delete m_pPlaylistTableModel;
    delete m_pCreatePlaylistAction;
    delete m_pDeletePlaylistAction;
}

QVariant PlaylistFeature::title() {
    return "Playlists";
}

QIcon PlaylistFeature::getIcon() {
    return QIcon(":/images/library/rhythmbox.png");
}

int PlaylistFeature::numChildren() {
    //qDebug() << "PlaylistFeature::numChildren()" << m_pTrackCollection->playlistCount();
    return m_pTrackCollection->playlistCount();//playlists.size();
}

QVariant PlaylistFeature::child(int n) {
    //qDebug() << m_pTrackCollection->getPlaylistName(n);
    //Q_ASSERT(n < numChildren());

    if (n < 0 || n >= numChildren())
        return QVariant(QVariant::Invalid); //As per QAbstractItemModel specs

    return m_pTrackCollection->getPlaylistName(n); //QVariant(playlists[n]);
}

void PlaylistFeature::activate() {
    qDebug("PlaylistFeature::activate()");
}

void PlaylistFeature::activateChild(int n) {
    qDebug("PlaylistFeature::activateChild(%d)", n);
    //qDebug() << "Activating " << playlists[n];

    //Switch the playlist table model's playlist.
    int playlistId = m_pTrackCollection->getPlaylistId(n);
    m_pPlaylistTableModel->setPlaylist(playlistId);
    emit(showTrackModel(m_pPlaylistTableModel));
}

void PlaylistFeature::onRightClick(const QPoint& globalPos, QModelIndex index) {

    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;

    //Create the right-click menu
    QMenu menu(NULL);
    menu.addAction(m_pCreatePlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pDeletePlaylistAction);
    menu.exec(globalPos);
}
void PlaylistFeature::onClick(QModelIndex index) {
}

void PlaylistFeature::slotCreatePlaylist()
{
    QString name = QInputDialog::getText(NULL, tr("New Playlist"), tr("Playlist name:"), QLineEdit::Normal, tr("New Playlist"));
    if (name == "")
        return;
    else {
        m_pTrackCollection->createPlaylist(name);
        qDebug() << "TODO: Force the view to refresh" << __FILE__ << ":" << __LINE__;
    }

    emit(featureUpdated());
}

void PlaylistFeature::slotDeletePlaylist()
{
    qDebug() << "slotDeletePlaylist() row:" << m_lastRightClickedIndex.row();
    if (m_lastRightClickedIndex.isValid()) {
        int playlistId = m_pTrackCollection->getPlaylistId(m_lastRightClickedIndex.row());
        Q_ASSERT(playlistId >= 0);
        m_pTrackCollection->deletePlaylist(playlistId);

    }

    emit(featureUpdated());
}

bool PlaylistFeature::dropAccept(const QModelIndex& index, QUrl url)
{
    //TODO: Filter by supported formats regex and reject anything that doesn't match.

    int playlistId = m_pTrackCollection->getPlaylistId(index.row());
    m_pTrackCollection->appendTrackToPlaylist(url.toLocalFile(), playlistId);

    return true;
}

bool PlaylistFeature::dragMoveAccept(const QModelIndex& index, QUrl url)
{
    //TODO: Filter by supported formats regex and reject anything that doesn't match.

    return true;
}
